#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiClient.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SPI.h>
#include <Wire.h>
#include <algorithm>
#include <bitset>



#include "Config.h"
#include "FlipDot.h"


// SCL GPIO5
// SDA GPIO4

#define OLED_RESET 0               // GPIO0
Adafruit_SSD1306 oled(OLED_RESET); // will often mirror the expected content of
                                   // the flipdot display
FlipDot<Adafruit_SSD1306> flipDot(oled);
ESP8266WebServer server(80);
const int led = 13; // blinks sometimes

void resetFlipDots() {
  flipDot.reset();
  server.send(200, "text/plain", "Display was blinked");
}

void handleRoot() {
  digitalWrite(led, 1);
  if (server.args() > 0) {
    flipDot.fillScreen(0);
    flipDot.setCursor(10, 2);
    flipDot.setTextSize(1);
    flipDot.println(server.argName(0));
    flipDot.println(server.arg(0));
    flipDot.display();
    server.send(200, "text/plain", "Dots were flipped");
  } else {
    server.send(403, "text/plain",
                "I'm sorry Dave, I'm Afraid I can't do that");
  }
}

void handleNotFound() {
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(led, 0);
}

void handleSay() {

  String text = "";
  unsigned char x = 0, y = 0, color = 0, size = 1, text_color =1;

  for (uint8_t i = 0; i < server.args(); i++) {
    const auto &name = server.argName(i);
    const auto &arg = server.arg(i);
    if (name == "text") text = arg;
    if (name == "x") x = arg.toInt();
    if (name == "y") y = arg.toInt();
    if (name == "size") size = arg.toInt();
    if (name == "color") text_color = arg.toInt();
  }
  flipDot.setTextColor(text_color ? WHITE : BLACK);
  flipDot.setCursor(x, y);
  flipDot.setTextSize(size);
  flipDot.println(text);
  flipDot.display();
  server.send(200, "text/plain", "flipped");
}

void setup(void) {
  pinMode(led, OUTPUT);
  digitalWrite(led, 0);
  Serial.begin(38400);
  WiFi.begin(SSID, PASSWORD);
  Serial.println("");
  oled.begin(SSD1306_SWITCHCAPVCC,
             0x3C); // initialize with the I2C addr 0x3C (for the 64x48)
  // Clear the buffer.
  oled.clearDisplay();
  oled.setTextSize(1);
  oled.setTextColor(WHITE);

  oled.println("Connect to");
  oled.println(SSID);

  oled.display();
  // Wait for connection
  int idx = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    oled.println("Connect to");
    oled.println(SSID);

    for (int i = 0; i < SSD1306_LCDWIDTH; i++)
      oled.drawPixel(i, SSD1306_LCDHEIGHT - 1, (i < idx) ? WHITE : BLACK);
    idx++;
    if (SSD1306_LCDWIDTH - 1 == idx)
      idx = 0;
  }

  oled.clearDisplay();
  oled.setCursor(0, 12);
  oled.print("IP address: ");
  oled.setCursor(0, 24);
  oled.println(WiFi.localIP());
  oled.display();

  if (MDNS.begin(MDNS_NAME)) {
    // Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);
  server.on("/say", handleSay);

  server.on("/reset", resetFlipDots);
  server.on("/fill", [flipDot, server]() {
    flipDot.fillScreen(1);
    flipDot.display();
    server.send(200, "text/plain", "Display was filled");
  });
  server.on("/clear", [flipDot, server]() {
    flipDot.fillScreen(0);
    flipDot.display();
    server.send(200, "text/plain", "Display was cleared");
  });

  server.on("/invert", [flipDot, server]() {
    flipDot.invert();
    flipDot.display();
    server.send(200, "text/plain", "Display was inverted");
  });

  server.onNotFound(handleNotFound);

  server.begin();
}

void loop(void) { server.handleClient(); }
