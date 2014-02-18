var Plex = {
	clients: [],
	isPlay: false,

	makeRequest: function(player, request, cb, fb) {
		var xhr = new XMLHttpRequest();
		xhr.open('GET', 'http://' + player.server + request, true);
		xhr.setRequestHeader('Accept', 'application/json');
		xhr.onload = function() {
			if (xhr.readyState == 4 && xhr.status == 200) {
				cb(xhr);
			} else {
				fb(xhr);
			}
		};
		xhr.onerror = function() {
			fb(xhr);
		};
		xhr.send(null);
	},

	sendControlCommand: function(player, client, controller, command) {
		Plex.makeRequest(player, '/system/players/' + client.host + '/' + controller + '/' + command, function(e) {
			// command sent successfully
		}, function(e) {
			console.log('sendControlCommand:fb:' + JSON.stringify(e));
		});
	},

	sendClients: function(player) {
		Plex.makeRequest(player, '/clients', function(e) {
			Plex.clients = [];
			appMessageQueue.clear();
			var res = JSON.parse(e.responseText);
			if (res._children) {
				var i = 0;
				for (var c in res._children) {
					var name = res._children[c].name.replace(/[^\w\s]/gi, '') || '';
					var host = res._children[c].host || '';
					Plex.clients.push({name:name, host:host});
					appMessageQueue.add({player:mediaPlayer.PLEX, index:i++, title:name, status:host});
				}
				appMessageQueue.add({player:mediaPlayer.PLEX, index:i});
			} else {
				// invalid response received
			}
			appMessageQueue.send();
		}, function(e) {
			console.log('sendClients:fb:' + JSON.stringify(e));
		});
	},

	handleIncomingAppMessage: function(payload) {
		var player = players[payload.index];
		var request = payload.request || '';
		if (!isset(player.server)) {
			console.log('[Plex] Server not set!');
			appMessageQueue.add({player: mediaPlayer.PLEX, title: 'Error: No server set!'});
			appMessageQueue.send();
			return;
		}
		var client = payload.client;
		if (!isset(client)) {
			Plex.sendClients(player);
		}
		client = Plex.clients[client];
		switch (request) {
			case 'play_pause':
				Plex.sendControlCommand(player, client, 'playback', (Plex.isPlay ? 'play' : 'pause'));
				Plex.isPlay = !Plex.isPlay;
				break;
			case 'forward_short':
				Plex.sendControlCommand(player, client, 'playback', 'stepForward');
				break;
			case 'backword_short':
				Plex.sendControlCommand(player, client, 'playback', 'stepBack');
				break;
			case 'forward_long':
				Plex.sendControlCommand(player, client, 'playback', 'bigStepForward');
				break;
			case 'backword_long':
				Plex.sendControlCommand(player, client, 'playback', 'bigStepBack');
				break;
			case 'select':
				Plex.sendControlCommand(player, client, 'navigation', 'select');
				break;
			case 'back':
				Plex.sendControlCommand(player, client, 'navigation', 'back');
				break;
			case 'up':
				Plex.sendControlCommand(player, client, 'navigation', 'moveUp');
				break;
			case 'down':
				Plex.sendControlCommand(player, client, 'navigation', 'moveDown');
				break;
			case 'right':
				Plex.sendControlCommand(player, client, 'navigation', 'moveRight');
				break;
			case 'left':
				Plex.sendControlCommand(player, client, 'navigation', 'moveLeft');
				break;
		}
	}
};
