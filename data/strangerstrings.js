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
				ajaxGet('api?message=' + encodeURIComponent(this.message));
			},
			brightnessChange() {
				ajaxGet('api?brightness=' + this.brightness);
			},
		},
		watch: {
			mode: function(value) {
				ajaxGet('api?mode=' + value);
			},
			pattern: function(value) {
				ajaxGet('api?pattern=' + value);
			},
		},
	});
	document.title = app.name.join(" ");
});