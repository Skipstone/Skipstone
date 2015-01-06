var XBMC = {
	playerid: 1,

	makeRequest: function(player, data, requestUpdates) {
		var url = 'http://' + player.server + '/jsonrpc';
		var headers = { 'Content-type': 'application/json', 'Authorization': 'Basic ' + base64_encode(player.username + ':' + player.password) };
		http('POST', url, JSON.stringify(data), headers, function(xhr) {
			var res = JSON.parse(xhr.responseText);
			if (res instanceof Array) {
				res.forEach(function(r) {
					XBMC.sendData(r);
				});
			} else if (res instanceof Object) {
				if (res.result && res.result instanceof Array) {
					if (isset(res.result[0].playerid)) XBMC.playerid = res.result[0].playerid;
				}
				XBMC.sendData(res);
			}
			if (requestUpdates) XBMC.requestUpdates(player);
		}, function(e) {
			appMessageQueue.send({type:TYPE.XBMC, status:'Error', title:e});
		});
	},

	sendData: function(res) {
		if (!res.result) return;
		var msg = {};
		msg.type = TYPE.XBMC;
		if (isset(res.result.volume)) {
			var volume = res.result.volume || 0;
			volume = (volume > 100) ? 100 : volume;
			msg.volume = Math.round(volume);
		}
		if (isset(res.result.item)) {
			var title = res.result.item.title || res.result.item.label || player.title || '';
			msg.title = title.substring(0,30);
		}
		if (isset(res.result.percentage)) {
			var seek = res.result.percentage || 0;
			msg.seek = Math.round(seek);
		}
		if (isset(res.result.speed)) {
			msg.status = res.result.speed ? 'Playing' : 'Paused';
		}
		appMessageQueue.send(msg);
	},

	requestUpdates: function(player) {
		var data = [
			{ jsonrpc:'2.0', id:1, method:'Application.GetProperties', params:{properties:['volume']} },
			{ jsonrpc:'2.0', id:1, method:'Player.GetItem', params:{playerid:XBMC.playerid, properties:['title']} },
			{ jsonrpc:'2.0', id:1, method:'Player.GetProperties', params:{playerid:XBMC.playerid, properties:['percentage','speed']} }
		];
		XBMC.makeRequest(player, data);
	},

	handleAppMessage: function(payload) {
		var player = Skipstone.players[payload.player];
		var request = payload.request || null;
		var data = { jsonrpc:'2.0', id:1, params:{} };

		if (!isset(player.server)) {
			console.log('[XBMC] Server not set!');
			return appMessageQueue.send({type:TYPE.XBMC, status:'Error', title:'Server not set.'});
		}

		switch (request) {
			case REQUEST.UP:
				data.method = 'Input.Up';
				break;
			case REQUEST.DOWN:
				data.method = 'Input.Down';
				break;
			case REQUEST.LEFT:
				data.method = 'Input.Left';
				break;
			case REQUEST.RIGHT:
				data.method = 'Input.Right';
				break;
			case REQUEST.SELECT:
				data.method = 'Input.Select';
				break;
			case REQUEST.BACK:
				data.method = 'Input.Back';
				break;
			case REQUEST.PLAY_PAUSE:
				data.method = 'Player.PlayPause';
				data.params.playerid = XBMC.playerid;
				break;
			case REQUEST.VOLUME_INCREMENT:
				data.method = 'Application.SetVolume';
				data.params.volume = 'increment';
				break;
			case REQUEST.VOLUME_DECREMENT:
				data.method = 'Application.SetVolume';
				data.params.volume = 'decrement';
				break;
			case REQUEST.VOLUME_MIN:
				data.method = 'Application.SetVolume';
				data.params.volume = 0;
				break;
			case REQUEST.VOLUME_MAX:
				data.method = 'Application.SetVolume';
				data.params.volume = 100;
				break;
			case REQUEST.FORWARD_SHORT:
				data.method = 'Player.Seek';
				data.params.playerid = XBMC.playerid;
				data.params.value = 'smallforward';
				break;
			case REQUEST.BACKWARD_SHORT:
				data.method = 'Player.Seek';
				data.params.playerid = XBMC.playerid;
				data.params.value = 'smallbackward';
				break;
			case REQUEST.FORWARD_LONG:
				data.method = 'Player.Seek';
				data.params.playerid = XBMC.playerid;
				data.params.value = 'bigforward';
				break;
			case REQUEST.BACKWARD_LONG:
				data.method = 'Player.Seek';
				data.params.playerid = XBMC.playerid;
				data.params.value = 'bigbackward';
				break;
			case REQUEST.NEXT:
				data.method = 'Player.GoTo';
				data.params.playerid = XBMC.playerid;
				data.params.to = 'next';
				break;
			case REQUEST.PREV:
				data.method = 'Player.GoTo';
				data.params.playerid = XBMC.playerid;
				data.params.to = 'previous';
				break;
			case REQUEST.REFRESH:
				data.method = 'Player.GetActivePlayers';
				break;
			default:
				return;
		}

		XBMC.makeRequest(player, data, true);
	}
};
