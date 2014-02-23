$(document).bind('pageinit', function() {

	var players = [];
	var panels = 0;

	var data = /data=([^&|^\/]*)/.exec(location.search);
	if (data && data[1]) {
		try {
			var d = JSON.parse(atob(decodeURIComponent(data[1])));
			players = d.players;
		} catch(err) {

		}
	}

	$.each(players, function(index, player) {
		var newPanel = $('.panel-template-' + player.player).first().clone();
		addNewPanel(player.title, newPanel);
		$.each(player, function(index, value) {
			newPanel.find('#'+index).val(value);
		});
	});

	function addNewPanel(title, newPanel) {
		newPanel.find('.accordion-toggle').attr('href', '#panel' + (++panels));
		newPanel.find('.panel-collapse').attr('id', 'panel' + panels);
		newPanel.find('.panel-title').text(title);
		$('#accordion').append(newPanel.fadeIn());
		newPanel.find('#title').change(function() {
			newPanel.find('.panel-title').text($(this).val());
		});
		newPanel.find('.btn-delete').click(function() {
			newPanel.remove();
		});
		newPanel.find('.panel-heading').on('swipe', function() {
			if(newPanel.find('.btn-delete').is(':hidden')) { newPanel.find('.btn-delete').show(); } else { newPanel.find('.btn-delete').hide(); }
		});
	}

	$('.btn-add-plex').on('click', function() {
		addNewPanel('New Plex Remote', $('.panel-template-0').first().clone());
	});

	$('.btn-add-vlc').on('click', function() {
		addNewPanel('New VLC Remote', $('.panel-template-1').first().clone());
	});

	$('.btn-add-xbmc').on('click', function() {
		addNewPanel('New XBMC Remote', $('.panel-template-2').first().clone());
	});

	$('.btn-add-wdtv').on('click', function() {
		addNewPanel('New WDTV Remote', $('.panel-template-3').first().clone());
	});

	$('.btn-save').on('click', function() {
		var players = $.map($('.panel'), function(e) {
			if ($(e).find('.panel-collapse').attr('id').indexOf('template') != -1)
				return;
			var player = {};
			$(e).find('input').map(function() {
				player[this.id] = $(this).val();
			});
			return player;
		});
		var ret = {
			players: players
		};
		document.location = 'pebblejs://close#' + encodeURIComponent(btoa(JSON.stringify(ret)));
	});

	$('.panel-container').sortable({
		handle: '.handle',
		axis: 'y'
	});
});
