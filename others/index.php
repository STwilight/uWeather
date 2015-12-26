<?php
	function getRequest ($url, $header, $errors) {
		$opts = array(
			"http" => array(
				"method"		=> "GET",
				"header"		=> $header,
				"ignore_errors" => $errors
			)
		);
		$context = stream_context_create($opts);
		$page = file_get_contents($url, false, $context);
		
		return $page;
	}
	function parseData ($page, $start_phraze, $end_phraze) {
		$start = strpos($page, $start_phraze) + strlen($start_phraze);
		$length = strpos($page, $end_phraze) - $start;
		$value = substr($page, $start, $length);
		
		return $value;
	}
	function getData ($page) {
		$time_start_phraze = '<b>Погода в Киеве на</b></td><td><span style="font-size:22px;color:black">';
		$time_end_phraze = '</span> Kyiv</td>';
		
		$temp_start_phraze = '<td>Температура воздуха </td><td><span class="dat">';
		$temp_end_phraze = '&deg;</span></td></tr><tr><td>Температура комфорта </td>';
		
		$comfort_start_phraze = '<td>Температура комфорта </td><td><span class="dat">';
		$comfort_end_phraze = '&deg;</span></td></tr><tr><td>Точка росы </td>';
		
		$humidity_start_phraze = '<td>Влажность </td><td><span class="dat">';
		$humidity_end_phraze = '%</span></td></tr><tr><td>Давление (на уровне моря) </td>';
		
		$pressure_start_phraze = '<td>Давление (на станции) </td><td><span class="dat">';
		$pressure_end_phraze = ' мм.рт.ст.</span></td>';
		
		$wind_start_phraze = '<td>Ветер </td><td><span class="dat">';
		$wind_end_phraze = '</span></td></tr><tr><td>Скорость ветра </td>';
		
		$wind_speed_start_phraze = '<td>Скорость ветра </td><td><span class="dat">';
		$wind_speed_end_phraze = ' м/с</span></td>';
		
		$weather_start_phraze = '<td>Пог. явления </td><td><span class="dat">';
		$weather_end_phraze = '</span></td></tr><tr><td>Облачность </td>';
		
		$result = array(
			"time"	   =>		parseData($page, $time_start_phraze, $time_end_phraze),
			"temp"	   => (int) parseData($page, $temp_start_phraze, $temp_end_phraze),
			"comfort"  => (int) parseData($page, $comfort_start_phraze, $comfort_end_phraze),
			"humidity" => (int) parseData($page, $humidity_start_phraze, $humidity_end_phraze),
			"pressure" => (int) parseData($page, $pressure_start_phraze, $pressure_end_phraze),
			"wind"	   =>		parseData($page, $wind_start_phraze, $wind_end_phraze),
			"speed"	   => (int) parseData($page, $wind_speed_start_phraze, $wind_speed_end_phraze),
			"weather"  =>		parseData($page, $weather_start_phraze, $weather_end_phraze)
		);
		
		return $result;
	}
	function showData($array) {
		echo	'<!DOCTYPE html>
					<html>
						<head>
							<meta charset="utf-8">
								<title>Meteopost Parsing</title>
								<link rel="shortcut icon" href="favicon.ico" type="image/x-icon">
						</head>
						<body>
							<p>
								<h4>
									Парсинг данных сайта <img src="favicon.ico"> <a href="http://meteopost.com">meteopost.com</a>
								</h4>
							<p>';
		echo 				"<b>Прогноз обновлен в: </b>".$array["time"]."<br>".
							"<b>Текущее время: </b>".date("H:i")."<br><br>".
							"<b>Температура воздуха: </b>".$array["temp"]."&deg;C<br>".
							"<b>Температура комфорта: </b>".$array["comfort"]."&deg;C<br>".
							"<b>Влажность: </b>".$array["humidity"]."%<br>".
							"<b>Атмосферное давление: </b>".$array["pressure"]." мм. рт. ст.<br>".
							"<b>Ветер: </b>".$array["wind"]."<br>".
							"<b>Скорость ветра: </b>".$array["speed"]." м/с<br>".
							"<b>Погодные явления: </b>".$array["weather"]."<br>";
		echo			'</body>
					</html>';	
	}
	
	$url	= "http://m.meteopost.com/weather/kiev/";
	$header = "User agent: Mozilla/5.0 (Windows NT 6.1; WOW64; rv:43.0) Gecko/20100101 Firefox/43.0";	
	
	showData(getData(getRequest($url, $header, false)));
?>
