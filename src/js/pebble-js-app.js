var count = 0;

var emit = function() {
	count += 1;

	var dict = {"status": count, "temp": count};

	Pebble.sendAppMessage(dict);
	dict = {"temp": count};
	Pebble.sendAppMessage(dict);
};

Pebble.addEventListener('ready', function(e) {
  console.log('PebbleKit JS ready!');

  // Send periodic updates every 3 seconds
  setInterval(emit, 3000);
});

Pebble.addEventListener('appmessage', function(e) {
  console.log('AppMessage received!' + e);
});
