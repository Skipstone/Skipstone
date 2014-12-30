var WDTV = {
	handleAppMessage: function(payload) {
		var player = Skipstone.players[payload.player];
		var request = payload.request || null;
		var data = {};

		if (!isset(player.server)) {
			return appMessageQueue.send({type:TYPE.WDTV, method:METHOD.ERROR, status:'Server not set.'});
		}

		switch (request) {
			case REQUEST.POWER:
				data.remote = 'w';
				break;
			case REQUEST.HOME:
				data.remote = 'o';
				break;
			case REQUEST.PREV:
				data.remote = '[';
				break;
			case REQUEST.STOP:
				data.remote = 't';
				break;
			case REQUEST.NEXT:
				data.remote = ']';
				break;
			case REQUEST.REWIND:
				data.remote = 'H';
				break;
			case REQUEST.PLAY_PAUSE:
				data.remote = 'p';
				break;
			case REQUEST.FORWARD:
				data.remote = 'I';
				break;
			case REQUEST.BACK:
				data.remote = 'T';
				break;
			case REQUEST.LEFT:
				data.remote = 'l';
				break;
			case REQUEST.OK:
				data.remote = 'n';
				break;
			case REQUEST.UP:
				data.remote = 'u';
				break;
			case REQUEST.OPTION:
				data.remote = 'G';
				break;
			case REQUEST.RIGHT:
				data.remote = 'r';
				break;
			case REQUEST.PREV_PAGE:
				data.remote = 'U';
				break;
			case REQUEST.DOWN:
				data.remote = 'd';
				break;
			case REQUEST.NEXT_PAGE:
				data.remote = 'D';
				break;
			case REQUEST.MUTE:
				data.remote = 'M';
				break;
			case REQUEST.SETUP:
				data.remote = 's';
				break;
			case REQUEST.A:
				data.remote = 'x';
				break;
			case REQUEST.B:
				data.remote = 'y';
				break;
			case REQUEST.C:
				data.remote = 'z';
				break;
			case REQUEST.D:
				data.remote = 'A';
				break;
			case REQUEST.SEARCH:
				data.remote = 'E';
				break;
			case REQUEST.EJECT:
				data.remote = 'X';
				break;
		}

		http('POST', 'http://' + player.server + '/cgi-bin/toServerValue.cgi', JSON.stringify(data));
	}
};
