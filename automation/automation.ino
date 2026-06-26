#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <WiFiUdp.h>
#include <vector>
#include <ArduinoJson.h>
#include <EEPROM.h>
#include <string>
#include <Ticker.h>
#include <esp_task_wdt.h>
#include <TimeLib.h>
#include <LittleFS.h>
#include <WiFiClientSecure.h>
#define ENABLE_SMTP
#include <ReadyMail.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <time.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_SDA 21
#define OLED_SCL 22
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
bool oledBlinkState = false;
void updateOLED();

void handleRoot();
void handleFavicon();
void handleLogsPage();
void handleRelay1();
void handleRelay2();
void handleRelay3();
void handleTime();
void handleGetSchedules();
void handleAddSchedule();
void handleDeleteSchedule();
void handleUpdateSchedule();
void handleRelayStatus();
void handleClearError();
void handleGetErrorStatus();
void handleApiClearError();
void handleOneClickLight();
void handleTemperature();
void networkLoop(void*);
void mainLoop(void*);
void sendEmailWithLogs(const String&);
void checkoverride1();
void checkoverride2();
void overrideLEDState();
void checkSchedules();
void checkScheduleslaunch();
void activateRelay(int, bool);
void deactivateRelay(int, bool);
void broadcastRelayStates();
void handleGetTemporarySchedules();
void handleAddTemporarySchedule();
void handleDeleteTemporarySchedule();
void checkTemporarySchedules();
void handleTempCtrlPage();
void handleTempSchedulesPage();
void handleSchedulesPage();
void handleExternalTemperature();
void loadCalibrationSettings();
void saveCalibrationSettings();
void handleGetCalibrationSettings();
void handleSaveCalibrationSettings();
void tempTemperature();
void handleGetRawTemperatureData();
void handleDisplayCtrlPage();
void handleEmailConfigPage();
void handleGetEmailConfig();
void handleSaveEmailConfig();
void handleGetDisplaySchedule();
void handleSaveDisplaySchedule();
void loadDisplaySchedule();
void loadEmailConfig();
void saveEmailConfig();
void saveDisplaySchedule();
void applyOledSchedule();
void handleApiStatus();
void handleApiLogs();
void handleApiPing();
void handleDockerConfigPage();
void handleGetDockerConfig();
void handleSaveDockerConfig();
void loadDockerConfig();
void saveDockerConfig();

struct Schedule {
  int id;
  int relayNumber;
  int onHour;
  int onMinute;
  int offHour;
  int offMinute;
  bool enabled;
  bool daysOfWeek[7];
};

struct LogEntry {
  unsigned long id;
  String timestamp;
  String message;
};

struct TemporarySchedule {
  int id;
  int relayNumber;
  int onHour;
  int onMinute;
  int offHour;
  int offMinute;
  bool hasOnTime;
  bool hasOffTime;
  bool enabled;
  bool onFired;
  bool offFired;
};

struct CalibrationData {
  float internalOffset;
  float externalOffset;
};

struct DisplaySchedule {
  uint8_t magic;
  int onHour;
  int onMinute;
  int offHour;
  int offMinute;
  uint8_t overrideMode;
  bool enabled;
};

struct EmailConfig {
  uint8_t magic;
  bool enabled;
  char senderAccount[64];
  char senderPassword[64];
  char recipient[64];
};

struct DockerConfig {
  uint8_t magic;
  bool enabled;
};

const int relay1 = 18;
const int relay2 = 19;
const int relay3 = 23;
const int relay4 = 25;
const int switch1Pin = 33;
const int switch2Pin = 32;
const int errorLEDPin = 2;

bool overrideRelay1 = false;
bool overrideRelay2 = false;
bool relay1State = false;
bool relay2State = false;
bool relay3State = false;
bool relay4State = false;
const uint16_t ERR_WIFI = 1 << 0;
const uint16_t ERR_NTP = 1 << 1;
const uint16_t ERR_TEMP_INT = 1 << 2;
const uint16_t ERR_TEMP_EXT = 1 << 3;

uint16_t activeErrors = 0;
uint16_t acknowledgedErrors = 0;

const char* ssid = "Your_WiFi_SSID";
const char* password = "Your_WiFi_Password";
const char* authUsername = "admin";
const char* authPassword = "12345678";
std::vector<LogEntry> logBuffer;
bool spiffsInitialized = false;
WiFiUDP ntpUDP;
unsigned long lastTimeUpdate = 0;
const long timeUpdateInterval = 1000;
unsigned long lastNTPSync = 0;
unsigned long lastScheduleCheck = 0;
unsigned long lastSecond = 0;
bool validTimeSync = false;
unsigned long last90MinCheck = 0;
const unsigned long CHECK_90MIN_INTERVAL = 5400;
bool hasLaunchedSchedules = false;
bool startupemail = false;
bool pointemail = false;
unsigned long logIdCounter = 0;
SemaphoreHandle_t littleFsMutex = NULL;
volatile bool emailInProgress = false;
std::vector<Schedule> schedules;
std::vector<TemporarySchedule> temporarySchedules;
int tempScheduleIdCounter = 0;
const int EEPROM_SIZE = 1024;
const int SCHEDULE_SIZE = sizeof(Schedule);
const int MAX_SCHEDULES = 10;
const int SCHEDULE_START_ADDR = 0;
const int TOGGLE_DELAY = 500;
const int TOGGLE_COUNT = 3;
const std::vector<String> allowedIPs = {
  "192.168.29.3",   //Rpi
  "192.168.29.4",   //A Mac
  "192.168.29.5",   //A Ipad
  "192.168.29.6",   //A Phone
  "192.168.29.8",   //Acer
  "192.168.29.9",   //N Phone
  "192.168.29.10",  //F moto
  "192.168.29.11"   //S moto
};
unsigned long lastSwitch1Debounce = 0;
unsigned long lastSwitch2Debounce = 0;
const unsigned long DEBOUNCE_DELAY = 800;
unsigned long switch1PressStartTime = 0;
unsigned long switch2PressStartTime = 0;
bool switch1LastState = false;
bool switch2LastState = false;
const unsigned long HOLD_DURATION = 1500;
extern unsigned long switch1PressStartTime;
extern unsigned long switch2PressStartTime;
extern bool switch1LastState;
extern bool switch2LastState;
extern const unsigned long HOLD_DURATION;
unsigned long lastBlinkTime = 0;
const unsigned long BLINK_INTERVAL = 1000;
bool blinkState = false;

#define ONE_WIRE_BUS 26
#define EXTERNAL_ONE_WIRE_BUS 27
OneWire oneWire(ONE_WIRE_BUS);
OneWire externalOneWire(EXTERNAL_ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DallasTemperature externalSensors(&externalOneWire);
DeviceAddress sensorAddress = { 0x28, 0x59, 0x71, 0x80, 0xE3, 0xE1, 0x3C, 0x50 };
DeviceAddress externalSensorAddress = { 0x28, 0xCB, 0xBA, 0x57, 0x04, 0xE1, 0x3C, 0xE7 };
unsigned long lastTemp = 0;
unsigned long lastExternalTemp = 0;
float lastValidTemperature = 0;
float lastValidExternalTemperature = 0;
const int MAX_EXTERNAL_TEMP_FAILURES = 3;
int consecutiveExternalTempFailures = 0;

CalibrationData sensorCalibration = { 0.0, 0.0 };
const int CALIBRATION_START_ADDR = SCHEDULE_START_ADDR + (MAX_SCHEDULES * SCHEDULE_SIZE) + 1;
const int CALIBRATION_SIZE = sizeof(CalibrationData);

const int DISPLAY_SCHEDULE_ADDR = CALIBRATION_START_ADDR + CALIBRATION_SIZE + 1;
DisplaySchedule displaySchedule = { 0xDA, 8, 0, 22, 0, 0, true };

const int EMAIL_CONFIG_ADDR = DISPLAY_SCHEDULE_ADDR + sizeof(DisplaySchedule) + 1;
EmailConfig emailConfig = { 0xE2, false, "", "", "" };

const int DOCKER_CONFIG_ADDR = EMAIL_CONFIG_ADDR + sizeof(EmailConfig) + 1;
DockerConfig dockerConfig = { 0xD1, false };

bool oledPhysicalState = false;

WiFiEventId_t wifiConnectHandler;

#include "favicon.h"

#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT 465
const char* emailSubject = "Aquarium Control Logs";

WiFiClientSecure ssl_client;
SMTPClient smtp(ssl_client);

WebServer server(80);
WebServer apiServer(82);  // Collector API — handled on Core 0 (networkLoop)

WebSocketsServer webSocket = WebSocketsServer(81);

const int MAX_TEMP_FAILURES = 5;
int consecutiveTempFailures = 0;

const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 19800;  // 5 hours 30 minutes offset for IST
const int daylightOffset_sec = 0;  // 0 for no daylight saving time
unsigned long lastNtpRetry = 0;
const unsigned long NTP_RETRY_INTERVAL = 30000;

void handleGetLogs() {
  if (!spiffsInitialized) {
    server.send(500, "application/json", "{\"error\":\"LittleFS not initialized!\"}");
    return;
  }

  if (littleFsMutex != NULL) {
    if (xSemaphoreTake(littleFsMutex, pdMS_TO_TICKS(1000)) != pdTRUE) {
      server.send(503, "application/json", "{\"error\":\"Filesystem busy, try again\"}");
      return;
    }
  }

  StaticJsonDocument<2352> doc;
  doc.clear();

  File file = LittleFS.open("/logs.json", "r");
  if (!file) {
    if (littleFsMutex != NULL) xSemaphoreGive(littleFsMutex);
    server.send(404, "application/json", "{\"logs\":[]}");
    return;
  }

  DeserializationError error = deserializeJson(doc, file);
  file.close();

  if (littleFsMutex != NULL) xSemaphoreGive(littleFsMutex);

  if (error) {
    server.send(500, "application/json", "{\"error\":\"Failed to parse logs!\"}");
    return;
  }

  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

void storeLogEntry(const String& msg) {
  // Serial.println(msg);
  const int MAX_LOGS = 18;
  const int MAX_LOG_ID = 20;

  if (!spiffsInitialized) return;

  if (littleFsMutex != NULL) {
    if (xSemaphoreTake(littleFsMutex, pdMS_TO_TICKS(100)) != pdTRUE) {
      return;
    }
  }

  String timeStr;
  struct tm timeinfo;
  if (validTimeSync && getLocalTime(&timeinfo)) {
    char buffer[20];
    sprintf(buffer, "%02d/%02d/%d %02d:%02d:%02d",
            timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_year + 1900,
            timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    timeStr = String(buffer);
  } else {
    timeStr = "00/00/0000 00:00:00";
  }

  static StaticJsonDocument<2048> doc;
  doc.clear();

  File file = LittleFS.open("/logs.json", "r");
  bool fileExists = file;
  if (fileExists) {
    DeserializationError error = deserializeJson(doc, file);
    file.close();

    if (error) {
      doc.clear();
      doc.createNestedArray("logs");
    }
  } else {
    doc.createNestedArray("logs");
  }

  JsonArray logs = doc["logs"].as<JsonArray>();

  if (logs.size() >= MAX_LOGS) {
    logs.remove(0);
  }

  if (logIdCounter >= MAX_LOG_ID) {
    logIdCounter = 0;
  }

  JsonObject newLog = logs.createNestedObject();
  newLog["id"] = logIdCounter++;
  newLog["timestamp"] = timeStr;
  newLog["message"] = msg;

  File outFile = LittleFS.open("/logs.json", "w");
  if (outFile) {
    serializeJson(doc, outFile);
    outFile.close();
  }

  if (littleFsMutex != NULL) {
    xSemaphoreGive(littleFsMutex);
  }
}

void resetWatchdog() {
  esp_task_wdt_reset();
}

bool validDateSync = false;

TaskHandle_t networkTask;
TaskHandle_t controlTask;

void setup() {
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  pinMode(relay3, OUTPUT);
  pinMode(relay4, OUTPUT);
  digitalWrite(relay1, HIGH);
  digitalWrite(relay2, HIGH);
  digitalWrite(relay3, HIGH);
  digitalWrite(relay4, HIGH);
  pinMode(switch1Pin, INPUT_PULLUP);
  pinMode(switch2Pin, INPUT_PULLUP);
  pinMode(errorLEDPin, OUTPUT);
  digitalWrite(errorLEDPin, LOW);

  // Serial.begin(115200);
  // delay(2000);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  unsigned long wifiStartTime = millis();
  const unsigned long wifiTimeout = 20000;

  sensors.begin();
  externalSensors.begin();

  if (!LittleFS.begin(true)) {
    storeLogEntry("Failed to mount FS");
  } else {
    spiffsInitialized = true;
  }

  while (true) {
    if (WiFi.status() == WL_CONNECTED) {
      storeLogEntry("Connected to WiFi");
      //storeLogEntry("IP Address: " + WiFi.localIP().toString());
      attemptTimeSync();
      break;
    }

    if (millis() - wifiStartTime > wifiTimeout) {
      storeLogEntry("WiFi connection failed.");
      activeErrors |= ERR_WIFI;
      break;
    }

    delay(1000);
  }

  server.on("/", HTTP_GET, handleRoot);
  server.on("/favicon.png", HTTP_GET, handleFavicon);
  server.on("/logs", HTTP_GET, handleLogsPage);
  server.on("/logs/data", HTTP_GET, handleGetLogs);
  server.on("/tempcontrol", HTTP_GET, handleTempCtrlPage);
  server.on("/tempschedules", HTTP_GET, handleTempSchedulesPage);
  server.on("/mainSchedules", HTTP_GET, handleSchedulesPage);
  server.on("/relay/1", HTTP_ANY, handleRelay1);
  server.on("/relay/2", HTTP_ANY, handleRelay2);
  server.on("/relay/3", HTTP_ANY, handleRelay3);
  server.on("/time", HTTP_GET, handleTime);
  server.on("/schedules", HTTP_GET, handleGetSchedules);
  server.on("/schedule/add", HTTP_POST, handleAddSchedule);
  server.on("/schedule/delete", HTTP_DELETE, handleDeleteSchedule);
  server.on("/schedule/update", HTTP_POST, handleUpdateSchedule);
  server.on("/relay/status", HTTP_GET, handleRelayStatus);
  server.on("/error/clear", HTTP_POST, handleClearError);
  server.on("/error/status", HTTP_GET, handleGetErrorStatus);
  server.on("/relay/oneclick", HTTP_POST, handleOneClickLight);
  server.on("/temp-schedules", HTTP_GET, handleGetTemporarySchedules);
  server.on("/temp-schedule/add", HTTP_POST, handleAddTemporarySchedule);
  server.on("/temp-schedule/delete", HTTP_DELETE, handleDeleteTemporarySchedule);
  server.on("/calibration/settings", HTTP_GET, handleGetCalibrationSettings);
  server.on("/calibration/save", HTTP_POST, handleSaveCalibrationSettings);
  server.on("/temperature/raw", HTTP_GET, handleGetRawTemperatureData);
  server.on("/displayctrl", HTTP_GET, handleDisplayCtrlPage);
  server.on("/display/schedule", HTTP_GET, handleGetDisplaySchedule);
  server.on("/display/schedule/save", HTTP_POST, handleSaveDisplaySchedule);
  server.on("/emailConfig", HTTP_GET, handleEmailConfigPage);
  server.on("/api/emailConfig", HTTP_GET, handleGetEmailConfig);
  server.on("/api/emailConfig", HTTP_POST, handleSaveEmailConfig);
  server.on("/dockerConfig", HTTP_GET, handleDockerConfigPage);
  server.on("/api/dockerConfig", HTTP_GET, handleGetDockerConfig);
  server.on("/api/dockerConfig", HTTP_POST, handleSaveDockerConfig);
  server.begin();

  apiServer.on("/api/status", HTTP_GET, handleApiStatus);
  apiServer.on("/api/logs", HTTP_GET, handleApiLogs);
  apiServer.on("/api/ping", HTTP_GET, handleApiPing);
  apiServer.on("/api/errors/clear", HTTP_POST, handleApiClearError);
  apiServer.begin();
  EEPROM.begin(EEPROM_SIZE);
  loadSchedulesFromEEPROM();
  loadCalibrationSettings();
  loadDisplaySchedule();
  loadEmailConfig();
  loadDockerConfig();

  schedules.reserve(MAX_SCHEDULES);
  temporarySchedules.reserve(6);

  tempTemperature();

  Wire.begin(OLED_SDA, OLED_SCL);
  Wire.setClock(50000);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    storeLogEntry("OLED init failed");
  } else {
    Wire.setClock(50000);
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    updateOLED();
  }

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  const esp_task_wdt_config_t wdt_config = {
    .timeout_ms = 15000,  // 15 second timeout
    .idle_core_mask = (1 << portNUM_PROCESSORS) - 1,
    .trigger_panic = true
  };
  esp_err_t wdt_err = esp_task_wdt_reconfigure(&wdt_config);
  if (wdt_err != ESP_OK) {
    esp_task_wdt_deinit();
    esp_task_wdt_init(&wdt_config);
  }
  littleFsMutex = xSemaphoreCreateMutex();

  xTaskCreatePinnedToCore(
    networkLoop,
    "networkTask",
    12288,
    NULL,
    1,
    &networkTask,
    0);

  xTaskCreatePinnedToCore(
    mainLoop,
    "mainTask",
    12288,
    NULL,
    1,
    &controlTask,
    1);
}

void attemptTimeSync() {
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  struct tm timeinfo;
  bool synced = getLocalTime(&timeinfo, 10000);

  if (synced) {
    if (!validTimeSync || (activeErrors & ERR_NTP) || (acknowledgedErrors & ERR_NTP)) {
      storeLogEntry("Time and Date sync successful");
    }
    validTimeSync = true;
    validDateSync = true;
    lastNTPSync = millis();
    activeErrors &= ~ERR_NTP;
    acknowledgedErrors &= ~ERR_NTP;

    setTime(timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec,
            timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_year + 1900);
  } else {
    if (!(activeErrors & ERR_NTP) && !(acknowledgedErrors & ERR_NTP)) {
      storeLogEntry("Time sync failed.");
      activeErrors |= ERR_NTP;
    }
  }
}


void saveSchedulesToEEPROM() {
  int addr = SCHEDULE_START_ADDR;
  EEPROM.write(addr, schedules.size());
  addr++;

  for (const Schedule& schedule : schedules) {
    EEPROM.put(addr, schedule);
    addr += SCHEDULE_SIZE;
  }
  EEPROM.commit();
}

void loadSchedulesFromEEPROM() {
  schedules.clear();
  int addr = SCHEDULE_START_ADDR;
  int count = EEPROM.read(addr);
  addr++;

  for (int i = 0; i < count && i < MAX_SCHEDULES; i++) {
    Schedule schedule;
    EEPROM.get(addr, schedule);
    schedules.push_back(schedule);
    addr += SCHEDULE_SIZE;
  }
}

void loadCalibrationSettings() {
  CalibrationData storedData;
  EEPROM.get(CALIBRATION_START_ADDR, storedData);

  if (storedData.internalOffset >= -10.0 && storedData.internalOffset <= 10.0 && storedData.externalOffset >= -10.0 && storedData.externalOffset <= 10.0) {
    sensorCalibration = storedData;
    //storeLogEntry("Sensor calibration loaded from EEPROM");
  } else {
    sensorCalibration.internalOffset = 0.0;
    sensorCalibration.externalOffset = 0.0;
    storeLogEntry("Using default sensor calibration settings");
    saveCalibrationSettings();
  }
}

void saveCalibrationSettings() {
  EEPROM.put(CALIBRATION_START_ADDR, sensorCalibration);
  EEPROM.commit();
  storeLogEntry("Sensor calibration settings saved to EEPROM");
}

void handleGetCalibrationSettings() {
  String json = "{";
  json += "\"internalOffset\":" + String(sensorCalibration.internalOffset, 2) + ",";
  json += "\"externalOffset\":" + String(sensorCalibration.externalOffset, 2);
  json += "}";
  server.send(200, "application/json", json);
}

void handleSaveCalibrationSettings() {
  if (server.hasArg("plain")) {
    String body = server.arg("plain");
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, body);

    if (!error) {
      float internalOffset = doc["internalOffset"];
      float externalOffset = doc["externalOffset"];

      if (internalOffset >= -10.0 && internalOffset <= 10.0 && externalOffset >= -10.0 && externalOffset <= 10.0) {

        sensorCalibration.internalOffset = internalOffset;
        sensorCalibration.externalOffset = externalOffset;

        saveCalibrationSettings();

        server.send(200, "application/json", "{\"status\":\"success\"}");
        storeLogEntry("Sensor calibration updated: Internal=" + String(internalOffset, 2) + "°C, External=" + String(externalOffset, 2) + "°C");
        return;
      } else {
        server.send(400, "application/json", "{\"error\":\"Calibration offsets must be between -10°C and +10°C\"}");
        return;
      }
    }
  }
  server.send(400, "application/json", "{\"error\":\"Invalid request\"}");
}

void loadDisplaySchedule() {
  DisplaySchedule stored;
  EEPROM.get(DISPLAY_SCHEDULE_ADDR, stored);
  if (stored.magic == 0xDA && stored.onHour >= 0 && stored.onHour <= 23 && stored.onMinute >= 0 && stored.onMinute <= 59 && stored.offHour >= 0 && stored.offHour <= 23 && stored.offMinute >= 0 && stored.offMinute <= 59 && stored.overrideMode <= 2) {
    displaySchedule = stored;
    //storeLogEntry("Display schedule loaded from EEPROM");
  } else {
    saveDisplaySchedule();
    storeLogEntry("Using default display schedule");
  }
}

void saveDisplaySchedule() {
  displaySchedule.magic = 0xDA;
  EEPROM.put(DISPLAY_SCHEDULE_ADDR, displaySchedule);
  EEPROM.commit();
  storeLogEntry("Display schedule saved to EEPROM");
}

void loadEmailConfig() {
  EmailConfig stored;
  EEPROM.get(EMAIL_CONFIG_ADDR, stored);
  if (stored.magic == 0xE2) {
    emailConfig = stored;
    // storeLogEntry("Email config loaded from EEPROM");
  } else {
    saveEmailConfig();
    storeLogEntry("Using default email config");
  }
}

void saveEmailConfig() {
  emailConfig.magic = 0xE2;
  EEPROM.put(EMAIL_CONFIG_ADDR, emailConfig);
  EEPROM.commit();
  storeLogEntry("Email config saved to EEPROM");
}

void loadDockerConfig() {
  DockerConfig stored;
  EEPROM.get(DOCKER_CONFIG_ADDR, stored);
  if (stored.magic == 0xD1) {
    dockerConfig = stored;
    // storeLogEntry("Docker config loaded from EEPROM");
  } else {
    saveDockerConfig();
    storeLogEntry("Using default docker config");
  }
}

void saveDockerConfig() {
  dockerConfig.magic = 0xD1;
  EEPROM.put(DOCKER_CONFIG_ADDR, dockerConfig);
  EEPROM.commit();
  storeLogEntry("Docker config saved to EEPROM");
}

void applyOledSchedule() {
  if (!validTimeSync) {
    if (oledPhysicalState) {
      oledPhysicalState = false;
      updateOLED();
    }
    return;
  }

  bool newState = false;

  switch (displaySchedule.overrideMode) {
    case 1:
      newState = true;
      break;
    case 2:
      newState = false;
      break;
    default:
      if (!displaySchedule.enabled) {
        newState = true;
      } else {
        struct tm timeinfo;
        if (!getLocalTime(&timeinfo)) { return; }
        int nowMins = timeinfo.tm_hour * 60 + timeinfo.tm_min;
        int onMins = displaySchedule.onHour * 60 + displaySchedule.onMinute;
        int offMins = displaySchedule.offHour * 60 + displaySchedule.offMinute;
        if (onMins <= offMins) {
          newState = (nowMins >= onMins && nowMins < offMins);
        } else {
          newState = (nowMins >= onMins || nowMins < offMins);
        }
      }
      break;
  }

  if (newState != oledPhysicalState) {
    oledPhysicalState = newState;
    updateOLED();
  }
}

extern const char displayCtrlPage[] PROGMEM;
extern const char emailConfigPage[] PROGMEM;
extern const char dockerConfigPage[] PROGMEM;

void handleDisplayCtrlPage() {
  if (!checkAuthentication()) return;
  server.send_P(200, "text/html", displayCtrlPage);
}

void handleEmailConfigPage() {
  if (!checkAuthentication()) return;
  server.send_P(200, "text/html", emailConfigPage);
}

void handleGetEmailConfig() {
  DynamicJsonDocument doc(512);
  doc["enabled"] = emailConfig.enabled;
  doc["senderAccount"] = emailConfig.senderAccount;
  doc["senderPassword"] = emailConfig.senderPassword;
  doc["recipient"] = emailConfig.recipient;

  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

void handleSaveEmailConfig() {
  if (server.hasArg("plain") == false) {
    server.send(400, "application/json", "{\"error\":\"Body not received\"}");
    return;
  }

  DynamicJsonDocument doc(512);
  DeserializationError error = deserializeJson(doc, server.arg("plain"));

  if (error) {
    server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
    return;
  }

  emailConfig.enabled = doc["enabled"] | true;
  strlcpy(emailConfig.senderAccount, doc["senderAccount"] | "", sizeof(emailConfig.senderAccount));
  strlcpy(emailConfig.senderPassword, doc["senderPassword"] | "", sizeof(emailConfig.senderPassword));
  strlcpy(emailConfig.recipient, doc["recipient"] | "", sizeof(emailConfig.recipient));

  saveEmailConfig();

  server.send(200, "application/json", "{\"success\":true}");
}

void handleDockerConfigPage() {
  if (!checkAuthentication()) return;
  server.send_P(200, "text/html", dockerConfigPage);
}

void handleGetDockerConfig() {
  DynamicJsonDocument doc(256);
  doc["enabled"] = dockerConfig.enabled;

  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

void handleSaveDockerConfig() {
  if (server.hasArg("plain") == false) {
    server.send(400, "application/json", "{\"error\":\"Body not received\"}");
    return;
  }

  DynamicJsonDocument doc(256);
  DeserializationError error = deserializeJson(doc, server.arg("plain"));

  if (error) {
    server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
    return;
  }

  dockerConfig.enabled = doc["enabled"] | true;
  saveDockerConfig();

  server.send(200, "application/json", "{\"success\":true}");
}

void handleGetDisplaySchedule() {
  if (!checkAuthentication()) return;
  String json = "{";
  json += "\"onHour\":" + String(displaySchedule.onHour) + ",";
  json += "\"onMinute\":" + String(displaySchedule.onMinute) + ",";
  json += "\"offHour\":" + String(displaySchedule.offHour) + ",";
  json += "\"offMinute\":" + String(displaySchedule.offMinute) + ",";
  json += "\"overrideMode\":" + String(displaySchedule.overrideMode) + ",";
  json += "\"enabled\":" + String(displaySchedule.enabled ? "true" : "false") + ",";
  json += "\"displayOn\":" + String(oledPhysicalState ? "true" : "false");
  json += "}";
  server.send(200, "application/json", json);
}

void handleSaveDisplaySchedule() {
  if (!checkAuthentication()) return;
  if (!server.hasArg("plain")) {
    server.send(400, "application/json", "{\"error\":\"No body\"}");
    return;
  }
  String body = server.arg("plain");
  StaticJsonDocument<256> doc;
  if (deserializeJson(doc, body)) {
    server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
    return;
  }

  if (doc.containsKey("overrideMode")) {
    int om = doc["overrideMode"].as<int>();
    if (om < 0 || om > 2) {
      server.send(400, "application/json", "{\"error\":\"overrideMode must be 0, 1 or 2\"}");
      return;
    }
    displaySchedule.overrideMode = (uint8_t)om;
    if (om == 0) displaySchedule.enabled = true;
    saveDisplaySchedule();
    applyOledSchedule();
    server.send(200, "application/json", "{\"status\":\"success\"}");
    storeLogEntry("Display override mode set to " + String(om));
    return;
  }

  int onH = doc["onHour"] | displaySchedule.onHour;
  int onM = doc["onMinute"] | displaySchedule.onMinute;
  int offH = doc["offHour"] | displaySchedule.offHour;
  int offM = doc["offMinute"] | displaySchedule.offMinute;
  bool en = doc.containsKey("enabled") ? doc["enabled"].as<bool>() : displaySchedule.enabled;

  if (onH < 0 || onH > 23 || onM < 0 || onM > 59 || offH < 0 || offH > 23 || offM < 0 || offM > 59) {
    server.send(400, "application/json", "{\"error\":\"Time values out of range\"}");
    return;
  }

  displaySchedule.onHour = onH;
  displaySchedule.onMinute = onM;
  displaySchedule.offHour = offH;
  displaySchedule.offMinute = offM;
  displaySchedule.enabled = en;
  saveDisplaySchedule();
  applyOledSchedule();
  server.send(200, "application/json", "{\"status\":\"success\"}");
  storeLogEntry("Display schedule updated: ON=" + String(onH) + ":" + String(onM) + " OFF=" + String(offH) + ":" + String(offM) + " enabled=" + String(en));
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
      if (length > 0) {
        //storeLogEntry("WebSocket " + String(num) + " Disconnected");
      }
      break;
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(num);
        //storeLogEntry("WebSocket " + String(num) + " Connected from " + ip.toString());

        String message = "{\"relay1\":" + String(relay1State || overrideRelay1) + ",\"relay2\":" + String(relay2State || overrideRelay2) + ",\"relay3\":" + String(relay3State || overrideRelay1) + ",\"temperature\":" + String(lastValidTemperature, 1) + ",\"relay1Name\":\"WaveMaker\"" + ",\"relay2Name\":\"Light\"" + ",\"relay3Name\":\"Air Pump\"}";
        webSocket.sendTXT(num, message);
      }
      break;
    case WStype_TEXT:
      break;
    case WStype_ERROR:
      storeLogEntry("WebSocket " + String(num) + " Error");
      break;
    default:
      break;
  }
}

bool checkAuthentication() {
  String clientIP = server.client().remoteIP().toString();
  for (const auto& ip : allowedIPs) {
    if (clientIP == ip) {
      return true;
    }
  }
  if (!server.authenticate(authUsername, authPassword)) {
    server.requestAuthentication();
    return false;
  }
  return true;
}

void handleFavicon() {
  server.sendHeader("Cache-Control", "max-age=31536000");
  server.send_P(200, "image/png", (const char*)favicon_png, favicon_png_len);
}

const char mainPage[] PROGMEM = R"html(
<!DOCTYPE html>
<html>
<head>
    <link rel="icon" type="image/png" href="/favicon.png">
    <link rel="shortcut icon" type="image/png" href="/favicon.png">
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Aquarium Control</title>
    <style>
        :root {
            --primary-color: #1976D2;
            --primary-dark: #0D47A1;
            --primary-light: #BBDEFB;
            --accent-color: #03A9F4;
            --success-color: #4CAF50;
            --warning-color: #FFC107;
            --lightbtn-color: #94730eff;
            --error-color: #F44336;
            --text-color: #333;
            --text-light: #757575;
            --background-color: #f5f7fa;
            --card-color: #ffffff;
            --border-radius: 8px;
            --shadow: 0 2px 10px rgba(0,0,0,0.1);
            --transition: all 0.3s ease;
        }

        * {
            box-sizing: border-box;
            margin: 0;
            padding: 0;
        }

        body {
            margin: 0;
            padding: 0;
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background-color: var(--background-color);
            color: var(--text-color);
            line-height: 1.6;
        }

        header {
            background: linear-gradient(135deg, var(--primary-color), var(--primary-dark));
            color: white;
            padding: 20px;
            text-align: center;
            box-shadow: 0 2px 10px rgba(0,0,0,0.1);
            position: relative;
            z-index: 10;
        }

        header h1 {
            margin: 0;
            font-size: 2rem;
            letter-spacing: 0.5px;
        }

        .container {
            padding: 20px;
            max-width: 1000px;
            margin: auto;
        }

        .time-container {
            margin: 20px 0;
            padding: 20px;
            background-color: var(--card-color);
            border-radius: var(--border-radius);
            box-shadow: var(--shadow);
            text-align: center;
        }

        #time {
            font-size: 2.5rem;
            font-weight: bold;
            color: var(--primary-color);
            margin: 10px 0;
            transition: var(--transition);
        }

        #day {
            font-size: 1.5rem;
            color: var(--text-light);
            margin: 5px 0;
        }

        #date {
            font-size: 1.5rem;
            color: var(--text-light);
            margin: 5px 0;
        }

        .temperature-container {
            margin: 20px 0;
            padding: 20px;
            background-color: var(--card-color);
            border-radius: var(--border-radius);
            box-shadow: var(--shadow);
            transition: var(--transition);
        }

        .temperature-container:hover {
            box-shadow: 0 5px 15px rgba(0,0,0,0.15);
        }

        .temperature-grid {
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 20px;
            align-items: center;
        }

        .temperature-item {
            text-align: center;
            padding: 15px;
            background-color: #f8f9fa;
            border-radius: var(--border-radius);
            transition: var(--transition);
        }

        .temperature-item:hover {
            background-color: #e9ecef;
        }

        .temperature-label {
            font-size: 1rem;
            color: var(--text-light);
            margin-bottom: 8px;
            font-weight: 500;
        }

        .temperature-value {
            font-size: 1.8rem;
            font-weight: bold;
            color: var(--primary-color);
        }

        .temperature-item.internal {
            border-left: 4px solid var(--success-color);
        }

        .temperature-item.external {
            border-left: 4px solid var(--accent-color);
        }

        .control-section {
            background-color: var(--card-color);
            padding: 25px;
            border-radius: var(--border-radius);
            box-shadow: var(--shadow);
            margin-bottom: 25px;
            transition: var(--transition);
        }

        .control-section:hover {
            box-shadow: 0 5px 15px rgba(0,0,0,0.15);
        }

        .control-section h3 {
            color: var(--primary-color);
            margin-bottom: 20px;
            font-size: 1.5rem;
            border-bottom: 2px solid var(--primary-light);
            padding-bottom: 10px;
            text-align: center;
        }

        .relay-buttons {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(180px, 1fr));
            gap: 15px;
            margin-bottom: 10px;
        }

        .navigation-buttons {
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 15px;
            margin-bottom: 10px;
        }

        .navigation-buttons .nav-full {
            grid-column: 1 / -1;
        }

        .button {
            padding: 15px;
            border: none;
            border-radius: var(--border-radius);
            font-size: 1.1rem;
            font-weight: 500;
            cursor: pointer;
            transition: var(--transition);
            text-align: center;
            box-shadow: var(--shadow);
            background-color: var(--primary-color);
            color: white;
        }

        .button:hover {
            transform: translateY(-2px);
            box-shadow: 0 4px 15px rgba(0,0,0,0.2);
        }

        .button:active {
            transform: translateY(1px);
        }

        .button.on {
            background-color: var(--success-color);
        }

        .button.off {
            background-color: var(--error-color);
        }

        .button.override {
            background-color: var(--warning-color);
            color: #333;
        }

        .button.override:hover {
            background-color: #e6ac00;
            color: #333;
        }

        .nav-button {
            background-color: var(--primary-color);
        }

        .nav-button:hover {
            background-color: var(--primary-dark);
        }

        .special-button {
            background-color: var(--lightbtn-color);
            color: #333;
        }

        .special-button:hover {
            background-color: #64532bff;
        }

        #errorSection {
            text-align: center;
            margin: 20px 0;
            color: white;
            background-color: var(--error-color);
            padding: 20px;
            border-radius: var(--border-radius);
            display: none;
            animation: pulse 2s infinite;
            box-shadow: 0 4px 10px rgba(244, 67, 54, 0.3);
        }

        @keyframes pulse {
            0% { box-shadow: 0 0 0 0 rgba(244, 67, 54, 0.4); }
            70% { box-shadow: 0 0 0 10px rgba(244, 67, 54, 0); }
            100% { box-shadow: 0 0 0 0 rgba(244, 67, 54, 0); }
        }

        #clearErrorBtn {
            padding: 12px 24px;
            background-color: white;
            color: var(--error-color);
            border: none;
            border-radius: 4px;
            cursor: pointer;
            font-size: 1rem;
            font-weight: 500;
            margin-top: 15px;
            transition: var(--transition);
        }

        #clearErrorBtn:hover {
            background-color: #f5f5f5;
            transform: scale(1.05);
        }

        @media (max-width: 768px) {
            .relay-buttons {
                grid-template-columns: 1fr;
            }
            
            .navigation-buttons {
                grid-template-columns: 1fr;
            }

            .navigation-buttons .nav-full {
                grid-column: 1;
            }

            .temperature-grid {
                grid-template-columns: 1fr;
                gap: 15px;
            }

            .temperature-value {
                font-size: 1.5rem;
            }
            
            #time {
                font-size: 2rem;
            }
            
            #day, #date {
                font-size: 1.2rem;
            }
            
            .container {
                padding: 10px;
            }
            
            .control-section {
                padding: 15px;
                margin-bottom: 15px;
            }
            
            .control-section h3 {
                font-size: 1.3rem;
            }
        }


        #errorSection h3 {
            color: #fff;
            border-bottom: 2px solid rgba(255,255,255,0.3);
            margin-bottom: 15px;
            padding-bottom: 10px;
        }
        .error-row {
            display: flex;
            justify-content: space-between;
            align-items: center;
            background: rgba(0,0,0,0.15);
            padding: 10px 15px;
            border-radius: 8px;
            margin-bottom: 10px;
            font-size: 1.1rem;
        }
        .dismiss-btn {
            background-color: #fff;
            color: var(--error-color);
            padding: 5px 15px;
            font-size: 0.9rem;
            margin: 0;
        }
        .dismiss-btn:hover {
            background-color: #f1f1f1;
            transform: scale(1.05);
        }
        .dismiss-all {
            background-color: transparent;
            color: #fff;
            border: 2px solid #fff;
            width: 100%;
        }
        .dismiss-all:hover {
            background-color: #fff;
            color: var(--error-color);
        }

        /* Toast */
        #toast {
            position: fixed;
            bottom: 28px;
            left: 50%;
            transform: translateX(-50%) translateY(80px);
            background: #323232;
            color: white;
            padding: 12px 28px;
            border-radius: 24px;
            font-size: 0.95rem;
            opacity: 0;
            transition: all 0.35s ease;
            z-index: 1000;
            pointer-events: none;
        }
        #toast.show { opacity: 1; transform: translateX(-50%) translateY(0); }
        #toast.success { background: var(--success-color); }
        #toast.error   { background: var(--error-color); }
    </style>
