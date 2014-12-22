var locationOptions = { "timeout": 15000, "maximumAge": 60000 }; 

var emit = function() {
	navigator.geolocation.getCurrentPosition(
			locationGetSuccess, locationGetError, locationOptions);
};

Pebble.addEventListener('ready', function(e) {
	console.log('PebbleKit JS ready!');

	// Send periodic updates every 15 minutes
	emit();
	navigator.geolocation.getCurrentPosition(
		locationGetSuccess, locationGetError, locationOptions);
	setInterval(emit, 900000);
	if (localStorage.getItem("apiKey") === null) {
		localStorage.setItem("apiKey", "");
	}
	if (localStorage.getItem("units") === null) {
		localStorage.setItem("units", "F");
	}
});

//Handle configuration events
Pebble.addEventListener('showConfiguration', function(e) {
	// Show config page
	Pebble.openURL('https://darkpebble.rthr.me/dp-configuration.html?apiKey=' + 
					localStorage.getItem("apiKey") + '&units=' + localStorage.getItem("units"));
});

Pebble.addEventListener('webviewclosed',
	function(e) {
		var configuration = JSON.parse(decodeURIComponent(e.response));
		console.log('Configuration window returned: ', JSON.stringify(configuration));
		if (configuration.apiKey !== null) {
			localStorage.setItem("apiKey", configuration.apiKey);
		}
		if (configuration.units !== null) {
			localStorage.setItem("units", configuration.units);
		}
		emit();
	}
);

Pebble.addEventListener('appmessage', function(e) {
  console.log('AppMessage received!' + e);
});

function iconToIconCode(weatherIcon)
{
	if (weatherIcon == "clear-day")
		return 1;
	else if (weatherIcon == "clear-night")
		return 2;
	else if (weatherIcon == "rain")
		return 7;
	else if (weatherIcon == "snow")
		return 9;
	else if (weatherIcon == "sleet")
		return 8;
	else if (weatherIcon == "wind")
		return 10;
	else if (weatherIcon == "fog")
		return 4;
	else if (weatherIcon == "cloudy")
		return 3;
	else if (weatherIcon == "partly-cloudy-day")
		return 5;
	else if (weatherIcon == "partly-cloudy-night")
		return 6;
	else
		return 1;
}

function locationGetSuccess(pos)
{
	var coords = pos.coords;
	getCurrentWeather(coords.latitude, coords.longitude);
}

function locationGetError(err)
{
	console.warn("Error.");
	Pebble.showSimpleNotificationOnPebble("DarkPebble", "Couldn't get device location!");
}

function getCurrentWeather(latitude, longitude)
{
	var response;
	var request = new XMLHttpRequest();
	request.open('GET', "https://api.forecast.io/forecast/" + 
		localStorage.getItem("apiKey") + "/" + latitude + "," + longitude, true);

	request.onload = function(e)
	{
		if (request.readyState == 4)
		{
			var status, icon, temp, fTemp, summ;
			if (request.status == 200)
			{
				response = JSON.parse(request.responseText);
				status = 1;
				icon = iconToIconCode(response.currently.icon);

				temp = Math.round(response.currently.temperature);
				fTemp = Math.round(response.currently.apparentTemperature);	

				summ = response.minutely.summary;
			}
			else if (request.status == 403 || request.status == 404) {
				status = 0;
				icon = 0;
				temp = 0;
				fTemp = 0;
				summ = "Invalid API key. Check settings.";
			}
			else
			{
				console.log(request.status);
				status = 0;
				icon = 0;
				temp = 0;
				fTemp = 0;
				summ = "Service is down. Try later.";
			}
			console.log("<p>"+icon+"</p>");
			console.log("<p>"+temp+"</p>");
			console.log("<p>"+fTemp+"</p>");
			console.log("<p>"+summ+"</p>");
			var dict = {"icon": icon, "temp": fTemp, "forecast": summ};
			Pebble.sendAppMessage(dict);
		}
	};
	request.send();
}
