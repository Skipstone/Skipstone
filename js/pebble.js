var players = JSON.parse(localStorage.getItem('players')) || [];

var mediaPlayer = {
	PLEX: 0,
	VLC: 1,
	XBMC: 2
};

function sendPlayerList() {
	if (players.length === 0) {
		appMessageQueue.add({list: true, index: 0, title: 'No players found', status: '', player: 255});
	}
	for (var i = 0; i < players.length; i++) {
		appMessageQueue.add({
			list: true,
			index: i,
			title: players[i].title,
			status: players[i].subtitle,
			player: parseInt(players[i].player)
		});
	}
	appMessageQueue.send();
}

Pebble.addEventListener('ready', function() {
	sendPlayerList();
});

Pebble.addEventListener('appmessage', function(e) {
	console.log('AppMessage received from Pebble: ' + JSON.stringify(e.payload));
	var index = e.payload.index;
	if (!isset(index)) {
		sendPlayerList();
		return;
	}
	switch (parseInt(players[index].player)) {
		case mediaPlayer.PLEX:
			Plex.handleIncomingAppMessage(e.payload);
			break;
		case mediaPlayer.VLC:
			VLC.handleIncomingAppMessage(e.payload);
			break;
		case mediaPlayer.XBMC:
			XBMC.handleIncomingAppMessage(e.payload);
			break;
	}
});

Pebble.addEventListener('showConfiguration', function() {
	var data = {
		players: players
	};
	var uri = 'http://skipstone.github.io/Skipstone/index.html?data=' + encodeURIComponent(base64_encode(JSON.stringify(data)));
	console.log('[configuration] uri: ' + uri);
	Pebble.openURL(uri);
});

Pebble.addEventListener('webviewclosed', function(e) {
	if (e.response) {
		var data = JSON.parse(base64_decode(decodeURIComponent(e.response))) || [];
		console.log('[configuration] data received: ' + JSON.stringify(data));
		players = data.players;
		localStorage.setItem('players', JSON.stringify(players));
		sendPlayerList();
	} else {
		console.log('[configuration] no data received');
	}
});