</head>
<body>
    <header>
        <h1>Aquarium Control Panel</h1>
    </header>
    <div class="container">
        <div class="time-container">
            <div id="time">Loading time...</div>
            <div id="day">Loading day...</div>
            <div id="date">Loading date...</div>
        </div>
        
        <div class="temperature-container">
            <div class="temperature-grid">
                <div class="temperature-item internal">
                    <div class="temperature-label">Internal Temperature</div>
                    <div class="temperature-value" id="temperature">-- °C</div>
                </div>
                <div class="temperature-item external">
                    <div class="temperature-label">External Temperature</div>
                    <div class="temperature-value" id="externalTemperature">-- °C</div>
                </div>
            </div>
        </div>

        <div class="control-section">
            <h3>Relay Controls</h3>
            <div class="relay-buttons">
                <button class="button" onclick="toggleRelay(1)" id="btn1">WaveMaker</button>
                <button class="button" onclick="toggleRelay(3)" id="btn3">Air Pump</button>
                <button class="button" onclick="toggleRelay(2)" id="btn2">Light</button>
                <button class="button special-button" onclick="oneClickLight()" id="btnOneClick">Change Light Color</button>
            </div>
        </div>

        <div id="errorSection" style="display: none;" class="control-section"></div>
        
        <div class="control-section">
            <h3>System Navigation</h3>
            <div class="navigation-buttons">
                <button class="button nav-button" onclick="showTempSchedules()">Temporary Schedules</button>
                <button class="button nav-button" onclick="showSchedules()">Main Schedules</button>
                <button class="button nav-button" onclick="showTempControl()">Sensor Calibration</button>
                <button class="button nav-button" onclick="showDisplayCtrl()">Display Control</button>
                <button class="button nav-button" onclick="showEmailConfig()">Email Settings</button>
                <button class="button nav-button" onclick="showDockerConfig()">Docker Settings</button>
                <button class="button nav-button nav-full" onclick="showLogs()">System Logs</button>
            </div>
        </div>
    </div>
    <div id="toast"></div>
    <script>
        function showToast(msg, type) {
            const t = document.getElementById('toast');
            t.textContent = msg;
            t.className = 'show ' + (type || '');
            setTimeout(() => { t.className = ''; }, 3000);
        }

        let relayStates = {
            1: false,
            2: false,
            3: false
        };

        let overrideStates = {
            1: false,
            2: false,
            3: false
        };

        let relayNames = {
            1: "WaveMaker",
            2: "Light",
            3: "Air Pump"
        };

        let lastReceivedTemp = '--';
        let lastReceivedExtTemp = '--';
        let currentActiveErrors = 0;
        let currentAckErrors = 0;

        function updateTemperatureDisplay() {
            let tempEl = document.getElementById('temperature');
            let extTempEl = document.getElementById('externalTemperature');
            
            if (tempEl) {
                if ((currentActiveErrors & 4) || (currentAckErrors & 4)) { // ERR_TEMP_INT
                    tempEl.textContent = '-- °C';
                    tempEl.parentElement.style.backgroundColor = 'rgba(244, 67, 54, 0.15)';
                    tempEl.parentElement.style.color = 'var(--error-color)';
                } else {
                    tempEl.textContent = lastReceivedTemp + ' °C';
                    tempEl.parentElement.style.backgroundColor = '';
                    tempEl.parentElement.style.color = '';
                }
            }
            if (extTempEl) {
                if ((currentActiveErrors & 8) || (currentAckErrors & 8)) { // ERR_TEMP_EXT
                    extTempEl.textContent = '-- °C';
                    extTempEl.parentElement.style.backgroundColor = 'rgba(244, 67, 54, 0.15)';
                    extTempEl.parentElement.style.color = 'var(--error-color)';
                } else {
                    extTempEl.textContent = lastReceivedExtTemp + ' °C';
                    extTempEl.parentElement.style.backgroundColor = '';
                    extTempEl.parentElement.style.color = '';
                }
            }
        }

        let socket = null;
        let reconnectAttempts = 0;
        const maxReconnectAttempts = 5;
        let reconnectInterval = 1000; // Start with 1 second
        const maxReconnectInterval = 30000; // Max 30 seconds

        function connectWebSocket() {
            if (socket && (socket.readyState === WebSocket.CONNECTING || socket.readyState === WebSocket.OPEN)) {
                return; // Already connected or connecting
            }
            
            console.log('Attempting WebSocket connection...');
            socket = new WebSocket('ws://' + window.location.hostname + ':81/');
            
            socket.onopen = () => {
                console.log('WebSocket connected');
                reconnectAttempts = 0;
                reconnectInterval = 1000; // Reset interval
                // Request initial data
                getInitialStates();
            };
            
            socket.onmessage = (event) => {
                try {
                    let data = JSON.parse(event.data);
                    
                    if (data.relay1Name) relayNames[1] = data.relay1Name;
                    if (data.relay2Name) relayNames[2] = data.relay2Name;
                    if (data.relay3Name) relayNames[3] = data.relay3Name;
                    
                    if (data.override1 !== undefined) overrideStates[1] = data.override1;
                    if (data.override2 !== undefined) overrideStates[2] = data.override2;
                    // Relay 3 shares override1 (physical switch 1)
                    if (data.override1 !== undefined) overrideStates[3] = data.override1;

                    if (data.relay1 !== undefined) {
                        relayStates[1] = data.relay1;
                        updateButtonStyle(1);
                    }
                    if (data.relay2 !== undefined) {
                        relayStates[2] = data.relay2;
                        updateButtonStyle(2);
                    }
                    if (data.relay3 !== undefined) {
                        relayStates[3] = data.relay3;
                        updateButtonStyle(3);
                    }
                    if (data.temperature !== undefined) {
                        lastReceivedTemp = data.temperature;
                    }
                    if (data.externalTemperature !== undefined) {
                        lastReceivedExtTemp = data.externalTemperature;
                    }
                    updateTemperatureDisplay();
                } catch (e) {
                    console.error('WebSocket message parsing error:', e);
                }
            };
            
            socket.onclose = (event) => {
                console.log('WebSocket disconnected:', event.code, event.reason);
                socket = null;
                scheduleReconnect();
                checkErrorStatus();
            };
            
            socket.onerror = (error) => {
                console.error('WebSocket error:', error);
                checkErrorStatus();
            };
        }

        function scheduleReconnect() {
        if (reconnectAttempts >= maxReconnectAttempts) {
            console.log('Max reconnection attempts reached');
            return;
        }
        
        reconnectAttempts++;
        console.log(`Scheduling reconnect attempt ${reconnectAttempts} in ${reconnectInterval}ms`);
        
        setTimeout(() => {
            connectWebSocket();
        }, reconnectInterval);
        
        // Exponential backoff
        reconnectInterval = Math.min(reconnectInterval * 1.5, maxReconnectInterval);
    }

    // Initialize connection
    connectWebSocket();

    // Fallback: try to reconnect every 30 seconds if disconnected
    setInterval(() => {
        if (!socket || socket.readyState === WebSocket.CLOSED) {
            console.log('WebSocket check: attempting reconnection');
            reconnectAttempts = 0; // Reset attempts for periodic check
            connectWebSocket();
        }
    }, 30000);

        function updateTime() {
            fetch('/time')
                .then(response => response.text())
                .then(data => {
                    const [time, day, date] = data.split(' ');
                    document.getElementById('time').textContent = time;
                    document.getElementById('day').textContent = day;
                    document.getElementById('date').textContent = date;
                })
                .catch(() => {
                    document.getElementById('time').textContent = "Time unavailable";
                    document.getElementById('day').textContent = "Day unavailable";
                    document.getElementById('date').textContent = "Date unavailable";
                });
        }

        function toggleRelay(relay) {
            fetch('/relay/' + relay, { method: 'POST', headers: { 'Content-Type': 'application/json' } })
                .then(response => response.ok ? response.json() : response.json().then(data => { throw new Error(data.error); }))
                .then(data => {
                    relayStates[relay] = data.state;
                    updateButtonStyle(relay);
                    let stateText = data.state ? "turned ON" : "turned OFF";
                    let relayLabel = relayNames[relay] || ("Relay " + relay);
                    showToast(relayLabel + " " + stateText, "success");
                })
                .catch(error => { showToast(error.message, 'error'); checkErrorStatus(); });
        }

        function updateButtonStyle(relay) {
            const btn = document.getElementById('btn' + relay);
            if (btn) {
                let relayLabel = relayNames[relay] || "Unknown";
                if (overrideStates[relay]) {
                    btn.className = 'button override';
                    btn.textContent = `${relayLabel} (Override)`;
                } else if (relayStates[relay]) {
                    btn.className = 'button on';
                    btn.textContent = `${relayLabel} (ON)`;
                } else {
                    btn.className = 'button off';
                    btn.textContent = `${relayLabel} (OFF)`;
                }
            }
        }

        function getInitialStates() {
            if (socket && socket.readyState === WebSocket.OPEN) {
                return;
            }

            fetch('/relay/status')
                .then(response => response.json())
                .then(data => { 
                    relayStates = data;
                    if (data.override1 !== undefined) {
                        overrideStates[1] = data.override1;
                        overrideStates[3] = data.override1; // relay3 shares override1
                    }
                    if (data.override2 !== undefined) overrideStates[2] = data.override2;
                    for(let relay in relayStates) {
                        if (relay <= 3) {
                            updateButtonStyle(relay);
                        }
                    }
                    if (data.temperature !== undefined) {
                        lastReceivedTemp = data.temperature;
                    }
                    if (data.externalTemperature !== undefined) {
                        lastReceivedExtTemp = data.externalTemperature;
                    }
                    updateTemperatureDisplay();
                })
                .catch(error => {
                    console.error('Failed to get initial states:', error);
                    checkErrorStatus();
                });
        }

        function checkErrorStatus() {
            fetch('/error/status')
                .then(response => response.json())
                .then(data => {
                    const errSec = document.getElementById('errorSection');
                    if (!errSec) return;
                    currentActiveErrors = data.activeErrors || 0;
                    currentAckErrors = data.acknowledgedErrors || 0;
                    updateTemperatureDisplay();
                    let activeErrors = data.activeErrors || 0;
                    let ackErrors = data.acknowledgedErrors || 0;
                    if (activeErrors > 0 || ackErrors > 0) {
                        let html = '';
                        if (activeErrors > 0) {
                            html += '<h3>System Errors Detected</h3>';
                            if (activeErrors & 1) html += '<p class="error-row"><span>WiFi Disconnected</span> <button class="button dismiss-btn" onclick="clearError(1)">Dismiss</button></p>';
                            if (activeErrors & 2) html += '<p class="error-row"><span>Time Sync Failed</span> <button class="button dismiss-btn" onclick="clearError(2)">Dismiss</button></p>';
                            if (activeErrors & 4) html += '<p class="error-row"><span>Internal Temp Sensor Failed</span> <button class="button dismiss-btn" onclick="clearError(4)">Dismiss</button></p>';
                            if (activeErrors & 8) html += '<p class="error-row"><span>External Temp Sensor Failed</span> <button class="button dismiss-btn" onclick="clearError(8)">Dismiss</button></p>';
                            html += '<div style="margin-top: 15px;"><button class="button dismiss-btn dismiss-all" onclick="clearError(\'all\')">Dismiss All</button></div>';
                        }
                        if (ackErrors > 0) {
                            html += '<h3 style="margin-top: ' + (activeErrors > 0 ? '20px' : '0') + ';">Acknowledged Errors</h3>';
                            if (ackErrors & 1) html += '<p class="error-row" style="opacity: 0.7;"><span>WiFi Disconnected</span></p>';
                            if (ackErrors & 2) html += '<p class="error-row" style="opacity: 0.7;"><span>Time Sync Failed</span></p>';
                            if (ackErrors & 4) html += '<p class="error-row" style="opacity: 0.7;"><span>Internal Temp Sensor Failed</span></p>';
                            if (ackErrors & 8) html += '<p class="error-row" style="opacity: 0.7;"><span>External Temp Sensor Failed</span></p>';
                        }
                        errSec.innerHTML = html;
                        errSec.style.display = 'block';
                        
                        if (activeErrors === 0 && ackErrors > 0) {
                            errSec.style.backgroundColor = '#ff9800';
                            errSec.style.animation = 'none';
                            errSec.style.boxShadow = '0 4px 10px rgba(255, 152, 0, 0.3)';
                        } else {
                            errSec.style.backgroundColor = 'var(--error-color)';
                            errSec.style.animation = 'pulse 2s infinite';
                            errSec.style.boxShadow = '0 4px 10px rgba(244, 67, 54, 0.3)';
                        }
                    } else {
                        errSec.style.display = 'none';
                    }
                })
                .catch(() => {
                    const errSec = document.getElementById('errorSection');
                    if (errSec) errSec.style.display = 'none';
                });
        }

        function clearError(errId) {
            fetch('/error/clear', { 
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ error_id: errId })
            })
            .then(response => response.ok ? response.json() : { status: 'error' })
            .then(data => { 
                if (data.status === 'success') { 
                    checkErrorStatus(); 
                    if (errId === 'all') {
                        showToast('All errors dismissed', 'success');
                    } else {
                        showToast('Error dismissed', 'success');
                    }
                } 
                else { throw new Error('Failed to clear error'); }
            })
            .catch(error => { showToast('Failed to clear error: ' + error.message, 'error'); });
        }

        function showLogs() {
            window.location.href = '/logs';
        }
        function showEmailConfig() {
            window.location.href = '/emailConfig';
        }

        function showDockerConfig() {
            window.location.href = '/dockerConfig';
        }
        function showTempControl() {
            window.location.href = '/tempcontrol';
        }
        function showTempSchedules() {
            window.location.href = '/tempschedules';
        }
        function showSchedules() {
            window.location.href = '/mainSchedules';
        }
        function showDisplayCtrl() {
            window.location.href = '/displayctrl';
        }

        function oneClickLight() {
            fetch('/relay/oneclick', { method: 'POST' })
            .then(response => response.json().then(data => {
                if (!response.ok) throw new Error(data.error);
                showToast('Light colour changed successfully.', 'success');
            }))
            .catch(error => showToast(error.message, 'error'));
        }

        setInterval(updateTime, 1000);
        setInterval(checkErrorStatus, 2000);
        updateTime();
        getInitialStates();
        checkErrorStatus();
        
        document.getElementById('btn1').textContent = `${relayNames[1]} (OFF)`;
        document.getElementById('btn2').textContent = `${relayNames[2]} (OFF)`;
        document.getElementById('btn3').textContent = `${relayNames[3]} (OFF)`;
    </script>
