var XBMC = {
	volumeReceived: false,
	titleReceived: false,
	seekReceived: false,
	volume: 0,
	title: '',
	status: 'Loading...',
	seek: 0,

	requestUpdates: function(player) {
		this.makeRequest(player, {jsonrpc:'2.0', id:1, method:'Application.GetProperties', params:{properties:['volume']}}, true);
		this.makeRequest(player, {jsonrpc:'2.0', id:1, method:'Player.GetItem', params:{playerid:1, properties:['title']}}, true);
		this.makeRequest(player, {jsonrpc:'2.0', id:1, method:'Player.GetProperties', params:{playerid:1, properties:['percentage','speed']}}, true);
	},

	checkForReadyUpdate: function(player) {
		if (this.volumeReceived && this.titleReceived && this.seekReceived) {
			this.sendUpdate(player);
			this.reset();
		}
	},

	sendUpdate: function(player) {
		var title = this.title || player.title;
		appMessageQueue.add({player: mediaPlayer.XBMC, volume: this.volume, title: title, status: this.status, seek: this.seek});
		appMessageQueue.send();
	},

	reset: function() {
		this.volumeReceived = this.titleReceived = this.seekReceived = false;
		this.volume = 0;
		this.title = '';
		this.status = 'Loading...';
		this.seek = 0;
	},

	makeRequest: function(player, request, statusRefresh) {
		request = request || {};
		var xhr = new XMLHttpRequest();
		xhr.open('POST', 'http://' + player.server + '/jsonrpc?', true);
		xhr.setRequestHeader('Authorization', 'Basic ' + base64_encode(player.username + ':' + player.password));
		xhr.timeout = 20000;
		xhr.onload = function() {
			if (xhr.readyState == 4) {
				if (xhr.status == 200) {
					if (xhr.responseText) {
						res = JSON.parse(xhr.responseText);
						if (statusRefresh) {
							if (res.hasOwnProperty('error')) {
								XBMC.sendUpdate(player);
							}
							if (!res.hasOwnProperty('result')) return;
							if (res.result.volume) {
								var volume = res.result.volume || 0;
								volume = (volume > 100) ? 100 : volume;
								volume = Math.round(volume);
								XBMC.volume = volume;
								XBMC.volumeReceived = true;
								XBMC.checkForReadyUpdate(player);
							} else if (res.result.item) {
								var title = res.result.item.title || player.title;
								title = title.substring(0,30);
								XBMC.title = title;
								XBMC.titleReceived = true;
								XBMC.checkForReadyUpdate(player);
							} else if (res.result.percentage) {
								var status = res.result.speed ? 'Playing' : 'Paused';
								var seek = res.result.percentage || 0;
								seek = Math.round(seek);
								XBMC.status = status;
								XBMC.seek = seek;
								XBMC.seekReceived = true;
								XBMC.checkForReadyUpdate(player);
							}
						} else {
							XBMC.requestUpdates(player);
						}
					} else {
						console.log('Invalid response received! ' + JSON.stringify(xhr));
						appMessageQueue.add({player: mediaPlayer.XBMC, title: 'Error: Invalid response received!'});
					}
				} else {
					console.log('Request returned error code ' + xhr.status.toString());
					appMessageQueue.add({player: mediaPlayer.XBMC, title: 'Error: ' + xhr.statusText});
				}
			}
			appMessageQueue.send();
		};
		xhr.ontimeout = function() {
			console.log('HTTP request timed out');
			appMessageQueue.add({player: mediaPlayer.XBMC, title: 'Error: Request timed out!'});
			appMessageQueue.send();
		};
		xhr.onerror = function() {
			console.log('HTTP request returned error');
			appMessageQueue.add({player: mediaPlayer.XBMC, title: 'Error: Failed to connect!'});
			appMessageQueue.send();
		};
		xhr.setRequestHeader('Content-type','application/json');
		xhr.send(JSON.stringify(request));
	},

	handleIncomingAppMessage: function(payload) {
		var player = players[payload.index];
		var request = payload.request || '';
		var requestObj = {jsonrpc:'2.0', id:1, params:{}};
		if (!isset(player.server)) {
			console.log('[XBMC] Server not set!');
			appMessageQueue.add({player: mediaPlayer.XBMC, title: 'Error: No server set!'});
			appMessageQueue.send();
			return;
		}
		switch (request) {
			case 'input_up':
				requestObj.method = 'Input.Up';
				break;
			case 'input_down':
				requestObj.method = 'Input.Down';
				break;
			case 'input_left':
				requestObj.method = 'Input.Left';
				break;
			case 'input_right':
				requestObj.method = 'Input.Right';
				break;
			case 'input_select':
				requestObj.method = 'Input.Select';
				break;
			case 'input_back':
				requestObj.method = 'Input.Back';
				break;
			case 'play_pause':
				requestObj.method = 'Player.PlayPause';
				requestObj.params.playerid = 1;
				break;
			case 'volume_increment':
				requestObj.method = 'Application.SetVolume';
				requestObj.params.volume = 'increment';
				break;
			case 'volume_decrement':
				requestObj.method = 'Application.SetVolume';
				requestObj.params.volume = 'decrement';
				break;
			case 'volume_min':
				requestObj.method = 'Application.SetVolume';
				requestObj.params.volume = 0;
				break;
			case 'volume_max':
				requestObj.method = 'Application.SetVolume';
				requestObj.params.volume = 100;
				break;
			case 'forward_short':
				requestObj.method = 'Player.Seek';
				requestObj.params.playerid = 1;
				requestObj.params.value = 'smallforward';
				break;
			case 'backword_short':
				requestObj.method = 'Player.Seek';
				requestObj.params.playerid = 1;
				requestObj.params.value = 'smallbackward';
				break;
			case 'forward_long':
				requestObj.method = 'Player.Seek';
				requestObj.params.playerid = 1;
				requestObj.params.value = 'bigforward';
				break;
			case 'backword_long':
				requestObj.method = 'Player.Seek';
				requestObj.params.playerid = 1;
				requestObj.params.value = 'bigbackward';
				break;
			case 'update':
				XBMC.requestUpdates(player);
				break;
			default:
				XBMC.requestUpdates(player);
				return;
		}
		XBMC.makeRequest(player, requestObj, false);
		setTimeout(function() {
			XBMC.requestUpdates(player);
		}, 5000);
	}
};
