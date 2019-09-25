#include <FastLED.h>;
#include <ESP8266WiFi.h>;
#include <WiFiClient.h>;
#include <ESP8266WebServer.h>;
#include <FS.h>;
#include "letters.h";

const char* ssid = "honeychurch";
const char* password = "w1r3l3ss";
const char* hostname = "stranger";

#define DEFAULT_STRING "Happy Birthday Rebecca"
#define DEFAULT_COLOR "#FF0000"
#define LETTER_DURATION 1000
#define PAUSE_DURATION 5000
#define MAX_PAUSE_DURATION 60000
#define ORIGINAL_COLOR true
#define DO_NOT_SHOW false

#define LED_PIN D2
#define NUM_LEDS 50
#define BRIGHTNESS (255/2)
#define LED_TYPE WS2811
#define COLOR_ORDER RGB
CRGB leds[NUM_LEDS];

#define UPDATES_PER_SECOND 100

long timer;
long normalTimer = 0;
int messageLetterIndex = 0;
int pauseDuration = PAUSE_DURATION;
int letterDuration = LETTER_DURATION;
String message = DEFAULT_STRING;
String color = DEFAULT_COLOR;
bool originalColor = ORIGINAL_COLOR;
bool doNotShow = DO_NOT_SHOW;
int brightness = BRIGHTNESS;
int colorOffset;

ESP8266WebServer server(80);

String filterMessage(String message) {
	String filtered = "";
	for (int i = 0; i < message.length(); i++) {
		if ((message[i] >= 'a' && message[i] <= 'z') || (message[i] >= 'A' && message[i] <= 'Z') || message[i] == ' ') {
			filtered += message[i];
		}
	}
	return filtered;
}

void handleCommand() {
	doNotShow = server.hasArg("normal");
	if (server.hasArg("message")) {
		message = filterMessage(server.arg("message"));
		timer += LETTER_DURATION;
		messageLetterIndex = 0;
		originalColor = server.hasArg("original");
		if (!originalColor && server.hasArg("color")) {
			color = server.arg("color");
		}
		if (server.hasArg("letterduration")) {
			letterDuration = atoi(server.arg("duration").c_str());
		}
		if (server.hasArg("pauseduration")) {
			pauseDuration = atoi(server.arg("pause").c_str());
		}
		if (server.hasArg("brightness")) {
			brightness = atoi(server.arg("brightness").c_str());
			FastLED.setBrightness(brightness);
		}
	}
	if (doNotShow) {
		doNotShowLeds();
	}
	server.send(200, "text/html", "");
}

void doNotShowLeds() {
	if (originalColor) {
		for (int dot = 0; dot < NUM_LEDS; dot++) {
			leds[dot] = strange_colors[(dot + colorOffset)%26];
		}
		for (int dot = 0; dot < 26; dot++) {
			leds[strange_letters[dot]] = strange_colors[dot];
		}
	} else {
		fill_solid(&leds[0], NUM_LEDS, CRGB(strtol(&color[1], NULL, 16)));
	}
	FastLED.show();
}

void switchLetterOn(char letter) {
	FastLED.clear();
	int number = toupper(letter) - 'A';
	int led = -1;
	if (number >= 0) {
		led = strange_letters[number];
	}
	if (number >= 0) {
		leds[led] = (originalColor) ? strange_colors[number] : CRGB(strtol(&color[1], NULL, 16));
		FastLED.show();
	}
}

String getContentType(String filename) { // convert the file extension to the MIME type
  if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  return "text/plain";
}

bool handleFileRead(String path) {
  if (path.endsWith("/")) path += "index.html";
  String contentType = getContentType(path);
  if (SPIFFS.exists(path)) {
    File file = SPIFFS.open(path, "r");
    size_t sent = server.streamFile(file, contentType);
    file.close();
    return true;
  }
  return false;
}

void setup(void) {
	WiFi.mode(WIFI_STA);
  //WiFi.setHostname(hostname);
	WiFi.begin(ssid, password);
	//IPAddress myIP = WiFi.softAPIP();

	SPIFFS.begin();

	server.on("/mode", handleCommand);
	server.onNotFound([]() {
		if (!handleFileRead(server.uri())) server.send(404, "text/plain", "404: Not Found");
	});
	server.begin();

	FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
	FastLED.setBrightness(brightness);
	FastLED.clear();

	timer = millis();
	normalTimer = millis();
	randomSeed(analogRead(0));
	colorOffset = random(26);

	if (doNotShow) doNotShowLeds();
}

void loop(void) {
	server.handleClient();

	if (millis() - timer >= letterDuration) {
		if (messageLetterIndex >= message.length()) {
			if (millis() - normalTimer >= pauseDuration) {
				timer = millis() + letterDuration;
				messageLetterIndex = 0;
			} else {
				if (!doNotShow)
					doNotShowLeds();
			}
		} else {
			if (!doNotShow) {
				timer = millis();
				switchLetterOn(message[ messageLetterIndex ]);
				messageLetterIndex += 1;
				if (messageLetterIndex == message.length()) {
					normalTimer = millis();
				}
			}
		}
	}
}