</body>
</html>
)html";

const char emailConfigPage[] PROGMEM = R"html(
<!DOCTYPE html>
<html>
<head>
    <link rel="icon" type="image/png" href="/favicon.png">
    <link rel="shortcut icon" type="image/png" href="/favicon.png">
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Email Settings</title>
    <style>
        :root {
            --primary-color: #1976D2;
            --primary-dark: #0D47A1;
            --primary-light: #BBDEFB;
            --accent-color: #03A9F4;
            --success-color: #4CAF50;
            --warning-color: #FFC107;
            --error-color: #F44336;
            --text-color: #333;
            --text-light: #757575;
            --background-color: #f5f7fa;
            --card-color: #ffffff;
            --border-radius: 8px;
            --shadow: 0 2px 10px rgba(0,0,0,0.1);
            --transition: all 0.3s ease;
        }
        * { box-sizing: border-box; margin: 0; padding: 0; }
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background-color: var(--background-color);
            color: var(--text-color);
            line-height: 1.6;
            padding-bottom: 40px;
        }
        header {
            background: linear-gradient(135deg, var(--primary-color), var(--primary-dark));
            color: white;
            padding: 20px;
            text-align: center;
            box-shadow: 0 2px 10px rgba(0,0,0,0.1);
            position: relative;
            z-index: 10;
            margin-bottom: 30px;
        }
        header h1 { margin: 0; font-size: 2rem; letter-spacing: 0.5px; }
        header p { margin: 5px 0 0; opacity: 0.85; font-size: 0.95rem; }
        .button {
            display: inline-block;
            padding: 12px 24px;
            background-color: var(--primary-color);
            color: white;
            text-decoration: none;
            border-radius: var(--border-radius);
            margin: 5px 0 20px 0;
            transition: var(--transition);
            border: none;
            cursor: pointer;
            font-size: 1rem;
            font-weight: 500;
            box-shadow: var(--shadow);
            text-align: center;
        }
        .button:hover {
            background-color: var(--primary-dark);
            transform: translateY(-2px);
            box-shadow: 0 4px 10px rgba(0,0,0,0.15);
        }
        .header-actions {
            margin-bottom: 20px;
            overflow: hidden;
            display: flex;
            justify-content: space-between;
            align-items: center;
            flex-wrap: wrap;
            gap: 10px;
        }
        .container {
            padding: 20px;
            max-width: 1200px;
            margin: auto;
        }
        .card {
            background: var(--card-color);
            border-radius: var(--border-radius);
            box-shadow: var(--shadow);
            padding: 25px;
            margin-bottom: 25px;
            transition: var(--transition);
        }
        .card:hover { box-shadow: 0 5px 15px rgba(0,0,0,0.15); }
        .card h3 {
            color: var(--primary-color);
            font-size: 1.4rem;
            border-bottom: 2px solid var(--primary-light);
            padding-bottom: 10px;
            margin-bottom: 20px;
        }
        .form-row {
            display: flex;
            flex-wrap: wrap;
            gap: 16px;
            margin-bottom: 18px;
            align-items: flex-end;
        }
        .form-group { display: flex; flex-direction: column; flex: 1; min-width: 120px; }
        .form-group label {
            font-size: 0.85rem;
            color: var(--text-light);
            margin-bottom: 5px;
            font-weight: 500;
        }
        .form-group input[type=email],
        .form-group input[type=text],
        .form-group input[type=password] {
            padding: 10px 12px;
            border: 1.5px solid #ddd;
            border-radius: var(--border-radius);
            font-size: 1rem;
            transition: var(--transition);
            width: 100%;
        }
        .form-group input:focus {
            outline: none;
            border-color: var(--primary-color);
            box-shadow: 0 0 0 3px rgba(25,118,210,0.12);
        }
        .toggle-row {
            display: flex;
            align-items: center;
            gap: 12px;
            margin-bottom: 18px;
        }
        .toggle-row label { font-size: 1rem; cursor: pointer; user-select: none; }
        input[type=checkbox] {
            width: 18px; height: 18px;
            accent-color: var(--primary-color);
            cursor: pointer;
        }
        .save-btn {
            width: 100%;
            padding: 14px;
            background: var(--primary-color);
            color: white;
            border: none;
            border-radius: var(--border-radius);
            font-size: 1.05rem;
            font-weight: 600;
            cursor: pointer;
            transition: var(--transition);
            box-shadow: var(--shadow);
            margin-top: 6px;
        }
        .save-btn:hover { background: var(--primary-dark); transform: translateY(-2px); box-shadow: 0 4px 15px rgba(0,0,0,0.2); }
        .save-btn:active { transform: translateY(1px); }
        #toast {
            position: fixed;
            bottom: 28px;
            left: 50%;
            transform: translateX(-50%) translateY(80px);
            background: #323232;
            color: white;
            padding: 12px 28px;
            border-radius: 24px;
            font-size: 0.95rem;
            opacity: 0;
            transition: all 0.35s ease;
            z-index: 1000;
            pointer-events: none;
        }
        #toast.show { opacity: 1; transform: translateX(-50%) translateY(0); }
        #toast.success { background: var(--success-color); }
        #toast.error   { background: var(--error-color); }
        @media (max-width: 600px) {
            .form-row { flex-direction: column; align-items: stretch; }
        }

        #errorSection h3 {
            color: #fff;
            border-bottom: 2px solid rgba(255,255,255,0.3);
            margin-bottom: 15px;
            padding-bottom: 10px;
        }
        .error-row {
            display: flex;
            justify-content: space-between;
            align-items: center;
            background: rgba(0,0,0,0.15);
            padding: 10px 15px;
            border-radius: 8px;
            margin-bottom: 10px;
            font-size: 1.1rem;
        }
        .dismiss-btn {
            background-color: #fff;
            color: var(--error-color);
            padding: 5px 15px;
            font-size: 0.9rem;
            margin: 0;
        }
        .dismiss-btn:hover {
            background-color: #f1f1f1;
            transform: scale(1.05);
        }
        .dismiss-all {
            background-color: transparent;
            color: #fff;
            border: 2px solid #fff;
            width: 100%;
        }
        .dismiss-all:hover {
            background-color: #fff;
            color: var(--error-color);
        }
        .password-wrapper {
            position: relative;
            display: flex;
            align-items: center;
        }
        .password-wrapper input {
            padding-right: 40px;
        }
        .password-toggle {
            position: absolute;
            right: 12px;
            cursor: pointer;
            color: #666;
            display: flex;
            align-items: center;
            justify-content: center;
            height: 100%;
        }
        .password-toggle:hover {
            color: #333;
        }
    </style>
</head>
<body>
    <header>
        <h1>Email Settings</h1>
        <p>Manage system notifications and credentials</p>
    </header>

    <div class="container">
        <div class="header-actions">
            <button onclick="goBack()" class="button">Back to Dashboard</button>
        </div>

        <div class="card">
            <h3>Configuration</h3>
            <div class="toggle-row">
                <input type="checkbox" id="emailEnabled">
                <label for="emailEnabled">Enable Email Notifications</label>
            </div>
            
            <div class="form-row">
                <div class="form-group" style="flex: 1 1 100%;">
                    <label for="senderAccount">Sender Email</label>
                    <input type="email" id="senderAccount" placeholder="e.g. your_email@gmail.com">
                </div>
            </div>
            
            <div class="form-row">
                <div class="form-group" style="flex: 1 1 100%;">
                    <label for="senderPassword">Sender App Password</label>
                    <div class="password-wrapper">
                        <input type="password" id="senderPassword" placeholder="e.g. abcd efgh ijkl mnop">
                        <span class="password-toggle" id="togglePassword">
                            <svg xmlns="http://www.w3.org/2000/svg" width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" id="eyeIcon">
                                <path d="M1 12s4-8 11-8 11 8 11 8-4 8-11 8-11-8-11-8z"></path>
                                <circle cx="12" cy="12" r="3"></circle>
                            </svg>
                        </span>
                    </div>
                </div>
            </div>
            
            <div class="form-row">
                <div class="form-group" style="flex: 1 1 100%;">
                    <label for="recipient">Receiver Email</label>
                    <input type="email" id="recipient" placeholder="e.g. receiver_email@gmail.com">
                </div>
            </div>

            <button class="save-btn" onclick="saveEmailConfig()">Save Settings</button>
        </div>
    </div>
    
    <div id="toast"></div>

    <script>
        document.addEventListener('DOMContentLoaded', function() {
            const togglePassword = document.getElementById('togglePassword');
            const passwordInput = document.getElementById('senderPassword');
            const eyeIcon = document.getElementById('eyeIcon');

            togglePassword.addEventListener('click', function () {
                const type = passwordInput.getAttribute('type') === 'password' ? 'text' : 'password';
                passwordInput.setAttribute('type', type);
                
                if (type === 'text') {
                    eyeIcon.innerHTML = '<path d="M17.94 17.94A10.07 10.07 0 0 1 12 20c-7 0-11-8-11-8a18.45 18.45 0 0 1 5.06-5.94M9.9 4.24A9.12 9.12 0 0 1 12 4c7 0 11 8 11 8a18.5 18.5 0 0 1-2.16 3.19m-6.72-1.07a3 3 0 1 1-4.24-4.24"></path><line x1="1" y1="1" x2="23" y2="23"></line>';
                } else {
                    eyeIcon.innerHTML = '<path d="M1 12s4-8 11-8 11 8 11 8-4 8-11 8-11-8-11-8z"></path><circle cx="12" cy="12" r="3"></circle>';
                }
            });
        });

        function goBack() { window.location.href = '/'; }
        
        function showToast(msg, type='success') {
            const t = document.getElementById('toast');
            t.textContent = msg;
            t.className = type + ' show';
            setTimeout(() => t.className = '', 3000);
        }
        
        function loadConfig(retryCount) {
            retryCount = retryCount || 0;
            fetch('/api/emailConfig', { credentials: 'include' })
                .then(res => {
                    if (!res.ok) throw new Error('HTTP ' + res.status);
                    return res.json();
                })
                .then(data => {
                    document.getElementById('emailEnabled').checked = data.enabled;
                    document.getElementById('senderAccount').value = data.senderAccount || '';
                    document.getElementById('senderPassword').value = data.senderPassword || '';
                    document.getElementById('recipient').value = data.recipient || '';
                })
                .catch(err => {
                    console.error('Failed to load email config (attempt ' + (retryCount + 1) + ')', err);
                    if (retryCount < 2) {
                        setTimeout(() => loadConfig(retryCount + 1), 1000);
                    } else {
                        showToast('Failed to load configuration', 'error');
                    }
                });
        }
        
        function saveEmailConfig() {
            const data = {
                enabled: document.getElementById('emailEnabled').checked,
                senderAccount: document.getElementById('senderAccount').value,
                senderPassword: document.getElementById('senderPassword').value,
                recipient: document.getElementById('recipient').value
            };
            
            const btn = document.querySelector('.save-btn');
            btn.textContent = "Saving...";
            btn.disabled = true;
            
            fetch('/api/emailConfig', {
                method: 'POST',
                credentials: 'include',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify(data)
            })
            .then(res => res.json())
            .then(data => {
                if (data.success) {
                    showToast('Configuration saved successfully!', 'success');
                } else {
                    showToast('Failed to save configuration', 'error');
                }
            })
            .catch(err => {
                console.error('Save error', err);
                showToast('Error saving configuration', 'error');
            })
            .finally(() => {
                btn.textContent = "Save Settings";
                btn.disabled = false;
            });
        }
        
        window.addEventListener('load', () => loadConfig(0));
    </script>
</body>
</html>
)html";

const char dockerConfigPage[] PROGMEM = R"html(
<!DOCTYPE html>
<html>
<head>
    <link rel="icon" type="image/png" href="/favicon.png">
    <link rel="shortcut icon" type="image/png" href="/favicon.png">
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Docker Settings</title>
    <style>
        :root {
            --primary-color: #1976D2;
            --primary-dark: #0D47A1;
            --primary-light: #BBDEFB;
            --accent-color: #03A9F4;
            --success-color: #4CAF50;
            --warning-color: #FFC107;
            --error-color: #F44336;
            --text-color: #333;
            --text-light: #757575;
            --background-color: #f5f7fa;
            --card-color: #ffffff;
            --border-radius: 8px;
            --shadow: 0 2px 10px rgba(0,0,0,0.1);
            --transition: all 0.3s ease;
        }
        * { box-sizing: border-box; margin: 0; padding: 0; }
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background-color: var(--background-color);
            color: var(--text-color);
            line-height: 1.6;
            padding-bottom: 40px;
        }
        header {
            background: linear-gradient(135deg, var(--primary-color), var(--primary-dark));
            color: white;
            padding: 20px;
            text-align: center;
            box-shadow: 0 2px 10px rgba(0,0,0,0.1);
            position: relative;
            z-index: 10;
            margin-bottom: 30px;
        }
        header h1 { margin: 0; font-size: 2rem; letter-spacing: 0.5px; }
        header p { margin: 5px 0 0; opacity: 0.85; font-size: 0.95rem; }
        .button {
            display: inline-block;
            padding: 12px 24px;
            background-color: var(--primary-color);
            color: white;
            text-decoration: none;
            border-radius: var(--border-radius);
            margin: 5px 0 20px 0;
            transition: var(--transition);
            border: none;
            cursor: pointer;
            font-size: 1rem;
            font-weight: 500;
            box-shadow: var(--shadow);
            text-align: center;
        }
        .button:hover {
            background-color: var(--primary-dark);
            transform: translateY(-2px);
            box-shadow: 0 4px 10px rgba(0,0,0,0.15);
        }
        .header-actions {
            margin-bottom: 20px;
            overflow: hidden;
            display: flex;
            justify-content: space-between;
            align-items: center;
            flex-wrap: wrap;
            gap: 10px;
        }
        .container {
            padding: 20px;
            max-width: 1200px;
            margin: auto;
        }
        .card {
            background: var(--card-color);
            border-radius: var(--border-radius);
            box-shadow: var(--shadow);
            padding: 25px;
            margin-bottom: 25px;
            transition: var(--transition);
        }
        .card h3 {
            color: var(--primary-color);
            font-size: 1.4rem;
            border-bottom: 2px solid var(--primary-light);
            padding-bottom: 10px;
            margin-bottom: 20px;
        }
        .toggle-row {
            display: flex;
            align-items: center;
            gap: 12px;
            margin-bottom: 18px;
        }
        .toggle-row label { font-size: 1rem; cursor: pointer; user-select: none; }
        input[type=checkbox] {
            width: 18px; height: 18px;
            accent-color: var(--primary-color);
            cursor: pointer;
        }
        .save-btn {
            width: 100%;
            padding: 14px;
            background: var(--primary-color);
            color: white;
            border: none;
            border-radius: var(--border-radius);
            font-size: 1.05rem;
            font-weight: 600;
            cursor: pointer;
            transition: var(--transition);
            box-shadow: var(--shadow);
            margin-top: 6px;
        }
        .save-btn:hover { background: var(--primary-dark); transform: translateY(-2px); box-shadow: 0 4px 15px rgba(0,0,0,0.2); }
        .save-btn:active { transform: translateY(1px); }
        #toast {
            position: fixed;
            bottom: 28px;
            left: 50%;
            transform: translateX(-50%) translateY(80px);
            background: #323232;
            color: white;
            padding: 12px 28px;
            border-radius: 24px;
            font-size: 0.95rem;
            opacity: 0;
            transition: all 0.35s ease;
            z-index: 1000;
            pointer-events: none;
        }
        #toast.show { opacity: 1; transform: translateX(-50%) translateY(0); }
        #toast.success { background: var(--success-color); }
        #toast.error   { background: var(--error-color); }

        #errorSection h3 {
            color: #fff;
            border-bottom: 2px solid rgba(255,255,255,0.3);
            margin-bottom: 15px;
            padding-bottom: 10px;
        }
        .error-row {
            display: flex;
            justify-content: space-between;
            align-items: center;
            background: rgba(0,0,0,0.15);
            padding: 10px 15px;
            border-radius: 8px;
            margin-bottom: 10px;
            font-size: 1.1rem;
        }
        .dismiss-btn {
            background-color: #fff;
            color: var(--error-color);
            padding: 5px 15px;
            font-size: 0.9rem;
            margin: 0;
        }
        .dismiss-btn:hover {
            background-color: #f1f1f1;
            transform: scale(1.05);
        }
        .dismiss-all {
            background-color: transparent;
            color: #fff;
            border: 2px solid #fff;
            width: 100%;
        }
        .dismiss-all:hover {
            background-color: #fff;
            color: var(--error-color);
        }
    </style>
</head>
<body>
    <header>
        <h1>Docker Settings</h1>
        <p>Manage Docker integration endpoints</p>
    </header>

    <div class="container">
        <div class="header-actions">
            <button onclick="goBack()" class="button">Back to Dashboard</button>
        </div>

        <div class="card">
            <h3>Configuration</h3>
            <div class="toggle-row">
                <input type="checkbox" id="dockerEnabled">
                <label for="dockerEnabled">Enable Docker Integration (API Endpoints)</label>
            </div>
            
            <button class="save-btn" onclick="saveDockerConfig()">Save Settings</button>
        </div>
    </div>
    
    <div id="toast"></div>

    <script>
        function goBack() { window.location.href = '/'; }
        
        function showToast(msg, type='success') {
            const t = document.getElementById('toast');
            t.textContent = msg;
            t.className = type + ' show';
            setTimeout(() => t.className = '', 3000);
        }
        
        function loadConfig() {
            fetch('/api/dockerConfig')
                .then(res => res.json())
                .then(data => {
                    document.getElementById('dockerEnabled').checked = data.enabled;
                })
                .catch(err => {
                    console.error('Failed to load docker config', err);
                    showToast('Failed to load configuration', 'error');
                });
        }
        
        function saveDockerConfig() {
            const data = {
                enabled: document.getElementById('dockerEnabled').checked
            };
            
            const btn = document.querySelector('.save-btn');
            btn.textContent = "Saving...";
            btn.disabled = true;
            
            fetch('/api/dockerConfig', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify(data)
            })
            .then(res => res.json())
            .then(data => {
                if (data.success) {
                    showToast('Configuration saved successfully!', 'success');
                } else {
                    showToast('Failed to save configuration', 'error');
                }
            })
            .catch(err => {
                console.error('Save error', err);
                showToast('Error saving configuration', 'error');
            })
            .finally(() => {
                btn.textContent = "Save Settings";
                btn.disabled = false;
            });
        }
        
        document.addEventListener('DOMContentLoaded', loadConfig);
    </script>
</body>
</html>
)html";

