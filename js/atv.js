var ATV = {
	makeRequest: function(player, request) {
		request = request || {};
		var xhr = new XMLHttpRequest();
		xhr.open('GET', 'http://' + player.server + '/remoteAction=' + request, true);
		xhr.send(null);
	},

	handleIncomingAppMessage: function(payload) {
		var player = players[payload.index];
		var request = payload.request || '';
		if (!isset(player.server)) {
			console.log('[ATV] Server not set!');
			appMessageQueue.add({player: mediaPlayer.ATV, title: 'Error: No server set!'});
			appMessageQueue.send();
			return;
		}
		switch (request) {
			case 'menu':
				request = '1';
				break;
			case 'menu_hold':
				request = '2';
				break;
			case 'arrow_up':
				request = '3';
				break;
			case 'arrow_down':
				request = '4';
				break;
			case 'select':
				request = '5';
				break;
			case 'arrow_left':
				request = '6';
				break;
			case 'arrow_right':
				request = '7';
				break;
			case 'play_pause':
				request = '10';
				break;
			case 'pause':
				request = '15';
				break;
			case 'play':
				request = '16';
				break;
			case 'stop':
				request = '17';
				break;
			case 'forward':
				request = '18';
				break;
			case 'rewind':
				request = '19';
				break;
			case 'skip_forward':
				request = '20';
				break;
			case 'skip_backward':
				request = '21';
				break;
			case 'list':
				request = '22';
				break;
		}
		ATV.makeRequest(player, request);
	}
};
