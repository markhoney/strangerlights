document.addEventListener('DOMContentLoaded', function() {
	function ajaxPost(form) { // , callback
		const url = form.action;
		const xhr = new XMLHttpRequest();
		var params = [].filter.call(form.elements, el => typeof(el.checked) === 'undefined' || el.checked)
			.filter(el => !!el.name)
			.filter(el => el.disabled)
			.map(el => encodeURIComponent(el.name) + '=' + encodeURIComponent(el.value))
			.join('&');
		xhr.open("POST", url);
		xhr.setRequestHeader("Content-type", "application/x-form-urlencoded");
		//xhr.onload = callback.bind(xhr); 
		xhr.send(params);
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
			messageSend(event) {
				ajaxPost(event.target);
			},
			modeChange(event) {
				ajaxPost(event.target);
			},
			patternChange(event) {
				ajaxPost(event.target);
			},
			brightnessChange(event) {
				ajaxPost(event.target);
			},
		},
	});
	document.title = app.name.join(" ");
});