const char displayCtrlPage[] PROGMEM = R"html(
<!DOCTYPE html>
<html>
<head>
    <link rel="icon" type="image/png" href="/favicon.png">
    <link rel="shortcut icon" type="image/png" href="/favicon.png">
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Display Control</title>
    <style>
        :root {
            --primary-color: #1976D2;
            --primary-dark: #0D47A1;
            --primary-light: #BBDEFB;
            --accent-color: #03A9F4;
            --success-color: #4CAF50;
            --warning-color: #FFC107;
            --error-color: #F44336;
            --text-color: #333;
            --text-light: #757575;
            --background-color: #f5f7fa;
            --card-color: #ffffff;
            --border-radius: 8px;
            --shadow: 0 2px 10px rgba(0,0,0,0.1);
            --transition: all 0.3s ease;
        }
        * { box-sizing: border-box; margin: 0; padding: 0; }
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background-color: var(--background-color);
            color: var(--text-color);
            line-height: 1.6;
        }
        header {
            background: linear-gradient(135deg, var(--primary-color), var(--primary-dark));
            color: white;
            padding: 20px;
            text-align: center;
            box-shadow: 0 2px 10px rgba(0,0,0,0.1);
            position: relative;
            z-index: 10;
            margin-bottom: 30px;
        }
        header h1 { margin: 0; font-size: 2rem; letter-spacing: 0.5px; }
        header p { margin: 5px 0 0; opacity: 0.85; font-size: 0.95rem; }
        .button {
            display: inline-block;
            padding: 12px 24px;
            background-color: var(--primary-color);
            color: white;
            text-decoration: none;
            border-radius: var(--border-radius);
            margin: 5px 0 20px 0;
            transition: var(--transition);
            border: none;
            cursor: pointer;
            font-size: 1rem;
            font-weight: 500;
            box-shadow: var(--shadow);
            text-align: center;
        }
        .button:hover {
            background-color: var(--primary-dark);
            transform: translateY(-2px);
            box-shadow: 0 4px 10px rgba(0,0,0,0.15);
        }
        .header-actions {
            margin-bottom: 20px;
            overflow: hidden;
            display: flex;
            justify-content: space-between;
            align-items: center;
            flex-wrap: wrap;
            gap: 10px;
        }
        .container {
            padding: 20px;
            max-width: 1200px;
            margin: auto;
        }
        .card {
            background: var(--card-color);
            border-radius: var(--border-radius);
            box-shadow: var(--shadow);
            padding: 25px;
            margin-bottom: 25px;
            transition: var(--transition);
        }
        .card:hover { box-shadow: 0 5px 15px rgba(0,0,0,0.15); }
        .card h3 {
            color: var(--primary-color);
            font-size: 1.4rem;
            border-bottom: 2px solid var(--primary-light);
            padding-bottom: 10px;
            margin-bottom: 20px;
        }
        /* Status badge */
        .status-row {
            display: flex;
            align-items: center;
            gap: 12px;
            margin-bottom: 8px;
        }
        .status-badge {
            display: inline-block;
            padding: 6px 16px;
            border-radius: 20px;
            font-weight: 600;
            font-size: 0.95rem;
            letter-spacing: 0.5px;
            transition: var(--transition);
        }
        .status-badge.on  { background: #e8f5e9; color: var(--success-color); border: 1.5px solid var(--success-color); }
        .status-badge.off { background: #fce4ec; color: var(--error-color);   border: 1.5px solid var(--error-color); }
        .mode-label { color: var(--text-light); font-size: 0.9rem; }
        /* Schedule form */
        .form-row {
            display: flex;
            flex-wrap: wrap;
            gap: 16px;
            margin-bottom: 18px;
            align-items: flex-end;
        }
        .form-group { display: flex; flex-direction: column; flex: 1; min-width: 120px; }
        .form-group label {
            font-size: 0.85rem;
            color: var(--text-light);
            margin-bottom: 5px;
            font-weight: 500;
        }
        .form-group input[type=time],
        .form-group input[type=number] {
            padding: 10px 12px;
            border: 1.5px solid #ddd;
            border-radius: var(--border-radius);
            font-size: 1rem;
            transition: var(--transition);
            width: 100%;
        }
        .form-group input:focus {
            outline: none;
            border-color: var(--primary-color);
            box-shadow: 0 0 0 3px rgba(25,118,210,0.12);
        }
        .toggle-row {
            display: flex;
            align-items: center;
            gap: 12px;
            margin-bottom: 18px;
        }
        .toggle-row label { font-size: 1rem; cursor: pointer; user-select: none; }
        input[type=checkbox] {
            width: 18px; height: 18px;
            accent-color: var(--primary-color);
            cursor: pointer;
        }
        /* Override buttons */
        .override-group {
            display: grid;
            grid-template-columns: 1fr 1fr 1fr;
            gap: 12px;
            margin-top: 4px;
        }
        .override-btn {
            padding: 14px 8px;
            border: 2px solid transparent;
            border-radius: var(--border-radius);
            font-size: 1rem;
            font-weight: 600;
            cursor: pointer;
            transition: var(--transition);
            background: #f5f7fa;
            color: var(--text-color);
        }
        .override-btn:hover { transform: translateY(-2px); box-shadow: 0 4px 12px rgba(0,0,0,0.12); }
        .override-btn.active-schedule { background: var(--primary-light); border-color: var(--primary-color); color: var(--primary-dark); }
        .override-btn.active-on       { background: #e8f5e9; border-color: var(--success-color); color: #2e7d32; }
        .override-btn.active-off      { background: #fce4ec; border-color: var(--error-color);   color: #c62828; }
        /* Save button */
        .save-btn {
            width: 100%;
            padding: 14px;
            background: var(--primary-color);
            color: white;
            border: none;
            border-radius: var(--border-radius);
            font-size: 1.05rem;
            font-weight: 600;
            cursor: pointer;
            transition: var(--transition);
            box-shadow: var(--shadow);
            margin-top: 6px;
        }
        .save-btn:hover { background: var(--primary-dark); transform: translateY(-2px); box-shadow: 0 4px 15px rgba(0,0,0,0.2); }
        .save-btn:active { transform: translateY(1px); }
        /* Toast */
        #toast {
            position: fixed;
            bottom: 28px;
            left: 50%;
            transform: translateX(-50%) translateY(80px);
            background: #323232;
            color: white;
            padding: 12px 28px;
            border-radius: 24px;
            font-size: 0.95rem;
            opacity: 0;
            transition: all 0.35s ease;
            z-index: 1000;
            pointer-events: none;
        }
        #toast.show { opacity: 1; transform: translateX(-50%) translateY(0); }
        #toast.success { background: var(--success-color); }
        #toast.error   { background: var(--error-color); }
        @media (max-width: 600px) {
            .override-group { grid-template-columns: 1fr; }
            .form-row { flex-direction: column; align-items: stretch; }
        }

        #errorSection h3 {
            color: #fff;
            border-bottom: 2px solid rgba(255,255,255,0.3);
            margin-bottom: 15px;
            padding-bottom: 10px;
        }
        .error-row {
            display: flex;
            justify-content: space-between;
            align-items: center;
            background: rgba(0,0,0,0.15);
            padding: 10px 15px;
            border-radius: 8px;
            margin-bottom: 10px;
            font-size: 1.1rem;
        }
        .dismiss-btn {
            background-color: #fff;
            color: var(--error-color);
            padding: 5px 15px;
            font-size: 0.9rem;
            margin: 0;
        }
        .dismiss-btn:hover {
            background-color: #f1f1f1;
            transform: scale(1.05);
        }
        .dismiss-all {
            background-color: transparent;
            color: #fff;
            border: 2px solid #fff;
            width: 100%;
        }
        .dismiss-all:hover {
            background-color: #fff;
            color: var(--error-color);
        }
    </style>
</head>
<body>
    <header>
        <h1>Display Control</h1>
        <p>Manage OLED display on/off schedule</p>
    </header>
    <div class="container">
        <div class="header-actions">
            <button onclick="goBack()" class="button">Back to Dashboard</button>
        </div>

        <!-- Status Card -->
        <div class="card">
            <h3>Current Status</h3>
            <div class="status-row">
                <span>Display:</span>
                <span class="status-badge" id="displayBadge">...</span>
            </div>
            <div class="mode-label" id="modeLabel">Loading...</div>
        </div>

        <!-- Override Card -->
        <div class="card">
            <h3>Override</h3>
            <p style="color:var(--text-light);font-size:0.9rem;margin-bottom:16px;">Instantly control the display, or let the schedule decide.</p>
            <div class="override-group">
                <button class="override-btn" id="btn-follow" onclick="setOverride(0)">Follow Schedule</button>
                <button class="override-btn" id="btn-on"     onclick="setOverride(1)">Force ON</button>
                <button class="override-btn" id="btn-off"    onclick="setOverride(2)">Force OFF</button>
            </div>
        </div>

        <!-- Schedule Card -->
        <div class="card">
            <h3>Schedule</h3>
            <div class="form-row">
                <div class="form-group">
                    <label for="onTime">Turn ON at</label>
                    <input type="time" id="onTime" value="08:00">
                </div>
                <div class="form-group">
                    <label for="offTime">Turn OFF at</label>
                    <input type="time" id="offTime" value="22:00">
                </div>
            </div>
            <button class="save-btn" onclick="saveSchedule()">Save Schedule</button>
        </div>

    </div>
    <div id="toast"></div>
    <script>
        let currentData = {};
        let isEditing = false;

        const modeNames = ['Following schedule', 'Forced ON', 'Forced OFF'];

        function pad(n) { return String(n).padStart(2, '0'); }

        function showToast(msg, type) {
            const t = document.getElementById('toast');
            t.textContent = msg;
            t.className = 'show ' + (type || '');
            setTimeout(() => { t.className = ''; }, 3000);
        }

        function applyData(d) {
            currentData = d;
            // Status badge
            const badge = document.getElementById('displayBadge');
            badge.textContent = d.displayOn ? 'ON' : 'OFF';
            badge.className = 'status-badge ' + (d.displayOn ? 'on' : 'off');
            // Mode label
            document.getElementById('modeLabel').textContent = modeNames[d.overrideMode] || '';
            // Override buttons
            document.getElementById('btn-follow').className = 'override-btn' + (d.overrideMode === 0 ? ' active-schedule' : '');
            document.getElementById('btn-on').className     = 'override-btn' + (d.overrideMode === 1 ? ' active-on'       : '');
            document.getElementById('btn-off').className    = 'override-btn' + (d.overrideMode === 2 ? ' active-off'      : '');
            // Schedule fields
            if (!isEditing) {
                document.getElementById('onTime').value  = pad(d.onHour)  + ':' + pad(d.onMinute);
                document.getElementById('offTime').value = pad(d.offHour) + ':' + pad(d.offMinute);
            }
        }

        function loadData() {
            fetch('/display/schedule')
                .then(r => r.json())
                .then(applyData)
                .catch(() => showToast('Failed to load settings', 'error'));
        }

        function setOverride(mode) {
            fetch('/display/schedule/save', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ overrideMode: mode })
            })
            .then(r => r.json())
            .then(d => {
                if (d.status === 'success') {
                    showToast('Override updated', 'success');
                    setTimeout(loadData, 400);
                } else { showToast('Error: ' + (d.error || 'unknown'), 'error'); }
            })
            .catch(() => showToast('Request failed', 'error'));
        }

        function saveSchedule() {
            const onParts  = document.getElementById('onTime').value.split(':');
            const offParts = document.getElementById('offTime').value.split(':');
            fetch('/display/schedule/save', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({
                    onHour:    parseInt(onParts[0]),
                    onMinute:  parseInt(onParts[1]),
                    offHour:   parseInt(offParts[0]),
                    offMinute: parseInt(offParts[1]),
                    enabled:   true
                })
            })
            .then(r => r.json())
            .then(d => {
                if (d.status === 'success') {
                    showToast('Schedule saved', 'success');
                    isEditing = false;
                    setTimeout(loadData, 400);
                } else { showToast('Error: ' + (d.error || 'unknown'), 'error'); }
            })
            .catch(() => showToast('Request failed', 'error'));
        }

        document.getElementById('onTime').addEventListener('input', () => isEditing = true);
        document.getElementById('offTime').addEventListener('input', () => isEditing = true);

        // Refresh status every 5 s
        loadData();
        setInterval(loadData, 5000);

        function goBack() {
            window.history.back();
        }
    </script>
</body>
</html>
)html";

const char logsPage[] PROGMEM = R"html(
<!DOCTYPE html>
<html>
<head>
    <link rel="icon" type="image/png" href="/favicon.png">
    <link rel="shortcut icon" type="image/png" href="/favicon.png">
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>System Logs</title>
    <style>
        :root {
            --primary-color: #1976D2;
            --primary-dark: #0D47A1;
            --primary-light: #BBDEFB;
            --accent-color: #03A9F4;
            --success-color: #4CAF50;
            --warning-color: #FFC107;
            --error-color: #F44336;
            --text-color: #333;
            --text-light: #757575;
            --background-color: #f5f7fa;
            --card-color: #ffffff;
            --border-radius: 8px;
            --shadow: 0 2px 10px rgba(0,0,0,0.1);
            --transition: all 0.3s ease;
        }

        * {
            box-sizing: border-box;
            margin: 0;
            padding: 0;
        }

        body {
            margin: 0;
            padding: 0;
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background-color: var(--background-color);
            color: var(--text-color);
            line-height: 1.6;
        }

        header {
            background: linear-gradient(135deg, var(--primary-color), var(--primary-dark));
            color: white;
            padding: 20px;
            text-align: center;
            box-shadow: 0 2px 10px rgba(0,0,0,0.1);
            position: relative;
            z-index: 10;
            margin-bottom: 30px;
        }

        header h1 {
            margin: 0;
            font-size: 2rem;
            letter-spacing: 0.5px;
        }

        .container {
            padding: 20px;
            max-width: 1200px;
            margin: auto;
        }

        .logs-table {
            width: 100%;
            border-collapse: separate;
            border-spacing: 0;
            background-color: var(--card-color);
            box-shadow: var(--shadow);
            border-radius: var(--border-radius);
            overflow: hidden;
            margin-bottom: 30px;
        }

        .logs-table th, .logs-table td {
            padding: 15px;
            text-align: left;
        }

        .logs-table th {
            background-color: var(--primary-color);
            color: white;
            font-weight: 500;
        }

        .logs-table tr:nth-child(even) {
            background-color: #f9f9f9;
        }

        .logs-table tr {
            transition: var(--transition);
            border-bottom: 1px solid #eee;
        }

        .logs-table tr:last-child {
            border-bottom: none;
        }

        .logs-table tr:hover {
            background-color: #f1f1f1;
        }

        .button {
            display: inline-block;
            padding: 12px 24px;
            background-color: var(--primary-color);
            color: white;
            text-decoration: none;
            border-radius: var(--border-radius);
            margin: 5px 0 20px 0;
            transition: var(--transition);
            border: none;
            cursor: pointer;
            font-size: 1rem;
            font-weight: 500;
            box-shadow: var(--shadow);
            text-align: center;
        }

        .button:hover {
            background-color: var(--primary-dark);
            transform: translateY(-2px);
            box-shadow: 0 4px 10px rgba(0,0,0,0.15);
        }

        .button:active {
            transform: translateY(1px);
        }

        .refresh-button {
            float: right;
            background-color: var(--success-color);
        }

        .refresh-button:hover {
            background-color: #388E3C;
        }

        .header-actions {
            margin-bottom: 20px;
            overflow: hidden;
            display: flex;
            justify-content: space-between;
            align-items: center;
            flex-wrap: wrap;
            gap: 10px;
        }

        @media (max-width: 768px) {
            .logs-table {
                font-size: 14px;
            }
            
            .logs-table th, .logs-table td {
                padding: 10px;
            }
            
            .container {
                padding: 10px;
            }
            
        }

        .loading {
            display: none;
            text-align: center;
            padding: 20px;
        }

        .loading-spinner {
            border: 4px solid #f3f3f3;
            border-top: 4px solid var(--primary-color);
            border-radius: 50%;
            width: 40px;
            height: 40px;
            animation: spin 1s linear infinite;
            margin: 0 auto;
        }

        @keyframes spin {
            0% { transform: rotate(0deg); }
            100% { transform: rotate(360deg); }
        }

        #errorSection h3 {
            color: #fff;
            border-bottom: 2px solid rgba(255,255,255,0.3);
            margin-bottom: 15px;
            padding-bottom: 10px;
        }
        .error-row {
            display: flex;
            justify-content: space-between;
            align-items: center;
            background: rgba(0,0,0,0.15);
            padding: 10px 15px;
            border-radius: 8px;
            margin-bottom: 10px;
            font-size: 1.1rem;
        }
        .dismiss-btn {
            background-color: #fff;
            color: var(--error-color);
            padding: 5px 15px;
            font-size: 0.9rem;
            margin: 0;
        }
        .dismiss-btn:hover {
            background-color: #f1f1f1;
            transform: scale(1.05);
        }
        .dismiss-all {
            background-color: transparent;
            color: #fff;
            border: 2px solid #fff;
            width: 100%;
        }
        .dismiss-all:hover {
            background-color: #fff;
            color: var(--error-color);
        }
    </style>
</head>
<body>
    <header>
        <h1>System Logs</h1>
        <p>View recent activity and system events</p>
    </header>
    <div class="container">
        <div class="header-actions">
            <button onclick="goBack()" class="button">Back to Dashboard</button>
            <button onclick="refreshLogs()" class="button refresh-button">Refresh Logs</button>
        </div>
        <div id="loading" class="loading">
            <div class="loading-spinner"></div>
            <p>Loading logs...</p>
        </div>
        <table class="logs-table">
            <thead>
                <tr>
                    <th>ID</th>
                    <th>Timestamp</th>
                    <th>Message</th>
                </tr>
            </thead>
            <tbody id="logsTableBody">
            </tbody>
        </table>
    </div>
    <script>
        function loadLogs(showSpinner = true) {
            if (showSpinner) {
                document.getElementById('loading').style.display = 'block';
            }
            
            fetch('/logs/data')
                .then(response => response.json())
                .then(data => {
                    const tableBody = document.getElementById('logsTableBody');
                    tableBody.innerHTML = '';
                    
                    if (data.logs && Array.isArray(data.logs)) {
                        data.logs.reverse().forEach(log => {
                            const row = tableBody.insertRow();
                            row.insertCell(0).textContent = log.id;
                            row.insertCell(1).textContent = log.timestamp;
                            row.insertCell(2).textContent = log.message;
                        });
                    }
                    
                    if (showSpinner) {
                        document.getElementById('loading').style.display = 'none';
                    }
                })
                .catch(error => {
                    console.error('Error loading logs:', error);
                    if (showSpinner) {
                        document.getElementById('loading').style.display = 'none';
                    }
                });
        }

        function refreshLogs() {
            loadLogs(true);
        }

        function goBack() {
            window.history.back();
        }

        loadLogs(true);
        setInterval(() => loadLogs(false), 10000);
    </script>
</body>
</html>
)html";

const char tempctrl[] PROGMEM = R"html(
<!DOCTYPE html>
<html>
<head>
    <link rel="icon" type="image/png" href="/favicon.png">
    <link rel="shortcut icon" type="image/png" href="/favicon.png">
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Sensor Calibration</title>
    <style>
        :root {
            --primary-color: #1976D2;
            --primary-dark: #0D47A1;
            --primary-light: #BBDEFB;
            --accent-color: #03A9F4;
            --success-color: #4CAF50;
            --warning-color: #FFC107;
            --error-color: #F44336;
            --text-color: #333;
            --text-light: #757575;
            --background-color: #f5f7fa;
            --card-color: #ffffff;
            --border-radius: 8px;
            --shadow: 0 2px 10px rgba(0,0,0,0.1);
            --transition: all 0.3s ease;
        }

        * {
            box-sizing: border-box;
            margin: 0;
            padding: 0;
        }

        body {
            margin: 0;
            padding: 0;
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background-color: var(--background-color);
            color: var(--text-color);
            line-height: 1.6;
        }

        header {
            background: linear-gradient(135deg, var(--primary-color), var(--primary-dark));
            color: white;
            padding: 20px;
            text-align: center;
            box-shadow: 0 2px 10px rgba(0,0,0,0.1);
            position: relative;
            z-index: 10;
            margin-bottom: 30px;
        }

        header h1 {
            margin: 0;
            font-size: 2rem;
            letter-spacing: 0.5px;
        }

        .container {
            padding: 20px;
            max-width: 1200px;
            margin: auto;
        }

        .button {
            display: inline-block;
            padding: 12px 24px;
            background-color: var(--primary-color);
            color: white;
            text-decoration: none;
            border-radius: var(--border-radius);
            margin: 5px 0 20px 0;
            transition: var(--transition);
            border: none;
            cursor: pointer;
            font-size: 1rem;
            font-weight: 500;
            box-shadow: var(--shadow);
            text-align: center;
        }

        .button:hover {
            background-color: var(--primary-dark);
            transform: translateY(-2px);
            box-shadow: 0 4px 10px rgba(0,0,0,0.15);
        }

        .button:active {
            transform: translateY(1px);
        }

        .header-actions {
            margin-bottom: 20px;
            overflow: hidden;
            display: flex;
            justify-content: space-between;
            align-items: center;
            flex-wrap: wrap;
            gap: 10px;
        }

        .calibration-section {
            background-color: var(--card-color);
            padding: 25px;
            border-radius: var(--border-radius);
            box-shadow: var(--shadow);
            margin-bottom: 25px;
            transition: var(--transition);
        }

        .calibration-section:hover {
            box-shadow: 0 5px 15px rgba(0,0,0,0.15);
        }

        .calibration-section h3 {
            color: var(--primary-color);
            margin-bottom: 15px;
            font-size: 1.5rem;
            border-bottom: 2px solid var(--primary-light);
            padding-bottom: 10px;
        }

        .calibration-grid {
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 20px;
            margin-bottom: 20px;
        }

        .calibration-item label {
            display: block;
            margin-bottom: 8px;
            font-weight: 500;
            color: var(--text-color);
        }

        .calibration-item input[type="number"] {
            width: 100%;
            padding: 12px;
            margin: 8px 0 20px 0;
            border-radius: var(--border-radius);
            border: 1px solid #ddd;
            font-size: 1rem;
            transition: var(--transition);
        }

        .calibration-item input[type="number"]:focus {
            outline: none;
            border-color: var(--primary-color);
            box-shadow: 0 0 0 3px var(--primary-light);
        }

        .calibration-note {
            background-color: #e3f2fd;
            border-left: 4px solid var(--primary-color);
            padding: 12px;
            margin-bottom: 20px;
            border-radius: 4px;
            font-size: 0.9rem;
            color: #1565c0;
        }

        .raw-data-section {
            background-color: var(--card-color);
            padding: 25px;
            border-radius: var(--border-radius);
            box-shadow: var(--shadow);
            margin-bottom: 25px;
            transition: var(--transition);
        }

        .raw-data-section:hover {
            box-shadow: 0 5px 15px rgba(0,0,0,0.15);
        }

        .raw-data-section h3 {
            color: var(--primary-color);
            margin-bottom: 15px;
            font-size: 1.5rem;
            border-bottom: 2px solid var(--primary-light);
            padding-bottom: 10px;
        }

        .raw-data-grid {
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 20px;
            margin-bottom: 20px;
        }

        .raw-data-item {
            text-align: center;
            padding: 15px;
            background-color: #f8f9fa;
            border-radius: var(--border-radius);
            transition: var(--transition);
        }

        .raw-data-item:hover {
            background-color: #e9ecef;
        }

        .raw-data-label {
            font-size: 1rem;
            color: var(--text-light);
            margin-bottom: 8px;
            font-weight: 500;
        }

        .raw-data-value {
            font-size: 1.8rem;
            font-weight: bold;
            color: var(--accent-color);
        }

        .raw-data-item.internal {
            border-left: 4px solid var(--success-color);
        }

        .raw-data-item.external {
            border-left: 4px solid var(--accent-color);
        }

        .calibration-save-button {
            width: 100%;
            padding: 12px 24px;
            background-color: var(--accent-color);
            color: white;
            border: none;
            border-radius: var(--border-radius);
            cursor: pointer;
            transition: var(--transition);
            font-weight: 500;
            font-size: 1rem;
            box-shadow: var(--shadow);
        }

        .calibration-save-button:hover {
            background-color: #0288D1;
            transform: translateY(-2px);
            box-shadow: 0 4px 10px rgba(0,0,0,0.15);
        }

        .calibration-save-button:active {
            transform: translateY(1px);
        }

        .changed-indicator {
            background-color: #fff3cd !important;
            border-left: 4px solid var(--warning-color) !important;
        }

        .calibration-save-button.changes-pending {
            background-color: var(--warning-color) !important;
            color: #333 !important;
            animation: pulse 2s infinite;
        }

        @keyframes pulse {
            0% { transform: scale(1); }
            50% { transform: scale(1.05); }
            100% { transform: scale(1); }
        }

        @media (max-width: 768px) {
            .container {
                padding: 10px;
            }
            
            .calibration-grid {
                grid-template-columns: 1fr;
                gap: 15px;
            }

            .raw-data-grid {
                grid-template-columns: 1fr;
                gap: 15px;
            }

            .calibration-section {
                padding: 15px;
            }

            .raw-data-section {
                padding: 15px;
            }

            .calibration-section h3 {
                font-size: 1.3rem;
            }

            .raw-data-section h3 {
                font-size: 1.3rem;
            }

            .raw-data-value {
                font-size: 1.5rem;
            }
            
            .calibration-save-button {
                padding: 12px;
            }
        }

        #errorSection h3 {
            color: #fff;
            border-bottom: 2px solid rgba(255,255,255,0.3);
            margin-bottom: 15px;
            padding-bottom: 10px;
        }
        .error-row {
            display: flex;
            justify-content: space-between;
            align-items: center;
            background: rgba(0,0,0,0.15);
            padding: 10px 15px;
            border-radius: 8px;
            margin-bottom: 10px;
            font-size: 1.1rem;
        }
        .dismiss-btn {
            background-color: #fff;
            color: var(--error-color);
            padding: 5px 15px;
            font-size: 0.9rem;
            margin: 0;
        }
        .dismiss-btn:hover {
            background-color: #f1f1f1;
            transform: scale(1.05);
        }
        .dismiss-all {
            background-color: transparent;
            color: #fff;
            border: 2px solid #fff;
            width: 100%;
        }
        .dismiss-all:hover {
            background-color: #fff;
            color: var(--error-color);
        }

        /* Toast */
        #toast {
            position: fixed;
            bottom: 28px;
            left: 50%;
            transform: translateX(-50%) translateY(80px);
            background: #323232;
            color: white;
            padding: 12px 28px;
            border-radius: 24px;
            font-size: 0.95rem;
            opacity: 0;
            transition: all 0.35s ease;
            z-index: 1000;
            pointer-events: none;
        }
        #toast.show { opacity: 1; transform: translateX(-50%) translateY(0); }
        #toast.success { background: var(--success-color); }
        #toast.error   { background: var(--error-color); }
    </style>
