#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include <Libraries/DHT/DHT.h>
#include <Libraries/BMP180/BMP180.h>
#include <Libraries/Adafruit_PCD8544/Adafruit_PCD8544.h>

/* Определение выводов */
#define LCD_SCLK 5
#define LCD_DIN  15
#define LCD_DC   13
#define LCD_CS   12
#define LCD_RST  14
#define DHT_PIN	 0
#define BMP_SCL  2
#define BMP_SDA  4

/* Создание объектов */
Adafruit_PCD8544 display = Adafruit_PCD8544(LCD_SCLK, LCD_DIN, LCD_DC, LCD_CS, LCD_RST);
DHT dht(DHT_PIN);
BMP180 barometer;
Timer sensorsUpdate;
Timer displayRefresh;
NtpClient *ntpClient;

/* Переменные для хранения значений */
float temp     = 0.0;
float humidity = 0.0;
float pressure = 0.0;
char wifi_ssid[] = "Symrak";
char wifi_psw[]  = "s1PKo1Dj";
char ntp_srv[]   = "pool.ntp.org";
int ntp_interval = 600;
double timezone  = -2.0;

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
	WDT.alive();

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
void wifiInit()
{
	WifiAccessPoint.enable(false);
	WifiStation.config(wifi_ssid, wifi_psw);
	WifiStation.enable(true);
}
void wifiConnectOk()
{
	ntpClient = new NtpClient(ntp_srv, ntp_interval, timeReceived);
}
void wifiConnectFail()
{
	WifiStation.waitConnection(wifiConnectOk, 10, wifiConnectFail);
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE);
	Serial.systemDebugOutput(false);

	SystemClock.setTimeZone(timezone);

	wifiInit();

	dhtInit();
	bmpInit();
	displayInit();

	sensorsUpdate.initializeMs(5000, sensorsGet).start();
	displayRefresh.initializeMs(1000, displayContent).start();

	WifiStation.waitConnection(wifiConnectOk, 30, wifiConnectFail);
}
