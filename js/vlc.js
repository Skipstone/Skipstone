var VLC = {
	makeRequest: function(player, request) {
		var xhr = new XMLHttpRequest();
		xhr.open('GET', 'http://' + player.server + '/requests/status.json?' + request, true);
		xhr.setRequestHeader('Authorization', 'Basic ' + base64_encode(':' + player.password));
		xhr.timeout = 20000;
		xhr.onload = function() {
			if (xhr.readyState == 4) {
				if (xhr.status == 200) {
					if (xhr.responseText) {
						res    = JSON.parse(xhr.responseText);
						title  = res.information || player.title;
						title  = title.category || player.title;
						title  = title.meta || player.title;
						title  = title.filename || player.title;
						title  = title.substring(0,30);
						status = res.state ? res.state.charAt(0).toUpperCase()+res.state.slice(1) : 'Unknown';
						status = status.substring(0,30);
						volume = res.volume || 0;
						volume = (volume / 512) * 200;
						volume = (volume > 200) ? 200 : volume;
						volume = Math.round(volume);
						length = res.length || 0;
						seek   = res.time || 0;
						seek   = length ? (seek / length) * 100 : 0;
						seek   = Math.round(seek);
						appMessageQueue.add({player: mediaPlayer.VLC, title: title, status: status, volume: volume, seek: seek});
					} else {
						console.log('Invalid response received! ' + JSON.stringify(xhr));
						appMessageQueue.add({player: mediaPlayer.VLC, title: 'Error: Invalid response received!'});
					}
				} else {
					console.log('Request returned error code ' + xhr.status.toString());
					appMessageQueue.add({player: mediaPlayer.VLC, title: 'Error: ' + xhr.statusText});
				}
			}
			appMessageQueue.send();
		};
		xhr.ontimeout = function() {
			console.log('HTTP request timed out');
			appMessageQueue.add({player: mediaPlayer.VLC, title: 'Error: Request timed out!'});
			appMessageQueue.send();
		};
		xhr.onerror = function() {
			console.log('HTTP request returned error');
			appMessageQueue.add({player: mediaPlayer.VLC, title: 'Error: Failed to connect!'});
			appMessageQueue.send();
		};
		xhr.send(null);
	},

	handleIncomingAppMessage: function(payload) {
		var player = players[payload.index];
		var request = payload.request || '';
		if (!isset(player.server)) {
			console.log('[VLC] Server not set!');
			appMessageQueue.add({player: mediaPlayer.VLC, title: 'Error: No server set!'});
			appMessageQueue.send();
			return;
		}
		switch (request) {
			case 'play_pause':
				request = 'command=pl_pause';
				break;
			case 'volume_increment':
				request = 'command=volume&val=%2B12.8'; // +5%
				break;
			case 'volume_decrement':
				request = 'command=volume&val=-12.8';   // -5%
				break;
			case 'volume_min':
				request = 'command=volume&val=0';       // 0%
				break;
			case 'volume_max':
				request = 'command=volume&val=512';     // 100%
				break;
			case 'forward_short':
				request = 'command=seek&val=%2B10S';    // +10secs
				break;
			case 'backword_short':
				request = 'command=seek&val=-10S';      // -10secs
				break;
			case 'forward_long':
				request = 'command=seek&val=%2B1M';     // +1min
				break;
			case 'backword_long':
				request = 'command=seek&val=-1M';       // -1min
				break;
		}
		VLC.makeRequest(player, request);
	}
};
