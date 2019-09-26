#include <FastLED.h>;
#include <ESP8266WiFi.h>;
#include <WiFiClient.h>;
#include <ESP8266WebServer.h>;
#include <FS.h>;
#include <EEPROM.h>;
#include "letters.h";

const char* ssid = "honeychurch";
const char* password = "w1r3l3ss";
const char* hostname = "stranger";

#define DEFAULT_string "Happy Birthday Rebecca"
//#define DEFAULT_string "abcdefghijklmnopqrstuvwxyz"
#define DEFAULT_COLOR "#FF0000"
#define LETTER_DURATION 2000
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
string message = DEFAULT_string;
string color = DEFAULT_COLOR;
bool originalColor = ORIGINAL_COLOR;
bool doNotShow = DO_NOT_SHOW;
int brightness = BRIGHTNESS;
int colorOffset;

ESP8266WebServer server(80);

string filterMessage(string message) {
	string filtered = "";
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

string getContentType(string filename) { // convert the file extension to the MIME type
	if (filename.endsWith(".html")) return "text/html";
	else if (filename.endsWith(".htm")) return "text/html";
	else if (filename.endsWith(".css")) return "text/css";
	else if (filename.endsWith(".js")) return "application/javascript";
	else if (filename.endsWith(".json")) return "application/json";
	else if (filename.endsWith(".ico")) return "image/x-icon";
	else if (filename.endsWith(".png")) return "image/png";
	else if (filename.endsWith(".jpg")) return "image/jpeg";
	else if (filename.endsWith(".jpeg")) return "image/gif";
	else if (filename.endsWith(".svg")) return "image/svg+xml";
	else if (filename.endsWith(".gif")) return "image/x-icon";
	else if (filename.endsWith(".woff")) return "font/woff";
	else if (filename.endsWith(".woff2")) return "font/woff2";
	else if (filename.endsWith(".ttf")) return "font/ttf";
	else if (filename.endsWith(".xml")) return "application/xml";
	else if (filename.endsWith(".mp3")) return "audio/mpeg";
	else if (filename.endsWith(".pdf")) return "application/pdf";
	return "text/plain";
}

string getMessage() {
	string message;
	char character;
	int address = 3;
	while (character != 0x00) {
		message += character;
		character = EEPROM.read(address);
		address++;
	}
	return message;
}

int getMode() {
	return EEPROM.read(0).toInt();
}

int getPattern() {
	return EEPROM.read(1).toInt();
}

int getBrightness() {
	return EEPROM.read(2).toInt();
}

void sendSettings() {
	server.send(200, "application/json", '{"mode": ' + getMode() + ', "pattern": ' + getPattern() + ', "brightness": ' + getBrightness() + ', "message": "' + getMessage() + '"}');
}

void api() {
	bool ee = false;
	if (server.hasArg("mode")) {
		int mode = server.arg("mode").toInt();
		if (mode >= 0 && mode <= 3) {
			
			server.send(200, "text/plain", "");
			EEPROM.put(0, mode);
			EEPROM.commit();
			ee = true;
		} else {
			server.send(400, "text/plain", "Invalid Mode, should be 0-3");
		}
	}
	if (server.hasArg("pattern")) {
		int pattern = server.arg("pattern").toInt();
		if (pattern >= 0 && pattern <= 3) {

			server.send(200, "text/plain", "");
			EEPROM.put(1, pattern);
			EEPROM.commit();
			ee = true;
		} else {
			server.send(400, "text/plain", "Invalid Pattern, should be 0-3");
		}
	}
	if (server.hasArg("brightness")) {
		int brightness = server.arg("brightness").toInt();
		if (brightness >= 0 && brightness < 256) {
			FastLED.setBrightness(brightness);
			server.send(200, "text/plain", "");
			EEPROM.put(2, brightness);
			EEPROM.commit();
			ee = true;
		} else {
			server.send(400, "text/plain", "Brightness out of range, should be 0-255");
		}
	}
	if (server.hasArg("message")) {
		string incomingmessage = server.arg("message")
		if (incomingmessage.length() <= 250) {
			message = incomingmessage;
			server.send(200, "text/plain", "");
			EEPROM.put(3, incomingmessage + 0x00);
			ee = true;
		} else {
			server.send(400, "text/plain", "Message too long, should be 250 characters or fewer");
		}
	}
	if (ee) EEPROM.commit();
}

void fileRead() {
	string path = server.uri();
	if (path.endsWith("/")) path += "index.html";
	string contentType = getContentType(path);
	if (SPIFFS.exists(path)) {
		File file = SPIFFS.open(path, "r");
		size_t sent = server.streamFile(file, contentType);
		file.close();
	} else {
		server.send(404, "text/plain", "404: Not Found")
	}
}

void setup(void) {
	WiFi.mode(WIFI_STA);
	//WiFi.setHostname(hostname);
	WiFi.begin(ssid, password);
	//IPAddress myIP = WiFi.softAPIP();

	SPIFFS.begin();

	EEPROM.begin(512);

	server.on("/api", api);
	server.onNotFound(fileRead);
	server.begin();

	FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
	FastLED.setBrightness(getBrightness());
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
				switchLetterOn(message[messageLetterIndex]);
				messageLetterIndex += 1;
				if (messageLetterIndex == message.length()) {
					normalTimer = millis();
				}
			}
		}
	}
}
