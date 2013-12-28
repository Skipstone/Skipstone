var appMessageQueue = [];

var options = {
	appMessage: {
		maxTries: 3,
		retryTimeout: 3000,
		timeout: 100
	},
	http: {
		timeout: 20000
	}
};

var mediaPlayer = {
	PLEX: 0,
	VLC: 1,
	XBMC: 2
};

// planned players structure:
//	title: the title to be displayed in menulayer
//	subtitle: the subtitle to be displayed in menulayer
//	player: either of mediaPlayer
//	server: object of server info (would be different for each player)
var players = [];

// test data until we have configuration page
players.push({title: 'Living Room', subtitle: 'hostname', player: mediaPlayer.PLEX});
players.push({title: 'iMac', subtitle: 'imac.local', player: mediaPlayer.VLC, server: 'alum.local:8080', password: 'pass'});
players.push({title: 'Bedroom 1', subtitle: '10.0.1.4', player: mediaPlayer.XBMC});
players.push({title: 'MacBook Pro', subtitle: 'home.example.com', player: mediaPlayer.VLC});

function sendPlayerList() {
	for (var i = 0; i < players.length; i++) {
		appMessageQueue.push({'message': {
			'index': i,
			'request': true,
			'title': players[i].title,
			'status': players[i].subtitle,
			'player': players[i].player
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
			console.log('Failed sending AppMessage for transactionId:' + currentAppMessage.transactionId + '. Bailing. ' + JSON.stringify(currentAppMessage.message));
		}
	}
}

function makeRequestToPLEX(index, request) {

}

function makeRequestToVLC(index, request) {
	var xhr = new XMLHttpRequest();
	xhr.open('GET', 'http://' + players[index].server.host + '/requests/status.json?' + request, true, '', players[index].server.pass);
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

function makeRequestToXBMC(index, request) {
	request = request || {};
	var xhr = new XMLHttpRequest();
	xhr.open('POST', 'http://' + players[index].server.host + '/jsonrpc?', true, '', players[index].server.pass);
	xhr.timeout = options.http.timeout;
	xhr.onload = function(e) {
		if (xhr.readyState == 4) {
			if (xhr.status == 200) {
				if (xhr.responseText) {
					res = JSON.parse(xhr.responseText);
					//TODO: handle response if necessary
				} else {
					console.log('Invalid response received! ' + JSON.stringify(xhr));
					appMessageQueue.push({'message': {'player': mediaPlayer.XBMC, 'title': 'Error: Invalid response received!'}});
				}
			} else {
				console.log('Request returned error code ' + xhr.status.toString());
				appMessageQueue.push({'message': {'player': mediaPlayer.XBMC, 'title': 'Error: ' + xhr.statusText}});
			}
		}
	}
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
		var request = e.payload.request || '';
		if (!isset(players[index].server) || !isset(players[index].server.host) || !isset(players[index].server.pass)) {
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
		var request = e.payload.request || '';
		if (!isset(players[index].server) || !isset(players[index].server.host)) {
			console.log('[XBMC] Server options not set!');
			appMessageQueue.push({'message': {'player': mediaPlayer.XBMC, 'title': 'Set options via Pebble app'}});
			sendAppMessageQueue();
			return;
		}
		requestObj = { "jsonrpc": "2.0", "id": 1, "params": {} };
		switch (request) {
			case 'input_up':
				requestObj["method"] = 'Input.Up';
				break;
			case 'input_down':
				requestObj["method"] = 'Input.Down';
				break;
			case 'input_left':
				requestObj["method"] = 'Input.Left';
				break;
			case 'input_right':
				requestObj["method"] = 'Input.Right';
				break;
			case 'input_select':
				requestObj["method"] = 'Input.Select';
				break;
			case 'input_back':
				requestObj["method"] = 'Input.Back';
				break;
			case 'play_pause':
			{
				requestObj["method"] = 'Player.PlayPause';
				requestObj["params"]["playerid"] = 1;
				break;
			}
			case 'volume_up':
			{
				requestObj["method"] = 'Application.SetVolume';
				requestObj["params"]["volume"] = 'increment';
				break;
			}
			case 'volume_down':
			{
				requestObj["method"] = 'Application.SetVolume';
				requestObj["params"]["volume"] = 'decrement';
				break;
			}
			case 'volume_min':
			{
				requestObj["method"] = 'Application.SetVolume';
				requestObj["params"]["volume"] = 0;
				break;
			}
			case 'volume_max':
			{
				requestObj["method"] = 'Application.SetVolume';
				requestObj["params"]["volume"] = 100;
				break;
			}
			case 'seek_forward_short':
			{
				requestObj["method"] = 'Player.Seek';
				requestObj["params"]["playerid"] = 1;
				requestObj["params"]["value"] = 'smallforward';
				break;
			}
			case 'seek_rewind_short':
			{
				requestObj["method"] = 'Player.Seek';
				requestObj["params"]["playerid"] = 1;
				requestObj["params"]["value"] = 'smallbackward';
				break;
			}
			case 'seek_forward_long':
			{
				requestObj["method"] = 'Player.Seek';
				requestObj["params"]["playerid"] = 1;
				requestObj["params"]["value"] = 'bigforward';
				break;
			}
			case 'seek_rewind_long':
			{
				requestObj["method"] = 'Player.Seek';
				requestObj["params"]["playerid"] = 1;
				requestObj["params"]["value"] = 'bigbackward';
				break;
			}
		}
		makeRequestToXBMC(index, requestObj);
		return;
	}
});

Pebble.addEventListener('showConfiguration', function(e) {
	var uri = '';
	console.log('[configuration] uri: ' + uri);
	Pebble.openURL(uri);
});

Pebble.addEventListener('webviewclosed', function(e) {
	if (e.response) {
		var options = JSON.parse(decodeURIComponent(e.response));
		console.log('[configuration] options received: ' + JSON.stringify(options));
		// do something. store configuration to localStore and call refresh.
	} else {
		console.log('[configuration] no options received');
	}
});

function isset(i) {
	return (typeof i != 'undefined')
}