</head>
<body>
    <header>
        <h1>Sensor Calibration</h1>
        <p>Manage sensor's calibration</p>
    </header>
    <div class="container">
        <div class="header-actions">
            <button onclick="goBack()" class="button">Back to Dashboard</button>
        </div>

        <div class="raw-data-section">
            <h3>Raw Sensor Data</h3>
            <div class="raw-data-grid">
                <div class="raw-data-item internal">
                    <div class="raw-data-label">Internal Sensor (Raw)</div>
                    <div class="raw-data-value" id="internal-raw-temp">--</div>
                </div>
                <div class="raw-data-item external">
                    <div class="raw-data-label">External Sensor (Raw)</div>
                    <div class="raw-data-value" id="external-raw-temp">--</div>
                </div>
            </div>
        </div>

        <div class="calibration-section">
            <h3>Sensor Calibration</h3>
            <div class="calibration-note">
                Calibration allows you to adjust sensor readings to match a reference thermometer. 
                Positive values increase the reading, negative values decrease it. Range: -10.00°C to +10.00°C
            </div>
            
            <div class="calibration-grid">
                <div class="calibration-item">
                    <label for="internal-calibration">Internal Sensor Offset (°C):</label>
                    <input type="number" id="internal-calibration" min="-10" max="10" step="0.01" value="0.00">
                </div>
                <div class="calibration-item">
                    <label for="external-calibration">External Sensor Offset (°C):</label>
                    <input type="number" id="external-calibration" min="-10" max="10" step="0.01" value="0.00">
                </div>
            </div>
            
            <div class="temp-buttons">
                <button class="calibration-save-button" id="save-calibration-settings" onclick="saveCalibrationSettings()">Save Calibration</button>
            </div>
        </div>
    </div>
    <div id="toast"></div>
    <script>
        function showToast(msg, type) {
            const t = document.getElementById('toast');
            t.textContent = msg;
            t.className = 'show ' + (type || '');
            setTimeout(() => { t.className = ''; }, 3000);
        }

        let socket = new WebSocket('ws://' + window.location.hostname + ':81/');
        let userChangedCalibration = false;
        let lastSavedCalibration = {};

        socket.onopen = () => {
            console.log('WebSocket connected');
            loadCalibrationSettings();
        };
        
        socket.onmessage = (event) => {
            try {
                let data = JSON.parse(event.data);
                if (data.internalRawTemp !== undefined) {
                    document.getElementById('internal-raw-temp').textContent = data.internalRawTemp.toFixed(2) + ' °C';
                }
                if (data.externalRawTemp !== undefined) {
                    document.getElementById('external-raw-temp').textContent = data.externalRawTemp.toFixed(2) + ' °C';
                }
            } catch (e) {
                console.error('WebSocket error:', e);
            }
        };

        socket.onclose = () => console.log('WebSocket disconnected');
        socket.onerror = () => console.log('WebSocket error');

        function goBack() {
          if (userChangedCalibration) {
                if (!confirm('You have unsaved changes. Are you sure you want to leave?')) {
                    return;
                }
            }
            window.history.back();
        }

        function checkForCalibrationChanges() {
            const currentInternal = parseFloat(document.getElementById('internal-calibration').value);
            const currentExternal = parseFloat(document.getElementById('external-calibration').value);
            
            const hasChanges = (
                Math.abs(currentInternal - lastSavedCalibration.internalOffset) > 0.001 ||
                Math.abs(currentExternal - lastSavedCalibration.externalOffset) > 0.001
            );
            
            userChangedCalibration = hasChanges;
            updateCalibrationUI();
        }

        function updateCalibrationUI() {
            const saveButton = document.getElementById('save-calibration-settings');
            const calibrationSection = document.querySelector('.calibration-section');
            
            if (userChangedCalibration) {
                saveButton.classList.add('changes-pending');
                saveButton.textContent = 'Save Changes';
                calibrationSection.classList.add('changed-indicator');
            } else {
                saveButton.classList.remove('changes-pending');
                saveButton.textContent = 'Save Calibration';
                calibrationSection.classList.remove('changed-indicator');
            }
        }

        function loadCalibrationSettings() {
            fetch('/calibration/settings')
                .then(response => response.json())
                .then(data => {
                    // Only update if user hasn't made changes
                    if (!userChangedCalibration) {
                        document.getElementById('internal-calibration').value = data.internalOffset.toFixed(2);
                        document.getElementById('external-calibration').value = data.externalOffset.toFixed(2);
                    }
                    
                    // Always update the saved state
                    lastSavedCalibration = {
                        internalOffset: data.internalOffset,
                        externalOffset: data.externalOffset
                    };
                })
                .catch(error => {
                    console.error('Error loading calibration settings:', error);
                });
        }

        function saveCalibrationSettings() {
            const internalOffset = parseFloat(document.getElementById('internal-calibration').value);
            const externalOffset = parseFloat(document.getElementById('external-calibration').value);
            
            if (internalOffset < -10 || internalOffset > 10 || externalOffset < -10 || externalOffset > 10) {
                showToast('Calibration offsets must be between -10°C and +10°C!', 'error');
                return;
            }
            
            const settings = {
                internalOffset: internalOffset,
                externalOffset: externalOffset
            };
            
            fetch('/calibration/save', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify(settings)
            })
            .then(response => {
                if (!response.ok) {
                    return response.json().then(data => { throw new Error(data.error); });
                }
                return response.json();
            })
            .then(data => {
                showToast('Calibration settings saved successfully!', 'success');
                // Update saved state and reset change tracking
                lastSavedCalibration = { ...settings };
                userChangedCalibration = false;
                updateCalibrationUI();
                loadCalibrationSettings();
            })
            .catch(error => {
                showToast('Failed to save calibration: ' + error.message, 'error');
            });
        }

        function loadRawSensorData() {
            fetch('/temperature/raw')
                .then(response => response.json())
                .then(data => {
                    if (data.internalRaw !== undefined) {
                        document.getElementById('internal-raw-temp').textContent = data.internalRaw.toFixed(2) + ' °C';
                    }
                    if (data.externalRaw !== undefined) {
                        document.getElementById('external-raw-temp').textContent = data.externalRaw.toFixed(2) + ' °C';
                    }
                })
                .catch(error => {
                    console.error('Error loading raw sensor data:', error);
                });
        }
        
        document.getElementById('internal-calibration').addEventListener('input', checkForCalibrationChanges);
        document.getElementById('external-calibration').addEventListener('input', checkForCalibrationChanges);

        // Initialize everything
        loadCalibrationSettings();
        loadRawSensorData();
        
        setInterval(() => {
            if (!userChangedCalibration) {
                loadCalibrationSettings();
            }
            loadRawSensorData();
        }, 10000);
    </script>
</body>
</html>
)html";

const char tempschedules[] PROGMEM = R"html(
<!DOCTYPE html>
<html>
<head>
    <link rel="icon" type="image/png" href="/favicon.png">
    <link rel="shortcut icon" type="image/png" href="/favicon.png">
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Temporary Schedules</title>
    <style>
        :root {
            --primary-color: #1976D2;
            --primary-dark: #0D47A1;
            --primary-light: #BBDEFB;
            --accent-color: #03A9F4;
            --success-color: #4CAF50;
            --warning-color: #FFC107;
            --error-color: #F44336;
            --text-color: #333;
            --text-light: #757575;
            --background-color: #f5f7fa;
            --card-color: #ffffff;
            --border-radius: 8px;
            --shadow: 0 2px 10px rgba(0,0,0,0.1);
            --transition: all 0.3s ease;
        }

        * {
            box-sizing: border-box;
            margin: 0;
            padding: 0;
        }

        body {
            margin: 0;
            padding: 0;
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background-color: var(--background-color);
            color: var(--text-color);
            line-height: 1.6;
        }

        header {
            background: linear-gradient(135deg, var(--primary-color), var(--primary-dark));
            color: white;
            padding: 20px;
            text-align: center;
            box-shadow: 0 2px 10px rgba(0,0,0,0.1);
            position: relative;
            z-index: 10;
            margin-bottom: 30px;
        }

        header h1 {
            margin: 0;
            font-size: 2rem;
            letter-spacing: 0.5px;
        }

        .container {
            padding: 20px;
            max-width: 1200px;
            margin: auto;
        }

        .temp-schedule-form {
            background-color: var(--card-color);
            padding: 25px;
            border-radius: var(--border-radius);
            box-shadow: var(--shadow);
            margin-bottom: 25px;
            transition: var(--transition);
        }

        .temp-schedule-form:hover {
            box-shadow: 0 5px 15px rgba(0,0,0,0.15);
        }

        .loading-message {
            text-align: center;
            padding: 20px;
            color: var(--text-light);
            font-style: italic;
        }

        .button {
            display: inline-block;
            padding: 12px 24px;
            background-color: var(--primary-color);
            color: white;
            text-decoration: none;
            border-radius: var(--border-radius);
            margin: 5px 0 20px 0;
            transition: var(--transition);
            border: none;
            cursor: pointer;
            font-size: 1rem;
            font-weight: 500;
            box-shadow: var(--shadow);
            text-align: center;
        }

        .button:hover {
            background-color: var(--primary-dark);
            transform: translateY(-2px);
            box-shadow: 0 4px 10px rgba(0,0,0,0.15);
        }

        .button:active {
            transform: translateY(1px);
        }

        .header-actions {
            margin-bottom: 20px;
            overflow: hidden;
            display: flex;
            justify-content: space-between;
            align-items: center;
            flex-wrap: wrap;
            gap: 10px;
        }

        .card {
            background-color: var(--card-color);
            padding: 25px;
            border-radius: var(--border-radius);
            box-shadow: var(--shadow);
            margin-bottom: 25px;
            transition: var(--transition);
        }

        .card:hover {
            box-shadow: 0 5px 15px rgba(0,0,0,0.15);
        }

        .card h3 {
            color: var(--primary-color);
            margin-bottom: 15px;
            font-size: 1.5rem;
            border-bottom: 2px solid var(--primary-light);
            padding-bottom: 10px;
        }

        .schedule-form, .log-section {
            background-color: var(--card-color);
            padding: 25px;
            border-radius: var(--border-radius);
            box-shadow: var(--shadow);
            margin-bottom: 25px;
            transition: var(--transition);
        }

        .schedule-form:hover, .log-section:hover {
            box-shadow: 0 5px 15px rgba(0,0,0,0.15);
        }

        .schedule-form h3, .log-section h3 {
            color: var(--primary-color);
            margin-bottom: 15px;
            font-size: 1.5rem;
            border-bottom: 2px solid var(--primary-light);
            padding-bottom: 10px;
        }

        .schedule-form label {
            display: block;
            margin-bottom: 8px;
            font-weight: 500;
            color: var(--text-color);
        }

        .schedule-form input, .schedule-form select {
            width: 100%;
            padding: 12px;
            margin: 8px 0 20px 0;
            border-radius: var(--border-radius);
            border: 1px solid #ddd;
            font-size: 1rem;
            transition: var(--transition);
        }

        .schedule-form input:focus, .schedule-form select:focus {
            outline: none;
            border-color: var(--primary-color);
            box-shadow: 0 0 0 3px var(--primary-light);
        }

        .schedule-form select {
            appearance: none;
            background-color: #fff;
            background-image: url('data:image/svg+xml;utf8,<svg fill="%23333" height="24" viewBox="0 0 24 24" width="24" xmlns="http://www.w3.org/2000/svg"><path d="M7 10l5 5 5-5z"/><path d="M0 0h24v24H0z" fill="none"/></svg>');
            background-repeat: no-repeat;
            background-position: right 10px center;
            padding-right: 40px;
            cursor: pointer;
        }

        .schedule-form button {
            width: 100%;
            padding: 12px;
            background-color: var(--primary-color);
            color: white;
            border: none;
            border-radius: var(--border-radius);
            font-size: 1.1rem;
            cursor: pointer;
            transition: var(--transition);
            margin-top: 10px;
        }

        .schedule-form button:hover {
            background-color: var(--primary-dark);
            transform: translateY(-2px);
            box-shadow: 0 4px 10px rgba(0,0,0,0.15);
        }

        .schedule-form button:active {
            transform: translateY(1px);
        }

        .schedule-table {
            width: 100%;
            border-collapse: separate;
            border-spacing: 0;
            margin-top: 20px;
            overflow: hidden;
            border-radius: var(--border-radius);
            box-shadow: var(--shadow);
        }

        .schedule-table {
            width: 100%;
            border: none;
            margin-top: 10px;
        }

        .schedule-table tbody {
            display: flex;
            flex-direction: column;
            gap: 15px;
        }

        .schedule-table tr:first-child {
            display: none;
        }

        .schedule-table tr {
            display: flex;
            flex-direction: row;
            align-items: center;
            background-color: #fff;
            border: 1px solid #ddd;
            border-radius: var(--border-radius);
            box-shadow: var(--shadow);
            padding: 15px 25px;
            transition: var(--transition);
            width: 100%;
            box-sizing: border-box;
            justify-content: space-between;
        }

        .schedule-table tr:hover {
            transform: translateY(-2px);
            box-shadow: 0 5px 15px rgba(0,0,0,0.1);
        }

        .schedule-table td {
            display: flex;
            flex-direction: column;
            align-items: flex-start;
            padding: 0 10px;
            border-bottom: none;
            text-align: left;
            font-size: 1rem;
            color: var(--text-color);
        }

        .schedule-table td:last-child {
            flex-direction: row;
            align-items: center;
            justify-content: flex-end;
            gap: 10px;
            padding: 0;
            margin: 0;
            min-width: 200px;
        }

        .schedule-table td:nth-child(1)::before { content: "ID"; font-weight: 600; color: var(--text-light); font-size: 0.85rem; margin-bottom: 5px; text-transform: uppercase; letter-spacing: 0.5px; }
        .schedule-table td:nth-child(2)::before { content: "Relay"; font-weight: 600; color: var(--text-light); font-size: 0.85rem; margin-bottom: 5px; text-transform: uppercase; letter-spacing: 0.5px; }
        .schedule-table td:nth-child(3)::before { content: "Start Time"; font-weight: 600; color: var(--text-light); font-size: 0.85rem; margin-bottom: 5px; text-transform: uppercase; letter-spacing: 0.5px; }
        .schedule-table td:nth-child(4)::before { content: "End Time"; font-weight: 600; color: var(--text-light); font-size: 0.85rem; margin-bottom: 5px; text-transform: uppercase; letter-spacing: 0.5px; }

        .action-button {
            flex: 1;
            padding: 12px 16px;
            margin: 0;
            border: none;
            border-radius: 20px;
            cursor: pointer;
            font-weight: 500;
            font-size: 0.95rem;
            transition: var(--transition);
            color: white;
            box-shadow: 0 2px 5px rgba(0,0,0,0.1);
            text-align: center;
            max-width: 140px;
        }

        .action-button:hover {
            transform: translateY(-2px);
            box-shadow: 0 4px 8px rgba(0,0,0,0.15);
        }
        
        .action-button:active {
            transform: translateY(1px);
        }

        .action-button.activate { background-color: var(--success-color); color: white; }
        .action-button.activate:hover { background-color: #388E3C; }
        
        .action-button.deactivate { background-color: var(--warning-color); color: #333; }
        .action-button.deactivate:hover { background-color: #FFA000; }
        
        .action-button.delete { background-color: var(--error-color); color: white; }
        .action-button.delete:hover { background-color: #D32F2F; }

        #errorSection {
            text-align: center;
            margin: 20px 0;
            color: white;
            background-color: var(--error-color);
            padding: 20px;
            border-radius: var(--border-radius);
            display: none;
            animation: pulse 2s infinite;
            box-shadow: 0 4px 10px rgba(244, 67, 54, 0.3);
        }

        @keyframes pulse {
            0% { box-shadow: 0 0 0 0 rgba(244, 67, 54, 0.4); }
            70% { box-shadow: 0 0 0 10px rgba(244, 67, 54, 0); }
            100% { box-shadow: 0 0 0 0 rgba(244, 67, 54, 0); }
        }

        #clearErrorBtn {
            padding: 12px 24px;
            background-color: white;
            color: var(--error-color);
            border: none;
            border-radius: 4px;
            cursor: pointer;
            font-size: 1rem;
            font-weight: 500;
            margin-top: 15px;
            transition: var(--transition);
        }

        #clearErrorBtn:hover {
            background-color: #f5f5f5;
            transform: scale(1.05);
        }

        #logSection {
            display: none;
        }

        pre {
            background-color: #f8f9fa;
            padding: 15px;
            border-radius: var(--border-radius);
            max-height: 300px;
            overflow-y: auto;
            font-family: 'Consolas', 'Monaco', monospace;
            border: 1px solid #eee;
            white-space: pre-wrap;
        }

        .error {
            color: var(--error-color);
            display: none;
            margin-top: -15px;
            margin-bottom: 12px;
            font-size: 0.9rem;
            transition: var(--transition);
        }

        .error2 {
            color: var(--error-color);
            display: none;
            margin-top: 2px;
            margin-bottom: 12px;
            font-size: 0.9rem;
            transition: var(--transition);
        }

        .ready {
            background-color: var(--success-color);
            cursor: pointer;
        }

        #successDialog {
            display: none;
            position: fixed;
            z-index: 1000;
            left: 50%;
            top: 50%;
            transform: translate(-50%, -50%);
            background-color: var(--success-color);
            color: white;
            padding: 25px;
            border-radius: var(--border-radius);
            box-shadow: 0 5px 20px rgba(0,0,0,0.3);
            text-align: center;
            min-width: 300px;
            animation: fadeIn 0.3s ease-out;
        }

        @keyframes fadeIn {
            from { opacity: 0; transform: translate(-50%, -60%); }
            to { opacity: 1; transform: translate(-50%, -50%); }
        }

        #successDialog p {
            font-size: 1.2rem;
            margin-bottom: 15px;
        }

        #successDialog button {
            margin-top: 15px;
            padding: 10px 25px;
            background-color: white;
            color: var(--success-color);
            border: none;
            border-radius: 4px;
            cursor: pointer;
            font-size: 1rem;
            font-weight: 500;
            transition: var(--transition);
        }

        #successDialog button:hover {
            background-color: #f5f5f5;
            transform: scale(1.05);
        }

        .day-checkboxes {
            display: flex;
            flex-wrap: wrap;
            gap: 10px;
            margin-bottom: 20px;
        }

        .day-checkboxes label {
            display: inline-flex;
            align-items: center;
            position: relative;
            padding-left: 30px;
            cursor: pointer;
            font-size: 1rem;
            user-select: none;
            margin-right: 15px;
            margin-bottom: 5px;
        }

        .day-checkboxes input {
            position: absolute;
            opacity: 0;
            cursor: pointer;
            height: 0;
            width: 0;
        }

        .day-checkboxes .checkmark {
            position: absolute;
            top: 0;
            left: 0;
            height: 20px;
            width: 20px;
            background-color: #eee;
            border-radius: 4px;
            transition: var(--transition);
        }

        .day-checkboxes label:hover input ~ .checkmark {
            background-color: #ccc;
        }

        .day-checkboxes input:checked ~ .checkmark {
            background-color: var(--primary-color);
        }

        .day-checkboxes .checkmark:after {
            content: "";
            position: absolute;
            display: none;
            left: 7px;
            top: 3px;
            width: 5px;
            height: 10px;
            border: solid white;
            border-width: 0 2px 2px 0;
            transform: rotate(45deg);
        }

        .day-checkboxes input:checked ~ .checkmark:after {
            display: block;
        }
        
        .limitation-note {
            margin-top: 10px;
            padding: 10px;
            background-color: #fff3cd;
            border-left: 4px solid var(--warning-color);
            color: #856404;
            border-radius: 4px;
            font-size: 0.9rem;
        }

        @media (max-width: 768px) {
            .buttons {
                grid-template-columns: repeat(auto-fit, minmax(150px, 1fr));
            }
            
            #time {
                font-size: 2rem;
            }
            
            #day, #date {
                font-size: 1.2rem;
            }
            
            .day-checkboxes {
                flex-direction: column;
                gap: 5px;
            }
            
            .day-checkboxes label {
                margin-right: 0;
            }
            
            .card {
                padding: 15px;
            }
            .schedule-form {
                padding: 15px;
            }
            .schedule-table tr {
                flex-direction: column;
                padding: 15px;
            }
            .schedule-table td {
                flex-direction: row;
                justify-content: space-between;
                align-items: center;
                width: 100%;
                border-bottom: 1px solid #eee;
                padding: 12px 0;
                text-align: right;
            }
            .schedule-table td:last-child {
                border-bottom: none;
                justify-content: center;
                padding-top: 15px;
                margin-top: 5px;
            }
            .schedule-table td::before {
                margin-bottom: 0 !important;
            }
            
            .action-button {
                padding: 10px;
                margin: 3px;
                font-size: 0.9rem;
                flex: 1;
                max-width: 120px;
            }

            .container {
                padding: 10px;
            }
        }


        #errorSection h3 {
            color: #fff;
            border-bottom: 2px solid rgba(255,255,255,0.3);
            margin-bottom: 15px;
            padding-bottom: 10px;
        }
        .error-row {
            display: flex;
            justify-content: space-between;
            align-items: center;
            background: rgba(0,0,0,0.15);
            padding: 10px 15px;
            border-radius: 8px;
            margin-bottom: 10px;
            font-size: 1.1rem;
        }
        .dismiss-btn {
            background-color: #fff;
            color: var(--error-color);
            padding: 5px 15px;
            font-size: 0.9rem;
            margin: 0;
        }
        .dismiss-btn:hover {
            background-color: #f1f1f1;
            transform: scale(1.05);
        }
        .dismiss-all {
            background-color: transparent;
            color: #fff;
            border: 2px solid #fff;
            width: 100%;
        }
        .dismiss-all:hover {
            background-color: #fff;
            color: var(--error-color);
        }

        /* Toast */
        #toast {
            position: fixed;
            bottom: 28px;
            left: 50%;
            transform: translateX(-50%) translateY(80px);
            background: #323232;
            color: white;
            padding: 12px 28px;
            border-radius: 24px;
            font-size: 0.95rem;
            opacity: 0;
            transition: all 0.35s ease;
            z-index: 1000;
            pointer-events: none;
        }
        #toast.show { opacity: 1; transform: translateX(-50%) translateY(0); }
        #toast.success { background: var(--success-color); }
        #toast.error   { background: var(--error-color); }
    </style>
</head>
<body>
    <header>
        <h1>Temporary Schedules</h1>
        <p>One-time schedules that auto-delete after running</p>
    </header>
    <div class="container">
        <div class="header-actions">
            <button onclick="goBack()" class="button">Back to Dashboard</button>
        </div>

        <div class="schedule-form">
            <h3>Add Temporary Schedule (One-time only)</h3>
            <div class="limitation-note">
                <strong>Note:</strong> Each relay can have a maximum of 2 temporary schedules at a time.
            </div>
            <label for="tempRelaySelect">Select Relay:</label>
            <select id="tempRelaySelect">
                <option value="" disabled selected>Select Relay</option>
                <option value="1">WaveMaker</option>
                <option value="2">Light</option>
                <option value="3">Air Pump</option>
            </select>
            <div id="tempRelayError" class="error">Please select a relay.</div>

            <label for="tempOnTime">Start Time (optional):</label>
            <input type="time" id="tempOnTime" placeholder="On Time">

            <label for="tempOffTime">End Time (optional):</label>
            <input type="time" id="tempOffTime" placeholder="Off Time">
            <div id="tempTimeError" class="error">Please enter at least start time or end time.</div>

            <button id="addTempScheduleBtn" onclick="addTemporarySchedule()">Add Temporary Schedule</button>
        </div>

        <div class="card">
            <h3>Active Temporary Schedules</h3>
            <table class="schedule-table" id="tempScheduleTable">
                <tr>
                    <th>ID</th>
                    <th>Relay</th>
                    <th>Start Time</th>
                    <th>End Time</th>
                    <th>Action</th>
                </tr>
            </table>
        </div>
    </div>
    <div id="toast"></div>
    <script>
    function showToast(msg, type) {
        const t = document.getElementById('toast');
        t.textContent = msg;
        t.className = 'show ' + (type || '');
        setTimeout(() => { t.className = ''; }, 3000);
    }

    function goBack() {
        window.history.back();
    }

    function addTemporarySchedule() {
        document.getElementById('tempRelayError').style.display = 'none';
        document.getElementById('tempTimeError').style.display = 'none';

        const relay = document.getElementById('tempRelaySelect').value;
        const onTime = document.getElementById('tempOnTime').value;
        const offTime = document.getElementById('tempOffTime').value;
        
        let hasError = false;

        if (relay === "") {
            document.getElementById('tempRelayError').style.display = 'block';
            hasError = true;
        }
        
        if (!onTime && !offTime) {
            document.getElementById('tempTimeError').style.display = 'block';
            hasError = true;
        }
        
        if (hasError) {
            return;
        }

        let requestBody = { relay };
        if (onTime) requestBody.onTime = onTime;
        if (offTime) requestBody.offTime = offTime;

        fetch('/temp-schedule/add', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(requestBody)
        })
        .then(response => {
            if (!response.ok) {
                return response.json().then(data => { throw new Error(data.error); });
            }
            return response.json();
        })
        .then(() => { 
            loadTemporarySchedules(); 
            checkErrorStatus(); 
            document.getElementById('tempRelaySelect').value = '';
            document.getElementById('tempOnTime').value = '';
            document.getElementById('tempOffTime').value = '';
            showToast('Temporary schedule added successfully!', 'success');
        })
        .catch(error => { 
            showToast('Failed to add temporary schedule: ' + error.message, 'error'); 
            checkErrorStatus(); 
        });
    }

    function deleteTemporarySchedule(id) {
        fetch('/temp-schedule/delete?id=' + id, { method: 'DELETE', headers: { 'Content-Type': 'application/json' } })
            .then(response => response.ok ? response.json() : { status: 'error' })
            .then(data => { 
                if (data.status === 'success') { 
                    loadTemporarySchedules(); 
                    checkErrorStatus(); 
                    showToast('Temporary schedule deleted', 'success');
                } else { 
                    throw new Error('Failed to delete temporary schedule'); 
                } 
            })
            .catch(error => { 
                showToast('Failed to delete temporary schedule: ' + error.message, 'error'); 
                checkErrorStatus(); 
            });
    }

    function loadTemporarySchedules() {
        fetch('/temp-schedules')
            .then(response => response.json())
            .then(schedules => {
                const table = document.getElementById('tempScheduleTable');
                table.innerHTML = `<tr>
                    <th>ID</th>
                    <th>Relay</th>
                    <th>Start Time</th>
                    <th>End Time</th>
                    <th>Action</th>
                </tr>`;
                
                schedules.forEach(schedule => {
                    const row = table.insertRow();
                    let relayName = "Unknown";
                    if (schedule.relay == 1) relayName = "WaveMaker";
                    else if (schedule.relay == 2) relayName = "Light";
                    else if (schedule.relay == 3) relayName = "Air Pump";
                    
                    row.insertCell(0).textContent = schedule.id;
                    row.insertCell(1).textContent = relayName;
                    
                    let startTime = schedule.hasOnTime ? 
                        `${String(schedule.onHour).padStart(2, '0')}:${String(schedule.onMinute).padStart(2, '0')}` : 
                        'Not set';
                    row.insertCell(2).textContent = startTime;
                    
                    let endTime = schedule.hasOffTime ? 
                        `${String(schedule.offHour).padStart(2, '0')}:${String(schedule.offMinute).padStart(2, '0')}` : 
                        'Not set';
                    row.insertCell(3).textContent = endTime;
                    
                    const actionCell = row.insertCell(4);
                    const deleteBtn = document.createElement('button');
                    deleteBtn.textContent = 'Delete';
                    deleteBtn.className = 'action-button delete';
                    deleteBtn.onclick = () => deleteTemporarySchedule(schedule.id);
                    actionCell.appendChild(deleteBtn);
                });
            })
            .catch(() => checkErrorStatus());
    }

    function checkErrorStatus() {
        fetch('/error/status')
            .then(response => response.json())
            .then(data => {
                console.log('Error status checked:', data.hasError);
            })
            .catch(() => {
                console.log('Failed to check error status');
            });
    }

    loadTemporarySchedules();
    setInterval(loadTemporarySchedules, 5000);
