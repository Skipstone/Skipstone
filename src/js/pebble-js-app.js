var options = {
	appMessage: {
		maxTries: 3,
		retryTimeout: 3000,
		timeout: 100
	},
	http: {
		timeout: 20000
	},
	timer: {
		statusTimer: 5000,
	}
};

var mediaPlayer = {
	PLEX: 0,
	VLC: 1,
	XBMC: 2
};

var appMessageQueue = [];
var players = JSON.parse(localStorage.getItem('players')) || [];

function sendPlayerList() {
	if (players.length === 0) {
		appMessageQueue.push({'message': {index: 0, request: true, title: 'No players found', status: '', player: 255}});
	}
	for (var i = 0; i < players.length; i++) {
		appMessageQueue.push({message: {
			index: i,
			request: true,
			title: players[i].title,
			status: players[i].subtitle,
			player: parseInt(players[i].player)
		}});
	}
	sendAppMessageQueue();
}

function sendAppMessageQueue() {
	if (appMessageQueue.length > 0) {
		currentAppMessage = appMessageQueue[0];
		currentAppMessage.numTries = currentAppMessage.numTries || 0;
		currentAppMessage.transactionId = currentAppMessage.transactionId || -1;
		if (currentAppMessage.numTries < options.appMessage.maxTries) {
			console.log('Sending AppMessage to Pebble: ' + JSON.stringify(currentAppMessage.message));
			Pebble.sendAppMessage(
				currentAppMessage.message,
				function(e) {
					appMessageQueue.shift();
					setTimeout(function() {
						sendAppMessageQueue();
					}, options.appMessage.timeout);
				}, function(e) {
					console.log('Failed sending AppMessage for transactionId:' + e.data.transactionId + '. Error: ' + e.data.error.message);
					appMessageQueue[0].transactionId = e.data.transactionId;
					appMessageQueue[0].numTries++;
					setTimeout(function() {
						sendAppMessageQueue();
					}, options.appMessage.retryTimeout);
				}
			);
		} else {
			appMessageQueue.shift();
			console.log('Failed sending AppMessage for transactionId:' + currentAppMessage.transactionId + '. Bailing. ' + JSON.stringify(currentAppMessage.message));
		}
	} else {
		console.log('AppMessage queue is empty.');
	}
}

function makeRequestToPLEX(index, request) {

}

function makeRequestToVLC(index, request) {
	var xhr = new XMLHttpRequest();
	xhr.open('GET', 'http://' + players[index].server + '/requests/status.json?' + request, true);
	xhr.setRequestHeader('Authorization', 'Basic ' + base64_encode(':' + players[index].password));
	xhr.timeout = options.http.timeout;
	xhr.onload = function(e) {
		if (xhr.readyState == 4) {
			if (xhr.status == 200) {
				if (xhr.responseText) {
					res    = JSON.parse(xhr.responseText);
					title  = res.information || players[index].title;
					title  = title.category || players[index].title;
					title  = title.meta || players[index].title;
					title  = title.filename || players[index].title;
					title  = title.substring(0,30);
					status = res.state ? res.state.charAt(0).toUpperCase()+res.state.slice(1) : 'Unknown';
					status = status.substring(0,30);
					volume = res.volume || 0;
					volume = (volume / 512) * 200;
					volume = (volume > 200) ? 200 : volume;
					volume = Math.round(volume);
					length = res.length || 0;
					seek   = res.time || 0;
					seek   = (seek / length) * 100;
					seek   = Math.round(seek);
					appMessageQueue.push({'message': {'player': mediaPlayer.VLC, 'title': title, 'status': status, 'volume': volume, 'seek': seek}});
				} else {
					console.log('Invalid response received! ' + JSON.stringify(xhr));
					appMessageQueue.push({'message': {'player': mediaPlayer.VLC, 'title': 'Error: Invalid response received!'}});
				}
			} else {
				console.log('Request returned error code ' + xhr.status.toString());
				appMessageQueue.push({'message': {'player': mediaPlayer.VLC, 'title': 'Error: ' + xhr.statusText}});
			}
		}
		sendAppMessageQueue();
	};
	xhr.ontimeout = function() {
		console.log('HTTP request timed out');
		appMessageQueue.push({'message': {'player': mediaPlayer.VLC, 'title': 'Error: Request timed out!'}});
		sendAppMessageQueue();
	};
	xhr.onerror = function() {
		console.log('HTTP request returned error');
		appMessageQueue.push({'message': {'player': mediaPlayer.VLC, 'title': 'Error: Failed to connect!'}});
		sendAppMessageQueue();
	};
	xhr.send(null);
}

