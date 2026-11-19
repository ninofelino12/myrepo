#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Firebase_ESP_Client.h>
#include <ESP8266mDNS.h>
#include <ESP8266SSDP.h>
#include "config.h"

#include <time.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define SCREEN_ADDRESS 0x3C

// NodeMCU ESP8266 I2C pins: SDA = GPIO 4 (D2), SCL = GPIO 5 (D1)
#define SDA_PIN 14
#define SCL_PIN 12

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
bool oledReady = false;
ESP8266WebServer server(80);

class OledSerialClass : public Print {
  HardwareSerial &serial;
  Adafruit_SSD1306 &disp;
  String buf;
  unsigned long lastDisplayUpdate = 0;
  bool pendingUpdate = false;
public:
  OledSerialClass(HardwareSerial &s, Adafruit_SSD1306 &d) : serial(s), disp(d) {}
  void begin(unsigned long baud) { serial.begin(baud); }
  size_t write(uint8_t c) override {
    serial.write(c);
    if (!oledReady) return 1;
    if (c == '\r') return 1;
    buf += (char)c;
    // Keep buffer reasonable for SSD1306 (approx 8 lines of 21 chars)
    if (buf.length() > 200) buf = buf.substring(buf.length() - 200);
    
    // Only update display if newline or after 100ms to avoid blocking CPU
    if (c == '\n' || millis() - lastDisplayUpdate > 100) {
      updateDisplay();
    } else {
      pendingUpdate = true;
    }
    return 1;
  }

  void updateDisplay() {
    disp.clearDisplay();
    disp.setCursor(0, 0);
    disp.print(buf);
    disp.display();
    lastDisplayUpdate = millis();
    pendingUpdate = false;
  }

  void handle() {
    if (pendingUpdate && millis() - lastDisplayUpdate > 100) {
      updateDisplay();
    }
  }
};
OledSerialClass OledSerial(Serial, display);
#define Serial OledSerial

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig fbConfig;

bool firebaseInitialized = false;
bool firebaseBootSent = false;
unsigned long lastFirebaseAttempt = 0;
unsigned long lastRtdbUploadAttempt = 0;
const unsigned long RTDB_UPLOAD_INTERVAL = 300000; // 5 minutes
const unsigned long FIREBASE_RETRY_INTERVAL = 5000;

bool timeSyncStarted = false;
bool timeSynced = false;
unsigned long timeSyncStart = 0;

String fbLastLog = "idle";
unsigned long rtdbPushCount = 0;

const char* ssid = "Repeater-9924";
const char* password = "nino2222";

char hostName[32] = "esp8266";

void handleInfo() {
  String json = "{";
  json += "\"name\":\"" + String(hostName) + "\",";
  json += "\"hostname\":\"" + String(hostName) + ".local\",";
  json += "\"ip\":\"" + WiFi.localIP().toString() + "\",";
  json += "\"mac\":\"" + WiFi.macAddress() + "\",";
  json += "\"chipId\":" + String(ESP.getChipId()) + ",";
  json += "\"port\":80";
  json += "}";
  server.sendHeader("Connection", "close");
  server.send(200, "application/json", json);
}

void handleRoot() {
  server.sendHeader("Connection", "close");
  
  String html = "<!DOCTYPE html><html>";
  html += "<head><meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<title>ESP32 System Info</title>";
  html += "<style>";
  html += "body{font-family:Arial;margin:0;padding:20px;background:#f4f4f9;color:#333;text-align:center;}";
  html += "h2{color:#007bff;} h3{margin-top:0;color:#333;}";
  html += ".card{background:#fff;padding:20px;border-radius:8px;box-shadow:0 0 10px rgba(0,0,0,0.1);display:inline-block;max-width:420px;width:100%;text-align:left;margin-bottom:20px;}";
  html += "table{width:100%;border-collapse:collapse;}";
  html += "th,td{padding:8px;border-bottom:1px solid #ddd;font-size:14px;}";
  html += "th{text-align:left;color:#555;width:45%;}";
  html += "input[type=file]{margin:15px 0;}";
  html += "input[type=submit]{background:#007bff;color:#fff;border:none;padding:10px 20px;border-radius:4px;cursor:pointer;font-size:16px;width:100%;}";
  html += "input[type=submit]:hover{background:#0056b3;}";
  html += "</style></head>";
  html += "<body>";
  html += "<h2>ESP32 System Info</h2>";
  
  // System Info Card
  html += "<div class='card'>";
  html += "<h3>System Info</h3>";
  html += "<table>";
  html += "<tr><th>Chip Model</th><td>ESP8266 (ID " + String(ESP.getChipId()) + ")</td></tr>";
  html += "<tr><th>CPU Frequency</th><td>" + String(ESP.getCpuFreqMHz()) + " MHz</td></tr>";
  html += "<tr><th>MAC Address</th><td>" + String(WiFi.macAddress()) + "</td></tr>";
  html += "<tr><th>Free Heap</th><td>" + String(ESP.getFreeHeap() / 1024) + " KB</td></tr>";
  html += "<tr><th>Flash Size</th><td>" + String(ESP.getFlashChipSize() / (1024 * 1024)) + " MB</td></tr>";
  html += "<tr><th>Sketch Size</th><td>" + String(ESP.getSketchSize() / 1024) + " KB</td></tr>";
  html += "<tr><th>Uptime</th><td>" + String(millis() / 1000) + " s</td></tr>";
  html += "</table>";
  html += "</div><br>";


  html += "</body></html>";

  server.send(200, "text/html", html);
}