</script>
</body>
</html>
)html";

const char mainSchedules[] PROGMEM = R"html(
<!DOCTYPE html>
<html>
<head>
    <link rel="icon" type="image/png" href="/favicon.png">
    <link rel="shortcut icon" type="image/png" href="/favicon.png">
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Schedules</title>
    <style>
        :root {
            --primary-color: #1976D2;
            --primary-dark: #0D47A1;
            --primary-light: #BBDEFB;
            --accent-color: #03A9F4;
            --success-color: #4CAF50;
            --warning-color: #FFC107;
            --error-color: #F44336;
            --text-color: #333;
            --text-light: #757575;
            --background-color: #f5f7fa;
            --card-color: #ffffff;
            --border-radius: 8px;
            --shadow: 0 2px 10px rgba(0,0,0,0.1);
            --transition: all 0.3s ease;
        }

        * {
            box-sizing: border-box;
            margin: 0;
            padding: 0;
        }

        body {
            margin: 0;
            padding: 0;
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background-color: var(--background-color);
            color: var(--text-color);
            line-height: 1.6;
        }

        header {
            background: linear-gradient(135deg, var(--primary-color), var(--primary-dark));
            color: white;
            padding: 20px;
            text-align: center;
            box-shadow: 0 2px 10px rgba(0,0,0,0.1);
            position: relative;
            z-index: 10;
            margin-bottom: 30px;
        }

        header h1 {
            margin: 0;
            font-size: 2rem;
            letter-spacing: 0.5px;
        }

        .container {
            padding: 20px;
            max-width: 1200px;
            margin: auto;
        }

        .button {
            display: inline-block;
            padding: 12px 24px;
            background-color: var(--primary-color);
            color: white;
            text-decoration: none;
            border-radius: var(--border-radius);
            margin: 5px 0 20px 0;
            transition: var(--transition);
            border: none;
            cursor: pointer;
            font-size: 1rem;
            font-weight: 500;
            box-shadow: var(--shadow);
            text-align: center;
        }

        .button:hover {
            background-color: var(--primary-dark);
            transform: translateY(-2px);
            box-shadow: 0 4px 10px rgba(0,0,0,0.15);
        }

        .button:active {
            transform: translateY(1px);
        }

        .header-actions {
            margin-bottom: 20px;
            overflow: hidden;
            display: flex;
            justify-content: space-between;
            align-items: center;
            flex-wrap: wrap;
            gap: 10px;
        }

        .schedule-form, .log-section {
            background-color: var(--card-color);
            padding: 25px;
            border-radius: var(--border-radius);
            box-shadow: var(--shadow);
            margin-bottom: 25px;
            transition: var(--transition);
        }

        .schedule-form:hover, .log-section:hover {
            box-shadow: 0 5px 15px rgba(0,0,0,0.15);
        }

        .schedule-form h3, .log-section h3 {
            color: var(--primary-color);
            margin-bottom: 15px;
            font-size: 1.5rem;
            border-bottom: 2px solid var(--primary-light);
            padding-bottom: 10px;
        }

        .schedule-form label {
            display: block;
            margin-bottom: 8px;
            font-weight: 500;
            color: var(--text-color);
        }

        .schedule-form input, .schedule-form select {
            width: 100%;
            padding: 12px;
            margin: 8px 0 20px 0;
            border-radius: var(--border-radius);
            border: 1px solid #ddd;
            font-size: 1rem;
            transition: var(--transition);
        }

        .schedule-form input:focus, .schedule-form select:focus {
            outline: none;
            border-color: var(--primary-color);
            box-shadow: 0 0 0 3px var(--primary-light);
        }

        .schedule-form select {
            appearance: none;
            background-color: #fff;
            background-image: url('data:image/svg+xml;utf8,<svg fill="%23333" height="24" viewBox="0 0 24 24" width="24" xmlns="http://www.w3.org/2000/svg"><path d="M7 10l5 5 5-5z"/><path d="M0 0h24v24H0z" fill="none"/></svg>');
            background-repeat: no-repeat;
            background-position: right 10px center;
            padding-right: 40px;
            cursor: pointer;
        }

        .schedule-form button {
            width: 100%;
            padding: 12px;
            background-color: var(--primary-color);
            color: white;
            border: none;
            border-radius: var(--border-radius);
            font-size: 1.1rem;
            cursor: pointer;
            transition: var(--transition);
            margin-top: 10px;
        }

        .schedule-form button:hover {
            background-color: var(--primary-dark);
            transform: translateY(-2px);
            box-shadow: 0 4px 10px rgba(0,0,0,0.15);
        }

        .schedule-form button:active {
            transform: translateY(1px);
        }

        .schedule-table {
            width: 100%;
            border: none;
            margin-top: 20px;
        }

        .schedule-table tbody {
            display: flex;
            flex-direction: column;
            gap: 15px;
        }

        .schedule-table tr:first-child {
            display: none;
        }

        .schedule-table tr {
            display: flex;
            flex-direction: row;
            align-items: center;
            background-color: #fff;
            border: 1px solid #ddd;
            border-radius: var(--border-radius);
            box-shadow: var(--shadow);
            padding: 15px 25px;
            transition: var(--transition);
            width: 100%;
            box-sizing: border-box;
            justify-content: space-between;
        }

        .schedule-table tr:hover {
            transform: translateY(-2px);
            box-shadow: 0 5px 15px rgba(0,0,0,0.1);
        }

        .schedule-table td {
            display: flex;
            flex-direction: column;
            align-items: flex-start;
            padding: 0 10px;
            border-bottom: none;
            text-align: left;
            font-size: 1rem;
            color: var(--text-color);
        }

        .schedule-table td:last-child {
            flex-direction: row;
            align-items: center;
            justify-content: flex-end;
            gap: 10px;
            padding: 0;
            margin: 0;
            min-width: 200px;
        }

        .schedule-table td:nth-child(1)::before { content: "Relay"; font-weight: 600; color: var(--text-light); font-size: 0.85rem; margin-bottom: 5px; text-transform: uppercase; letter-spacing: 0.5px; }
        .schedule-table td:nth-child(2)::before { content: "On Time"; font-weight: 600; color: var(--text-light); font-size: 0.85rem; margin-bottom: 5px; text-transform: uppercase; letter-spacing: 0.5px; }
        .schedule-table td:nth-child(3)::before { content: "Off Time"; font-weight: 600; color: var(--text-light); font-size: 0.85rem; margin-bottom: 5px; text-transform: uppercase; letter-spacing: 0.5px; }
        .schedule-table td:nth-child(4)::before { content: "Days"; font-weight: 600; color: var(--text-light); font-size: 0.85rem; margin-bottom: 5px; text-transform: uppercase; letter-spacing: 0.5px; }
        .schedule-table td:nth-child(5)::before { content: "Status"; font-weight: 600; color: var(--text-light); font-size: 0.85rem; margin-bottom: 5px; text-transform: uppercase; letter-spacing: 0.5px; }

        .action-button {
            flex: 1;
            padding: 12px 16px;
            margin: 0;
            border: none;
            border-radius: 20px;
            cursor: pointer;
            font-weight: 500;
            font-size: 0.95rem;
            transition: var(--transition);
            color: white;
            box-shadow: 0 2px 5px rgba(0,0,0,0.1);
            text-align: center;
            max-width: 140px;
        }

        .action-button:hover {
            transform: translateY(-2px);
            box-shadow: 0 4px 8px rgba(0,0,0,0.15);
        }
        
        .action-button:active {
            transform: translateY(1px);
        }

        .action-button.activate { background-color: var(--success-color); color: white; }
        .action-button.activate:hover { background-color: #388E3C; }
        
        .action-button.deactivate { background-color: var(--warning-color); color: #333; }
        .action-button.deactivate:hover { background-color: #FFA000; }
        
        .action-button.delete { background-color: var(--error-color); color: white; }
        .action-button.delete:hover { background-color: #D32F2F; }

        #errorSection {
            text-align: center;
            margin: 20px 0;
            color: white;
            background-color: var(--error-color);
            padding: 20px;
            border-radius: var(--border-radius);
            display: none;
            animation: pulse 2s infinite;
            box-shadow: 0 4px 10px rgba(244, 67, 54, 0.3);
        }

        @keyframes pulse {
            0% { box-shadow: 0 0 0 0 rgba(244, 67, 54, 0.4); }
            70% { box-shadow: 0 0 0 10px rgba(244, 67, 54, 0); }
            100% { box-shadow: 0 0 0 0 rgba(244, 67, 54, 0); }
        }

        #clearErrorBtn {
            padding: 12px 24px;
            background-color: white;
            color: var(--error-color);
            border: none;
            border-radius: 4px;
            cursor: pointer;
            font-size: 1rem;
            font-weight: 500;
            margin-top: 15px;
            transition: var(--transition);
        }

        #clearErrorBtn:hover {
            background-color: #f5f5f5;
            transform: scale(1.05);
        }

        #logSection {
            display: none;
        }

        pre {
            background-color: #f8f9fa;
            padding: 15px;
            border-radius: var(--border-radius);
            max-height: 300px;
            overflow-y: auto;
            font-family: 'Consolas', 'Monaco', monospace;
            border: 1px solid #eee;
            white-space: pre-wrap;
        }

        .error {
            color: var(--error-color);
            display: none;
            margin-top: -15px;
            margin-bottom: 12px;
            font-size: 0.9rem;
            transition: var(--transition);
        }

        .error2 {
            color: var(--error-color);
            display: none;
            margin-top: 2px;
            margin-bottom: 12px;
            font-size: 0.9rem;
            transition: var(--transition);
        }

        .ready {
            background-color: var(--success-color);
            cursor: pointer;
        }

        #successDialog {
            display: none;
            position: fixed;
            z-index: 1000;
            left: 50%;
            top: 50%;
            transform: translate(-50%, -50%);
            background-color: var(--success-color);
            color: white;
            padding: 25px;
            border-radius: var(--border-radius);
            box-shadow: 0 5px 20px rgba(0,0,0,0.3);
            text-align: center;
            min-width: 300px;
            animation: fadeIn 0.3s ease-out;
        }

        @keyframes fadeIn {
            from { opacity: 0; transform: translate(-50%, -60%); }
            to { opacity: 1; transform: translate(-50%, -50%); }
        }

        #successDialog p {
            font-size: 1.2rem;
            margin-bottom: 15px;
        }

        #successDialog button {
            margin-top: 15px;
            padding: 10px 25px;
            background-color: white;
            color: var(--success-color);
            border: none;
            border-radius: 4px;
            cursor: pointer;
            font-size: 1rem;
            font-weight: 500;
            transition: var(--transition);
        }

        #successDialog button:hover {
            background-color: #f5f5f5;
            transform: scale(1.05);
        }

        .day-checkboxes {
            display: flex;
            flex-wrap: wrap;
            gap: 10px;
            margin-bottom: 20px;
        }

        .day-checkboxes label {
            display: inline-flex;
            align-items: center;
            position: relative;
            padding-left: 30px;
            cursor: pointer;
            font-size: 1rem;
            user-select: none;
            margin-right: 15px;
            margin-bottom: 5px;
        }

        .day-checkboxes input {
            position: absolute;
            opacity: 0;
            cursor: pointer;
            height: 0;
            width: 0;
        }

        .day-checkboxes .checkmark {
            position: absolute;
            top: 0;
            left: 0;
            height: 20px;
            width: 20px;
            background-color: #eee;
            border-radius: 4px;
            transition: var(--transition);
        }

        .day-checkboxes label:hover input ~ .checkmark {
            background-color: #ccc;
        }

        .day-checkboxes input:checked ~ .checkmark {
            background-color: var(--primary-color);
        }

        .day-checkboxes .checkmark:after {
            content: "";
            position: absolute;
            display: none;
            left: 7px;
            top: 3px;
            width: 5px;
            height: 10px;
            border: solid white;
            border-width: 0 2px 2px 0;
            transform: rotate(45deg);
        }

        .day-checkboxes input:checked ~ .checkmark:after {
            display: block;
        }

        @media (max-width: 768px) {

            .container {
                padding: 10px;
            }

            .buttons {
                grid-template-columns: repeat(auto-fit, minmax(150px, 1fr));
            }

            
            .day-checkboxes {
                flex-direction: column;
                gap: 5px;
            }
            
            .day-checkboxes label {
                margin-right: 0;
            }
            
            .card {
                padding: 15px;
            }
            .schedule-form {
                padding: 15px;
            }
            .schedule-table tr {
                flex-direction: column;
                padding: 15px;
            }
            .schedule-table td {
                flex-direction: row;
                justify-content: space-between;
                align-items: center;
                width: 100%;
                border-bottom: 1px solid #eee;
                padding: 12px 0;
                text-align: right;
            }
            .schedule-table td:last-child {
                border-bottom: none;
                justify-content: center;
                padding-top: 15px;
                margin-top: 5px;
            }
            .schedule-table td::before {
                margin-bottom: 0 !important;
            }
        }


        #errorSection h3 {
            color: #fff;
            border-bottom: 2px solid rgba(255,255,255,0.3);
            margin-bottom: 15px;
            padding-bottom: 10px;
        }
        .error-row {
            display: flex;
            justify-content: space-between;
            align-items: center;
            background: rgba(0,0,0,0.15);
            padding: 10px 15px;
            border-radius: 8px;
            margin-bottom: 10px;
            font-size: 1.1rem;
        }
        .dismiss-btn {
            background-color: #fff;
            color: var(--error-color);
            padding: 5px 15px;
            font-size: 0.9rem;
            margin: 0;
        }
        .dismiss-btn:hover {
            background-color: #f1f1f1;
            transform: scale(1.05);
        }
        .dismiss-all {
            background-color: transparent;
            color: #fff;
            border: 2px solid #fff;
            width: 100%;
        }
        .dismiss-all:hover {
            background-color: #fff;
            color: var(--error-color);
        }

        /* Toast */
        #toast {
            position: fixed;
            bottom: 28px;
            left: 50%;
            transform: translateX(-50%) translateY(80px);
            background: #323232;
            color: white;
            padding: 12px 28px;
            border-radius: 24px;
            font-size: 0.95rem;
            opacity: 0;
            transition: all 0.35s ease;
            z-index: 1000;
            pointer-events: none;
        }
        #toast.show { opacity: 1; transform: translateX(-50%) translateY(0); }
        #toast.success { background: var(--success-color); }
        #toast.error   { background: var(--error-color); }
    </style>
</head>
<body>
    <header>
        <h1>Schedules</h1>
        <p>Manage daily recurring schedules</p>
    </header>
    <div class="container">
        <div class="header-actions">
            <button onclick="goBack()" class="button">Back to Dashboard</button>
        </div>

        <div class="schedule-form">
            <h3>Add Schedule</h3>
            <label for="relaySelect">Select Relay:</label>
            <select id="relaySelect">
                <option value="" disabled selected>Select Relay</option>
                <option value="1">WaveMaker</option>
                <option value="2">Light</option>
                <option value="3">Air Pump</option>
            </select>
            <div id="relayError" class="error">Please select a relay.</div>

            <label for="onTime">Start Time:</label>
            <input type="time" id="onTime" placeholder="On Time">
            <div id="onTimeError" class="error">Please enter a start time.</div>

            <label for="offTime">End Time:</label>
            <input type="time" id="offTime" placeholder="Off Time">
            <div id="offTimeError" class="error">Please enter an end time.</div>

            <label>Select Days:</label>
            <div class="day-checkboxes">
                <label>
                    <input type="checkbox" value="0" class="dayCheckbox">
                    <span class="checkmark"></span> Sun
                </label>
                <label>
                    <input type="checkbox" value="1" class="dayCheckbox">
                    <span class="checkmark"></span> Mon
                </label>
                <label>
                    <input type="checkbox" value="2" class="dayCheckbox">
                    <span class="checkmark"></span> Tue
                </label>
                <label>
                    <input type="checkbox" value="3" class="dayCheckbox">
                    <span class="checkmark"></span> Wed
                </label>
                <label>
                    <input type="checkbox" value="4" class="dayCheckbox">
                    <span class="checkmark"></span> Thu
                </label>
                <label>
                    <input type="checkbox" value="5" class="dayCheckbox">
                    <span class="checkmark"></span> Fri
                </label>
                <label>
                    <input type="checkbox" value="6" class="dayCheckbox">
                    <span class="checkmark"></span> Sat
                </label>
            </div>
            <div id="dayError" class="error2">Please select at least one day.</div>

            <button id="addScheduleBtn" onclick="addSchedule()">Add Schedule</button>
        </div>
        <table class="schedule-table" id="scheduleTable">
            <tr>
                <th>Relay</th>
                <th>On Time</th>
                <th>Off Time</th>
                <th>Days</th>
                <th>Status</th>
                <th>Action</th>
            </tr>
        </table>

                <div id="successDialog">
        <p>Schedule added successfully!</p>
        <button onclick="closeSuccessDialog()">OK</button>
    </div>
    </div>
    <div id="toast"></div>
    <script>
        function showToast(msg, type) {
            const t = document.getElementById('toast');
            t.textContent = msg;
            t.className = 'show ' + (type || '');
            setTimeout(() => { t.className = ''; }, 3000);
        }

        function goBack() {
            window.history.back();
        }

                function addSchedule() {
            document.getElementById('relayError').style.display = 'none';
            document.getElementById('onTimeError').style.display = 'none';
            document.getElementById('offTimeError').style.display = 'none';

            const relay = document.getElementById('relaySelect').value;
            const onTime = document.getElementById('onTime').value;
            const offTime = document.getElementById('offTime').value;
            const dayCheckboxes = document.querySelectorAll('.dayCheckbox');
            let days = Array(7).fill(false);
            dayCheckboxes.forEach(cb => {
                if(cb.checked) {
                    days[parseInt(cb.value)] = true;
                }
            });
            let hasError = false;

            if (relay === "") {
                document.getElementById('relayError').style.display = 'block';
                hasError = true;
            }
            if (!onTime) {
                document.getElementById('onTimeError').style.display = 'block';
                hasError = true;
            }
            if (!offTime) {
                document.getElementById('offTimeError').style.display = 'block';
                hasError = true;
            }
            if (days.every(day => day === false)) {
                document.getElementById('dayError').style.display = 'block';
                hasError = true;
            } else {
                document.getElementById('dayError').style.display = 'none';
            }
            if (hasError) {
                return;
            }

            fetch('/schedule/add', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ relay, onTime, offTime, days })
            })
            .then(response => response.ok ? response.json() : response.json().then(data => { throw new Error(data.error); }))
            .then(() => { 
                loadSchedules(); 
                checkErrorStatus(); 
                showSuccessDialog(); // Show success dialog
            })
            .catch(error => { 
                showToast('Failed to add schedule: ' + error.message, 'error'); 
                checkErrorStatus(); 
            });
        }

        function checkErrorStatus() {
            fetch('/error/status')
                .then(response => response.json())
                .then(data => {
                    const errSec = document.getElementById('errorSection');
                    if (!errSec) return;
                    let activeErrors = data.activeErrors || 0;
                    if (activeErrors > 0) {
                                                let html = '<h3>System Errors Detected</h3>';
                        if (activeErrors & 1) html += '<p class="error-row"><span>WiFi Disconnected</span> <button class="button dismiss-btn" onclick="clearError(1)">Dismiss</button></p>';
                        if (activeErrors & 2) html += '<p class="error-row"><span>Time Sync Failed</span> <button class="button dismiss-btn" onclick="clearError(2)">Dismiss</button></p>';
                        if (activeErrors & 4) html += '<p class="error-row"><span>Internal Temp Sensor Failed</span> <button class="button dismiss-btn" onclick="clearError(4)">Dismiss</button></p>';
                        if (activeErrors & 8) html += '<p class="error-row"><span>External Temp Sensor Failed</span> <button class="button dismiss-btn" onclick="clearError(8)">Dismiss</button></p>';
                        html += '<div style="margin-top: 15px;"><button class="button dismiss-btn dismiss-all" onclick="clearError(\'all\')">Dismiss All</button></div>';
                        errSec.innerHTML = html;
                        errSec.style.display = 'block';
                    } else {
                        errSec.style.display = 'none';
                    }
                })
                .catch(() => {
                    const errSec = document.getElementById('errorSection');
                    if (errSec) errSec.style.display = 'none';
                });
        }

        function clearError(errId) {
            fetch('/error/clear', { 
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ error_id: errId })
            })
            .then(response => response.ok ? response.json() : { status: 'error' })
            .then(data => { 
                if (data.status === 'success') { 
                    checkErrorStatus(); 
                    if (errId === 'all') {
                        showToast('All errors dismissed', 'success');
                    } else {
                        showToast('Error dismissed', 'success');
                    }
                } 
                else { throw new Error('Failed to clear error'); }
            })
            .catch(error => { showToast('Failed to clear error: ' + error.message, 'error'); });
        }

        function showSuccessDialog() {
            document.getElementById('successDialog').style.display = 'block';
        }

        function closeSuccessDialog() {
            document.getElementById('successDialog').style.display = 'none';
        }

        function deleteSchedule(id) {
            fetch('/schedule/delete?id=' + id, { method: 'DELETE', headers: { 'Content-Type': 'application/json' } })
                .then(response => response.ok ? response.json() : { status: 'error' })
                .then(data => { 
                    if (data.status === 'success') { 
                        loadSchedules(); 
                        checkErrorStatus(); 
                        showToast('Schedule deleted successfully', 'success');
                    } else { 
                        throw new Error('Failed to delete schedule'); 
                    } 
                })
                .catch(error => { showToast('Failed to delete schedule: ' + error.message, 'error'); checkErrorStatus(); });
        }

        function loadSchedules() {
            fetch('/schedules')
                .then(response => response.json())
                .then(schedules => {
                    const table = document.getElementById('scheduleTable');
                    table.innerHTML = `<tr>
                        <th>Relay</th>
                        <th>On Time</th>
                        <th>Off Time</th>
                        <th>Days</th>
                        <th>Status</th>
                        <th>Action</th>
                    </tr>`;
                    let dayNames = ["Sun","Mon","Tue","Wed","Thu","Fri","Sat"];
                    schedules.forEach((schedule, index) => {
                        const row = table.insertRow();
                        let relayName = "Unknown";
                        if (schedule.relay == 1) relayName = "WaveMaker";
                        else if (schedule.relay == 2) relayName = "Light";
                        else if (schedule.relay == 3) relayName = "Air Pump";
                        
                        row.insertCell(0).textContent = relayName;
                        row.insertCell(1).textContent = `${String(schedule.onHour).padStart(2, '0')}:${String(schedule.onMinute).padStart(2, '0')}`;
                        row.insertCell(2).textContent = `${String(schedule.offHour).padStart(2, '0')}:${String(schedule.offMinute).padStart(2, '0')}`;
                        
                        const activeDays = [];
                        schedule.daysOfWeek.forEach((active, i) => {
                            if (active) activeDays.push(dayNames[i]);
                        });
                        row.insertCell(3).textContent = activeDays.join(", ");
                        
                        row.insertCell(4).textContent = schedule.enabled ? 'Active' : 'Inactive';
                        const actionCell = row.insertCell(5);
                        const toggleBtn = document.createElement('button');
                        toggleBtn.textContent = schedule.enabled ? 'Deactivate' : 'Activate';
                        toggleBtn.className = 'action-button ' + (schedule.enabled ? 'deactivate' : 'activate');
                        toggleBtn.onclick = () => toggleSchedule(index, !schedule.enabled);
                        
                        const deleteBtn = document.createElement('button');
                        deleteBtn.textContent = 'Delete';
                        deleteBtn.className = 'action-button delete';
                        deleteBtn.onclick = () => deleteSchedule(index);
                        
                        actionCell.appendChild(toggleBtn);
                        actionCell.appendChild(deleteBtn);
                    });
                })
                .catch(() => checkErrorStatus());
        }

        function toggleSchedule(id, enabled) {
            fetch('/schedule/update', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ id, enabled })
            })
            .then(response => response.ok ? response.json() : response.json().then(data => { throw new Error(data.error); }))
            .then(() => { 
                loadSchedules(); 
                checkErrorStatus(); 
                showToast(enabled ? 'Schedule activated' : 'Schedule deactivated', 'success');
            })
            .catch(error => { showToast('Failed to update schedule: ' + error.message, 'error'); checkErrorStatus(); });
        }

        function checkFields() {
            const relay = document.getElementById('relaySelect').value;
            const onTime = document.getElementById('onTime').value;
            const offTime = document.getElementById('offTime').value;
            const addBtn = document.getElementById('addScheduleBtn');
            const dayCheckboxes = document.querySelectorAll('.dayCheckbox');
            const oneDayChecked = Array.from(dayCheckboxes).some(cb => cb.checked);

            if (relay && onTime && offTime && oneDayChecked) {
                addBtn.classList.add('ready');
            } else {
                addBtn.classList.remove('ready');
            }
        }

        document.getElementById('relaySelect').addEventListener('change', checkFields);
        document.getElementById('onTime').addEventListener('input', checkFields);
        document.getElementById('offTime').addEventListener('input', checkFields);
        document.querySelectorAll('.dayCheckbox').forEach(cb => cb.addEventListener('change', checkFields));

        loadSchedules();
    </script>
