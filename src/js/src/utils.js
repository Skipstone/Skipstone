function isset(i) {
	return (typeof i !== 'undefined');
}

function serialize(obj) {
	var s = [];
	for (var p in obj) {
		if (obj.hasOwnProperty(p)) {
			s.push(encodeURIComponent(p) + '=' + encodeURIComponent(obj[p]));
		}
	}
	return s.join('&');
}

function capitaliseFirstLetter(str) {
	return str.charAt(0).toUpperCase() + str.slice(1);
}

function zeroPad(num, places) {
	var zero = places - num.toString().length + 1;
	return Array(+(zero > 0 && zero)).join('0') + num;
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