var XBMCStatusUpdate = {
	volumeReceived: false,
	titleReceived: false,
	seekReceived: false,
	volume: 0,
	title: "",
	status: "Loading...",
	seek: 0,
	requestUpdates: function(index) {
		volumeRequest = {"jsonrpc":"2.0","method":"Application.GetProperties","params":{"properties":["volume"]},"id":"1"};
		makeRequestToXBMC(index, volumeRequest, true);
		titleRequest = {"jsonrpc":"2.0","method":"Player.GetItem","params":{"playerid":1,"properties":["title"]},"id":"1"};
		makeRequestToXBMC(index, titleRequest, true);
		seekRequest = {"jsonrpc":"2.0","method":"Player.GetProperties","id": "1","params":{"playerid":1,"properties":["percentage","speed"]}};
		makeRequestToXBMC(index, seekRequest, true);
	},
	checkForReadyUpdate: function(index) {
		if (this.volumeReceived && this.titleReceived && this.seekReceived) {
			this.sendUpdate();
			this.reset();
		}
	},
	sendUpdate: function(index) {
		var title  = this.title || players[index].title;
		appMessageQueue.push({'message': {'player': mediaPlayer.XBMC, 'volume': this.volume, 'title': title, 'status': this.status, 'seek': this.seek}});
		sendAppMessageQueue();
	},
	reset: function() {
		this.volumeReceived = this.titleReceived = this.seekReceived = false;
		this.volume = 0;
		this.title = "";
		this.status = "Loading..."
		this.seek = 0;
	}
};

function makeRequestToXBMC(index, request, statusRefresh) {
	request = request || {};
	var xhr = new XMLHttpRequest();
	xhr.open('POST', 'http://' + players[index].server + '/jsonrpc?', true);
	xhr.setRequestHeader('Authorization', 'Basic ' + base64_encode(':' + players[index].password));
	xhr.timeout = options.http.timeout;
	xhr.onload = function(e) {
		if (xhr.readyState == 4) {
			if (xhr.status == 200) {
				if (xhr.responseText) {
					res = JSON.parse(xhr.responseText);
					if (statusRefresh) {
						if (res.hasOwnProperty('error')) {
							XBMCStatusUpdate.sendUpdate(index);
						};
						if (!res.hasOwnProperty('result')) return;
						if (res.result.volume) {
							var volume = res.result.volume || 0;
							volume = (volume > 100) ? 100 : volume;
							volume = Math.round(volume);
							XBMCStatusUpdate.volume = volume;
							XBMCStatusUpdate.volumeReceived = true;
							XBMCStatusUpdate.checkForReadyUpdate(index);
						} else if (res.result.item) {
							title  = res.result.item.title || players[index].title;
							title  = title.substring(0,30);
							XBMCStatusUpdate.title = title;
							XBMCStatusUpdate.titleReceived = true;
							XBMCStatusUpdate.checkForReadyUpdate(index);
						} else if (res.result.percentage) {
							status = res.result.speed ? "Playing" : "Paused";
							seek   = res.result.percentage || 0;
							seek   = Math.round(seek);
							XBMCStatusUpdate.status = status;
							XBMCStatusUpdate.seek = seek;
							XBMCStatusUpdate.seekReceived = true;
							XBMCStatusUpdate.checkForReadyUpdate(index);
						}
					} else {
						XBMCStatusUpdate.requestUpdates(index);
					}
				} else {
					console.log('Invalid response received! ' + JSON.stringify(xhr));
					appMessageQueue.push({'message': {'player': mediaPlayer.XBMC, 'title': 'Error: Invalid response received!'}});
				}
			} else {
				console.log('Request returned error code ' + xhr.status.toString());
				appMessageQueue.push({'message': {'player': mediaPlayer.XBMC, 'title': 'Error: ' + xhr.statusText}});
			}
		}
	};
	xhr.ontimeout = function() {
		console.log('HTTP request timed out');
		appMessageQueue.push({'message': {'player': mediaPlayer.XBMC, 'title': 'Error: Request timed out!'}});
		sendAppMessageQueue();
	};
	xhr.onerror = function() {
		console.log('HTTP request returned error');
		appMessageQueue.push({'message': {'player': mediaPlayer.XBMC, 'title': 'Error: Failed to connect!'}});
		sendAppMessageQueue();
	};
	xhr.setRequestHeader('Content-type','application/json');
	xhr.send(JSON.stringify(request));
}

Pebble.addEventListener('ready', function(e) {
	sendPlayerList();
});