</body>
</html>
)html";

unsigned long lastWifiConnectAttempt = 0;
const unsigned long WIFI_RECONNECT_INTERVAL = 30000;

void handleLogsPage() {
  server.send_P(200, "text/html", logsPage);
}

void handleTempCtrlPage() {
  server.send_P(200, "text/html", tempctrl);
}

void handleTempSchedulesPage() {
  server.send_P(200, "text/html", tempschedules);
}

void handleSchedulesPage() {
  server.send_P(200, "text/html", mainSchedules);
}

void loop() {
  vTaskDelete(NULL);
}

void networkLoop(void* parameter) {
  esp_task_wdt_add(NULL);
  unsigned long lastEmailAttempt = 0;
  const unsigned long EMAIL_RETRY_INTERVAL = 30000;
  unsigned long lastOledBlink = 0;
  unsigned long lastOledScheduleCheck = 0;
  for (;;) {
    resetWatchdog();
    apiServer.handleClient();
    handleTemperature();
    handleExternalTemperature();

    if (((activeErrors & ERR_TEMP_INT) || (activeErrors & ERR_TEMP_EXT)) && millis() - lastOledBlink >= 500) {
      oledBlinkState = !oledBlinkState;
      updateOLED();
      lastOledBlink = millis();
    }

    if (millis() - lastOledScheduleCheck >= 10000) {
      applyOledSchedule();
      lastOledScheduleCheck = millis();
    }

    if (WiFi.status() != WL_CONNECTED) {
      if (!(activeErrors & ERR_WIFI) && !(acknowledgedErrors & ERR_WIFI)) {
        storeLogEntry("WiFi disconnected");
        activeErrors |= ERR_WIFI;
      }
    } else {
      if ((activeErrors & ERR_WIFI) || (acknowledgedErrors & ERR_WIFI)) {
        storeLogEntry("WiFi reconnected");
        activeErrors &= ~ERR_WIFI;
        acknowledgedErrors &= ~ERR_WIFI;
      }
    }

    if (!validTimeSync) {
      unsigned long currentMillis = millis();
      if (currentMillis - lastNtpRetry >= NTP_RETRY_INTERVAL) {
        lastNtpRetry = currentMillis;
        attemptTimeSync();
        resetWatchdog();
      }
    }

    unsigned long currentTime = millis();

    if (!startupemail && (currentTime - lastEmailAttempt > EMAIL_RETRY_INTERVAL)) {
      lastEmailAttempt = currentTime;
      sendEmailWithLogs("Device is powered on");
      startupemail = true;

      struct tm timeinfo;
      if (getLocalTime(&timeinfo)) {
        last90MinCheck = timeinfo.tm_hour * 3600 + timeinfo.tm_min * 60 + timeinfo.tm_sec;
      }
    }

    if (pointemail && (currentTime - lastEmailAttempt > EMAIL_RETRY_INTERVAL)) {
      lastEmailAttempt = currentTime;
      sendEmailWithLogs("Status Check");
      pointemail = false;
    }
    delay(5);
  }
}

void mainLoop(void* parameter) {
  esp_task_wdt_add(NULL);
  for (;;) {
    server.handleClient();
    webSocket.loop();
    resetWatchdog();
    checkoverride1();
    checkoverride2();
    overrideLEDState();

    static unsigned long lastSecondCheck = 0;
    if (millis() - lastSecondCheck >= 1000) {
      lastSecondCheck = millis();

      if (validTimeSync) {
        checkSchedules();
        checkTemporarySchedules();

        struct tm timeinfo;
        if (getLocalTime(&timeinfo)) {
          unsigned long currentSeconds = timeinfo.tm_hour * 3600 + timeinfo.tm_min * 60 + timeinfo.tm_sec;

          // 90 minute check
          if (currentSeconds - last90MinCheck >= CHECK_90MIN_INTERVAL) {
            if (hasLaunchedSchedules) {
              String timeStr = String(timeinfo.tm_hour) + ":" + (timeinfo.tm_min < 10 ? "0" : "") + String(timeinfo.tm_min);
              storeLogEntry("Device is powered on at " + timeStr);
            }
            last90MinCheck = currentSeconds;

            if (startupemail && !pointemail) {
              pointemail = true;
            }
          }

          static int prevDay = -1;
          if (prevDay == -1) {
            prevDay = timeinfo.tm_mday;
          } else if (timeinfo.tm_mday != prevDay) {
            storeLogEntry("Day changed to: " + String(timeinfo.tm_mday));
            prevDay = timeinfo.tm_mday;
            last90MinCheck = 0;
          }
        }
      }
    }

    if (!hasLaunchedSchedules && validTimeSync) {
      checkScheduleslaunch();
      storeLogEntry("Startup Schedule Check Success");
      hasLaunchedSchedules = true;
      startupemail = false;

      if (WiFi.status() == WL_CONNECTED) {
        delay(100);
      }
    }

    delay(1);
  }
}

void checkSchedules() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return;
  }

  int hours = timeinfo.tm_hour;
  int minutes = timeinfo.tm_min;
  int seconds = timeinfo.tm_sec;
  int weekdayIndex = timeinfo.tm_wday;

  for (const Schedule& schedule : schedules) {
    if (!schedule.enabled || !schedule.daysOfWeek[weekdayIndex]) continue;

    if (hours == schedule.onHour && minutes == schedule.onMinute && seconds == 0) {
      if (schedule.relayNumber == 1) {
        if (!relay1State && !overrideRelay1) {
          activateRelay(1, false);
        }
      } else if (schedule.relayNumber == 2) {
        if (!relay2State && !overrideRelay2) {
          activateRelay(2, false);
        }
      } else if (schedule.relayNumber == 3) {
        if (!relay3State && !overrideRelay1) {
          activateRelay(3, false);
        }
      }
    } else if (hours == schedule.offHour && minutes == schedule.offMinute && seconds == 0) {
      if (schedule.relayNumber == 1) {
        if (relay1State && !overrideRelay1) {
          deactivateRelay(1, false);
        }
      } else if (schedule.relayNumber == 2) {
        if (relay2State && !overrideRelay2) {
          deactivateRelay(2, false);
        }
      } else if (schedule.relayNumber == 3) {
        if (relay3State && !overrideRelay1) {
          deactivateRelay(3, false);
        }
      }
    }
  }
}

void checkScheduleslaunch() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return;
  }

  int hours = timeinfo.tm_hour;
  int minutes = timeinfo.tm_min;
  unsigned long currentTime = hours * 60 + minutes;
  int weekdayIndex = timeinfo.tm_wday;

  bool relay1ShouldBeOn = false;
  bool relay2ShouldBeOn = false;
  bool relay3ShouldBeOn = false;

  for (const Schedule& schedule : schedules) {
    if (!schedule.enabled || !schedule.daysOfWeek[weekdayIndex]) {
      continue;
    }

    unsigned long onMinutes = schedule.onHour * 60 + schedule.onMinute;
    unsigned long offMinutes = schedule.offHour * 60 + schedule.offMinute;

    bool shouldBeOn = false;
    if (offMinutes > onMinutes) {
      shouldBeOn = (currentTime >= onMinutes && currentTime < offMinutes);
    } else {
      shouldBeOn = (currentTime >= onMinutes || currentTime < offMinutes);
    }

    if (schedule.relayNumber == 1) {
      relay1ShouldBeOn |= shouldBeOn;
    } else if (schedule.relayNumber == 2) {
      relay2ShouldBeOn |= shouldBeOn;
    } else if (schedule.relayNumber == 3) {
      relay3ShouldBeOn |= shouldBeOn;
    }
  }

  if (!overrideRelay1) {
    if (relay1ShouldBeOn) {
      if (!relay1State) {
        activateRelay(1, false);
        //storeLogEntry("Relay 1 activated by startup schedule check");
      }
    } else {
      if (relay1State) {
        deactivateRelay(1, false);
        //storeLogEntry("Relay 1 deactivated by startup schedule check");
      }
    }

    if (relay3ShouldBeOn) {
      if (!relay3State) {
        activateRelay(3, false);
        // storeLogEntry("Relay 3 activated by startup schedule check");
      }
    } else {
      if (relay3State) {
        deactivateRelay(3, false);
        // storeLogEntry("Relay 3 deactivated by startup schedule check");
      }
    }
  }

  if (!overrideRelay2) {
    if (relay2ShouldBeOn) {
      if (!relay2State) {
        activateRelay(2, false);
        // storeLogEntry("Relay 2 activated by startup schedule check");
      }
    } else {
      if (relay2State) {
        deactivateRelay(2, false);
        // storeLogEntry("Relay 2 deactivated by startup schedule check");
      }
    }
  }
}

void activateRelay(int relayNum, bool manual) {
  if (!manual && ((relayNum == 1 && overrideRelay1) || (relayNum == 2 && overrideRelay2) || (relayNum == 3 && overrideRelay1))) {
    storeLogEntry("Relay " + String(relayNum) + " is overridden. Activation skipped.");
    return;
  }

  switch (relayNum) {
    case 1:
      digitalWrite(relay1, LOW);
      relay1State = true;
      storeLogEntry("Wavemaker activated.");
      break;
    case 2:
      digitalWrite(relay4, LOW);
      relay4State = true;
      for (int i = 0; i < TOGGLE_COUNT; i++) {
        digitalWrite(relay2, HIGH);
        delay(TOGGLE_DELAY);
        digitalWrite(relay2, LOW);
        delay(TOGGLE_DELAY);
      }
      digitalWrite(relay2, LOW);
      relay2State = true;
      storeLogEntry("Lights activated.");
      break;
    case 3:
      digitalWrite(relay3, LOW);
      relay3State = true;
      storeLogEntry("Air Pump activated.");
      break;
  }
  broadcastRelayStates();
}

void deactivateRelay(int relayNum, bool manual) {
  if (!manual && ((relayNum == 1 && overrideRelay1) || (relayNum == 2 && overrideRelay2) || (relayNum == 3 && overrideRelay1))) {
    storeLogEntry("Relay " + String(relayNum) + " is overridden. Deactivation skipped.");
    return;
  }

  switch (relayNum) {
    case 1:
      digitalWrite(relay1, HIGH);
      relay1State = false;
      storeLogEntry("Wavemaker deactivated.");
      break;
    case 2:
      digitalWrite(relay2, HIGH);
      relay2State = false;
      digitalWrite(relay4, HIGH);
      relay4State = false;
      storeLogEntry("Lights deactivated.");
      break;
    case 3:
      digitalWrite(relay3, HIGH);
      relay3State = false;
      storeLogEntry("Air Pump deactivated.");
      break;
  }
  broadcastRelayStates();
}

void broadcastRelayStates() {
  float internalRaw = lastValidTemperature - sensorCalibration.internalOffset;
  float externalRaw = lastValidExternalTemperature - sensorCalibration.externalOffset;

  String message;
  message.reserve(300);
  message = "{\"relay1\":" + String(relay1State || overrideRelay1) + ",\"relay2\":" + String(relay2State || overrideRelay2) + ",\"relay3\":" + String(relay3State || overrideRelay1) + ",\"temperature\":" + String(lastValidTemperature, 1) + ",\"externalTemperature\":" + String(lastValidExternalTemperature, 1) + ",\"internalRawTemp\":" + String(internalRaw, 2) + ",\"externalRawTemp\":" + String(externalRaw, 2);
  message += ",\"override1\":" + String(overrideRelay1 ? "true" : "false");
  message += ",\"override2\":" + String(overrideRelay2 ? "true" : "false");

  message += ",\"relay1Name\":\"WaveMaker\"";
  message += ",\"relay2Name\":\"Light\"";
  message += ",\"relay3Name\":\"Air Pump\"}";

  webSocket.broadcastTXT(message);
}

void handleGetSchedules() {
  String json = "[";
  for (size_t i = 0; i < schedules.size(); i++) {
    if (i > 0) json += ",";
    const Schedule& s = schedules[i];
    json += "{";
    json += "\"id\":" + String(i) + ",";
    json += "\"relay\":" + String(s.relayNumber) + ",";
    json += "\"onHour\":" + String(s.onHour) + ",";
    json += "\"onMinute\":" + String(s.onMinute) + ",";
    json += "\"offHour\":" + String(s.offHour) + ",";
    json += "\"offMinute\":" + String(s.offMinute) + ",";
    json += "\"enabled\":" + String(s.enabled ? "true" : "false") + ",";
    json += "\"daysOfWeek\":[";
    for (int d = 0; d < 7; d++) {
      if (d > 0) json += ",";
      json += (s.daysOfWeek[d] ? "true" : "false");
    }
    json += "]";
    json += "}";
  }
  json += "]";
  server.send(200, "application/json", json);
}

void handleAddSchedule() {
  if (server.hasArg("plain")) {
    String body = server.arg("plain");
    StaticJsonDocument<300> doc;
    DeserializationError error = deserializeJson(doc, body);

    if (!error) {
      if (!doc.containsKey("relay") || !doc.containsKey("onTime") || !doc.containsKey("offTime") || doc["relay"].isNull() || doc["onTime"].isNull() || doc["offTime"].isNull()) {
        server.send(400, "application/json", "{\"error\":\"Missing relay, onTime, or offTime\"}");
        storeLogEntry("Add Schedule failed: Missing fields.");
        return;
      }

      Schedule newSchedule;
      newSchedule.id = schedules.size();
      newSchedule.relayNumber = doc["relay"].as<int>();
      String onTime = doc["onTime"].as<String>();
      String offTime = doc["offTime"].as<String>();

      if (onTime.length() < 5 || offTime.length() < 5) {
        server.send(400, "application/json", "{\"error\":\"Invalid time format\"}");
        storeLogEntry("Add Schedule failed: Invalid time format.");
        return;
      }

      newSchedule.onHour = onTime.substring(0, 2).toInt();
      newSchedule.onMinute = onTime.substring(3).toInt();
      newSchedule.offHour = offTime.substring(0, 2).toInt();
      newSchedule.offMinute = offTime.substring(3).toInt();
      newSchedule.enabled = true;

      for (int i = 0; i < 7; i++) {
        newSchedule.daysOfWeek[i] = doc["days"][i] | false;
      }

      String dayConfig = "Schedule days: ";
      for (int i = 0; i < 7; i++) {
        dayConfig += String(newSchedule.daysOfWeek[i] ? "1" : "0");
      }
      storeLogEntry(dayConfig + " (Sun,Mon,Tue,Wed,Thu,Fri,Sat)");

      bool conflict = false;
      for (const Schedule& existing : schedules) {
        if (existing.relayNumber == newSchedule.relayNumber && existing.enabled) {
          bool shareDay = false;
          for (int i = 0; i < 7; i++) {
            if (newSchedule.daysOfWeek[i] && existing.daysOfWeek[i]) {
              shareDay = true;
              break;
            }
          }
          if (!shareDay) {
            continue;
          }
          int existingStart = existing.onHour * 60 + existing.onMinute;
          int existingEnd = existing.offHour * 60 + existing.offMinute;
          int newStart = newSchedule.onHour * 60 + newSchedule.onMinute;
          int newEnd = newSchedule.offHour * 60 + newSchedule.offMinute;

          if (existingEnd <= existingStart) existingEnd += 1440;
          if (newEnd <= newStart) newEnd += 1440;

          if ((newStart < existingEnd) && (existingStart < newEnd)) {
            conflict = true;
            break;
          }
        }
      }

      if (conflict) {
        server.send(409, "application/json", "{\"error\":\"Schedule conflict detected\"}");
        storeLogEntry("Schedule conflict detected for relay " + String(newSchedule.relayNumber));
        return;
      }

      schedules.push_back(newSchedule);
      saveSchedulesToEEPROM();
      server.send(200, "application/json", "{\"status\":\"success\"}");
      broadcastRelayStates();
      return;
    }
  }
  server.send(400, "application/json", "{\"error\":\"Invalid request\"}");
}

void handleDeleteSchedule() {
  if (server.hasArg("id")) {
    int id = server.arg("id").toInt();
    storeLogEntry("Delete request for schedule ID: " + String(id));

    if (id >= 0 && id < schedules.size()) {
      schedules.erase(schedules.begin() + id);
      saveSchedulesToEEPROM();
      storeLogEntry("Schedule deleted successfully");
      server.send(200, "application/json", "{\"status\":\"success\"}");
      broadcastRelayStates();
      return;
    }
  }
  //storeLogEntry("Invalid delete request");
  server.send(400, "application/json", "{\"error\":\"Invalid schedule ID\"}");
}

void handleUpdateSchedule() {
  if (server.hasArg("plain")) {
    String body = server.arg("plain");
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, body);

    if (!error) {
      int id = doc["id"];
      bool enabled = doc["enabled"];

      if (id >= 0 && id < schedules.size()) {
        schedules[id].enabled = enabled;
        saveSchedulesToEEPROM();
        server.send(200, "application/json", "{\"status\":\"success\"}");
        storeLogEntry("Schedule ID " + String(id) + " " + String(enabled ? "activated." : "deactivated."));
        broadcastRelayStates();
        return;
      } else {
        server.send(400, "application/json", "{\"error\":\"Invalid schedule ID\"}");
        storeLogEntry("Invalid schedule update request for ID: " + String(id));
        return;
      }
    }
  }
  server.send(400, "application/json", "{\"error\":\"Invalid request\"}");
}

void handleRoot() {
  if (!checkAuthentication()) return;
  server.send_P(200, "text/html", mainPage);
}

void handleRelay1() {
  if (server.method() == HTTP_POST) {
    if (overrideRelay1) {
      server.send(403, "application/json", "{\"error\":\"Physical override active\"}");
      return;
    }
    if (relay1State) deactivateRelay(1, true);
    else activateRelay(1, true);
    server.send(200, "application/json", "{\"state\":" + String(relay1State) + "}");
  } else if (server.method() == HTTP_GET) {
    server.send(200, "application/json", "{\"state\":" + String(relay1State) + "}");
  }
}

void handleRelay2() {
  if (server.method() == HTTP_POST) {
    if (overrideRelay2) {
      server.send(403, "application/json", "{\"error\":\"Physical override active\"}");
      return;
    }

    if (relay2State) deactivateRelay(2, true);
    else activateRelay(2, true);

    server.send(200, "application/json", "{\"state\":" + String(relay2State) + "}");
  } else if (server.method() == HTTP_GET) {
    server.send(200, "application/json", "{\"state\":" + String(relay2State) + "}");
  }
}

void handleRelay3() {
  if (server.method() == HTTP_POST) {
    if (overrideRelay1) {
      server.send(403, "application/json", "{\"error\":\"Physical override active\"}");
      return;
    }
    if (relay3State) deactivateRelay(3, true);
    else activateRelay(3, true);
    server.send(200, "application/json", "{\"state\":" + String(relay3State) + "}");
  } else if (server.method() == HTTP_GET) {
    server.send(200, "application/json", "{\"state\":" + String(relay3State) + "}");
  }
}



void handleTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    server.send(500, "text/plain", "Error getting time");
    return;
  }

  const char* daysOfWeek[7] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };
  String currentDayName = daysOfWeek[timeinfo.tm_wday];

  String formattedTime = String(timeinfo.tm_hour) + ":" + (timeinfo.tm_min < 10 ? "0" : "") + String(timeinfo.tm_min) + ":" + (timeinfo.tm_sec < 10 ? "0" : "") + String(timeinfo.tm_sec);

  String formattedDate = String(timeinfo.tm_mday) + "/" + String(timeinfo.tm_mon + 1) + "/" +

                         String(timeinfo.tm_year + 1900);

  String response = formattedTime + " " + currentDayName + " " + formattedDate;
  server.send(200, "text/plain", response);
}

void handleRelayStatus() {
  String json = "{";
  json += "\"1\":" + String(relay1State || overrideRelay1) + ",";
  json += "\"2\":" + String(relay2State || overrideRelay2) + ",";
  json += "\"3\":" + String(relay3State || overrideRelay1) + ",";
  json += "\"temperature\":" + String(lastValidTemperature, 1) + ",";
  json += "\"externalTemperature\":" + String(lastValidExternalTemperature, 1) + "}";
  server.send(200, "application/json", json);
}

void handleClearError() {
  if (server.hasArg("plain")) {
    String body = server.arg("plain");
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, body);
    if (!error && doc.containsKey("error_id")) {
      if (doc["error_id"].is<String>() && doc["error_id"].as<String>() == "all") {
        acknowledgedErrors |= activeErrors;
        activeErrors = 0;
      } else {
        uint16_t err_id = doc["error_id"].as<uint16_t>();
        acknowledgedErrors |= err_id;
        activeErrors &= ~err_id;
      }
      server.send(200, "application/json", "{\"status\":\"success\"}");
      return;
    }
  }
  server.send(400, "application/json", "{\"error\":\"Invalid request\"}");
}

void handleApiClearError() {
  if (apiServer.hasArg("plain")) {
    String body = apiServer.arg("plain");
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, body);
    if (!error && doc.containsKey("error_id")) {
      if (doc["error_id"].is<String>() && doc["error_id"].as<String>() == "all") {
        acknowledgedErrors |= activeErrors;
        activeErrors = 0;
      } else {
        uint16_t err_id = doc["error_id"].as<uint16_t>();
        acknowledgedErrors |= err_id;
        activeErrors &= ~err_id;
      }
      apiServer.send(200, "application/json", "{\"status\":\"success\"}");
      return;
    }
  }
  apiServer.send(400, "application/json", "{\"error\":\"Invalid request\"}");
}

