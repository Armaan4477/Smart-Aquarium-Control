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
#include "page_main.h"
#include "page_email_config.h"
#include "page_docker_config.h"
#include "page_display_ctrl.h"
#include "page_logs.h"
#include "page_temp_ctrl.h"
#include "page_temp_schedules.h"
#include "page_main_schedules.h"
#include "page_backup_restore.h"
#include "page_ota.h"
#include "page_device_settings.h"
#include "page_auth_config.h"
#include "page_wifi_config.h"
#include "page_ntp_config.h"
#include <Update.h>

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
void handleSyncTime();
void handleSyncNTP();
void handleFeedingModeToggle();
void handleBackup();
void handleRestore();
void handleBackupRestorePage();
void handleGetSchedules();
void handleAddSchedule();
void handleDeleteSchedule();
void handleEditSchedule();
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
void syncRelayHardware();
void handleGetTemporarySchedules();
void handleAddTemporarySchedule();
void handleDeleteTemporarySchedule();
void checkTemporarySchedules();
void handleDeviceSettingsPage();
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
void loadThemeConfig();
void saveThemeConfig();
void handleThemeJS();
void handleGetThemeConfig();
void handleSaveThemeConfig();
void handleOtaPage();
void handleRollback();
void handleReboot();
void handleFactoryReset();
void handleAuthConfigPage();
void handleGetAuthConfig();
void handleSaveAuthConfig();
void loadAuthConfig();
void saveAuthConfig();

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

struct ThemeConfig {
  uint8_t magic;
  bool isDarkMode;
};

struct AuthConfig {
  uint8_t magic;
  char username[32];
  char password[32];
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

struct AllowedIP {
  char ip[16];
  char note[16];
};


bool feedingModeActive = false;
unsigned long feedingModeEndTime = 0;
const uint16_t ERR_WIFI = 1 << 0;
const uint16_t ERR_NTP = 1 << 1;
const uint16_t ERR_TEMP_INT = 1 << 2;
const uint16_t ERR_TEMP_EXT = 1 << 3;

uint16_t activeErrors = 0;
uint16_t acknowledgedErrors = 0;

struct WifiConfig {
  uint8_t magic;
  char ssid[32];
  char password[64];
  char apSsid[32];
  char apPassword[64];
};
WifiConfig wifiConfig;

struct NtpConfig {
  uint8_t magic;
  char server[64];
};
NtpConfig ntpConfig;
const char* fallbackNtpServer = "pool.ntp.org";

const char* fallbackApSsid = "ESP32_Aquarium";
const char* fallbackApPassword = "aquarium123";
bool isApActive = false;
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
const int EEPROM_SIZE = 4096;
const int SCHEDULE_SIZE = sizeof(Schedule);
const int MAX_SCHEDULES = 10;
const int SCHEDULE_START_ADDR = 0;
const int TOGGLE_DELAY = 500;
const int TOGGLE_COUNT = 3;
std::vector<AllowedIP> allowedIPs;
const int MAX_ALLOWED_IPS = 20;
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

const int THEME_CONFIG_ADDR = DOCKER_CONFIG_ADDR + sizeof(DockerConfig) + 1;
ThemeConfig themeConfig = { 0xDC, false };

const int AUTH_CONFIG_ADDR = THEME_CONFIG_ADDR + sizeof(ThemeConfig) + 1;
AuthConfig authConfig = { 0xA1, "Admin", "Admin" };

const int WIFI_CONFIG_ADDR = AUTH_CONFIG_ADDR + sizeof(AuthConfig) + 1;
const int NTP_CONFIG_ADDR = WIFI_CONFIG_ADDR + sizeof(WifiConfig) + 1;
const int IP_ALLOWLIST_ADDR = NTP_CONFIG_ADDR + sizeof(NtpConfig) + 1;

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

  EEPROM.begin(EEPROM_SIZE);
  loadWifiConfig();
  loadNtpConfig();
  
  WiFi.mode(WIFI_AP_STA);
  WiFi.setSleep(false);
  const char* currentApSsid = strlen(wifiConfig.apSsid) > 0 ? wifiConfig.apSsid : fallbackApSsid;
  const char* currentApPassword = strlen(wifiConfig.apPassword) > 0 ? wifiConfig.apPassword : fallbackApPassword;
  WiFi.softAP(currentApSsid, currentApPassword);
  isApActive = true;
  storeLogEntry("AP started: " + String(currentApSsid));
  
  if (wifiConfig.magic == 0xA1 && strlen(wifiConfig.ssid) > 0) {
    WiFi.begin(wifiConfig.ssid, wifiConfig.password);
    unsigned long wifiStartTime = millis();
    const unsigned long wifiTimeout = 20000;

    while (millis() - wifiStartTime < wifiTimeout) {
      if (WiFi.status() == WL_CONNECTED) {
        storeLogEntry("Connected to WiFi");
        attemptTimeSync();
        break;
      }
      delay(1000);
    }

    if (WiFi.status() != WL_CONNECTED) {
      storeLogEntry("WiFi connection failed.");
      activeErrors |= ERR_WIFI;
    }
  } else {
    storeLogEntry("No WiFi config found. Starting AP mode only.");
  }

  sensors.begin();
  externalSensors.begin();

  if (!LittleFS.begin(true)) {
    storeLogEntry("Failed to mount FS");
  } else {
    spiffsInitialized = true;
  }

