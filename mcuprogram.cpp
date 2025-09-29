#include "esp_camera.h"
#include <WiFi.h>
#include <WebServer.h>
#include <FS.h>
#include <SD.h>
#include <SPI.h>

#define AP_SSID "SSID"
#define AP_PASS "PASSWORD"

WebServer server(80);

void handleCapture() {
  auto img = esp_camera_fb_get();
  if (!img) {
    server.send(500, "text/plain", "");
    return;
  }
  server.sendHeader("Content-Length", String(img->len));
  server.send(200, "image/jpeg");
  WiFiClient client = server.client();
  client.write(img->buf, img->len);
  esp_camera_fb_return(img);
}

void handleRoot() {
  String html = "<html><head><style>";
  html += "body{font-family:Inter,sans-serif;margin:20px;background:#0000FF;color:#7FC1FF;}";
  html += "h1{font-size:30px;}h2{display:block;margin-bottom:10px;color:#7FC1FF;}";
  html += "</style></head><body>";
  html += "<h1>Dash Camera</h1>";
  html += "<a href=\"/download/image1.jpg\"><img src=\"/downloadPhoto.png\" alt=\"Download Image 1\"></a>";
  html += "<a href=\"/download/video1.mp4\"><img src=\"/downloadVideo.png\" alt=\"Download Video 1\"></a>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

void handleDownload() {
  String filename = server.arg(0);
  if (filename == "/image1.jpg") {
    File file = SD.open("/image1.jpg", FILE_READ);
    if (file) {
      server.streamFile(file, "image/jpeg");
      file.close();
    } else {
      server.send(404, "text/plain", "File not found");
    }
  } else if (filename == "/video1.mp4") {
    File file = SD.open("/video1.mp4", FILE_READ);
    if (file) {
      server.streamFile(file, "video/mp4");
      file.close();
    } else {
      server.send(404, "text/plain", "File not found");
    }
  } else {
    server.send(404, "text/plain", "File not found");
  }
}

void setup() {
  auto res = esp_camera::Resolution::find(1024, 768);
  esp_camera::Config cfg;
  cfg.setPins(esp_camera::pins::AiThinker);
  cfg.setResolution(res);
  cfg.setJpeg(80);
  esp_camera::camera.begin(cfg);

  WiFi.softAP(AP_SSID, AP_PASS);

  if (!SD.begin()) {}

  server.on("/capture.jpg", handleCapture);
  server.on("/", handleRoot);
  server.on("/download", handleDownload);
  server.begin();
}

void loop() {
  server.handleClient();
}
