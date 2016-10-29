#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiClient.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <FS.h>
#include <algorithm>
#include <bitset>

#include "Config.h"
#include "FlipDot.h"
#include "PbmDraw.h"

// SCL GPIO5
// SDA GPIO4

#define OLED_RESET 0               // GPIO0
Adafruit_SSD1306 oled(OLED_RESET); // will often mirror the expected content of
                                   // the flipdot display

typedef FlipDot<Adafruit_SSD1306> FlipDot_t;
FlipDot_t flipDot(oled);
ESP8266WebServer server(80);
const int led = 13; // blinks sometimes

// holds the current upload
File fsUploadFile;

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
  unsigned char x = 0, y = 0, color = 0, size = 1, text_color = 1, clear = 1;

  for (uint8_t i = 0; i < server.args(); i++) {
    const auto &name = server.argName(i);
    const auto &arg = server.arg(i);
    if (name == "text")
      text = arg;
    if (name == "x")
      x = arg.toInt();
    if (name == "y")
      y = arg.toInt();
    if (name == "size")
      size = arg.toInt();
    if (name == "color")
      text_color = arg.toInt();
    if (name == "clear")
      clear = arg.toInt();
  }
  if (clear)
    flipDot.fillScreen(text_color ? BLACK : WHITE);
  flipDot.setTextColor(text_color ? WHITE : BLACK);
  flipDot.setCursor(x, y);
  flipDot.setTextSize(size);
  flipDot.println(text);
  flipDot.display();
  server.send(200, "text/plain", "flipped");
}

void handleShow() {
  int x = 0, y = 0;
  String fileName;
  for (uint8_t i = 0; i < server.args(); i++) {
    const auto &name = server.argName(i);
    const auto &arg = server.arg(i);
    if (name == "filename")
      fileName = arg;
    if (name == "x")
      x = arg.toInt();
    if (name == "y")
      y = arg.toInt();
  }

  if (!SPIFFS.exists(fileName)) {
    server.send(501, "text/plain", "unknown file");
    return;
  }
  File f = SPIFFS.open(fileName, "r");
  if (!f) {
    server.send(501, "text/plain", "Could not open file");
    return;
  }

  PbmDraw<FlipDot_t> pbm(flipDot, f);

  int chukSize = 512;
  if (!pbm.readHeader()) {
    server.send(501, "text/plain", "Could not read Header");
    return;
  }

  if (server.hasArg("dump")) {
    auto msg = f.readString();
    msg += "\n\n\t";
    msg += pbm.w;
    msg += "x";
    msg += pbm.h;

    server.send(200, "text/plain", msg);
    f.close();
    return;
  }

  pbm.blit(0, 0);
  int flipped = flipDot.display();
  String res = "Flipped bits for:";
  res += fileName;
  res += " ";
  res += flipped;
  f.close();
  server.send(200, "text/plain", res);
}
void handleFileUpload() {
  HTTPUpload &upload = server.upload();
  if (upload.status == UPLOAD_FILE_START) {
    String filename = upload.filename;
    if (!filename.startsWith("/"))
      filename = "/" + filename;
    //    DBG_OUTPUT_PORT.print("handleFileUpload Name: ");
    //    DBG_OUTPUT_PORT.println(filename);
    fsUploadFile = SPIFFS.open(filename, "w");
    filename = String();
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    // DBG_OUTPUT_PORT.print("handleFileUpload Data: ");
    // DBG_OUTPUT_PORT.println(upload.currentSize);
    if (fsUploadFile)
      fsUploadFile.write(upload.buf, upload.currentSize);
  } else if (upload.status == UPLOAD_FILE_END) {
    if (fsUploadFile)
      fsUploadFile.close();
    //    DBG_OUTPUT_PORT.print("handleFileUpload Size: ");
    //    DBG_OUTPUT_PORT.println(upload.totalSize);
  }
}

void handleList() {
  String path = "/";
  if (server.hasArg("dir")) {
    path = server.arg("dir");
  }

  Dir dir = SPIFFS.openDir(path);
  path = String();

  String output = "[";
  while (dir.next()) {
    File entry = dir.openFile("r");
    if (output != "[")
      output += ',';
    bool isDir = false;
    output += "{\"type\":\"";
    output += (isDir) ? "dir" : "file";
    output += "\",\"name\":\"";
    output += String(entry.name()).substring(1);
    output += "\"}";
    entry.close();
  }

  output += "]";
  server.send(200, "text/json", output);
}

void setup(void) {
  SPIFFS.begin();
  //  SPIFFS.format();
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
    oled.clearDisplay();
    flipDot.fillScreen(0);
    flipDot.display();
    server.send(200, "text/plain", "Display was cleared");
  });

  server.on("/invert", [flipDot, server]() {
    flipDot.invert();
    flipDot.display();
    server.send(200, "text/plain", "Display was inverted");
  });

  // first callback is called after the request has ended with all parsed
  // arguments
  // second callback handles file uploads at that location
  server.on("/upload", HTTP_POST, []() { server.send(200, "text/plain", "FileUploadhandler"); },
            handleFileUpload);

  server.on("/show", handleShow);
  server.on("/list", handleList);

  server.onNotFound(handleNotFound);

  server.begin();
}

void loop(void) { server.handleClient(); }
