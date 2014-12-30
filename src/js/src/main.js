var Skipstone = {
	players: JSON.parse(localStorage.getItem('players')) || [],

	init: function() {
		Skipstone.sendPlayers();
	},

	sendPlayers: function() {
		if (!Skipstone.players.length) {
			return appMessageQueue.send({type:TYPE.PLAYERS, method:METHOD.ERROR, status:'No players found. Add media player(s) via the Pebble mobile app.'});
		}
		appMessageQueue.send({type:TYPE.PLAYERS, method:METHOD.SIZE, index:Skipstone.players.length});
		for (var i = 0; i < Skipstone.players.length; i++) {
			var title = Skipstone.players[i].title.substring(0,24);
			var player = parseInt(Skipstone.players[i].player);
			appMessageQueue.send({type:TYPE.PLAYERS, method:METHOD.DATA, index:i, title:title, player:player});
		}
	},

	handleAppMessage: function(e) {
		console.log('AppMessage received: ' + JSON.stringify(e.payload));
		if (!e.payload.method) return;
		switch (e.payload.method) {
			case METHOD.REQUESTPLAYERS:
				Skipstone.sendPlayers();
				break;
			case METHOD.PLEX:
				Plex.handleAppMessage(e.payload);
				break;
			case METHOD.VLC:
				VLC.handleAppMessage(e.payload);
				break;
			case METHOD.XBMC:
				XBMC.handleAppMessage(e.payload);
				break;
			case METHOD.WDTV:
				WDTV.handleAppMessage(e.payload);
				break;
		}
	},

	showConfiguration: function() {
		var data = { players: Skipstone.players };
		Pebble.openURL('https://ineal.me/pebble/skipstone/configuration/?data=' + encodeURIComponent(base64_encode(JSON.stringify(data))));
	},

	handleConfiguration: function(e) {
		console.log('configuration received: ' + JSON.stringify(e));
		if (!e.response) return;
		if (e.response === 'CANCELLED') return;
		var data = JSON.parse(base64_decode(decodeURIComponent(e.response)));
		if (data.players) {
			Skipstone.players = data.players;
			localStorage.setItem('players', JSON.stringify(Skipstone.players));
			Skipstone.sendPlayers();
		}
	}
};

Pebble.addEventListener('ready', Skipstone.init);
Pebble.addEventListener('appmessage', Skipstone.handleAppMessage);
Pebble.addEventListener('showConfiguration', Skipstone.showConfiguration);
Pebble.addEventListener('webviewclosed', Skipstone.handleConfiguration);
