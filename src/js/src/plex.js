var Plex = {
	clients: [],
	isPaused: true,

	sendControlCommand: function(player, client, controller, command) {
		var url = 'http://' + player.server + '/system/players/' + client.host + '/' + controller + '/' + command;
		var params = {};
		if (player.token) params['X-Plex-Token'] = player.token;
		var headers = { 'Accept': 'application/json' };
		http('GET', url, params, headers, function(e) {
			Plex.requestStatus(player, client);
		});
	},

	requestStatus: function(player, client) {
		var url = 'http://' + player.server + '/status/sessions';
		var params = {};
		if (player.token) params['X-Plex-Token'] = player.token;
		var headers = { 'Accept': 'application/json' };
		http('GET', url, params, headers, function(e) {
			if (e.status >= 300) {
				return;
			}
			var res = JSON.parse(e.responseText);
			if (!res._children) return;
			res._children.forEach(function(a) {
				a._children.forEach(function(b) {
					if (b._elementType !== 'Player') return;
					if (b.machineIdentifier !== client.machineIdentifier) return;
					var title = a.title;
					title = title.substring(0,30);
					var subtitle = '';
					var status = capitaliseFirstLetter(b.state) || '';
					status  = status.substring(0,30);
					var duration  = a.duration || 0;
					var viewOffset = a.viewOffset || 0;
					var seek = duration ? (viewOffset / duration) * 100 : 0;
					seek = Math.round(seek);
					if (a.type === 'movie') {
						subtitle = a.year;
					}
					if (a.type === 'episode') {
						subtitle = 'S' + zeroPad(a.parentIndex,2) + ' E' + zeroPad(a.index,2);
					}
					appMessageQueue.send({type:TYPE.PLEX, method:METHOD.STATUS, title:title, subtitle:subtitle, status:status, seek:seek});
				});
			});
		});
	},

	requestClients: function(player) {
		var url = 'http://' + player.server + '/clients';
		var params = {};
		if (player.token) params['X-Plex-Token'] = player.token;
		var headers = { 'Accept': 'application/json' };
		http('GET', url, params, headers, function(e) {
			var res = JSON.parse(e.responseText);
			Plex.clients = res._children || [];
			Plex.sendClients();
		});
	},

	sendClients: function(player) {
		if (!Plex.clients.length) {
			return appMessageQueue.send({type:TYPE.PLEX, method:METHOD.ERROR, status:'No clients found.'});
		}
		appMessageQueue.send({type:TYPE.PLEX, method:METHOD.SIZE, index:Plex.clients.length});
		for (var i = 0; i < Plex.clients.length; i++) {
			var title = Plex.clients[i].name.replace(/[^\w\s]/gi, '') || '';
			var subtitle = Plex.clients[i].host || '';
			if (Plex.clients[i].product) subtitle += ' (' + Plex.clients[i].product + ')';
			appMessageQueue.send({type:TYPE.PLEX, method:METHOD.DATA, index:i, title:title, subtitle:subtitle});
		}
	},

	handleAppMessage: function(payload) {
		var player = Skipstone.players[payload.player];
		var request = payload.request || null;
		var controller = '';
		var command = '';

		if (!isset(player.server)) {
			return appMessageQueue.send({type:TYPE.PLEX, method:METHOD.ERROR, status:'Server not set.'});
		}

		switch (request) {
			case REQUEST.PLAY_PAUSE:
				controller = 'playback';
				command = Plex.isPaused ? 'pause' : 'play';
				Plex.isPaused = !Plex.isPaused;
				break;
			case REQUEST.FORWARD_SHORT:
				controller = 'playback';
				command = 'stepForward';
				break;
			case REQUEST.BACKWARD_SHORT:
				controller = 'playback';
				command = 'stepBack';
				break;
			case REQUEST.FORWARD_LONG:
				controller = 'playback';
				command = 'bigStepForward';
				break;
			case REQUEST.BACKWARD_LONG:
				controller = 'playback';
				command = 'bigStepBack';
				break;
			case REQUEST.SELECT:
				controller = 'navigation';
				command = 'select';
				break;
			case REQUEST.BACK:
				controller = 'navigation';
				command = 'back';
				break;
			case REQUEST.UP:
				controller = 'navigation';
				command = 'moveUp';
				break;
			case REQUEST.DOWN:
				controller = 'navigation';
				command = 'moveDown';
				break;
			case REQUEST.RIGHT:
				controller = 'navigation';
				command = 'moveRight';
				break;
			case REQUEST.LEFT:
				controller = 'navigation';
				command = 'moveLeft';
				break;
			case REQUEST.CLIENTS:
				return Plex.requestClients(player);
			case REQUEST.REFRESH:
				return Plex.requestStatus(player, Plex.clients[payload.index]);
			default:
				return;
		}

		Plex.sendControlCommand(player, Plex.clients[payload.index], controller, command);
	}
};