void setupOLED() {
  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(400000); // 400kHz I2C speed
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  oledReady = true;
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(F("ESP8266 NodeMCU"));
  display.println(F("System Info..."));
  display.display();
  delay(1000);
}

void setupMDNS() {
  if (!MDNS.begin(hostName)) {
    Serial.println("mDNS responder failed");
    return;
  }
  MDNS.addService("http", "tcp", 80);
  MDNS.addServiceTxt("http", "tcp", "path", "/");
  MDNS.addServiceTxt("http", "tcp", "id", String(ESP.getChipId()));
  Serial.println("mDNS responder started: " + String(hostName) + ".local");
}

void setupSSDP() {
  SSDP.setSchemaURL("description.xml");
  SSDP.setHTTPPort(80);
  SSDP.setName(String(hostName));
  SSDP.setSerialNumber(WiFi.macAddress());
  SSDP.setURL("/");
  SSDP.setModelName("ESP8266 NodeMCU");
  SSDP.setModelNumber(String(ESP.getChipId()));
  SSDP.setModelURL("http://" + String(hostName) + ".local");
  SSDP.setManufacturer("Espressif");
  SSDP.setManufacturerURL("https://www.espressif.com");
  SSDP.begin();
  Serial.println("SSDP started");
}

void setupWiFi() {
  sprintf(hostName, "esp8266-%06X", ESP.getChipId());
  WiFi.hostname(hostName);

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(F("Connecting to WiFi:"));
  display.println(ssid);
  display.display();

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    display.print(".");
    display.display();
    attempts++;
    if (attempts > 40) { // 20 seconds timeout, retry
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println(F("WiFi Failed!"));
      display.println(F("Retrying..."));
      display.display();
      WiFi.begin(ssid, password);
      attempts = 0;
    }
  }

  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(F("WiFi Connected!"));
  display.print(F("IP: "));
  display.println(WiFi.localIP());
  display.println(F("Web Ready..."));
  display.display();
  delay(2000);
}

void setupWebServer() {
  server.on("/", HTTP_GET, handleRoot);
  server.on("/info", HTTP_GET, handleInfo);
  server.on("/discover", HTTP_GET, handleInfo);
  server.on("/description.xml", HTTP_GET, []() {
    SSDP.schema(server.client());
  });

  server.begin();
  Serial.println("HTTP server started");
}

String sanitizeMac(const String& mac) {
  String s = mac;
  s.replace(":", "_");
  return s;
}

void syncTime() {
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  time_t now = 0;
  int tries = 0;
  while (now < 1000000000 && tries < 20) {
    delay(500);
    time(&now);
    tries++;
  }
  if (now >= 1000000000) {
    Serial.println("Time synced");
  } else {
    Serial.println("Time sync failed");
  }
}

String getIsoDate() {
  time_t now;
  time(&now);
  if (now < 1000000000) return String("unknown");
  struct tm t;
  gmtime_r(&now, &t);
  char buf[32];
  strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", &t);
  return String(buf);
}

void initFirebase() {
  if (WiFi.status() != WL_CONNECTED) return;

  Serial.println("Firebase init...");

  // Sanitize RTDB_URL: Firebase_ESP_Client expects database_url without protocol prefix (https://) or trailing slashes
  String url = RTDB_URL;
  url.replace("https://", "");
  url.replace("http://", "");
  if (url.endsWith("/")) url.remove(url.length() - 1);

  fbConfig.database_url = url.c_str();
  fbConfig.api_key = API_KEY;
  fbConfig.signer.test_mode = true;

  Firebase.reconnectNetwork(true);
  fbdo.setBSSLBufferSize(4096, 1024);

  Firebase.begin(&fbConfig, &auth);
  firebaseInitialized = true;
  lastFirebaseAttempt = millis();
  Serial.println("Firebase begin called; will retry in loop if not ready");
}