void handleGetErrorStatus() {
  StaticJsonDocument<256> doc;
  doc["activeErrors"] = activeErrors;
  doc["acknowledgedErrors"] = acknowledgedErrors;
  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

void handleOneClickLight() {
  if (relay2State || overrideRelay2) {
    digitalWrite(relay2, HIGH);
    relay2State = false;
    delay(500);
    digitalWrite(relay2, LOW);
    relay2State = true;
    server.send(200, "application/json", "{\"status\":\"success\"}");
    storeLogEntry("Light Colour changed via button.");
  } else {
    server.send(403, "application/json", "{\"error\":\"Light is off\"}");
    storeLogEntry("Colour Change failed: Light is off.");
  }
}

void checkoverride1() {
  bool currentReading = (digitalRead(switch1Pin) == LOW);

  if (currentReading != switch1LastState) {
    if (currentReading) {
      switch1PressStartTime = millis();
    }
    switch1LastState = currentReading;
  }

  if (currentReading && !overrideRelay1 && (millis() - switch1PressStartTime >= HOLD_DURATION)) {
    overrideRelay1 = true;
    if (!relay1State) {
      activateRelay(1, true);
    }
    if (!relay3State) {
      activateRelay(3, true);
    }
    storeLogEntry("Wavemaker and Air Pump override activated");
    broadcastRelayStates();
  } else if (!currentReading && overrideRelay1) {
    overrideRelay1 = false;
    if (relay1State) {
      deactivateRelay(1, true);
    }
    if (relay3State) {
      deactivateRelay(3, true);
    }
    storeLogEntry("Wavemaker and Air Pump override deactivated");
    broadcastRelayStates();
  }
}

void checkoverride2() {
  bool currentReading = (digitalRead(switch2Pin) == LOW);

  if (currentReading != switch2LastState) {
    if (currentReading) {
      switch2PressStartTime = millis();
    }
    switch2LastState = currentReading;
  }

  if (currentReading && !overrideRelay2 && (millis() - switch2PressStartTime >= HOLD_DURATION)) {
    overrideRelay2 = true;
    if (!relay2State) {
      activateRelay(2, true);
    }
    storeLogEntry("Lights override activated");
    broadcastRelayStates();
  } else if (!currentReading && overrideRelay2) {
    overrideRelay2 = false;
    if (relay2State) {
      deactivateRelay(2, true);
    }
    storeLogEntry("Lights override deactivated");
    broadcastRelayStates();
  }
}

void overrideLEDState() {
  bool anyOverrideActive = overrideRelay1 || overrideRelay2;

  if (activeErrors > 0) {
    unsigned long now = millis();
    uint16_t priorityError = 0;
    if (activeErrors & ERR_WIFI) priorityError = ERR_WIFI;
    else if (activeErrors & ERR_TEMP_INT) priorityError = ERR_TEMP_INT;
    else if (activeErrors & ERR_TEMP_EXT) priorityError = ERR_TEMP_EXT;
    else if (activeErrors & ERR_NTP) priorityError = ERR_NTP;

    static int blinkCount = 0;
    static unsigned long stateStart = 0;
    static bool isBlinking = false;

    if (priorityError == ERR_WIFI) {
      if (now - lastBlinkTime >= 250) {
        lastBlinkTime = now;
        blinkState = !blinkState;
        digitalWrite(errorLEDPin, blinkState);
      }
      return;
    }

    int targetBlinks = 0;
    if (priorityError == ERR_TEMP_INT) targetBlinks = 2;
    else if (priorityError == ERR_TEMP_EXT) targetBlinks = 3;
    else if (priorityError == ERR_NTP) targetBlinks = 4;
    else targetBlinks = 1;

    if (isBlinking) {
      if (now - stateStart >= 200) {
        stateStart = now;
        blinkState = !blinkState;
        digitalWrite(errorLEDPin, blinkState);
        if (!blinkState) {
          blinkCount++;
          if (blinkCount >= targetBlinks) {
            isBlinking = false;
            blinkCount = 0;
          }
        }
      }
    } else {
      if (now - stateStart >= 1500) {
        stateStart = now;
        isBlinking = true;
        blinkState = true;
        digitalWrite(errorLEDPin, HIGH);
      } else {
        digitalWrite(errorLEDPin, LOW);
      }
    }
  } else if (anyOverrideActive) {
    if (millis() - lastBlinkTime >= BLINK_INTERVAL) {
      lastBlinkTime = millis();
      blinkState = !blinkState;
      digitalWrite(errorLEDPin, blinkState);
    }
  } else {
    digitalWrite(errorLEDPin, LOW);
    blinkState = false;
  }
}

void sendEmailWithLogs(const String& trigger) {
  if (!emailConfig.enabled) {
    return;
  }

  if (!WiFi.isConnected()) {
    storeLogEntry("Failed to send email: No WiFi connection");
    return;
  }

  if (emailInProgress) {
    //storeLogEntry("Email already in progress, skipping");
    return;
  }

  if (littleFsMutex != NULL) {
    if (xSemaphoreTake(littleFsMutex, pdMS_TO_TICKS(3000)) != pdTRUE) {
      //storeLogEntry("Failed to send email: could not acquire FS mutex");
      return;
    }
  }

  if (!LittleFS.exists("/logs.json")) {
    if (littleFsMutex != NULL) xSemaphoreGive(littleFsMutex);
    // storeLogEntry("Failed to send email: logs.json does not exist");
    return;
  }

  if (emailInProgress) {
    if (littleFsMutex != NULL) xSemaphoreGive(littleFsMutex);
    // storeLogEntry("Email already in progress, skipping");
    return;
  }
  emailInProgress = true;

  const size_t MAX_LOG_BUFFER = 4096;
  static char fileBuffer[MAX_LOG_BUFFER];
  size_t bytesRead = 0;

  File logsFile = LittleFS.open("/logs.json", "r");
  if (!logsFile) {
    emailInProgress = false;
    if (littleFsMutex != NULL) xSemaphoreGive(littleFsMutex);
    // storeLogEntry("Failed to open logs file for email");
    return;
  }

  size_t fileSize = logsFile.size();
  if (fileSize == 0) {
    logsFile.close();
    emailInProgress = false;
    if (littleFsMutex != NULL) xSemaphoreGive(littleFsMutex);
    //storeLogEntry("Logs file is empty");
    return;
  }

  if (fileSize >= MAX_LOG_BUFFER) {
    fileSize = MAX_LOG_BUFFER - 1;
  }
  bytesRead = logsFile.readBytes(fileBuffer, fileSize);
  fileBuffer[bytesRead] = '\0';
  logsFile.close();

  if (littleFsMutex != NULL) xSemaphoreGive(littleFsMutex);

  tempTemperature();

  ssl_client.stop();
  ssl_client.setInsecure();

  SMTPMessage message;
  message.headers.add(rfc822_from, "Aquarium Control <" + String(emailConfig.senderAccount) + ">");
  message.headers.add(rfc822_to, "User <" + String(emailConfig.recipient) + ">");
  message.headers.add(rfc822_subject, String(emailSubject) + " - " + trigger);

  struct tm timeinfo;
  String formattedTime = "Unknown";
  if (getLocalTime(&timeinfo)) {
    char timeStr[20];
    sprintf(timeStr, "%02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    formattedTime = String(timeStr);
  }

  String textMsg;
  textMsg.reserve(512);
  textMsg = "Aquarium Control System Report\n";
  textMsg += "Event: " + trigger + "\n";
  textMsg += "Timestamp: " + formattedTime + "\n\n";
  textMsg += "System Status:\n";
  textMsg += "Internal Temperature: " + String(lastValidTemperature, 1) + " °C\n";
  textMsg += "External Temperature: " + String(lastValidExternalTemperature, 1) + " °C\n";
  textMsg += "Relay 1 (WaveMaker): " + String(relay1State ? "ON" : "OFF") + "\n";
  textMsg += "Relay 2 (Light): " + String(relay2State ? "ON" : "OFF") + "\n";
  textMsg += "Relay 3 (Air Pump): " + String(relay3State ? "ON" : "OFF") + "\n";
  textMsg += "Override 1: " + String(overrideRelay1 ? "Active" : "Inactive") + "\n";
  textMsg += "Override 2: " + String(overrideRelay2 ? "Active" : "Inactive") + "\n";
  textMsg += "Total Active Errors: " + String(__builtin_popcount(activeErrors)) + "\n";
  textMsg += "Total Acknowledged Errors: " + String(__builtin_popcount(acknowledgedErrors)) + "\n\n";
  textMsg += "Full logs are attached as logs.json";

  message.text.body(textMsg);
  message.text.charset("utf-8");
  message.text.transferEncoding("quoted-printable");
  message.timestamp = time(nullptr);

  Attachment attachment;
  attachment.filename = "logs.json";
  attachment.mime = "application/json";
  attachment.name = "logs";
  attachment.attach_file.blob = reinterpret_cast<const uint8_t*>(fileBuffer);
  attachment.attach_file.blob_size = bytesRead;
  message.attachments.add(attachment, attach_type_attachment);

  bool connected = false;
  resetWatchdog();
  try {
    connected = smtp.connect(SMTP_HOST, SMTP_PORT);
  } catch (...) {
    storeLogEntry("Exception during SMTP connection");
    connected = false;
  }
  resetWatchdog();

  if (!connected || !smtp.isConnected()) {
    storeLogEntry("Failed to connect to email server");
    emailInProgress = false;
    return;
  }

  bool authenticated = false;
  resetWatchdog();
  try {
    authenticated = smtp.authenticate(emailConfig.senderAccount, emailConfig.senderPassword, readymail_auth_password);
  } catch (...) {
    storeLogEntry("Exception during SMTP authentication");
    authenticated = false;
  }
  resetWatchdog();

  if (!authenticated || !smtp.isAuthenticated()) {
    storeLogEntry("Failed to authenticate with email server");
    emailInProgress = false;
    return;
  }

  bool sendSuccess = false;
  resetWatchdog();
  try {
    sendSuccess = smtp.send(message);
  } catch (...) {
    storeLogEntry("Exception during email sending");
    sendSuccess = false;
  }
  resetWatchdog();

  if (!sendSuccess) {
    storeLogEntry("Failed to send email");
  } else {
    storeLogEntry("Email sent successfully with logs");
  }

  resetWatchdog();
  try {
    smtp.stop();
  } catch (...) {
    storeLogEntry("Exception during SMTP session close");
  }
  resetWatchdog();

  emailInProgress = false;
}

void handleTemperature() {
  if (millis() - lastTemp >= 20000) {
    sensors.requestTemperatures();
    float tempC = sensors.getTempC(sensorAddress);

    if (tempC != DEVICE_DISCONNECTED_C) {
      lastValidTemperature = tempC + sensorCalibration.internalOffset;
      broadcastRelayStates();
      updateOLED();
      consecutiveTempFailures = 0;
      if ((activeErrors & ERR_TEMP_INT) || (acknowledgedErrors & ERR_TEMP_INT)) {
        activeErrors &= ~ERR_TEMP_INT;
        acknowledgedErrors &= ~ERR_TEMP_INT;
      }
    } else {
      consecutiveTempFailures++;
      if (consecutiveTempFailures >= MAX_TEMP_FAILURES) {
        if (!(activeErrors & ERR_TEMP_INT) && !(acknowledgedErrors & ERR_TEMP_INT)) {
          activeErrors |= ERR_TEMP_INT;
          storeLogEntry("Error: Internal Temperature sensor failed " + String(consecutiveTempFailures) + " times");
          sendEmailWithLogs("Internal Temperature Sensor Error");
        } else if (!(acknowledgedErrors & ERR_TEMP_INT)) {
          activeErrors |= ERR_TEMP_INT;
        }
      }
    }

    lastTemp = millis();
  }
}


void handleGetTemporarySchedules() {
  String json;
  json.reserve(temporarySchedules.size() * 160 + 4);
  json = "[";
  for (size_t i = 0; i < temporarySchedules.size(); i++) {
    if (i > 0) json += ",";
    const TemporarySchedule& s = temporarySchedules[i];
    json += "{";
    json += "\"id\":" + String(s.id) + ",";
    json += "\"relay\":" + String(s.relayNumber) + ",";
    json += "\"onHour\":" + String(s.onHour) + ",";
    json += "\"onMinute\":" + String(s.onMinute) + ",";
    json += "\"offHour\":" + String(s.offHour) + ",";
    json += "\"offMinute\":" + String(s.offMinute) + ",";
    json += "\"hasOnTime\":" + String(s.hasOnTime ? "true" : "false") + ",";
    json += "\"hasOffTime\":" + String(s.hasOffTime ? "true" : "false") + ",";
    json += "\"enabled\":" + String(s.enabled ? "true" : "false");
    json += "}";
  }
  json += "]";
  server.send(200, "application/json", json);
}

void handleAddTemporarySchedule() {
  if (server.hasArg("plain")) {
    String body = server.arg("plain");
    StaticJsonDocument<300> doc;
    DeserializationError error = deserializeJson(doc, body);

    if (!error) {
      if (!doc.containsKey("relay") || doc["relay"].isNull()) {
        server.send(400, "application/json", "{\"error\":\"Missing relay\"}");
        storeLogEntry("Add Temporary Schedule failed: Missing relay.");
        return;
      }

      int relayNumber = doc["relay"].as<int>();

      int existingSchedulesCount = 0;
      for (const auto& schedule : temporarySchedules) {
        if (schedule.relayNumber == relayNumber) {
          existingSchedulesCount++;
        }
      }

      if (existingSchedulesCount >= 2) {
        server.send(400, "application/json", "{\"error\":\"Each relay can have a maximum of 2 temporary schedules\"}");
        storeLogEntry("Add Temporary Schedule failed: Maximum schedules reached for relay " + String(relayNumber));
        return;
      }

      TemporarySchedule newSchedule;
      newSchedule.id = tempScheduleIdCounter++;
      newSchedule.relayNumber = relayNumber;
      newSchedule.enabled = true;
      newSchedule.onFired = false;
      newSchedule.offFired = false;

      if (doc.containsKey("onTime") && !doc["onTime"].isNull()) {
        String onTime = doc["onTime"].as<String>();
        if (onTime.length() >= 5) {
          newSchedule.onHour = onTime.substring(0, 2).toInt();
          newSchedule.onMinute = onTime.substring(3).toInt();
          newSchedule.hasOnTime = true;
        } else {
          newSchedule.hasOnTime = false;
        }
      } else {
        newSchedule.hasOnTime = false;
      }

      if (doc.containsKey("offTime") && !doc["offTime"].isNull()) {
        String offTime = doc["offTime"].as<String>();
        if (offTime.length() >= 5) {
          newSchedule.offHour = offTime.substring(0, 2).toInt();
          newSchedule.offMinute = offTime.substring(3).toInt();
          newSchedule.hasOffTime = true;
        } else {
          newSchedule.hasOffTime = false;
        }
      } else {
        newSchedule.hasOffTime = false;
      }

      if (!newSchedule.hasOnTime && !newSchedule.hasOffTime) {
        server.send(400, "application/json", "{\"error\":\"Must provide at least start time or end time\"}");
        storeLogEntry("Add Temporary Schedule failed: No times provided.");
        return;
      }

      temporarySchedules.push_back(newSchedule);
      server.send(200, "application/json", "{\"status\":\"success\",\"id\":" + String(newSchedule.id) + "}");

      String logMsg = "Temporary schedule added for relay " + String(newSchedule.relayNumber);
      if (newSchedule.hasOnTime) {
        logMsg += " ON at " + String(newSchedule.onHour) + ":" + (newSchedule.onMinute < 10 ? "0" : "") + String(newSchedule.onMinute);
      }
      if (newSchedule.hasOffTime) {
        logMsg += " OFF at " + String(newSchedule.offHour) + ":" + (newSchedule.offMinute < 10 ? "0" : "") + String(newSchedule.offMinute);
      }
      storeLogEntry(logMsg);
      return;
    }
  }
  server.send(400, "application/json", "{\"error\":\"Invalid request\"}");
}

void handleDeleteTemporarySchedule() {
  if (server.hasArg("id")) {
    int id = server.arg("id").toInt();
    storeLogEntry("Delete request for temporary schedule ID: " + String(id));

    for (auto it = temporarySchedules.begin(); it != temporarySchedules.end(); ++it) {
      if (it->id == id) {
        temporarySchedules.erase(it);
        storeLogEntry("Temporary schedule deleted successfully");
        server.send(200, "application/json", "{\"status\":\"success\"}");
        return;
      }
    }
  }
  //storeLogEntry("Invalid temporary schedule delete request");
  server.send(400, "application/json", "{\"error\":\"Invalid schedule ID\"}");
}

void checkTemporarySchedules() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return;
  }

  int hours = timeinfo.tm_hour;
  int minutes = timeinfo.tm_min;
  int seconds = timeinfo.tm_sec;

  for (auto it = temporarySchedules.begin(); it != temporarySchedules.end();) {
    TemporarySchedule& schedule = *it;
    bool shouldRemove = false;

    if (!schedule.enabled) {
      ++it;
      continue;
    }

    if (schedule.hasOnTime && !schedule.onFired && hours == schedule.onHour && minutes == schedule.onMinute && seconds <= 1) {
      if (schedule.relayNumber == 1) {
        if (!relay1State && !overrideRelay1) {
          activateRelay(1, false);
          // storeLogEntry("Temporary schedule activated Wavemaker");
        }
      } else if (schedule.relayNumber == 2) {
        if (!relay2State && !overrideRelay2) {
          activateRelay(2, false);
          // storeLogEntry("Temporary schedule activated Lights");
        }
      } else if (schedule.relayNumber == 3) {
        if (!relay3State && !overrideRelay1) {
          activateRelay(3, false);
          //  storeLogEntry("Temporary schedule activated Air Pump");
        }
      }
      schedule.onFired = true;
    }

    if (schedule.hasOffTime && !schedule.offFired && hours == schedule.offHour && minutes == schedule.offMinute && seconds <= 1) {
      if (schedule.relayNumber == 1) {
        if (relay1State && !overrideRelay1) {
          deactivateRelay(1, false);
          // storeLogEntry("Temporary schedule deactivated Wavemaker");
        }
      } else if (schedule.relayNumber == 2) {
        if (relay2State && !overrideRelay2) {
          deactivateRelay(2, false);
          //  storeLogEntry("Temporary schedule deactivated Lights");
        }
      } else if (schedule.relayNumber == 3) {
        if (relay3State && !overrideRelay1) {
          deactivateRelay(3, false);
          //  storeLogEntry("Temporary schedule deactivated Air Pump");
        }
      }
      schedule.offFired = true;
    }

    if (schedule.hasOnTime && schedule.hasOffTime) {
      if (schedule.onFired && schedule.offFired) {
        shouldRemove = true;
      }
    } else if (schedule.hasOnTime && !schedule.hasOffTime) {
      if (schedule.onFired) {
        shouldRemove = true;
      }
    } else if (!schedule.hasOnTime && schedule.hasOffTime) {
      if (schedule.offFired) {
        shouldRemove = true;
      }
    }

    if (shouldRemove) {
      storeLogEntry("Temporary schedule ID " + String(schedule.id) + " completed and removed");
      it = temporarySchedules.erase(it);
    } else {
      ++it;
    }
  }
}

void handleExternalTemperature() {
  if (millis() - lastExternalTemp >= 60000) {
    externalSensors.requestTemperatures();
    float tempC = externalSensors.getTempC(externalSensorAddress);

    if (tempC != DEVICE_DISCONNECTED_C) {
      lastValidExternalTemperature = tempC + sensorCalibration.externalOffset;
      broadcastRelayStates();
      updateOLED();
      consecutiveExternalTempFailures = 0;
      if ((activeErrors & ERR_TEMP_EXT) || (acknowledgedErrors & ERR_TEMP_EXT)) {
        storeLogEntry("External temperature sensor restored");
        activeErrors &= ~ERR_TEMP_EXT;
        acknowledgedErrors &= ~ERR_TEMP_EXT;
      }
    } else {
      consecutiveExternalTempFailures++;
      if (consecutiveExternalTempFailures >= MAX_EXTERNAL_TEMP_FAILURES) {
        if (!(activeErrors & ERR_TEMP_EXT) && !(acknowledgedErrors & ERR_TEMP_EXT)) {
          activeErrors |= ERR_TEMP_EXT;
          storeLogEntry("Error: External Temperature sensor failed " + String(consecutiveExternalTempFailures) + " times");
          sendEmailWithLogs("External Temperature Sensor Error");
        } else if (!(acknowledgedErrors & ERR_TEMP_EXT)) {
          activeErrors |= ERR_TEMP_EXT;
        }
      }
    }

    lastExternalTemp = millis();
  }
}

void updateOLED() {
  if (!oledPhysicalState) {
    display.clearDisplay();
    display.display();
    return;
  }
  display.clearDisplay();

  display.drawFastVLine(63, 0, SCREEN_HEIGHT, SSD1306_WHITE);

  display.setTextSize(1);

  display.setCursor(12, 0);
  display.print("INTERNAL");

  display.setCursor(74, 0);
  display.print("EXTERNAL");

  display.drawFastHLine(0, 10, 63, SSD1306_WHITE);
  display.drawFastHLine(65, 10, 63, SSD1306_WHITE);

  if (activeErrors & ERR_TEMP_INT) {
    if (oledBlinkState) {
      display.setTextSize(4);
      display.setCursor(16, 20);
      display.print("E");
    }
  } else {
    char intBuf[8];
    dtostrf(lastValidTemperature, 4, 1, intBuf);
    display.setTextSize(2);
    int intNumW = strlen(intBuf) * 12;
    int intX = max(0, (62 - intNumW) / 2);
    display.setCursor(intX, 20);
    display.print(intBuf);
    display.setTextSize(2);
    int unitTotalW = 9 + 12;
    int unitX = max(0, (62 - unitTotalW) / 2);
    display.drawCircle(unitX + 3, 43 + 4, 3, SSD1306_WHITE);
    display.setCursor(unitX + 9, 43);
    display.print("C");
  }
  if (activeErrors & ERR_TEMP_EXT) {
    if (oledBlinkState) {
      display.setTextSize(4);
      display.setCursor(79, 20);
      display.print("E");
    }
  } else {
    char extBuf[8];
    dtostrf(lastValidExternalTemperature, 4, 1, extBuf);
    display.setTextSize(2);
    int extNumW = strlen(extBuf) * 12;
    int extX = 65 + max(0, (62 - extNumW) / 2);
    display.setCursor(extX, 20);
    display.print(extBuf);
    display.setTextSize(2);
    int extUnitTotalW = 9 + 12;
    int extUnitX = 65 + max(0, (62 - extUnitTotalW) / 2);
    display.drawCircle(extUnitX + 3, 43 + 4, 3, SSD1306_WHITE);
    display.setCursor(extUnitX + 9, 43);
    display.print("C");
  }

  display.display();
}

void tempTemperature() {
  sensors.requestTemperatures();
  float tempC = sensors.getTempC(sensorAddress);

  if (tempC != DEVICE_DISCONNECTED_C) {
    lastValidTemperature = tempC + sensorCalibration.internalOffset;
  }

  externalSensors.requestTemperatures();
  float externalTempC = externalSensors.getTempC(externalSensorAddress);

  if (externalTempC != DEVICE_DISCONNECTED_C) {
    lastValidExternalTemperature = externalTempC + sensorCalibration.externalOffset;
  }
}

// Collector API handlers — served via apiServer (port 82) on Core 0 (networkLoop)
void handleApiPing() {
  if (!dockerConfig.enabled) {
    apiServer.send(403, "application/json", "{\"error\":\"Docker integration disabled\"}");
    return;
  }
  apiServer.send(200, "application/json", "{\"status\":\"ok\"}");
}

void handleApiStatus() {
  if (!dockerConfig.enabled) {
    apiServer.send(403, "application/json", "{\"error\":\"Docker integration disabled\"}");
    return;
  }

  String ts = "null";
  struct tm t;
  if (validTimeSync && getLocalTime(&t)) {
    char buf[20];
    sprintf(buf, "%02d/%02d/%d %02d:%02d:%02d",
            t.tm_mday, t.tm_mon + 1, t.tm_year + 1900,
            t.tm_hour, t.tm_min, t.tm_sec);
    ts = "\"" + String(buf) + "\"";
  }

  String json = "{";
  json += "\"internal_c\":" + String(lastValidTemperature, 2) + ",";
  json += "\"external_c\":" + String(lastValidExternalTemperature, 2) + ",";
  json += "\"relay1\":" + String((relay1State || overrideRelay1) ? "true" : "false") + ",";
  json += "\"relay2\":" + String((relay2State || overrideRelay2) ? "true" : "false") + ",";
  json += "\"relay3\":" + String((relay3State || overrideRelay1) ? "true" : "false") + ",";
  json += "\"override1\":" + String(overrideRelay1 ? "true" : "false") + ",";
  json += "\"override2\":" + String(overrideRelay2 ? "true" : "false") + ",";
  json += "\"active_errors\":" + String(activeErrors) + ",";
  json += "\"acknowledged_errors\":" + String(acknowledgedErrors) + ",";

  json += "\"time_synced\":" + String(validTimeSync ? "true" : "false") + ",";
  json += "\"timestamp\":" + ts;
  json += "}";

  apiServer.send(200, "application/json", json);
}

void handleApiLogs() {
  if (!dockerConfig.enabled) {
    apiServer.send(403, "application/json", "{\"error\":\"Docker integration disabled\"}");
    return;
  }
  if (!spiffsInitialized) {
    apiServer.send(500, "application/json", "{\"error\":\"LittleFS not initialized!\"}");
    return;
  }
  if (littleFsMutex != NULL) {
    if (xSemaphoreTake(littleFsMutex, pdMS_TO_TICKS(1000)) != pdTRUE) {
      apiServer.send(503, "application/json", "{\"error\":\"Filesystem busy, try again\"}");
      return;
    }
  }
  StaticJsonDocument<2352> doc;
  doc.clear();
  File file = LittleFS.open("/logs.json", "r");
  if (!file) {
    if (littleFsMutex != NULL) xSemaphoreGive(littleFsMutex);
    apiServer.send(404, "application/json", "{\"logs\":[]}");
    return;
  }
  DeserializationError error = deserializeJson(doc, file);
  file.close();
  if (littleFsMutex != NULL) xSemaphoreGive(littleFsMutex);
  if (error) {
    apiServer.send(500, "application/json", "{\"error\":\"Failed to parse logs!\"}");
    return;
  }
  String response;
  serializeJson(doc, response);
  apiServer.send(200, "application/json", response);
}

void handleGetRawTemperatureData() {
  sensors.requestTemperatures();
  externalSensors.requestTemperatures();

  float internalRaw = sensors.getTempC(sensorAddress);
  float externalRaw = externalSensors.getTempC(externalSensorAddress);

  if (internalRaw == DEVICE_DISCONNECTED_C) {
    internalRaw = lastValidTemperature - sensorCalibration.internalOffset;
  }
  if (externalRaw == DEVICE_DISCONNECTED_C) {
    externalRaw = lastValidExternalTemperature - sensorCalibration.externalOffset;
  }

  String json = "{";
  json += "\"internalRaw\":" + String(internalRaw, 2) + ",";
  json += "\"externalRaw\":" + String(externalRaw, 2);
  json += "}";

  server.send(200, "application/json", json);
}