  server.on("/", HTTP_GET, handleRoot);
  server.on("/favicon.png", HTTP_GET, handleFavicon);
  server.on("/logs", HTTP_GET, handleLogsPage);
  server.on("/logs/data", HTTP_GET, handleGetLogs);
  server.on("/devicesettings", HTTP_GET, handleDeviceSettingsPage);
  server.on("/tempcontrol", HTTP_GET, handleTempCtrlPage);
  server.on("/tempschedules", HTTP_GET, handleTempSchedulesPage);
  server.on("/mainSchedules", HTTP_GET, handleSchedulesPage);
  server.on("/relay/1", HTTP_ANY, handleRelay1);
  server.on("/relay/2", HTTP_ANY, handleRelay2);
  server.on("/relay/3", HTTP_ANY, handleRelay3);
  server.on("/api/feeding_mode", HTTP_POST, handleFeedingModeToggle);
  server.on("/backuprestore", HTTP_GET, handleBackupRestorePage);
  server.on("/api/backup", HTTP_GET, handleBackup);
  server.on("/api/restore", HTTP_POST, handleRestore);
  server.on("/time", HTTP_GET, handleTime);
  server.on("/api/sync_time", HTTP_POST, handleSyncTime);
  server.on("/api/sync_ntp", HTTP_POST, handleSyncNTP);
  server.on("/schedules", HTTP_GET, handleGetSchedules);
  server.on("/schedule/add", HTTP_POST, handleAddSchedule);
  server.on("/schedule/edit", HTTP_POST, handleEditSchedule);
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
  server.on("/theme.js", HTTP_GET, handleThemeJS);
  server.on("/api/themeConfig", HTTP_GET, handleGetThemeConfig);
  server.on("/api/themeConfig", HTTP_POST, handleSaveThemeConfig);
  server.on("/authconfig", HTTP_GET, handleAuthConfigPage);
  server.on("/api/authConfig", HTTP_GET, handleGetAuthConfig);
  server.on("/api/authConfig", HTTP_POST, handleSaveAuthConfig);
  server.on("/api/ipAllowlist", HTTP_GET, handleGetIpAllowlist);
  server.on("/api/ipAllowlist", HTTP_POST, handleSaveIpAllowlist);
  server.on("/wifiConfig", HTTP_GET, handleWifiConfigPage);
  server.on("/api/wifi/scan", HTTP_GET, handleWifiScan);
  server.on("/api/wifi/config", HTTP_GET, handleGetWifiConfig);
  server.on("/api/wifi/config", HTTP_POST, handleSaveWifiConfig);
  server.on("/api/wifi/status", HTTP_GET, handleGetWifiStatus);
  
  server.on("/ntp_settings", HTTP_GET, handleNtpSettingsPage);
  server.on("/api/ntpConfig", HTTP_GET, handleGetNtpConfig);
  server.on("/api/ntpConfig", HTTP_POST, handleSetNtpConfig);
  server.on("/api/testNtp", HTTP_POST, handleTestNtp);

  server.on("/ota", HTTP_GET, handleOtaPage);
  server.on("/api/rollback", HTTP_POST, handleRollback);
  server.on("/api/reboot", HTTP_POST, handleReboot);
  server.on("/api/reset", HTTP_POST, handleFactoryReset);
  server.on("/update", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    if (!Update.hasError()) {
      delay(100);
      ESP.restart();
    }
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
        storeLogEntry("OTA Update Error: " + String(Update.getError()));
      } else {
        storeLogEntry("OTA Update Started");
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
      }
      resetWatchdog();
      yield();
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) {
        storeLogEntry("OTA Update Success: " + String(upload.totalSize) + " bytes");
      } else {
        storeLogEntry("OTA Update Error: " + String(Update.getError()));
      }
    }
  });

  server.begin();

  apiServer.on("/api/status", HTTP_GET, handleApiStatus);
  apiServer.on("/api/logs", HTTP_GET, handleApiLogs);
  apiServer.on("/api/ping", HTTP_GET, handleApiPing);
  apiServer.on("/api/errors/clear", HTTP_POST, handleApiClearError);
  apiServer.begin();
  loadSchedulesFromEEPROM();
  loadIpAllowlistFromEEPROM();
  loadCalibrationSettings();
  loadDisplaySchedule();
  loadEmailConfig();
  loadDockerConfig();
  loadThemeConfig();
  loadAuthConfig();

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
  configTime(gmtOffset_sec, daylightOffset_sec, ntpConfig.server);

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

void saveIpAllowlistToEEPROM() {
  int addr = IP_ALLOWLIST_ADDR;
  EEPROM.write(addr, allowedIPs.size());
  addr++;
  
  for (const AllowedIP& ip : allowedIPs) {
    EEPROM.put(addr, ip);
    addr += sizeof(AllowedIP);
  }
  EEPROM.commit();
}

