var emit = function() {
	var dict = {"status": 1, "temp": 72};

	Pebble.sendAppMessage(dict);
};

Pebble.addEventListener('ready', function(e) {
	console.log('PebbleKit JS ready!');

	// Send periodic updates every 60 seconds
	emit();
	setInterval(emit, 60000);
});

Pebble.addEventListener('appmessage', function(e) {
  console.log('AppMessage received!' + e);
});
