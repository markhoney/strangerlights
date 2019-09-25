document.addEventListener('DOMContentLoaded', function() {
	function ajaxGet(url) {
		const xhr = new XMLHttpRequest();
		xhr.open("GET", url);
		xhr.send();
	}
	const app = new Vue({
		el: '#app',
		data: {
			name: [
				//"Stranger",
				//"Strings",
				"Rebecca's",
				"Lights",
			],
			normal: false,
			duration: 500,
			pause: 15000,
			message: "",
			mode: "off",
			brightness: 128,
			original: true,
			colour: "#FF0000",
			pattern: "rainbow",
		},
		methods: {
			messageSend() {
				ajaxGet('message/' + encodeURIComponent(this.message));
			},
			brightnessChange() {
				ajaxGet('brightness/' + this.brightness);
			},
		},
		watch: {
			mode: function(value) {
				ajaxGet('mode/' + value);
			},
			pattern: function(value) {
				ajaxGet('pattern/' + value);
			},
		},
	});
	document.title = app.name.join(" ");
});