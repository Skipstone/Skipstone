var VLC = {
	makeRequest: function(player, params) {
		var url = 'http://' + player.server + '/requests/status.json';
		var headers = { 'Authorization': 'Basic ' + base64_encode(':' + player.password) };
		http('GET', url, params, headers, function(xhr) {
			if (xhr.status >= 300) {
				return appMessageQueue.send({type:TYPE.VLC, status:'Error', title:'Server error: HTTP ' + xhr.statusText});
			}
			var res = JSON.parse(xhr.responseText);
			var title = res.information || player.title;
			title = title.category || player.title;
			title = title.meta || player.title;
			title = title.filename || player.title;
			title = title.substring(0,30);
			var status = res.state ? capitaliseFirstLetter(res.state) : 'Unknown';
			status = status.substring(0,30);
			var volume = res.volume || 0;
			volume = (volume / 512) * 200;
			volume = (volume > 200) ? 200 : volume;
			volume = Math.round(volume);
			var length = res.length || 0;
			var seek = res.time || 0;
			seek = length ? (seek / length) * 100 : 0;
			seek = Math.round(seek);
			appMessageQueue.send({type:TYPE.VLC, title:title, status:status, volume:volume, seek:seek});
		}, function(e) {
			appMessageQueue.send({type:TYPE.VLC, status:'Error', title:e});
		});
	},

	handleAppMessage: function(payload) {
		var player = Skipstone.players[payload.player];
		var request = payload.request || null;
		var params = {};

		if (!isset(player.server)) {
			return appMessageQueue.send({type:TYPE.VLC, status:'Error', title:'Server not set.'});
		}

		switch (request) {
			case REQUEST.PLAY_PAUSE:
				params.command = 'pl_pause';
				break;
			case REQUEST.VOLUME_INCREMENT:
				params.command = 'volume';
				params.val = '+12.8';
				break;
			case REQUEST.VOLUME_DECREMENT:
				params.command = 'volume';
				params.val = '-12.8';
				break;
			case REQUEST.VOLUME_MIN:
				params.command = 'volume';
				params.val = '0';
				break;
			case REQUEST.VOLUME_MAX:
				params.command = 'volume';
				params.val = '512';
				break;
			case REQUEST.FORWARD_SHORT:
				params.command = 'seek';
				params.val = '+10S';
				break;
			case REQUEST.FORWARD_LONG:
				params.command = 'seek';
				params.val = '+1M';
				break;
			case REQUEST.BACKWARD_SHORT:
				params.command = 'seek';
				params.val = '-10S';
				break;
			case REQUEST.BACKWARD_LONG:
				params.command = 'seek';
				params.val = '-1M';
				break;
			case REQUEST.NEXT:
				params.command = 'pl_next';
				break;
			case REQUEST.PREV:
				params.command = 'pl_previous';
				break;
		}

		VLC.makeRequest(player, params);
	}
};
