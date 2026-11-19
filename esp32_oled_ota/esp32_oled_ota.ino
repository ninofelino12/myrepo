#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Firebase_ESP_Client.h>
#include "config.h"

#include <time.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define SCREEN_ADDRESS 0x3C

// Wemos Lolin32 pins specified: SDA = GPIO 5, SCL = GPIO 4
#define SDA_PIN 5
#define SCL_PIN 4

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
bool oledReady = false;
WebServer server(80);

class OledSerialClass : public Print {
  HardwareSerial &serial;
  Adafruit_SSD1306 &disp;
  String buf;
public:
  OledSerialClass(HardwareSerial &s, Adafruit_SSD1306 &d) : serial(s), disp(d) {}
  void begin(unsigned long baud) { serial.begin(baud); }
  size_t write(uint8_t c) override {
    serial.write(c);
    if (!oledReady) return 1;
    if (c == '\r') return 1;
    buf += (char)c;
    if (buf.length() > 800) buf = buf.substring(buf.length() - 800);
    disp.clearDisplay();
    disp.setCursor(0, 0);
    disp.print(buf);
    disp.display();
    return 1;
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
const unsigned long FIREBASE_RETRY_INTERVAL = 5000;

bool timeSyncStarted = false;
bool timeSynced = false;
unsigned long timeSyncStart = 0;

String fbLastLog = "idle";

const char* ssid = "Repeater-9924";
const char* password = "nino2222";

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
  html += "<tr><th>Chip Model</th><td>" + String(ESP.getChipModel()) + " (Rev " + String(ESP.getChipRevision()) + ")</td></tr>";
  html += "<tr><th>CPU Frequency</th><td>" + String(ESP.getCpuFreqMHz()) + " MHz</td></tr>";
  html += "<tr><th>MAC Address</th><td>" + String(WiFi.macAddress()) + "</td></tr>";
  html += "<tr><th>Free Heap</th><td>" + String(ESP.getFreeHeap() / 1024) + " KB / " + String(ESP.getHeapSize() / 1024) + " KB</td></tr>";
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
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  oledReady = true;
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(F("ESP32 Lolin32"));
  display.println(F("System Info..."));
  display.display();
  delay(1000);
}

void setupWiFi() {
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
  if (firebaseBootSent) return;
  if (!timeSynced) return;

  String mac = WiFi.macAddress();
  FirebaseJson json;
  json.set("mac", mac);
  json.set("date", getIsoDate());
  json.set("gps", String(GPS_LAT) + "," + String(GPS_LON));
  json.set("bootTimeMs", (int)millis());
  json.set("ip", WiFi.localIP().toString());

  String path = "/bootEvents";
  bool pushOk = Firebase.RTDB.pushJSON(&fbdo, path.c_str(), &json);
  if (pushOk) {
    Serial.println("RTDB push OK: " + fbdo.dataPath());
    fbLastLog = "push OK";
  } else {
    String reason = fbdo.errorReason();
    String payload = fbdo.payload();
    Serial.printf("RTDB push FAIL (http=%d): %s | payload: %s\n", fbdo.httpCode(), reason.c_str(), payload.c_str());
    fbLastLog = "push FAIL: " + reason;
  }

  String devPath = "/devices/" + sanitizeMac(mac) + "/lastBoot";
  bool setOk = Firebase.RTDB.setJSON(&fbdo, devPath.c_str(), &json);
  if (setOk) {
    Serial.println("RTDB set OK: " + fbdo.dataPath());
    fbLastLog += " | set OK";
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

  sendBootDataToFirebase();
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

  // Update status screen every second
  if (millis() - lastUpdate > 1000) {
    lastUpdate = millis();

    display.clearDisplay();
    display.setCursor(0, 0);
    display.println(F("ESP32 Lolin32"));
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
    display.print(F("Log: "));
    display.println(fbLastLog);
    display.display();
  }
}