void sendBootDataToFirebase() {
  if (!Firebase.ready()) return;
  if (!timeSynced) return;

  String mac = WiFi.macAddress();
  FirebaseJson json;
  json.set("mac", mac);
  json.set("date", getIsoDate());
  json.set("gps", String(GPS_LAT) + "," + String(GPS_LON));
  json.set("bootTimeMs", (int)millis());
  json.set("ip", WiFi.localIP().toString());

  String path = "/bootEvents";
  bool pushOk = true;
  if (!firebaseBootSent) {
    pushOk = Firebase.RTDB.pushJSON(&fbdo, path.c_str(), &json);
    if (pushOk) {
      Serial.println("RTDB push OK: " + fbdo.dataPath());
      fbLastLog = "push OK";
      rtdbPushCount++;
    } else {
      String reason = fbdo.errorReason();
      String payload = fbdo.payload();
      Serial.printf("RTDB push FAIL (http=%d): %s | payload: %s\n", fbdo.httpCode(), reason.c_str(), payload.c_str());
      fbLastLog = "push FAIL: " + reason;
    }
  } else {
    fbLastLog = "update";
  }

  String devPath = "/devices/" + sanitizeMac(mac) + "/lastBoot";
  bool setOk = Firebase.RTDB.setJSON(&fbdo, devPath.c_str(), &json);
  if (setOk) {
    Serial.println("RTDB set OK: " + fbdo.dataPath());
    fbLastLog += " | set OK";
    if (firebaseBootSent) rtdbPushCount++;
  } else {
    String reason = fbdo.errorReason();
    String payload = fbdo.payload();
    Serial.printf("RTDB set FAIL (http=%d): %s | payload: %s\n", fbdo.httpCode(), reason.c_str(), payload.c_str());
    fbLastLog += " | set FAIL: " + reason;
  }

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(pushOk && setOk ? F("Firebase OK") : F("Firebase FAIL"));
  display.println(mac);
  display.println(fbLastLog);
  display.display();
  if (pushOk && setOk) firebaseBootSent = true;
}

void firebaseLoop() {
  if (WiFi.status() != WL_CONNECTED) return;

  if (!firebaseInitialized) {
    if (millis() - lastFirebaseAttempt >= FIREBASE_RETRY_INTERVAL) {
      lastFirebaseAttempt = millis();
      initFirebase();
    }
    return;
  }

  if (!Firebase.ready()) {
    if (millis() - lastFirebaseAttempt >= FIREBASE_RETRY_INTERVAL) {
      lastFirebaseAttempt = millis();
      Serial.println("Firebase not ready, re-initializing...");
      initFirebase();
    }
    return;
  }

  // Non-blocking time sync once
  if (!timeSynced) {
    if (!timeSyncStarted) {
      configTime(0, 0, "pool.ntp.org", "time.nist.gov");
      timeSyncStarted = true;
      timeSyncStart = millis();
    } else {
      time_t now;
      time(&now);
      if (now >= 1000000000) {
        timeSynced = true;
        Serial.println("Time synced");
      } else if (millis() - timeSyncStart > 8000) {
        timeSynced = true; // give up, continue with unknown date
        Serial.println("Time sync timeout, continuing");
      }
    }
    return;
  }

  // Upload to RTDB at most every 5 minutes (avoid hammering)
  if (lastRtdbUploadAttempt == 0 || millis() - lastRtdbUploadAttempt >= RTDB_UPLOAD_INTERVAL) {
    lastRtdbUploadAttempt = millis();
    sendBootDataToFirebase();
  }
}

void setup() {
  Serial.begin(115200);
  setupOLED();
  setupWiFi();
  setupWebServer();
  // Firebase init is handled in loop (non-blocking retry)
  lastFirebaseAttempt = millis() - FIREBASE_RETRY_INTERVAL; // allow immediate first attempt
}

unsigned long lastUpdate = 0;

void loop() {
  server.handleClient();
  firebaseLoop();
  OledSerial.handle();

  // Update status screen every second
  if (millis() - lastUpdate > 1000) {
    lastUpdate = millis();

    display.clearDisplay();
    display.setCursor(0, 0);
    display.println(F("ESP8266 NodeMCU"));
    display.println(F("-------------------"));
    display.print(F("WiFi: "));
    display.println(WiFi.status() == WL_CONNECTED ? F("OK") : F("FAIL"));
    display.print(F("IP: "));
    display.println(WiFi.localIP());
    display.print(F("FB: "));
    if (!firebaseInitialized) display.println(F("init..."));
    else display.println(Firebase.ready() ? F("OK") : F("retry"));
    display.print(F("Time: "));
    time_t now;
    time(&now);
    if (now >= 1000000000) {
      struct tm t;
      gmtime_r(&now, &t);
      char buf[12];
      strftime(buf, sizeof(buf), "%H:%M:%S", &t);
      display.println(buf);
    } else {
      display.println(F("syncing"));
    }
    display.print(F("Up: "));
    display.print(millis() / 1000);
    display.println(F("s"));
    display.print(F("Push: "));
    display.println(rtdbPushCount);
    display.print(F("Log: "));
    display.println(fbLastLog);
    display.display();
  }
}
