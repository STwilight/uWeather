#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include <Libraries/DHT/DHT.h>
#include <Libraries/BMP180/BMP180.h>
#include <Libraries/Adafruit_PCD8544/Adafruit_PCD8544.h>

/* Определение выводов */
#define LCD_SCLK 2
#define LCD_DIN  13
#define LCD_DC   12
#define LCD_CS   14
#define LCD_RST  16

#define DHT_PIN	 0

#define BMP_SDA  4
#define BMP_SCL  5

#define BTN_PIN	 3


/* Создание объектов */
Adafruit_PCD8544 display = Adafruit_PCD8544(LCD_SCLK, LCD_DIN, LCD_DC, LCD_CS, LCD_RST);
DHT dht(DHT_PIN);
BMP180 barometer;
Timer sensorsUpdate;
Timer displayRefresh;
Timer buttonStateCheck;
NtpClient *ntpClient;
HttpClient *httpClient;
FTPServer ftp;
int sensorValue = 0;

/* Переменные для хранения значений */
float  temp         = 0.0;
float  humidity     = 0.0;
float  pressure     = 0.0;
String ntp_srv      = "pool.ntp.org";
int    ntp_interval = 600;
int	   ftp_port		= 21;
String ftp_login	= "uWeather";
String ftp_psw		= "12345678";
double timezone     = -2.0;
bool   btn_pushed	= false;

void dhtInit()
{
	WDT.enable(false);
	delay(1000);
	dht.begin();
}
void dhtGet(bool uart)
{
	WDT.alive();
	humidity = dht.readHumidity();
	WDT.alive();
	temp = dht.readTemperature();

	if(isnan(temp))
		temp = 0.0;
	if(isnan(humidity))
		humidity = 0.0;

	if(uart)
	{
		if (isnan(temp) || isnan(humidity))
		{
			Serial.println("Failed to read from DHT");
		}
		else
		{
			Serial.print("Humidity: ");
			Serial.print(humidity);
			Serial.print("%, Temperature: ");
			Serial.print(temp);
			Serial.write(176);
			Serial.print("C\n");
		}
	}
}
void bmpInit()
{
	Wire.pins(BMP_SCL, BMP_SDA);
	Wire.begin();
	if(!barometer.EnsureConnected())
		Serial.println("Could not connect to BMP180.");
	barometer.Initialize();
}
void bmpGet(bool uart, bool temp)
{
	float bmpTemp = 0.0;
	long  paPressure = 0.0;

	paPressure = barometer.GetPressure();
	if(isnan(paPressure))
		paPressure = 0.0;

	if(temp)
		bmpTemp = barometer.GetTemperature();

	pressure = paPressure/133.3;

	if(uart)
	{
		Serial.print("Pressure: ");
		Serial.print(paPressure);
		Serial.print("Pa");
		Serial.print(", ");
		Serial.print(pressure);
		Serial.print("mm");

		if(!temp)
			Serial.print("\n");
		else
		{
			Serial.print(", Temperature: ");
			Serial.print(bmpTemp);
			Serial.write(176);
			Serial.print("C\n");
		}
	}
}
void sensorsGet()
{
	dhtGet(false);
	bmpGet(false, false);
}
void displayInit()
{
	display.begin();
	display.setContrast(0);
	display.clearDisplay();
	display.setRotation(0);

	display.setTextSize(1);
	display.setTextColor(BLACK);
	display.setCursor(0,0);
}
void displayContent()
{
	display.clearDisplay();

	display.print("T: ");
	display.print(temp);
	display.println("C");

	display.print("H: ");
	display.print(humidity);
	display.println("%");

	display.print("P: ");
	display.print(pressure);
	display.println("mm\n");

	DateTime currentDateTime = SystemClock.now(eTZ_UTC);
	display.println(currentDateTime.toShortDateString());
	display.println(currentDateTime.toShortTimeString(true));
	display.display();
}
void timeReceived(NtpClient& client, time_t timestamp)
{
	SystemClock.setTime(timestamp);
}

String getString(String begin_phraze, String end_phraze, String content)
{
	String result;
	uint32_t begin_num, end_num;

	begin_num = content.indexOf(begin_phraze) + begin_phraze.length();
	end_num = content.indexOf(end_phraze);

	result = content.substring(begin_num, end_num);

	return result;
}
void weatherParse(HttpClient& client, bool successful)
{
	Serial.println("Request was saved in \"index.html\" file successfully!");
	String response = client.getResponseString();
	//Serial.println(response);

	Serial.println("\n\r *** *** *** \n\r");

	WDT.enable(false);
	String tmp = getString("<table><tr><td><b>", "</td><td><img src=\"http://meteopost.com/pic/met/", response);
	Serial.println("Parsed: " + tmp);
	WDT.alive();
}
void weatherRequest()
{
	WDT.alive();
	if (httpClient->isProcessing()) return;

	httpClient->setRequestHeader("User agent", "Mozilla/5.0 (Linux; Android 5.1.1; Sensation Build/LMY48G) AppleWebKit/537.36 (KHTML, like Gecko) Version/4.0 Chrome/39.0.0.0 Mobile Safari/537.36");
	WDT.alive();
	//httpClient->downloadFile("http://meteopost.com/weather/kiev/", "index.html", weatherParse);
	httpClient->downloadString("http://meteopost.com/weather/kiev/", weatherParse);
}
void ftpInit()
{
	ftp.listen(ftp_port);
	ftp.addUser(ftp_login, ftp_psw);
}
void wifiInit()
{
	WifiAccessPoint.enable(false);
	WifiStation.config(WIFI_SSID, WIFI_PWD);
	WifiStation.enable(true);
}
void wifiConnectOk()
{
	ftpInit();
	ntpClient = new NtpClient(ntp_srv, ntp_interval, timeReceived);
	httpClient = new HttpClient();
	weatherRequest();
}
void wifiConnectFail()
{
	WifiStation.waitConnection(wifiConnectOk, 10, wifiConnectFail);
}

void IRAM_ATTR buttonPush()
{
	btn_pushed = digitalRead(BTN_PIN);
}
void displayButtonState()
{
	display.clearDisplay();

	if(btn_pushed)
		display.print("1");
	else
		display.print("0");

	display.display();
}

void init()
{
	//WDT.enable(false);
	//WDT.alive();
	//system_soft_wdt_stop();
	//system_soft_wdt_restart();

	spiffs_mount();

	WDT.alive();

		Serial.begin(SERIAL_BAUD_RATE);
		Serial.systemDebugOutput(true);

		SystemClock.setTimeZone(timezone);

		dhtInit();
		bmpInit();

		wifiInit();

	//attachInterrupt(BTN_PIN, buttonPush, CHANGE);

	displayInit();

	//display.setTextSize(5);
	//buttonStateCheck.initializeMs(100, displayButtonState).start();

		sensorsUpdate.initializeMs(5000, sensorsGet).start();
		displayRefresh.initializeMs(1000, displayContent).start();

		WifiStation.waitConnection(wifiConnectOk, 20, wifiConnectFail);
}
