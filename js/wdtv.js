var WDTV = {
	makeRequest: function(player, request) {
		request = request || {};
		var xhr = new XMLHttpRequest();
		xhr.open('POST', 'http://' + player.server + '/cgi-bin/toServerValue.cgi', true);
		xhr.send(JSON.stringify(request));
	},

	handleIncomingAppMessage: function(payload) {
		var player = players[payload.index];
		var request = payload.request || '';
		if (!isset(player.server)) {
			console.log('[WDTV] Server not set!');
			appMessageQueue.add({player: mediaPlayer.WDTV, title: 'Error: No server set!'});
			appMessageQueue.send();
			return;
		}
		switch (request) {
			case 'power':
				request = {remote:'w'};
				break;
			case 'home':
				request = {remote:'o'};
				break;
			case 'prev':
				request = {remote:'['};
				break;
			case 'stop':
				request = {remote:'t'};
				break;
			case 'next':
				request = {remote:']'};
				break;
			case 'rewind':
				request = {remote:'H'};
				break;
			case 'play_pause':
				request = {remote:'p'};
				break;
			case 'forward':
				request = {remote:'I'};
				break;
			case 'back':
				request = {remote:'T'};
				break;
			case 'left':
				request = {remote:'l'};
				break;
			case 'ok':
				request = {remote:'n'};
				break;
			case 'up':
				request = {remote:'u'};
				break;
			case 'option':
				request = {remote:'G'};
				break;
			case 'right':
				request = {remote:'r'};
				break;
			case 'prev_page':
				request = {remote:'U'};
				break;
			case 'down':
				request = {remote:'d'};
				break;
			case 'next_page':
				request = {remote:'D'};
				break;
			case 'mute':
				request = {remote:'M'};
				break;
			case 'setup':
				request = {remote:'s'};
				break;
			case 'a':
				request = {remote:'x'};
				break;
			case 'b':
				request = {remote:'y'};
				break;
			case 'c':
				request = {remote:'z'};
				break;
			case 'd':
				request = {remote:'A'};
				break;
			case 'search':
				request = {remote:'E'};
				break;
			case 'eject':
				request = {remote:'X'};
				break;
		}
		WDTV.makeRequest(player, request);
	}
};