void loadIpAllowlistFromEEPROM() {
  allowedIPs.clear();
  int addr = IP_ALLOWLIST_ADDR;
  int count = EEPROM.read(addr);
  
  if (count == 255 || count < 0 || count > MAX_ALLOWED_IPS) {
    count = 0;
  }
  addr++;
  
  for (int i = 0; i < count; i++) {
    AllowedIP ip;
    EEPROM.get(addr, ip);
    ip.ip[15] = '\0';
    ip.note[15] = '\0';
    allowedIPs.push_back(ip);
    addr += sizeof(AllowedIP);
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

void loadThemeConfig() {
  ThemeConfig stored;
  EEPROM.get(THEME_CONFIG_ADDR, stored);
  if (stored.magic == 0xDC) {
    themeConfig = stored;
  } else {
    saveThemeConfig();
    storeLogEntry("Using default theme config");
  }
}

void saveThemeConfig() {
  themeConfig.magic = 0xDC;
  EEPROM.put(THEME_CONFIG_ADDR, themeConfig);
  EEPROM.commit();
}

void handleThemeJS() {
  String js = "document.documentElement.setAttribute('data-theme', '";
  js += themeConfig.isDarkMode ? "dark" : "light";
  js += "');";
  server.sendHeader("Connection", "close");
  server.send(200, "application/javascript", js);
}

void handleGetThemeConfig() {
  DynamicJsonDocument doc(256);
  doc["isDarkMode"] = themeConfig.isDarkMode;
  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

void handleSaveThemeConfig() {
  if (server.hasArg("plain")) {
    String body = server.arg("plain");
    DynamicJsonDocument doc(256);
    DeserializationError error = deserializeJson(doc, body);
    if (!error) {
      if (doc.containsKey("isDarkMode")) {
        themeConfig.isDarkMode = doc["isDarkMode"];
        saveThemeConfig();
        server.send(200, "application/json", "{\"status\":\"success\"}");
        return;
      }
    }
  }
  server.send(400, "application/json", "{\"error\":\"Invalid request\"}");
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
  server.sendHeader("Connection", "close");
  server.send_P(200, "text/html", displayCtrlPage);
}

void handleEmailConfigPage() {
  if (!checkAuthentication()) return;
  server.sendHeader("Connection", "close");
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
  server.sendHeader("Connection", "close");
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

        float internalRaw = lastValidTemperature - sensorCalibration.internalOffset;
        float externalRaw = lastValidExternalTemperature - sensorCalibration.externalOffset;
        String message = "{\"relay1\":" + String(relay1State || overrideRelay1) + ",\"relay2\":" + String(relay2State || overrideRelay2) + ",\"relay3\":" + String(relay3State || overrideRelay1) + ",\"temperature\":" + String(lastValidTemperature, 1) + ",\"externalTemperature\":" + String(lastValidExternalTemperature, 1) + ",\"internalRawTemp\":" + String(internalRaw, 2) + ",\"externalRawTemp\":" + String(externalRaw, 2);
        message += ",\"override1\":" + String(overrideRelay1 ? "true" : "false");
        message += ",\"override2\":" + String(overrideRelay2 ? "true" : "false");
        long timeRemaining = 0;
        if (feedingModeActive) {
          unsigned long now = millis();
          if (feedingModeEndTime > now) {
            timeRemaining = (long)((feedingModeEndTime - now) / 1000UL);
          }
        }
        message += ",\"feedingModeActive\":" + String(feedingModeActive ? "true" : "false");
        message += ",\"feedingModeTimeRemaining\":" + String(timeRemaining);
        message += ",\"relay1Name\":\"WaveMaker\",\"relay2Name\":\"Light\",\"relay3Name\":\"Air Pump\"}";
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
  for (const auto& allowedIp : allowedIPs) {
    if (clientIP == String(allowedIp.ip)) {
      return true;
    }
  }
  if (!server.authenticate(authConfig.username, authConfig.password)) {
    server.requestAuthentication();
    return false;
  }
  return true;
}

void handleFavicon() {
  server.sendHeader("Cache-Control", "max-age=31536000");
  server.send_P(200, "image/png", (const char*)favicon_png, favicon_png_len);
}

unsigned long lastWifiConnectAttempt = 0;
const unsigned long WIFI_RECONNECT_INTERVAL = 30000;
unsigned long apShutdownTime = 0;
bool pendingApShutdown = false;

void handleLogsPage() {
  server.sendHeader("Connection", "close");
  server.send_P(200, "text/html", logsPage);
}

void handleDeviceSettingsPage() {
  server.sendHeader("Connection", "close");
  server.send_P(200, "text/html", deviceSettingsPage);
}

void handleTempCtrlPage() {
  server.sendHeader("Connection", "close");
  server.send_P(200, "text/html", tempctrl);
}

void handleTempSchedulesPage() {
  server.sendHeader("Connection", "close");
  server.send_P(200, "text/html", tempschedules);
}

void handleSchedulesPage() {
  server.sendHeader("Connection", "close");
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
      pendingApShutdown = false;
      
      if (!(activeErrors & ERR_WIFI) && !(acknowledgedErrors & ERR_WIFI)) {
        storeLogEntry("WiFi disconnected");
        activeErrors |= ERR_WIFI;
      }

      if (!isApActive) {
        WiFi.mode(WIFI_AP_STA);
        WiFi.setSleep(false);
        const char* currentApSsid = strlen(wifiConfig.apSsid) > 0 ? wifiConfig.apSsid : fallbackApSsid;
        const char* currentApPassword = strlen(wifiConfig.apPassword) > 0 ? wifiConfig.apPassword : fallbackApPassword;
        WiFi.softAP(currentApSsid, currentApPassword);
        isApActive = true;
        storeLogEntry("Fallback AP started");
      }

      if (millis() - lastWifiConnectAttempt >= WIFI_RECONNECT_INTERVAL) {
        if (!Update.isRunning() && strlen(wifiConfig.ssid) > 0) {
          WiFi.disconnect();
          WiFi.begin(wifiConfig.ssid, wifiConfig.password);
        }
        lastWifiConnectAttempt = millis();
      }
    } else {
      if ((activeErrors & ERR_WIFI) || (acknowledgedErrors & ERR_WIFI)) {
        storeLogEntry("WiFi reconnected");
        activeErrors &= ~ERR_WIFI;
        acknowledgedErrors &= ~ERR_WIFI;
      }

      if (isApActive) {
        if (!pendingApShutdown) {
          pendingApShutdown = true;
          apShutdownTime = millis() + 15000;
        } else if (millis() > apShutdownTime) {
          WiFi.softAPdisconnect(true);
          WiFi.mode(WIFI_STA);
          WiFi.setSleep(false);
          isApActive = false;
          pendingApShutdown = false;
          storeLogEntry("Fallback AP stopped");
        }
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

    if (feedingModeActive && millis() >= feedingModeEndTime) {
      feedingModeActive = false;
      storeLogEntry("Feeding Mode ended automatically");
      syncRelayHardware();
      broadcastRelayStates();
    }

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
      relay1State = true;
      if (feedingModeActive) {
        storeLogEntry("Wavemaker turned ON logically (paused due to Feeding Mode).");
      } else {
        digitalWrite(relay1, LOW);
        storeLogEntry("Wavemaker activated.");
      }
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
      relay3State = true;
      if (feedingModeActive) {
        storeLogEntry("Air Pump turned ON logically (paused due to Feeding Mode).");
      } else {
        digitalWrite(relay3, LOW);
        storeLogEntry("Air Pump activated.");
      }
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
      relay1State = false;
      if (feedingModeActive) {
        storeLogEntry("Wavemaker turned OFF logically (paused due to Feeding Mode).");
      } else {
        digitalWrite(relay1, HIGH);
        storeLogEntry("Wavemaker deactivated.");
      }
      break;
    case 2:
      digitalWrite(relay2, HIGH);
      relay2State = false;
      digitalWrite(relay4, HIGH);
      relay4State = false;
      storeLogEntry("Lights deactivated.");
      break;
    case 3:
      relay3State = false;
      if (feedingModeActive) {
        storeLogEntry("Air Pump turned OFF logically (paused due to Feeding Mode).");
      } else {
        digitalWrite(relay3, HIGH);
        storeLogEntry("Air Pump deactivated.");
      }
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

  long timeRemaining = 0;
  if (feedingModeActive) {
    unsigned long now = millis();
    if (feedingModeEndTime > now) {
      timeRemaining = (long)((feedingModeEndTime - now) / 1000UL);
    }
  }
  message += ",\"feedingModeActive\":" + String(feedingModeActive ? "true" : "false");
  message += ",\"feedingModeTimeRemaining\":" + String(timeRemaining);

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

void handleEditSchedule() {
  if (server.hasArg("plain")) {
    String body = server.arg("plain");
    StaticJsonDocument<300> doc;
    DeserializationError error = deserializeJson(doc, body);

    if (!error) {
      if (!doc.containsKey("id") || !doc.containsKey("relay") || !doc.containsKey("onTime") || !doc.containsKey("offTime") || doc["relay"].isNull() || doc["onTime"].isNull() || doc["offTime"].isNull()) {
        server.send(400, "application/json", "{\"error\":\"Missing fields\"}");
        storeLogEntry("Edit Schedule failed: Missing fields.");
        return;
      }

      int id = doc["id"].as<int>();
      if (id < 0 || id >= schedules.size()) {
        server.send(400, "application/json", "{\"error\":\"Invalid schedule ID\"}");
        storeLogEntry("Edit Schedule failed: Invalid ID.");
        return;
      }

      Schedule updatedSchedule;
      updatedSchedule.id = id;
      updatedSchedule.relayNumber = doc["relay"].as<int>();
      String onTime = doc["onTime"].as<String>();
      String offTime = doc["offTime"].as<String>();

      if (onTime.length() < 5 || offTime.length() < 5) {
        server.send(400, "application/json", "{\"error\":\"Invalid time format\"}");
        storeLogEntry("Edit Schedule failed: Invalid time format.");
        return;
      }

      updatedSchedule.onHour = onTime.substring(0, 2).toInt();
      updatedSchedule.onMinute = onTime.substring(3).toInt();
      updatedSchedule.offHour = offTime.substring(0, 2).toInt();
      updatedSchedule.offMinute = offTime.substring(3).toInt();
      updatedSchedule.enabled = schedules[id].enabled;

      for (int i = 0; i < 7; i++) {
        updatedSchedule.daysOfWeek[i] = doc["days"][i] | false;
      }

      String dayConfig = "Edit Schedule days: ";
      for (int i = 0; i < 7; i++) {
        dayConfig += String(updatedSchedule.daysOfWeek[i] ? "1" : "0");
      }
      storeLogEntry(dayConfig + " (Sun,Mon,Tue,Wed,Thu,Fri,Sat)");

      bool conflict = false;
      for (int i = 0; i < schedules.size(); i++) {
        if (i == id) continue;
        const Schedule& existing = schedules[i];
        if (existing.relayNumber == updatedSchedule.relayNumber && existing.enabled) {
          bool shareDay = false;
          for (int j = 0; j < 7; j++) {
            if (updatedSchedule.daysOfWeek[j] && existing.daysOfWeek[j]) {
              shareDay = true;
              break;
            }
          }
          if (!shareDay) continue;

          int existingStart = existing.onHour * 60 + existing.onMinute;
          int existingEnd = existing.offHour * 60 + existing.offMinute;
          int newStart = updatedSchedule.onHour * 60 + updatedSchedule.onMinute;
          int newEnd = updatedSchedule.offHour * 60 + updatedSchedule.offMinute;

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
        storeLogEntry("Edit Schedule conflict detected for relay " + String(updatedSchedule.relayNumber));
        return;
      }

      schedules[id] = updatedSchedule;
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
  server.sendHeader("Connection", "close");
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
  if (!validTimeSync) {
    server.send(503, "text/plain", "Time not synced");
    return;
  }

  struct tm timeinfo;
  if (!getLocalTime(&timeinfo, 10)) {
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

void handleSyncTime() {
  if (server.hasArg("timestamp")) {
    long timestamp = server.arg("timestamp").toInt();
    if (timestamp > 0) {
      struct timeval tv;
      tv.tv_sec = timestamp;
      tv.tv_usec = 0;
      settimeofday(&tv, NULL);
      
      validTimeSync = true;
      activeErrors &= ~ERR_NTP;
      acknowledgedErrors &= ~ERR_NTP;
      storeLogEntry("Time manually synced with device");
      server.send(200, "application/json", "{\"status\":\"success\"}");
      return;
    }
  }
  server.send(400, "application/json", "{\"error\":\"Invalid or missing timestamp\"}");
}

void handleSyncNTP() {
  attemptTimeSync();
  if (!(activeErrors & ERR_NTP) && !(acknowledgedErrors & ERR_NTP)) {
    server.send(200, "application/json", "{\"status\":\"success\"}");
  } else {
    server.send(500, "application/json", "{\"error\":\"Failed to sync with NTP server\"}");
  }
}

void handleRelayStatus() {
  String json = "{";
  json += "\"1\":" + String(relay1State || overrideRelay1) + ",";
  json += "\"2\":" + String(relay2State || overrideRelay2) + ",";
  json += "\"3\":" + String(relay3State || overrideRelay1) + ",";
  json += "\"temperature\":" + String(lastValidTemperature, 1) + ",";
  long timeRemaining = 0;
  if (feedingModeActive) {
    unsigned long now = millis();
    if (feedingModeEndTime > now) {
      timeRemaining = (long)((feedingModeEndTime - now) / 1000UL);
    }
  }
  json += "\"feedingModeActive\":" + String(feedingModeActive ? "true" : "false") + ",";
  json += "\"feedingModeTimeRemaining\":" + String(timeRemaining) + ",";
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
  doc["time_synced"] = validTimeSync;
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
  } else if (acknowledgedErrors & ERR_TEMP_INT) {
    display.setTextSize(4);
    display.setCursor(8, 20);
    display.print("--");
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
  } else if (acknowledgedErrors & ERR_TEMP_EXT) {
    display.setTextSize(4);
    display.setCursor(71, 20);
    display.print("--");
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

void syncRelayHardware() {
  if (!overrideRelay1) {
    if (relay1State) {
      digitalWrite(relay1, LOW);
      storeLogEntry("Wavemaker resumed ON after Feeding Mode.");
    } else {
      digitalWrite(relay1, HIGH);
      storeLogEntry("Wavemaker remained OFF after Feeding Mode.");
    }
    
    if (relay3State) {
      digitalWrite(relay3, LOW);
      storeLogEntry("Air Pump resumed ON after Feeding Mode.");
    } else {
      digitalWrite(relay3, HIGH);
      storeLogEntry("Air Pump remained OFF after Feeding Mode.");
    }
  }
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

void handleFeedingModeToggle() {
  if (server.method() != HTTP_POST) {
    server.send(405, "text/plain", "Method Not Allowed");
    return;
  }

  if (!server.hasArg("plain")) {
    server.send(400, "application/json", "{\"error\":\"Missing request body\"}");
    return;
  }

  String body = server.arg("plain");
  DynamicJsonDocument doc(256);
  DeserializationError error = deserializeJson(doc, body);

  if (error || !doc.containsKey("action")) {
    server.send(400, "application/json", "{\"error\":\"Missing or invalid action field\"}");
    return;
  }

  String action = doc["action"].as<String>();

  if (action == "stop") {
    if (!feedingModeActive) {
      server.send(200, "application/json", "{\"status\":\"already_stopped\"}");
      return;
    }
    feedingModeActive = false;
    storeLogEntry("Feeding Mode ended manually");
    syncRelayHardware();
    broadcastRelayStates();
    server.send(200, "application/json", "{\"status\":\"stopped\"}");
    return;
  }

  if (action == "start") {
    if (overrideRelay1 || overrideRelay2) {
      server.send(409, "application/json", "{\"error\":\"Cannot start Feeding Mode while physical override is active\"}");
      storeLogEntry("Feeding Mode start rejected: physical override is active");
      return;
    }

    if (feedingModeActive) {
      storeLogEntry("Feeding Mode reset while already active");
    } else {
      storeLogEntry("Feeding Mode activated for 5 minutes");
    }

    feedingModeActive = true;
    feedingModeEndTime = millis() + (5UL * 60UL * 1000UL);

    digitalWrite(relay1, HIGH);
    digitalWrite(relay3, HIGH);

    broadcastRelayStates();
    server.send(200, "application/json", "{\"status\":\"started\",\"duration\":5}");
    return;
  }

  server.send(400, "application/json", "{\"error\":\"Unknown action. Use start or stop\"}");
}

void handleBackupRestorePage() {
  server.sendHeader("Connection", "close");
  server.send_P(200, "text/html", page_backup_restore);
}

void handleBackup() {
  DynamicJsonDocument doc(4096);
  
  JsonArray scheds = doc.createNestedArray("schedules");
  for (const Schedule& schedule : schedules) {
    JsonObject sched = scheds.createNestedObject();
    sched["relayNumber"] = schedule.relayNumber;
    sched["onHour"] = schedule.onHour;
    sched["onMinute"] = schedule.onMinute;
    sched["offHour"] = schedule.offHour;
    sched["offMinute"] = schedule.offMinute;
    JsonArray days = sched.createNestedArray("daysOfWeek");
    for (int i = 0; i < 7; i++) {
      days.add(schedule.daysOfWeek[i]);
    }
    sched["enabled"] = schedule.enabled;
  }
  
  JsonObject calib = doc.createNestedObject("sensorCalibration");
  calib["internalOffset"] = sensorCalibration.internalOffset;
  calib["externalOffset"] = sensorCalibration.externalOffset;
  
  JsonObject disp = doc.createNestedObject("displaySchedule");
  disp["onHour"] = displaySchedule.onHour;
  disp["onMinute"] = displaySchedule.onMinute;
  disp["offHour"] = displaySchedule.offHour;
  disp["offMinute"] = displaySchedule.offMinute;
  disp["overrideMode"] = displaySchedule.overrideMode;
  
  JsonObject email = doc.createNestedObject("emailConfig");
  email["enabled"] = emailConfig.enabled;
  email["senderAccount"] = emailConfig.senderAccount;
  email["senderPassword"] = emailConfig.senderPassword;
  email["recipient"] = emailConfig.recipient;
  
  JsonObject docker = doc.createNestedObject("dockerConfig");
  docker["enabled"] = dockerConfig.enabled;
  
  JsonObject auth = doc.createNestedObject("authConfig");
  auth["username"] = authConfig.username;
  auth["password"] = authConfig.password;
  
  JsonObject wifi = doc.createNestedObject("wifiConfig");
  wifi["ssid"] = wifiConfig.ssid;
  wifi["password"] = wifiConfig.password;
  wifi["apSsid"] = strlen(wifiConfig.apSsid) > 0 ? wifiConfig.apSsid : fallbackApSsid;
  wifi["apPassword"] = strlen(wifiConfig.apPassword) > 0 ? wifiConfig.apPassword : fallbackApPassword;
  
  JsonObject theme = doc.createNestedObject("themeConfig");
  theme["isDarkMode"] = themeConfig.isDarkMode;
  
  JsonObject ntp = doc.createNestedObject("ntpConfig");
  ntp["server"] = ntpConfig.server;
  
  JsonArray ips = doc.createNestedArray("allowedIPs");
  for (const auto& ip : allowedIPs) {
    JsonObject obj = ips.createNestedObject();
    obj["ip"] = ip.ip;
    obj["note"] = ip.note;
  }
  
  String output;
  serializeJson(doc, output);
  server.send(200, "application/json", output);
}

void handleRestore() {
  if (server.method() != HTTP_POST) {
    server.send(405, "text/plain", "Method Not Allowed");
    return;
  }
  if (!server.hasArg("plain")) {
    server.send(400, "text/plain", "Bad Request");
    return;
  }
  
  DynamicJsonDocument doc(4096);
  DeserializationError error = deserializeJson(doc, server.arg("plain"));
  
  if (error) {
    server.send(400, "application/json", "{\"error\":\"Failed to parse JSON\"}");
    return;
  }
  
  if (doc.containsKey("schedules")) {
    schedules.clear();
    JsonArray scheds = doc["schedules"].as<JsonArray>();
    for (JsonObject sched : scheds) {
      Schedule s;

      s.id = sched["id"] | millis();
      s.relayNumber = sched["relayNumber"];
      s.onHour = sched["onHour"];
      s.onMinute = sched["onMinute"];
      s.offHour = sched["offHour"];
      s.offMinute = sched["offMinute"];
      JsonArray days = sched["daysOfWeek"].as<JsonArray>();
      for (int i = 0; i < 7 && i < days.size(); i++) {
        s.daysOfWeek[i] = days[i];
      }
      s.enabled = sched["enabled"];
      schedules.push_back(s);
    }
    saveSchedulesToEEPROM();
  }
  
  if (doc.containsKey("sensorCalibration")) {
    sensorCalibration.internalOffset = doc["sensorCalibration"]["internalOffset"];
    sensorCalibration.externalOffset = doc["sensorCalibration"]["externalOffset"];
    EEPROM.put(CALIBRATION_START_ADDR, sensorCalibration);
    EEPROM.commit();
  }
  
  if (doc.containsKey("displaySchedule")) {
    displaySchedule.magic = 0xDA;
    displaySchedule.onHour = doc["displaySchedule"]["onHour"];
    displaySchedule.onMinute = doc["displaySchedule"]["onMinute"];
    displaySchedule.offHour = doc["displaySchedule"]["offHour"];
    displaySchedule.offMinute = doc["displaySchedule"]["offMinute"];
    displaySchedule.overrideMode = doc["displaySchedule"]["overrideMode"];
    EEPROM.put(DISPLAY_SCHEDULE_ADDR, displaySchedule);
    EEPROM.commit();
  }
  
  if (doc.containsKey("emailConfig")) {
    emailConfig.magic = 0xE2;
    emailConfig.enabled = doc["emailConfig"]["enabled"];
    strlcpy(emailConfig.senderAccount, doc["emailConfig"]["senderAccount"] | "", sizeof(emailConfig.senderAccount));
    strlcpy(emailConfig.senderPassword, doc["emailConfig"]["senderPassword"] | "", sizeof(emailConfig.senderPassword));
    strlcpy(emailConfig.recipient, doc["emailConfig"]["recipient"] | "", sizeof(emailConfig.recipient));
    EEPROM.put(EMAIL_CONFIG_ADDR, emailConfig);
    EEPROM.commit();
  }
  
  if (doc.containsKey("dockerConfig")) {
    dockerConfig.magic = 0xD1;
    dockerConfig.enabled = doc["dockerConfig"]["enabled"];
    EEPROM.put(DOCKER_CONFIG_ADDR, dockerConfig);
    EEPROM.commit();
  }
  
  if (doc.containsKey("authConfig")) {
    authConfig.magic = 0xA1;
    strlcpy(authConfig.username, doc["authConfig"]["username"] | "Admin", sizeof(authConfig.username));
    strlcpy(authConfig.password, doc["authConfig"]["password"] | "Admin", sizeof(authConfig.password));
    EEPROM.put(AUTH_CONFIG_ADDR, authConfig);
    EEPROM.commit();
  }
  
  if (doc.containsKey("wifiConfig")) {
    wifiConfig.magic = 0xA1;
    strlcpy(wifiConfig.ssid, doc["wifiConfig"]["ssid"] | "", sizeof(wifiConfig.ssid));
    strlcpy(wifiConfig.password, doc["wifiConfig"]["password"] | "", sizeof(wifiConfig.password));
    strlcpy(wifiConfig.apSsid, doc["wifiConfig"]["apSsid"] | "", sizeof(wifiConfig.apSsid));
    strlcpy(wifiConfig.apPassword, doc["wifiConfig"]["apPassword"] | "", sizeof(wifiConfig.apPassword));
    EEPROM.put(WIFI_CONFIG_ADDR, wifiConfig);
    EEPROM.commit();
  }
  
  if (doc.containsKey("themeConfig")) {
    themeConfig.magic = 0xDC;
    themeConfig.isDarkMode = doc["themeConfig"]["isDarkMode"];
    EEPROM.put(THEME_CONFIG_ADDR, themeConfig);
    EEPROM.commit();
  }
  
  if (doc.containsKey("ntpConfig")) {
    ntpConfig.magic = 0xA2;
    strlcpy(ntpConfig.server, doc["ntpConfig"]["server"] | fallbackNtpServer, sizeof(ntpConfig.server));
    EEPROM.put(NTP_CONFIG_ADDR, ntpConfig);
    EEPROM.commit();
  }
  
  if (doc.containsKey("allowedIPs")) {
    allowedIPs.clear();
    JsonArray array = doc["allowedIPs"].as<JsonArray>();
    for (JsonObject obj : array) {
      if (allowedIPs.size() >= MAX_ALLOWED_IPS) break;
      AllowedIP allowedIp;
      strlcpy(allowedIp.ip, obj["ip"] | "", sizeof(allowedIp.ip));
      strlcpy(allowedIp.note, obj["note"] | "", sizeof(allowedIp.note));
      allowedIPs.push_back(allowedIp);
    }
    saveIpAllowlistToEEPROM();
  }
  
  storeLogEntry("Configuration completely restored from backup");
  server.send(200, "application/json", "{\"status\":\"success\"}");
  delay(1000);
  ESP.restart();
}

void handleReboot() {
  if (!checkAuthentication()) {
    return;
  }
  
  storeLogEntry("System reboot initiated by user");
  server.send(200, "application/json", "{\"status\":\"success\"}");
  delay(1000);
  ESP.restart();
}

void handleFactoryReset() {
  if (!checkAuthentication()) {
    return;
  }
  
  storeLogEntry("System factory reset initiated by user");

  for (int i = 0; i < EEPROM_SIZE; i++) {
    EEPROM.write(i, 0);
  }
  EEPROM.commit();
  
  server.send(200, "application/json", "{\"status\":\"success\"}");
  delay(1000);
  ESP.restart();
}

void handleOtaPage() {
  if (!checkAuthentication()) {
    return;
  }
  server.sendHeader("Connection", "close");
  server.send_P(200, "text/html", otaPage);
}

void handleRollback() {
  if (!checkAuthentication()) {
    return;
  }

  if (Update.canRollBack()) {
    if (Update.rollBack()) {
      storeLogEntry("Firmware Rollback Successful");
      server.send(200, "text/plain", "OK");
      delay(500);
      ESP.restart();
    } else {
      storeLogEntry("Firmware Rollback Failed");
      server.send(500, "text/plain", "Rollback failed");
    }
  } else {
    server.send(400, "text/plain", "No previous firmware available to rollback");
  }
}

void handleAuthConfigPage() {
  if (!checkAuthentication()) {
    return;
  }
  server.sendHeader("Connection", "close");
  server.send_P(200, "text/html", authConfigPage);
}

void loadAuthConfig() {
  AuthConfig stored;
  EEPROM.get(AUTH_CONFIG_ADDR, stored);
  if (stored.magic == 0xA1 && strlen(stored.username) > 0 && strlen(stored.password) > 0) {
    authConfig = stored;
  } else {
    strlcpy(authConfig.username, "Admin", sizeof(authConfig.username));
    strlcpy(authConfig.password, "Admin", sizeof(authConfig.password));
    saveAuthConfig();
    storeLogEntry("Using default auth config");
  }
}

void saveAuthConfig() {
  authConfig.magic = 0xA1;
  EEPROM.put(AUTH_CONFIG_ADDR, authConfig);
  EEPROM.commit();
  storeLogEntry("Auth config saved to EEPROM");
}

void handleGetAuthConfig() {
  if (!checkAuthentication()) return;
  String json = "{";
  json += "\"username\":\"" + String(authConfig.username) + "\",";
  json += "\"password\":\"" + String(authConfig.password) + "\"";
  json += "}";
  server.send(200, "application/json", json);
}

void handleSaveAuthConfig() {
  if (!checkAuthentication()) return;
  
  if (server.hasArg("plain")) {
    String body = server.arg("plain");
    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, body);

    if (!error) {
      if (doc.containsKey("username") && doc.containsKey("password")) {
        const char* un = doc["username"];
        const char* pw = doc["password"];
        
        if (strlen(un) > 0 && strlen(pw) > 0 && strlen(un) < 32 && strlen(pw) < 32) {
          strlcpy(authConfig.username, un, sizeof(authConfig.username));
          strlcpy(authConfig.password, pw, sizeof(authConfig.password));
          saveAuthConfig();
          server.send(200, "application/json", "{\"success\":true}");
          storeLogEntry("Authentication credentials updated");
          return;
        }
      }
    }
  }
  server.send(400, "application/json", "{\"error\":\"Invalid request\"}");
}

void handleGetIpAllowlist() {
  if (!checkAuthentication()) return;
  DynamicJsonDocument doc(2048);
  JsonArray array = doc.to<JsonArray>();
  
  for (const auto& ip : allowedIPs) {
    JsonObject obj = array.createNestedObject();
    obj["ip"] = ip.ip;
    obj["note"] = ip.note;
  }
  
  String output;
  serializeJson(doc, output);
  server.send(200, "application/json", output);
}

void handleSaveIpAllowlist() {
  if (!checkAuthentication()) return;
  
  if (server.hasArg("plain")) {
    DynamicJsonDocument doc(2048);
    DeserializationError error = deserializeJson(doc, server.arg("plain"));
    
    if (!error && doc.is<JsonArray>()) {
      allowedIPs.clear();
      JsonArray array = doc.as<JsonArray>();
      for (JsonObject obj : array) {
        if (allowedIPs.size() >= MAX_ALLOWED_IPS) break;
        AllowedIP allowedIp;
        strlcpy(allowedIp.ip, obj["ip"] | "", sizeof(allowedIp.ip));
        strlcpy(allowedIp.note, obj["note"] | "", sizeof(allowedIp.note));
        allowedIPs.push_back(allowedIp);
      }
      saveIpAllowlistToEEPROM();
      server.send(200, "application/json", "{\"success\":true}");
      storeLogEntry("IP Allowlist updated");
      return;
    }
  }
  server.send(400, "application/json", "{\"error\":\"Invalid request\"}");
}

void loadWifiConfig() {
  WifiConfig stored;
  EEPROM.get(WIFI_CONFIG_ADDR, stored);
  if (stored.magic == 0xA1) {
    wifiConfig = stored;
  } else {
    wifiConfig.magic = 0xA1;
    strlcpy(wifiConfig.ssid, "", sizeof(wifiConfig.ssid));
    strlcpy(wifiConfig.password, "", sizeof(wifiConfig.password));
    strlcpy(wifiConfig.apSsid, "", sizeof(wifiConfig.apSsid));
    strlcpy(wifiConfig.apPassword, "", sizeof(wifiConfig.apPassword));
    saveWifiConfig();
  }
}

void saveWifiConfig() {
  wifiConfig.magic = 0xA1;
  EEPROM.put(WIFI_CONFIG_ADDR, wifiConfig);
  EEPROM.commit();
}

void loadNtpConfig() {
  NtpConfig stored;
  EEPROM.get(NTP_CONFIG_ADDR, stored);
  if (stored.magic == 0xA2) {
    ntpConfig = stored;
  } else {
    ntpConfig.magic = 0xA2;
    strlcpy(ntpConfig.server, fallbackNtpServer, sizeof(ntpConfig.server));
    saveNtpConfig();
  }
}

void saveNtpConfig() {
  ntpConfig.magic = 0xA2;
  EEPROM.put(NTP_CONFIG_ADDR, ntpConfig);
  EEPROM.commit();
}

void handleWifiConfigPage() {
  if (!checkAuthentication()) return;
  server.sendHeader("Connection", "close");
  server.send_P(200, "text/html", wifiConfigPage);
}

void handleWifiScan() {
  if (!checkAuthentication()) return;
  int n = WiFi.scanNetworks();
  String json = "[";
  for (int i = 0; i < n; ++i) {
    if (i > 0) json += ",";
    json += "{\"ssid\":\"" + WiFi.SSID(i) + "\",\"rssi\":" + String(WiFi.RSSI(i)) + "}";
  }
  json += "]";
  server.send(200, "application/json", json);
}

void handleGetWifiConfig() {
  if (!checkAuthentication()) return;
  DynamicJsonDocument doc(512);
  doc["ssid"] = wifiConfig.ssid;
  doc["apSsid"] = strlen(wifiConfig.apSsid) > 0 ? wifiConfig.apSsid : fallbackApSsid;
  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

void handleSaveWifiConfig() {
  if (!checkAuthentication()) return;
  
  if (server.hasArg("plain")) {
    String body = server.arg("plain");
    DynamicJsonDocument doc(512);
    DeserializationError error = deserializeJson(doc, body);

    if (!error) {
      bool wifiChanged = false;
      bool apChanged = false;

      if (doc.containsKey("ssid") && doc.containsKey("password")) {
        const char* s = doc["ssid"];
        const char* p = doc["password"];
        if (strcmp(wifiConfig.ssid, s) != 0 || strcmp(wifiConfig.password, p) != 0) {
          strlcpy(wifiConfig.ssid, s, sizeof(wifiConfig.ssid));
          strlcpy(wifiConfig.password, p, sizeof(wifiConfig.password));
          wifiChanged = true;
        }
      }

      if (doc.containsKey("apSsid") && doc.containsKey("apPassword")) {
        const char* as = doc["apSsid"];
        const char* ap = doc["apPassword"];
        if (strcmp(wifiConfig.apSsid, as) != 0 || strcmp(wifiConfig.apPassword, ap) != 0) {
          strlcpy(wifiConfig.apSsid, as, sizeof(wifiConfig.apSsid));
          strlcpy(wifiConfig.apPassword, ap, sizeof(wifiConfig.apPassword));
          apChanged = true;
        }
      }

      saveWifiConfig();
      storeLogEntry("Wi-Fi configuration updated");

      if (wifiChanged && strlen(wifiConfig.ssid) > 0) {
        WiFi.disconnect();
        WiFi.begin(wifiConfig.ssid, wifiConfig.password);
      }

      if (apChanged) {
        const char* currentApSsid = strlen(wifiConfig.apSsid) > 0 ? wifiConfig.apSsid : fallbackApSsid;
        const char* currentApPassword = strlen(wifiConfig.apPassword) > 0 ? wifiConfig.apPassword : fallbackApPassword;
        WiFi.softAP(currentApSsid, currentApPassword);
      }

      server.send(200, "application/json", "{\"success\":true}");
      return;
    }
  }
  server.send(400, "application/json", "{\"error\":\"Invalid request\"}");
}

void handleGetWifiStatus() {
  if (!checkAuthentication()) return;
  DynamicJsonDocument doc(256);
  if (WiFi.status() == WL_CONNECTED) {
    doc["status"] = "connected";
    doc["ip"] = WiFi.localIP().toString();
  } else if (WiFi.status() == WL_NO_SSID_AVAIL || WiFi.status() == WL_CONNECT_FAILED) {
    doc["status"] = "failed";
  } else {
    doc["status"] = "connecting";
  }
  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

void handleNtpSettingsPage() {
  if (!checkAuthentication()) return;
  server.sendHeader("Connection", "close");
  server.send_P(200, "text/html", ntpConfigPage);
}

void handleGetNtpConfig() {
  if (!checkAuthentication()) return;
  String json = "{\"server\":\"" + String(ntpConfig.server) + "\"}";
  server.send(200, "application/json", json);
}

void handleSetNtpConfig() {
  if (!checkAuthentication()) return;
  if (server.hasArg("plain") == false) {
    server.send(400, "application/json", "{\"error\":\"Body not received\"}");
    return;
  }
  
  String body = server.arg("plain");
  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, body);
  if (error) {
    server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
    return;
  }
  
  if (doc.containsKey("server")) {
    strlcpy(ntpConfig.server, doc["server"], sizeof(ntpConfig.server));
    saveNtpConfig();
    
    configTime(gmtOffset_sec, daylightOffset_sec, ntpConfig.server);
    struct tm timeinfo;
    if(getLocalTime(&timeinfo, 10000)) {
        server.send(200, "application/json", "{\"success\":true}");
    } else {
        server.send(200, "application/json", "{\"success\":false,\"error\":\"Failed to sync time\"}");
    }
  } else {
    server.send(400, "application/json", "{\"error\":\"Missing server parameter\"}");
  }
}

bool testNtpServerPing(const char* ntpServerName) {
    WiFiUDP udp;
    if (!udp.begin(2390)) return false; 
    
    byte packetBuffer[48]; 
    memset(packetBuffer, 0, 48);
    packetBuffer[0] = 0b11100011;   
    packetBuffer[1] = 0;     
    packetBuffer[2] = 6;     
    packetBuffer[3] = 0xEC;  
    packetBuffer[12] = 49;
    packetBuffer[13] = 0x4E;
    packetBuffer[14] = 49;
    packetBuffer[15] = 52;
    
    if (udp.beginPacket(ntpServerName, 123) == 0) {
        udp.stop();
        return false;
    }
    udp.write(packetBuffer, 48);
    udp.endPacket();
    
    unsigned long startMs = millis();
    while (millis() - startMs < 5000) {
        int size = udp.parsePacket();
        if (size >= 48) {
            udp.stop();
            return true;
        }
        delay(10);
    }
    
    udp.stop();
    return false;
}

void handleTestNtp() {
  if (!checkAuthentication()) return;
  if (server.hasArg("plain") == false) {
    server.send(400, "application/json", "{\"error\":\"Body not received\"}");
    return;
  }
  
  String body = server.arg("plain");
  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, body);
  if (error) {
    server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
    return;
  }
  
  if (doc.containsKey("server")) {
    const char* serverStr = doc["server"];
    
    if (testNtpServerPing(serverStr)) {
        server.send(200, "application/json", "{\"success\":true}");
    } else {
        server.send(200, "application/json", "{\"success\":false,\"error\":\"Failed to reach NTP server\"}");
    }
  } else {
    server.send(400, "application/json", "{\"error\":\"Missing server parameter\"}");
  }
}