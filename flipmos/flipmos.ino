#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiClient.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SPI.h>
#include <Wire.h>

#include <bitset>

// SCL GPIO5
// SDA GPIO4

#define OLED_RESET 0               // GPIO0
Adafruit_SSD1306 oled(OLED_RESET); // will often mirror the expected content of
                                   // the flipdot display

const char *ssid = "osaa-g";
const char *password = "deadbeef42";

const int SIGN_W = 112;
const int SIGN_H = 20;

ESP8266WebServer server(80);
const int led = 13; // blinks sometimes

class FlipDot : public Adafruit_GFX {
public:
  FlipDot() : Adafruit_GFX(SIGN_W, SIGN_H) {
    framebuffer.reset();
    currentDisplay.set();
    display();
  };
  void drawPixel(int16_t x, int16_t y, uint16_t color) override {
    int idx = y * width() + x;
    framebuffer[idx] = (color != 0);
  }

  /// send changes to display
  void display() {
    for (int j = 0; j < height(); j++) {
      for (int i = 0; i < width(); i++) {
        int idx = j * width() + i;
        if (framebuffer[idx] != currentDisplay[idx])
          plot(i, j, framebuffer[idx]);
      }
    }
    currentDisplay = framebuffer;
    oled.display();
  }

  void invert() { framebuffer.flip(); }

private:
  void plot(char x, char y, char on) {
    /// the display is coded so the x axis is the short axis, but is mounted in a horisontal
    /// position essentially flipping the axis. For sanity this is the only place we do the
    /// flippiing.
    Serial.write((y & 0x7F) | on << 7);
    Serial.write(x);
    // wrap wemos display to show full screen in a manner
    if (x > SSD1306_LCDWIDTH) {
      x -= SSD1306_LCDWIDTH;
      y += SIGN_H + 2;
    }
    oled.drawPixel(x, y, on ? WHITE : BLACK);
  }

  std::bitset<SIGN_W * SIGN_H> framebuffer;
  std::bitset<SIGN_W * SIGN_H> currentDisplay;
};

FlipDot flipDot;

void resetFlipDots() {
  flipDot.fillScreen(0);
  flipDot.setTextSize(2);
  flipDot.setCursor(20, 2);
  flipDot.setTextColor(WHITE);
  flipDot.println("BLINK");

  for (int i = 0; i < 10; i++) {
    flipDot.display();
    delay(10);
    flipDot.invert();
  }
  flipDot.fillScreen(0);
  flipDot.display();
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
                "I am afraid, I can not let you do that dave");
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

void setup(void) {
  pinMode(led, OUTPUT);
  digitalWrite(led, 0);
  Serial.begin(38400);
  WiFi.begin(ssid, password);
  Serial.println("");
  oled.begin(SSD1306_SWITCHCAPVCC,
             0x3C); // initialize with the I2C addr 0x3C (for the 64x48)
  // Clear the buffer.
  oled.clearDisplay();
  oled.setTextSize(1);
  oled.setTextColor(WHITE);

  oled.println("Connect to");
  oled.println(ssid);

  oled.display();
  // Wait for connection
  int idx = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    oled.println("Connect to");
    oled.println(ssid);

    for (int i = 0; i < SSD1306_LCDWIDTH; i++)
      oled.drawPixel(i, SSD1306_LCDHEIGHT - 1, (i < idx) ? WHITE : BLACK);
    idx++;
    if (SSD1306_LCDWIDTH - 1 == idx)
      idx = 0;
  }

  oled.clearDisplay();
  /* oled.print("Connected to "); */
  /* oled.println(ssid); */
  oled.setCursor(0, 12);
  oled.print("IP address: ");
  oled.setCursor(0, 24);
  oled.println(WiFi.localIP());
  oled.display();

  if (MDNS.begin("esp8266")) {
    // Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);
  server.on("/reset", resetFlipDots);
  server.on("/invert", [flipDot, server]() {
    flipDot.invert();
    flipDot.display();
    server.send(200, "text/plain", "Display was inverted");
  });

  server.onNotFound(handleNotFound);

  server.begin();
}

void loop(void) { server.handleClient(); }