Pebble.addEventListener('appmessage', function(e) {
	console.log('AppMessage received from Pebble: ' + JSON.stringify(e.payload));

	var index = e.payload.index;

	if (!isset(index)) {
		sendPlayerList();
		return;
	}

	if (players[index].player == mediaPlayer.PLEX) {
		return;
	}

	if (players[index].player == mediaPlayer.VLC) {
		request = e.payload.request || '';
		if (!isset(players[index].server) || !isset(players[index].password)) {
			console.log('[VLC] Server options not set!');
			appMessageQueue.push({'message': {'player': mediaPlayer.VLC, 'title': 'Set options via Pebble app'}});
			sendAppMessageQueue();
			return;
		}
		switch (request) {
			case 'play_pause':
				request = 'command=pl_pause';
				break;
			case 'volume_up':
				request = 'command=volume&val=%2B12.8';
				break;
			case 'volume_down':
				request = 'command=volume&val=-12.8';
				break;
			case 'volume_min':
				request = 'command=volume&val=0';
				break;
			case 'volume_max':
				request = 'command=volume&val=512';
				break;
			case 'seek_forward_short':
				request = 'command=seek&val=%2B10S';
				break;
			case 'seek_rewind_short':
				request = 'command=seek&val=-10S';
				break;
			case 'seek_forward_long':
				request = 'command=seek&val=%2B1M';
				break;
			case 'seek_rewind_long':
				request = 'command=seek&val=-1M';
				break;
		}
		makeRequestToVLC(index, request);
		return;
	}

	if (players[index].player == mediaPlayer.XBMC) {
		request = e.payload.request || '';
		if (!isset(players[index].server)) {
			console.log('[XBMC] Server options not set!');
			appMessageQueue.push({'message': {'player': mediaPlayer.XBMC, 'title': 'Set options via Pebble app'}});
			sendAppMessageQueue();
			return;
		}
		var refreshStatus;
		var requestObj = { "jsonrpc": "2.0", "id": 1, "params": {} };
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
			case 'volume_up':
				requestObj.method = 'Application.SetVolume';
				requestObj.params.volume = 'increment';
				break;
			case 'volume_down':
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
			case 'seek_forward_short':
				requestObj.method = 'Player.Seek';
				requestObj.params.playerid = 1;
				requestObj.params.value = 'smallforward';
				break;
			case 'seek_rewind_short':
				requestObj.method = 'Player.Seek';
				requestObj.params.playerid = 1;
				requestObj.params.value = 'smallbackward';
				break;
			case 'seek_forward_long':
				requestObj.method = 'Player.Seek';
				requestObj.params.playerid = 1;
				requestObj.params.value = 'bigforward';
				break;
			case 'seek_rewind_long':
				requestObj.method = 'Player.Seek';
				requestObj.params.playerid = 1;
				requestObj.params.value = 'bigbackward';
				break;
			default:
				XBMCStatusUpdate.requestUpdates(index);
				return;
				break;
		}
		makeRequestToXBMC(index, requestObj, false);
		setTimeout(function() {
			XBMCStatusUpdate.requestUpdates(index);
		}, options.timer.statusTimer);
		return;
	}
});

Pebble.addEventListener('showConfiguration', function(e) {
	var data = {
		players: players
	};
	// will switch to gh-pages when we go live
	var uri = 'https://rawgithub.com/Skipstone/Skipstone/master/configuration/index.html?data=' + encodeURIComponent(base64_encode(JSON.stringify(data)));
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

function isset(i) {
	return (typeof i != 'undefined');
}

function base64_encode(input) {
	var keyStr = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=';
	var chr1, chr2, chr3, enc1, enc2, enc3, enc4, output = '', i = 0;
	do {
		chr1 = input.charCodeAt(i++);
		chr2 = input.charCodeAt(i++);
		chr3 = input.charCodeAt(i++);
		enc1 = chr1 >> 2;
		enc2 = ((chr1 & 3) << 4) | (chr2 >> 4);
		enc3 = ((chr2 & 15) << 2) | (chr3 >> 6);
		enc4 = chr3 & 63;
		if (isNaN(chr2)) { enc3 = enc4 = 64; } else if (isNaN(chr3)) { enc4 = 64; }
		output = output + keyStr.charAt(enc1) + keyStr.charAt(enc2) + keyStr.charAt(enc3) + keyStr.charAt(enc4);
		chr1 = chr2 = chr3 = enc1 = enc2 = enc3 = enc4 = '';
	} while (i < input.length);
	return output;
}

function base64_decode(input) {
	var keyStr = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=';
	var chr1, chr2, chr3, enc1, enc2, enc3, enc4, output = '', i = 0;
	input = input.replace(/[^A-Za-z0-9\+\/\=]/g, '');
	do {
		enc1 = keyStr.indexOf(input.charAt(i++));
		enc2 = keyStr.indexOf(input.charAt(i++));
		enc3 = keyStr.indexOf(input.charAt(i++));
		enc4 = keyStr.indexOf(input.charAt(i++));
		chr1 = (enc1 << 2) | (enc2 >> 4);
		chr2 = ((enc2 & 15) << 4) | (enc3 >> 2);
		chr3 = ((enc3 & 3) << 6) | enc4;
		output = output + String.fromCharCode(chr1);
		if (enc3 != 64) output = output + String.fromCharCode(chr2);
		if (enc4 != 64) output = output + String.fromCharCode(chr3);
		chr1 = chr2 = chr3 = enc1 = enc2 = enc3 = enc4 = '';
	} while (i < input.length);
	return output;
}
