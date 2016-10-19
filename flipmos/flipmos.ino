#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// SCL GPIO5
// SDA GPIO4
#define OLED_RESET 0  // GPIO0
Adafruit_SSD1306 display(OLED_RESET);

const char* ssid = "osaa-g";
const char* password = "deadbeef42";
const int SIGN_W = 20;
const int SIGN_H = 112;


ESP8266WebServer server(80);

const int led = 13;

void plot(char x, char y, char on) {
  Serial.write((x & 0x7F) | on << 7);
  Serial.write(y);
  // wrap wemos display to show full screen in a manner
  if ( y > SSD1306_LCDWIDTH ) {
    y -= SSD1306_LCDWIDTH;
    x += SIGN_W + 2;
  }
  display.drawPixel(y, x, on ? WHITE : BLACK);
}

void drawRect(int x0, int y0, int w, int h, char on) {
  for (int j = 0; j < h ; j++) {
    for (int i = 0; i < w ; i++) {
      plot(x0 + i, y0 +j, on);
    }
  }
}

void fillDots(char on) {
  drawRect(0, 0, SIGN_W, SIGN_H, on);
  display.display();
}
void resetFlipDots() {
  for(int i = 0; i < 5; i++) {
    fillDots(0);
    display.display();
    delay(100);
    fillDots(1);
    display.display();
    delay(100);
}
  server.send(200, "text/plain", "Display was blinked");
}


void heartbeat(int x) {
  for (int y = 0; y < SIGN_H; y++) {
    plot(x,y,1);
  }
  display.display();
  delay(100);
  for (int y = 0; y < SIGN_H; y++) {
    plot(x,y,0);
  }
  display.display();
}

void handleRoot() {
  digitalWrite(led, 1);
  for (uint8_t i=0; i<server.args(); i++){
    if (server.argName(1) != "bits")
      Serial.print("ignored argument");

    Serial.write(server.arg(i).c_str());
  }

  heartbeat(12);
  server.send(200, "text/plain", "hello from esp8266!");
  digitalWrite(led, 0);
}

void handleNotFound(){
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(led, 0);
}

void setup(void){
  pinMode(led, OUTPUT);
  digitalWrite(led, 0);
  Serial.begin(38400);
  WiFi.begin(ssid, password);
  Serial.println("");
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 64x48)
  // Clear the buffer.
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  display.println("Connect to");
  display.println(ssid);

  display.display();
  // Wait for connection
  int idx = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    display.println("Connect to");
    display.println(ssid);

    for(int i = 0; i < SSD1306_LCDWIDTH; i++) 
      display.drawPixel(i, SSD1306_LCDHEIGHT-1, (i<idx) ? WHITE : BLACK);
    idx++;
    if (SSD1306_LCDWIDTH-1 == idx)
      idx = 0;
  }

  display.clearDisplay();
  /* display.print("Connected to "); */
  /* display.println(ssid); */
  display.setCursor(0,12);
  display.print("IP address: ");
  display.setCursor(0,24);
  display.println(WiFi.localIP());
  display.display();

  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);

  server.on("/reset", resetFlipDots);

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void loop(void){
  server.handleClient();
  heartbeat(4);
  heartbeat(8);
}
