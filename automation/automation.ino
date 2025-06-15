#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <WiFiUdp.h>
#include <vector>
#include <ArduinoJson.h>
#include <EEPROM.h>
#include <string>
#include <Ticker.h>
#include <TimeLib.h>
#include <LittleFS.h>
#include <ESP_Mail_Client.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <time.h>

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
void handleOneClickLight();
void handleTemperature();
void templaunch();
void emailLoop(void*);
void mainLoop(void*);
void sendEmailWithLogs(const String&);
void checkoverride1();
void checkoverride2();
void overrideLEDState();
void checkSchedules();
void checkScheduleslaunch();
void activateRelay(int, bool);
void deactivateRelay(int, bool);
void toggleLightSequence();
void broadcastRelayStates();
void handleGetTemperatureSettings();
void handleSaveTemperatureSettings();
void handleToggleTemperatureControl();
void loadTemperatureSettings();
void saveTemperatureSettings();
void checkTemperatureControl();

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

struct TemperatureData {
  int minTemp;
  int maxTemp;
  bool enabled;
};

const int relay1 = 16;
const int relay2 = 17;
const int relay3 = 18;
const int relay4 = 19;
const int switch1Pin = 23;
const int switch2Pin = 22;
const int errorLEDPin = 21;

bool overrideRelay1 = false;
bool overrideRelay2 = false;
bool relay1State = false;
bool relay2State = false;
bool relay3State = false;
bool relay4State = false;
bool timeSyncErrorLogged = false;
bool tempErrorLogged = false;
bool triggerederror = false;
bool wifiConnectionErrorLogged = false;

const char* ssid = "Free Public Wi-Fi";
const char* password = "2A0R0M4AAN";
std::vector<LogEntry> logBuffer;
bool spiffsInitialized = false;
WiFiUDP ntpUDP;
Ticker watchdogTicker;
unsigned long lastLoopTime = 0;
const unsigned long watchdogTimeout = 10000;
unsigned long lastTimeUpdate = 0;
const long timeUpdateInterval = 1000;
unsigned long lastNTPSync = 0;
unsigned long lastScheduleCheck = 0;
unsigned long lastSecond = 0;
bool validTimeSync = false;
unsigned long last90MinCheck = 0;
const unsigned long CHECK_90MIN_INTERVAL = 5400;
bool hasError = false;
bool hasLaunchedSchedules = false;
bool startupemail = false;
bool pointemail = false;
unsigned long logIdCounter = 0;
std::vector<Schedule> schedules;
std::vector<TemperatureData> temperatureData;
void handleAddSchedule();
void handleDeleteSchedule();
void handleClearError();
void handleGetErrorStatus();
const int EEPROM_SIZE = 512;
const int SCHEDULE_SIZE = sizeof(Schedule);
const int MAX_SCHEDULES = 10;
const int SCHEDULE_START_ADDR = 0;
const int TEMP_SETTINGS_START_ADDR = SCHEDULE_START_ADDR + (MAX_SCHEDULES * SCHEDULE_SIZE) + 1;
const int TEMP_SETTINGS_SIZE = sizeof(TemperatureData);
const int TOGGLE_DELAY = 500;
const int TOGGLE_COUNT = 3;
const std::vector<String> allowedIPs = {
  "192.168.29.3",  //A Mac
  "192.168.29.4",  //A Ipad
  "192.168.29.5",  //A Moto
  "192.168.29.6",  //Acer
  "192.168.29.9",  //F Moto
  "192.168.29.10"  //N Vivo
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
const char* authUsername = "admin";
const char* authPassword = "12345678";

#define ONE_WIRE_BUS 26
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress tempDeviceAddress;
DeviceAddress sensorAddress = { 0x28, 0x59, 0x71, 0x80, 0xE3, 0xE1, 0x3C, 0x50 };
unsigned long lastTemp = 0;
float currentTemp = 0;
float lastValidTemperature = 0.0;

WiFiEventId_t wifiConnectHandler;

const unsigned char favicon_png[] PROGMEM = {
  0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d,
  0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x40,
  0x08, 0x06, 0x00, 0x00, 0x00, 0xaa, 0x69, 0x71, 0xde, 0x00, 0x00, 0x00,
  0x04, 0x67, 0x41, 0x4d, 0x41, 0x00, 0x00, 0xb1, 0x8f, 0x0b, 0xfc, 0x61,
  0x05, 0x00, 0x00, 0x00, 0x20, 0x63, 0x48, 0x52, 0x4d, 0x00, 0x00, 0x7a,
  0x26, 0x00, 0x00, 0x80, 0x84, 0x00, 0x00, 0xfa, 0x00, 0x00, 0x00, 0x80,
  0xe8, 0x00, 0x00, 0x75, 0x30, 0x00, 0x00, 0xea, 0x60, 0x00, 0x00, 0x3a,
  0x98, 0x00, 0x00, 0x17, 0x70, 0x9c, 0xba, 0x51, 0x3c, 0x00, 0x00, 0x00,
  0x50, 0x65, 0x58, 0x49, 0x66, 0x4d, 0x4d, 0x00, 0x2a, 0x00, 0x00, 0x00,
  0x08, 0x00, 0x02, 0x01, 0x12, 0x00, 0x03, 0x00, 0x00, 0x00, 0x01, 0x00,
  0x01, 0x00, 0x00, 0x87, 0x69, 0x00, 0x04, 0x00, 0x00, 0x00, 0x01, 0x00,
  0x00, 0x00, 0x26, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xa0, 0x01, 0x00,
  0x03, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0xa0, 0x02, 0x00,
  0x04, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x40, 0xa0, 0x03, 0x00,
  0x04, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00,
  0x00, 0x54, 0x8c, 0x6c, 0xae, 0x00, 0x00, 0x01, 0xc9, 0x69, 0x54, 0x58,
  0x74, 0x58, 0x4d, 0x4c, 0x3a, 0x63, 0x6f, 0x6d, 0x2e, 0x61, 0x64, 0x6f,
  0x62, 0x65, 0x2e, 0x78, 0x6d, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3c,
  0x78, 0x3a, 0x78, 0x6d, 0x70, 0x6d, 0x65, 0x74, 0x61, 0x20, 0x78, 0x6d,
  0x6c, 0x6e, 0x73, 0x3a, 0x78, 0x3d, 0x22, 0x61, 0x64, 0x6f, 0x62, 0x65,
  0x3a, 0x6e, 0x73, 0x3a, 0x6d, 0x65, 0x74, 0x61, 0x2f, 0x22, 0x20, 0x78,
  0x3a, 0x78, 0x6d, 0x70, 0x74, 0x6b, 0x3d, 0x22, 0x58, 0x4d, 0x50, 0x20,
  0x43, 0x6f, 0x72, 0x65, 0x20, 0x36, 0x2e, 0x30, 0x2e, 0x30, 0x22, 0x3e,
  0x0a, 0x20, 0x20, 0x20, 0x3c, 0x72, 0x64, 0x66, 0x3a, 0x52, 0x44, 0x46,
  0x20, 0x78, 0x6d, 0x6c, 0x6e, 0x73, 0x3a, 0x72, 0x64, 0x66, 0x3d, 0x22,
  0x68, 0x74, 0x74, 0x70, 0x3a, 0x2f, 0x2f, 0x77, 0x77, 0x77, 0x2e, 0x77,
  0x33, 0x2e, 0x6f, 0x72, 0x67, 0x2f, 0x31, 0x39, 0x39, 0x39, 0x2f, 0x30,
  0x32, 0x2f, 0x32, 0x32, 0x2d, 0x72, 0x64, 0x66, 0x2d, 0x73, 0x79, 0x6e,
  0x74, 0x61, 0x78, 0x2d, 0x6e, 0x73, 0x23, 0x22, 0x3e, 0x0a, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20, 0x3c, 0x72, 0x64, 0x66, 0x3a, 0x44, 0x65, 0x73,
  0x63, 0x72, 0x69, 0x70, 0x74, 0x69, 0x6f, 0x6e, 0x20, 0x72, 0x64, 0x66,
  0x3a, 0x61, 0x62, 0x6f, 0x75, 0x74, 0x3d, 0x22, 0x22, 0x0a, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x78, 0x6d,
  0x6c, 0x6e, 0x73, 0x3a, 0x65, 0x78, 0x69, 0x66, 0x3d, 0x22, 0x68, 0x74,
  0x74, 0x70, 0x3a, 0x2f, 0x2f, 0x6e, 0x73, 0x2e, 0x61, 0x64, 0x6f, 0x62,
  0x65, 0x2e, 0x63, 0x6f, 0x6d, 0x2f, 0x65, 0x78, 0x69, 0x66, 0x2f, 0x31,
  0x2e, 0x30, 0x2f, 0x22, 0x3e, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x3c, 0x65, 0x78, 0x69, 0x66, 0x3a, 0x43, 0x6f, 0x6c,
  0x6f, 0x72, 0x53, 0x70, 0x61, 0x63, 0x65, 0x3e, 0x31, 0x3c, 0x2f, 0x65,
  0x78, 0x69, 0x66, 0x3a, 0x43, 0x6f, 0x6c, 0x6f, 0x72, 0x53, 0x70, 0x61,
  0x63, 0x65, 0x3e, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
  0x20, 0x3c, 0x65, 0x78, 0x69, 0x66, 0x3a, 0x50, 0x69, 0x78, 0x65, 0x6c,
  0x58, 0x44, 0x69, 0x6d, 0x65, 0x6e, 0x73, 0x69, 0x6f, 0x6e, 0x3e, 0x36,
  0x34, 0x3c, 0x2f, 0x65, 0x78, 0x69, 0x66, 0x3a, 0x50, 0x69, 0x78, 0x65,
  0x6c, 0x58, 0x44, 0x69, 0x6d, 0x65, 0x6e, 0x73, 0x69, 0x6f, 0x6e, 0x3e,
  0x0a, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x3c, 0x65,
  0x78, 0x69, 0x66, 0x3a, 0x50, 0x69, 0x78, 0x65, 0x6c, 0x59, 0x44, 0x69,
  0x6d, 0x65, 0x6e, 0x73, 0x69, 0x6f, 0x6e, 0x3e, 0x36, 0x34, 0x3c, 0x2f,
  0x65, 0x78, 0x69, 0x66, 0x3a, 0x50, 0x69, 0x78, 0x65, 0x6c, 0x59, 0x44,
  0x69, 0x6d, 0x65, 0x6e, 0x73, 0x69, 0x6f, 0x6e, 0x3e, 0x0a, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20, 0x3c, 0x2f, 0x72, 0x64, 0x66, 0x3a, 0x44, 0x65,
  0x73, 0x63, 0x72, 0x69, 0x70, 0x74, 0x69, 0x6f, 0x6e, 0x3e, 0x0a, 0x20,
  0x20, 0x20, 0x3c, 0x2f, 0x72, 0x64, 0x66, 0x3a, 0x52, 0x44, 0x46, 0x3e,
  0x0a, 0x3c, 0x2f, 0x78, 0x3a, 0x78, 0x6d, 0x70, 0x6d, 0x65, 0x74, 0x61,
  0x3e, 0x0a, 0x21, 0x04, 0x39, 0xc4, 0x00, 0x00, 0x16, 0xdf, 0x49, 0x44,
  0x41, 0x54, 0x78, 0x01, 0xed, 0x5a, 0x07, 0x94, 0x1c, 0xd5, 0x95, 0xbd,
  0x55, 0xd5, 0xd5, 0xd3, 0x3d, 0xdd, 0x93, 0x93, 0x46, 0x23, 0x8d, 0x46,
  0xa3, 0x2c, 0x90, 0x40, 0x28, 0x80, 0x09, 0x42, 0x02, 0x24, 0xc1, 0xca,
  0x62, 0xc1, 0xd8, 0x5a, 0x58, 0xc3, 0xee, 0xda, 0x5e, 0x63, 0x83, 0xd9,
  0x05, 0x07, 0x9c, 0xf0, 0x1a, 0x70, 0xc2, 0xc0, 0x31, 0x36, 0xbb, 0x8b,
  0x01, 0x1f, 0x92, 0x16, 0x4c, 0x5c, 0x8c, 0x0d, 0xb2, 0x09, 0x02, 0x04,
  0x32, 0x12, 0x92, 0x40, 0x28, 0x67, 0x8d, 0xc2, 0xe4, 0xdc, 0xd3, 0xb9,
  0xbb, 0xaa, 0xab, 0xfe, 0xde, 0x5f, 0x1d, 0x34, 0xc3, 0x78, 0xd0, 0x08,
  0x18, 0xd6, 0xe7, 0x58, 0x6f, 0x4e, 0x57, 0xd5, 0x54, 0xfd, 0x50, 0xef,
  0xbe, 0xf0, 0xdf, 0x7b, 0xbf, 0x80, 0x13, 0x74, 0x02, 0x81, 0x13, 0x08,
  0x9c, 0x40, 0xe0, 0x6f, 0x18, 0x01, 0xe5, 0xaf, 0x89, 0x77, 0x21, 0x84,
  0x92, 0xea, 0x78, 0x6e, 0xab, 0x68, 0xde, 0x30, 0x43, 0xd1, 0x37, 0x40,
  0x14, 0x4f, 0x87, 0x28, 0xf9, 0xea, 0xb4, 0xbc, 0xc2, 0x99, 0x7b, 0x46,
  0xea, 0x3d, 0xff, 0x6a, 0x00, 0x30, 0x76, 0xfc, 0xdc, 0x56, 0x1b, 0x1f,
  0x51, 0x94, 0xbe, 0x0e, 0xc0, 0x04, 0x44, 0x22, 0x05, 0xa5, 0x5c, 0x83,
  0x98, 0x54, 0x02, 0xd4, 0xdf, 0xd6, 0xe4, 0xf2, 0x5f, 0x5e, 0x3b, 0x12,
  0x20, 0xa8, 0x23, 0x31, 0xe8, 0xf1, 0x8c, 0x99, 0x6c, 0x7f, 0xfd, 0x41,
  0xe3, 0x95, 0xf3, 0x84, 0xb6, 0xe9, 0x76, 0x45, 0xe9, 0xea, 0x80, 0x88,
  0xfb, 0xd2, 0xdd, 0x4d, 0x2f, 0x44, 0xb7, 0x05, 0x65, 0x7f, 0x00, 0x38,
  0xf8, 0xbd, 0xb1, 0x89, 0xc8, 0xd6, 0x0b, 0x8f, 0x67, 0xdc, 0xe1, 0xb6,
  0x75, 0x0d, 0xb7, 0xe1, 0x48, 0xb4, 0x93, 0x52, 0xd7, 0x5e, 0xbf, 0x4c,
  0x11, 0xe1, 0x14, 0xe0, 0xe1, 0xab, 0xc4, 0xbd, 0x50, 0x60, 0x53, 0x03,
  0x78, 0xe6, 0xbf, 0x22, 0xe8, 0x81, 0x70, 0xe5, 0x01, 0xcd, 0x1d, 0x50,
  0x8b, 0xee, 0x7f, 0x91, 0xef, 0xf0, 0xb1, 0x6b, 0xec, 0xff, 0x9b, 0x06,
  0xa4, 0xf6, 0xff, 0x52, 0xa8, 0x6b, 0x7f, 0xaa, 0x88, 0x5e, 0x1d, 0x70,
  0x15, 0x41, 0x84, 0x79, 0x26, 0x89, 0xa8, 0x0b, 0x22, 0x95, 0x72, 0x7e,
  0xd0, 0xbc, 0x40, 0x34, 0x08, 0xa5, 0x2f, 0x0f, 0x6a, 0xf0, 0x45, 0xd0,
  0x47, 0x7c, 0xec, 0xef, 0xfb, 0x89, 0x6b, 0x40, 0x22, 0xb1, 0x7d, 0x82,
  0xba, 0xe6, 0xfa, 0x03, 0xca, 0xba, 0x1f, 0x41, 0x50, 0xd2, 0x16, 0xfc,
  0xd0, 0xfa, 0x52, 0xb0, 0xf2, 0x78, 0x0e, 0x85, 0x61, 0xe5, 0x7b, 0xa0,
  0x25, 0x3d, 0x14, 0x75, 0x1f, 0x04, 0x3c, 0x40, 0xa4, 0x0e, 0x22, 0x42,
  0x64, 0xda, 0x82, 0x88, 0x26, 0x9e, 0xa4, 0xaa, 0xe0, 0x63, 0x05, 0xe1,
  0x63, 0x57, 0x29, 0x29, 0xc5, 0xa1, 0x28, 0x15, 0xdb, 0xb9, 0x1c, 0x6f,
  0x5c, 0xf6, 0x14, 0x0e, 0xb4, 0x90, 0x0d, 0xa9, 0xe6, 0xc4, 0x3f, 0x1a,
  0x21, 0xa3, 0xc5, 0x80, 0x15, 0x87, 0x23, 0x71, 0x9e, 0x45, 0xb4, 0x18,
  0xa9, 0x92, 0xd1, 0xb0, 0x2b, 0x66, 0x40, 0x3f, 0x98, 0xcc, 0x0d, 0xa7,
  0xf9, 0x8b, 0x11, 0xaf, 0x1e, 0x07, 0x57, 0x55, 0x19, 0xac, 0xca, 0x9a,
  0x8d, 0xf9, 0xd3, 0xcf, 0xf9, 0x94, 0xa2, 0x28, 0xb4, 0x99, 0x0f, 0x4f,
  0x9f, 0x18, 0x00, 0x66, 0x74, 0xe7, 0x4f, 0xc4, 0xda, 0x7f, 0xbb, 0x49,
  0xdd, 0xf3, 0x0e, 0xe0, 0x2b, 0xa2, 0xbd, 0x1b, 0x50, 0x92, 0x89, 0x34,
  0xf3, 0x86, 0xe6, 0x70, 0x20, 0x12, 0x04, 0xc4, 0x14, 0xb0, 0x4f, 0xff,
  0x02, 0xcc, 0xea, 0x7a, 0xe8, 0x9b, 0x0f, 0xc0, 0x15, 0x0b, 0xc1, 0x56,
  0x42, 0x83, 0x38, 0x14, 0xf9, 0x3a, 0x8c, 0xc2, 0xa9, 0x69, 0x30, 0x6a,
  0x27, 0x7e, 0xc9, 0x37, 0x61, 0xee, 0x43, 0x83, 0x1a, 0x0d, 0xe3, 0xc6,
  0x27, 0x02, 0x40, 0x2a, 0xf0, 0xce, 0x0a, 0xe5, 0xf5, 0x45, 0xff, 0x24,
  0xfa, 0xfc, 0x50, 0x4c, 0x0a, 0x2c, 0x2b, 0x75, 0xbe, 0xa0, 0x12, 0xa7,
  0xc4, 0x35, 0x3f, 0x7d, 0x80, 0x06, 0xc5, 0x3f, 0x13, 0xc9, 0x4b, 0x6e,
  0x80, 0xe2, 0xc9, 0x87, 0xb6, 0xea, 0xc5, 0xa3, 0xcc, 0x9b, 0x47, 0xb5,
  0x20, 0xcb, 0x93, 0xa2, 0x4b, 0xcd, 0x29, 0x73, 0xfe, 0x95, 0x60, 0x88,
  0xaa, 0x6a, 0xd8, 0x33, 0x96, 0xfc, 0xd0, 0x5b, 0x3f, 0xe7, 0xc7, 0xd9,
  0x36, 0xc3, 0x39, 0x8f, 0x38, 0x00, 0x32, 0xb8, 0x31, 0x57, 0x9d, 0x6f,
  0x6b, 0xdb, 0xdf, 0x85, 0xc8, 0xa3, 0x4d, 0x4b, 0xa2, 0x8d, 0x23, 0xd6,
  0x43, 0xe7, 0x57, 0xe9, 0xfc, 0x2b, 0x99, 0x17, 0x75, 0x17, 0x42, 0x7c,
  0xee, 0xeb, 0x48, 0xc5, 0x52, 0x70, 0xad, 0x7c, 0x7a, 0x00, 0xf3, 0x8a,
  0xf9, 0x3e, 0x0d, 0xd0, 0x85, 0xd3, 0x0f, 0x7a, 0x5a, 0x73, 0xd2, 0xff,
  0xd0, 0x81, 0x7a, 0x52, 0xb0, 0xab, 0xe7, 0x12, 0x88, 0xc5, 0x3f, 0xf0,
  0xd6, 0xcf, 0xfd, 0x69, 0xf6, 0xfe, 0x07, 0x9d, 0x47, 0x1c, 0x80, 0xc4,
  0xce, 0xfb, 0x92, 0xae, 0xb7, 0x6e, 0x74, 0x2b, 0xa2, 0xc0, 0x91, 0x3c,
  0x0c, 0x8b, 0x12, 0xcf, 0xa7, 0xcd, 0xfb, 0x69, 0x06, 0x3d, 0xf4, 0xf6,
  0x95, 0x88, 0x4d, 0xfa, 0x2a, 0x3c, 0x97, 0x5c, 0xea, 0x30, 0x9f, 0x7a,
  0xe5, 0x05, 0xf8, 0x19, 0x0c, 0xd9, 0xa9, 0x2e, 0xa8, 0xf1, 0xde, 0xc1,
  0xef, 0xae, 0xa5, 0xb5, 0x41, 0xb8, 0x08, 0x82, 0x9a, 0x89, 0x19, 0xb2,
  0xad, 0x32, 0x80, 0x88, 0x52, 0x0d, 0xc6, 0xa9, 0x97, 0x5b, 0xbe, 0x99,
  0x97, 0x1e, 0xd3, 0xc9, 0x1f, 0xb3, 0x41, 0x76, 0xec, 0x0f, 0x7b, 0x56,
  0x43, 0x8d, 0x6e, 0x25, 0xa2, 0x50, 0xfa, 0xd2, 0x81, 0x17, 0x43, 0xa1,
  0x93, 0x53, 0x2c, 0x4a, 0x8b, 0xc2, 0x93, 0xcc, 0xdb, 0xe3, 0xfe, 0xd9,
  0x61, 0x5e, 0x8e, 0x2f, 0xfe, 0xbc, 0x0a, 0xde, 0xe6, 0xbd, 0x40, 0xaa,
  0x11, 0x6a, 0x22, 0x31, 0x78, 0xca, 0x3c, 0x32, 0x6d, 0x71, 0x69, 0x24,
  0x29, 0x4e, 0x48, 0x10, 0x73, 0xae, 0x1d, 0x30, 0xe4, 0x95, 0x49, 0x40,
  0x32, 0x20, 0xb8, 0xb7, 0x3c, 0xa9, 0x19, 0xfb, 0x5f, 0x8e, 0xb9, 0x27,
  0x2d, 0x21, 0xda, 0x43, 0xd3, 0x88, 0x03, 0x20, 0x5a, 0xdf, 0x76, 0x66,
  0x97, 0x0e, 0x0f, 0x46, 0x14, 0xc2, 0x9d, 0x0f, 0xc5, 0x20, 0xb3, 0x54,
  0x7b, 0xc9, 0x3c, 0x96, 0x2e, 0x82, 0x9a, 0x17, 0x87, 0xb5, 0x61, 0x3b,
  0xf4, 0xad, 0x2b, 0xa1, 0x25, 0xa2, 0xf4, 0x13, 0x29, 0x98, 0xa9, 0x24,
  0x74, 0x19, 0x04, 0x65, 0x48, 0xe8, 0xf2, 0x55, 0x55, 0xa4, 0x54, 0x05,
  0x2e, 0x37, 0x81, 0xe0, 0x18, 0x70, 0xa7, 0x1f, 0x2a, 0x29, 0x02, 0x2c,
  0x35, 0xc2, 0xe6, 0xf8, 0xf4, 0x1f, 0x5c, 0x4e, 0xa0, 0xb4, 0x05, 0xd0,
  0xf2, 0xd4, 0x5d, 0xde, 0x70, 0xb8, 0xad, 0xa2, 0xa0, 0xa0, 0xba, 0x2b,
  0xdd, 0x72, 0xf0, 0x71, 0xc4, 0x4d, 0xc0, 0xba, 0x6f, 0xb4, 0x70, 0x9c,
  0x9e, 0x95, 0x97, 0x66, 0x5c, 0x50, 0x6a, 0xb4, 0x7d, 0xe1, 0x3a, 0x17,
  0xc6, 0xd2, 0x2f, 0x22, 0xaf, 0x9a, 0xb1, 0x40, 0xd4, 0x82, 0xeb, 0xa1,
  0x9f, 0xc1, 0xea, 0xda, 0xc2, 0xd8, 0xa0, 0x9c, 0x7c, 0xd1, 0xa9, 0x19,
  0x06, 0x82, 0x81, 0x20, 0xda, 0x7b, 0x22, 0x88, 0x09, 0x81, 0x2a, 0xbf,
  0x8e, 0xd1, 0xd5, 0x55, 0x10, 0x45, 0x34, 0x1d, 0x37, 0x43, 0x01, 0x0f,
  0xfd, 0x48, 0x06, 0x00, 0xc9, 0x56, 0x56, 0x0b, 0x44, 0x51, 0x3e, 0x52,
  0x89, 0x00, 0xd6, 0xac, 0x4b, 0xa2, 0xf3, 0xa4, 0xd9, 0xe8, 0xb3, 0xfd,
  0x98, 0x79, 0xd6, 0xa7, 0x22, 0xe7, 0x2e, 0xbe, 0x84, 0x36, 0x38, 0x98,
  0x46, 0x5c, 0x03, 0x0c, 0xdf, 0x44, 0xb8, 0xa3, 0x5b, 0x8e, 0xce, 0x2c,
  0x99, 0xf7, 0x2e, 0x86, 0x75, 0xca, 0x02, 0x87, 0x79, 0xe7, 0xc1, 0xa6,
  0xd7, 0x20, 0x5a, 0xb6, 0x3b, 0x0c, 0x49, 0xa9, 0xef, 0x3b, 0xd8, 0x8a,
  0x07, 0x7a, 0xa3, 0x88, 0x15, 0x4d, 0x42, 0xe5, 0xc9, 0xe7, 0x3b, 0x4d,
  0xba, 0xba, 0xda, 0x30, 0x73, 0xc7, 0x7b, 0xb8, 0xa2, 0xfc, 0x30, 0xfc,
  0xb5, 0x93, 0x61, 0x13, 0x00, 0xe1, 0x23, 0x4f, 0x31, 0x1b, 0x8a, 0x88,
  0xe6, 0xc6, 0x57, 0xb8, 0x94, 0x1a, 0x5a, 0x25, 0xa6, 0xfd, 0xf0, 0x07,
  0x58, 0x7c, 0xd2, 0xcc, 0xf4, 0xf0, 0x1b, 0x56, 0xfb, 0xa5, 0x33, 0x26,
  0x51, 0x4d, 0x06, 0xd2, 0x88, 0x03, 0xa0, 0x56, 0xce, 0x84, 0x38, 0xbc,
  0x09, 0xa0, 0x36, 0x0b, 0x4a, 0xdf, 0x2e, 0xfb, 0x07, 0x9e, 0xab, 0xa0,
  0x8e, 0x29, 0x85, 0x65, 0x84, 0xa1, 0x89, 0x3e, 0xb8, 0x76, 0xac, 0xe1,
  0x92, 0x46, 0xd2, 0xdd, 0x78, 0xfc, 0xdd, 0x46, 0xac, 0x74, 0x4f, 0xc0,
  0xbf, 0x7c, 0xef, 0x5a, 0xcc, 0x5f, 0x74, 0x31, 0xbc, 0xde, 0xb4, 0xcd,
  0xcb, 0xc7, 0xcd, 0x8d, 0x87, 0xd1, 0xf2, 0x3f, 0x3f, 0xc7, 0xf8, 0xcd,
  0xaf, 0xd1, 0x3c, 0x54, 0xfa, 0x15, 0xda, 0x7c, 0x3e, 0xb5, 0x81, 0xfc,
  0x4b, 0x33, 0x90, 0x24, 0x10, 0x82, 0x6b, 0xcc, 0x64, 0x8c, 0x9d, 0x3e,
  0x03, 0x91, 0x48, 0x04, 0x7d, 0xbd, 0xdd, 0x98, 0x7d, 0xfa, 0x42, 0xf9,
  0x48, 0x2e, 0x19, 0xd2, 0x11, 0x0d, 0xa0, 0x11, 0x07, 0x40, 0xa9, 0xbf,
  0x78, 0x97, 0xf2, 0xde, 0xc3, 0xd3, 0x45, 0x32, 0x06, 0x51, 0x36, 0xd9,
  0x61, 0x1e, 0xa3, 0xf8, 0xf2, 0x85, 0xba, 0xc3, 0x7c, 0xa2, 0xe5, 0x10,
  0xdc, 0xed, 0xbb, 0xa1, 0x94, 0xb9, 0xf1, 0xf4, 0x5b, 0x71, 0xac, 0xaf,
  0x9a, 0x85, 0x87, 0x1e, 0x7e, 0x02, 0x5e, 0x1f, 0x55, 0x3d, 0x43, 0xdd,
  0xdd, 0xdd, 0x48, 0xc4, 0x22, 0x18, 0x5d, 0x5b, 0x07, 0xf5, 0x07, 0xf7,
  0x21, 0xfa, 0xd4, 0x3d, 0xc0, 0xd3, 0xbf, 0x82, 0x8b, 0x7e, 0x41, 0x8c,
  0x2e, 0xcf, 0xf8, 0x95, 0xb4, 0x43, 0x94, 0x40, 0xb8, 0xfc, 0xd5, 0x58,
  0xb3, 0xea, 0x79, 0x3c, 0xf3, 0xc0, 0x03, 0x10, 0xa1, 0x1e, 0x67, 0x94,
  0x65, 0xd7, 0x5e, 0x6f, 0xca, 0x5c, 0xe2, 0xfd, 0x5a, 0x30, 0xe2, 0x00,
  0xe8, 0x13, 0x17, 0x9d, 0x6a, 0x14, 0x54, 0x19, 0x5a, 0x80, 0x4b, 0x5b,
  0xcd, 0x67, 0xa1, 0xed, 0xdc, 0x84, 0xd4, 0xd9, 0x8b, 0xa1, 0x5a, 0xad,
  0xf4, 0x69, 0xf9, 0x50, 0x09, 0x80, 0x8a, 0x5e, 0xec, 0x3d, 0x64, 0x51,
  0xf2, 0x73, 0xf1, 0xe8, 0xa3, 0x8f, 0xc0, 0x95, 0xa7, 0xc1, 0xb2, 0x2d,
  0x04, 0x7a, 0x7a, 0xf1, 0xe4, 0x3d, 0xbf, 0xc0, 0x9e, 0xf5, 0x6b, 0xd0,
  0x63, 0xa5, 0x93, 0xa5, 0x25, 0xcb, 0x2e, 0xc2, 0xe7, 0xff, 0xfd, 0xbb,
  0xc0, 0xfe, 0xad, 0xb0, 0x76, 0xad, 0x87, 0x56, 0x98, 0xcf, 0xa2, 0x49,
  0x01, 0x7d, 0x87, 0x2f, 0x67, 0x0a, 0x87, 0xb6, 0x35, 0xe2, 0x99, 0x6d,
  0x6f, 0x3b, 0xcc, 0xef, 0x09, 0xc4, 0x70, 0xc9, 0x45, 0x17, 0x62, 0xe5,
  0x63, 0x8f, 0x21, 0x19, 0x8f, 0xcb, 0xb0, 0x79, 0x80, 0xdf, 0xfb, 0x58,
  0x13, 0x8b, 0xac, 0xc4, 0xfa, 0x9f, 0x89, 0xb8, 0xa9, 0x54, 0x9d, 0x09,
  0xb3, 0x6e, 0x36, 0x94, 0x8e, 0x36, 0xaa, 0xb9, 0x80, 0x5a, 0xc2, 0xc4,
  0x27, 0xd9, 0x07, 0xdb, 0x8e, 0x41, 0x6f, 0x6d, 0x76, 0x9a, 0x3f, 0x72,
  0xc8, 0x87, 0x6f, 0xfd, 0xe4, 0x16, 0x32, 0x4f, 0x27, 0x46, 0x6d, 0x09,
  0xf4, 0x06, 0x70, 0xeb, 0xd7, 0xbe, 0x88, 0x4d, 0xcf, 0x3f, 0x04, 0xc9,
  0x44, 0x57, 0x28, 0x88, 0xf2, 0x92, 0x42, 0xbc, 0xf9, 0xc8, 0x5d, 0xb8,
  0xed, 0x6b, 0x57, 0xc3, 0xbc, 0xec, 0x2a, 0x88, 0x24, 0x97, 0xd4, 0x1e,
  0x06, 0x49, 0x26, 0xd7, 0x55, 0x19, 0x1c, 0xc9, 0x95, 0x81, 0xd4, 0x7e,
  0x68, 0x07, 0xc6, 0xd9, 0xbb, 0x9c, 0x7e, 0xdb, 0xf6, 0x37, 0xc0, 0x5f,
  0x5a, 0x8c, 0x4f, 0x5f, 0x79, 0x25, 0x9e, 0xbb, 0xfd, 0xdb, 0x68, 0x3e,
  0x72, 0xe8, 0x86, 0x74, 0xab, 0xf4, 0x71, 0xc4, 0x01, 0x90, 0xd3, 0x98,
  0x27, 0x5d, 0x0d, 0xd4, 0x5e, 0x08, 0xe5, 0xf0, 0x4b, 0xb0, 0xeb, 0x8e,
  0x06, 0x2f, 0x12, 0x04, 0xa5, 0xb7, 0x01, 0xdd, 0x9d, 0x16, 0xc6, 0xcc,
  0x9b, 0x87, 0xd3, 0x4e, 0x9f, 0x0c, 0xa3, 0xaf, 0xcd, 0x01, 0x61, 0xe5,
  0xe3, 0x0f, 0xc0, 0xdb, 0xb4, 0x1e, 0x4d, 0xfa, 0x68, 0x1c, 0xec, 0xec,
  0x73, 0x7e, 0xb7, 0x3f, 0xf1, 0x02, 0xbe, 0x72, 0xf7, 0x93, 0x58, 0xfb,
  0xdc, 0x0a, 0xbc, 0xba, 0xad, 0x01, 0xca, 0xac, 0x85, 0x48, 0x71, 0x95,
  0x50, 0x22, 0x54, 0x7f, 0xb9, 0xfe, 0xcb, 0x55, 0x81, 0xab, 0xcd, 0xd4,
  0x82, 0x74, 0x7e, 0x24, 0x99, 0x97, 0xf4, 0xaf, 0x37, 0x7c, 0x1f, 0xd7,
  0x5c, 0xff, 0x1d, 0xb4, 0xb6, 0x07, 0xb1, 0xe9, 0x4f, 0x2b, 0x7e, 0xe9,
  0xdc, 0xcc, 0x1c, 0x46, 0xdc, 0x04, 0xe4, 0x3c, 0x76, 0xd9, 0x58, 0x28,
  0x07, 0x5a, 0xa1, 0x84, 0xdb, 0x21, 0xc6, 0xd4, 0xe6, 0xe6, 0xb7, 0x98,
  0x13, 0xc8, 0xf8, 0xe0, 0x40, 0xdc, 0x8d, 0x59, 0x67, 0x5e, 0xe2, 0xdc,
  0x37, 0x0c, 0x15, 0xa1, 0xae, 0x08, 0x42, 0x0d, 0x7f, 0x76, 0xfe, 0xcf,
  0x32, 0xd1, 0xdb, 0x17, 0xc5, 0x77, 0xae, 0x58, 0x86, 0xee, 0x40, 0x3a,
  0x2c, 0xde, 0xb2, 0x66, 0x1d, 0x16, 0xcc, 0x9d, 0x02, 0xcf, 0xe6, 0xd5,
  0x80, 0x0c, 0x9a, 0x4c, 0x9a, 0x82, 0x42, 0x33, 0xb0, 0xa2, 0xa8, 0x50,
  0x5d, 0x98, 0x5b, 0x54, 0xcd, 0xfe, 0x9d, 0x90, 0xfd, 0x24, 0x35, 0x1e,
  0x69, 0x44, 0x9d, 0x57, 0x43, 0xa8, 0x73, 0x60, 0x48, 0xf0, 0x17, 0x01,
  0x30, 0x5a, 0x1f, 0x8b, 0xc1, 0x5c, 0xe7, 0x55, 0xfa, 0x76, 0x39, 0x9d,
  0xe5, 0x41, 0x16, 0x28, 0xd5, 0xb2, 0xf9, 0xbd, 0x9a, 0xef, 0xfc, 0x3a,
  0x45, 0xa9, 0x08, 0xe7, 0x1e, 0x1c, 0xe3, 0x42, 0x2e, 0x3f, 0xd1, 0x77,
  0x9e, 0xcc, 0xa9, 0x3a, 0x0a, 0x0a, 0x1d, 0xf5, 0xcf, 0x76, 0x53, 0x93,
  0xac, 0x01, 0xd2, 0x2c, 0x4b, 0x4a, 0x0b, 0x99, 0x11, 0xf7, 0x21, 0x44,
  0xfe, 0x6c, 0x29, 0xc9, 0x0c, 0x65, 0x19, 0x90, 0xff, 0xde, 0xfb, 0xbf,
  0x7f, 0x74, 0xee, 0x9e, 0x3f, 0x4a, 0x47, 0x6d, 0xb4, 0x9d, 0xd9, 0xf3,
  0x29, 0xe9, 0x56, 0x06, 0x25, 0x6e, 0xf6, 0x33, 0x6d, 0xc6, 0x09, 0xf9,
  0xfa, 0x0e, 0x7c, 0xfb, 0xbc, 0x4a, 0x7c, 0xf7, 0x77, 0x87, 0xd2, 0x6d,
  0x78, 0xd4, 0x8b, 0x06, 0x2b, 0xfc, 0x00, 0x00, 0x62, 0xb1, 0xad, 0x63,
  0xf4, 0xfd, 0xd7, 0x37, 0x29, 0xfb, 0x6e, 0xe6, 0xfa, 0xda, 0xc6, 0x4c,
  0xad, 0xdf, 0x9b, 0x80, 0x41, 0x8a, 0xf7, 0x91, 0x52, 0x2b, 0xbf, 0x3a,
  0x64, 0x1c, 0xb9, 0x76, 0xd8, 0xd5, 0xda, 0xd8, 0xee, 0x3f, 0xc7, 0xf4,
  0xf8, 0x16, 0x58, 0x41, 0x86, 0xb7, 0x7a, 0x3a, 0x2a, 0x15, 0x29, 0x93,
  0xb5, 0x00, 0x3a, 0x35, 0x06, 0x2c, 0x96, 0xe1, 0x41, 0x7d, 0x79, 0x0c,
  0x85, 0xc5, 0x82, 0xda, 0x10, 0x47, 0x24, 0x1c, 0x83, 0x9f, 0xcb, 0x7b,
  0x69, 0xd9, 0x78, 0x18, 0x35, 0x7b, 0x50, 0x5a, 0x9c, 0xc8, 0x49, 0x31,
  0xcb, 0xc9, 0xe8, 0x51, 0x45, 0x08, 0x15, 0xd1, 0xee, 0xfb, 0x91, 0x92,
  0x64, 0x14, 0xe8, 0xe7, 0xf8, 0x32, 0x55, 0x60, 0xc8, 0x5c, 0xeb, 0x99,
  0x8a, 0xf9, 0x93, 0x6c, 0x3c, 0x72, 0x55, 0x08, 0x8f, 0xae, 0x0a, 0x41,
  0xf6, 0xb9, 0xa0, 0xc0, 0x8d, 0xda, 0x29, 0x19, 0xd0, 0x32, 0x7d, 0x73,
  0x00, 0x88, 0x9d, 0x3b, 0xdd, 0xd6, 0x1b, 0x97, 0x35, 0x39, 0x55, 0x59,
  0xf9, 0xd0, 0xcc, 0x73, 0x2a, 0xb3, 0x4e, 0x3b, 0x0e, 0x4a, 0x57, 0x46,
  0x39, 0x71, 0xd9, 0xd1, 0x3b, 0xa1, 0xb5, 0x3e, 0xc1, 0xe5, 0xe7, 0xe5,
  0xdd, 0xb2, 0xac, 0xe5, 0x9a, 0xf4, 0xf5, 0xc1, 0xb0, 0x66, 0x06, 0x77,
  0x4e, 0xbb, 0x7e, 0xef, 0x41, 0x05, 0x97, 0xbc, 0x20, 0x45, 0xeb, 0x29,
  0x84, 0x92, 0x77, 0x34, 0xbc, 0xcd, 0x36, 0xf3, 0xeb, 0x0a, 0xac, 0xc0,
  0x21, 0x82, 0x51, 0x85, 0xb0, 0x74, 0x64, 0xe1, 0x1e, 0xd4, 0x9e, 0xb1,
  0x08, 0x68, 0x78, 0x0a, 0x77, 0x2d, 0xf3, 0x90, 0x01, 0x03, 0xaf, 0xb5,
  0xb3, 0x54, 0x4c, 0xba, 0xea, 0xd4, 0x72, 0x9c, 0xa9, 0xeb, 0x28, 0x9e,
  0xf5, 0x69, 0xe8, 0xc1, 0xf4, 0x12, 0x27, 0xef, 0xa7, 0x0c, 0xc5, 0x59,
  0xe8, 0xa1, 0x71, 0x7c, 0x62, 0x53, 0xc1, 0x92, 0x43, 0xa0, 0xea, 0x3c,
  0xd4, 0x8f, 0xdb, 0x8d, 0xfb, 0xa6, 0xac, 0x46, 0x57, 0x78, 0x3c, 0x5a,
  0xcb, 0x4f, 0xc7, 0xec, 0x73, 0xce, 0x95, 0xcd, 0x73, 0x94, 0x03, 0xc0,
  0xc4, 0x0b, 0x31, 0x15, 0x8d, 0x54, 0xf5, 0x5a, 0x4a, 0xbe, 0xfd, 0xa8,
  0x4a, 0xc9, 0x02, 0xa5, 0x29, 0xe3, 0x78, 0x66, 0x71, 0x71, 0xc6, 0xd8,
  0x9c, 0x08, 0xad, 0x9c, 0xa1, 0xa1, 0x1d, 0x4a, 0xed, 0x8f, 0x94, 0xd4,
  0x1b, 0x13, 0x84, 0x7d, 0xd2, 0x83, 0xdf, 0x70, 0x57, 0x2c, 0x18, 0xe0,
  0x5c, 0xe4, 0x0c, 0x54, 0x7f, 0x97, 0x75, 0xc7, 0x12, 0x2e, 0x7f, 0x73,
  0x08, 0x66, 0x88, 0x79, 0x7e, 0x61, 0x6e, 0x62, 0xe7, 0xc2, 0x94, 0xb1,
  0x09, 0x23, 0x94, 0xa4, 0x0b, 0xad, 0xfb, 0xf6, 0xc1, 0xc7, 0x0a, 0x50,
  0x34, 0x68, 0xc9, 0xb8, 0x86, 0x35, 0x93, 0x42, 0x14, 0xfe, 0xdd, 0x9d,
  0x38, 0xbc, 0xee, 0x19, 0xfc, 0xe4, 0xe2, 0xb7, 0x71, 0x15, 0xe7, 0x4e,
  0x93, 0x00, 0x26, 0x5c, 0x8a, 0xb3, 0xce, 0x9e, 0x0f, 0xfd, 0xee, 0x2f,
  0x33, 0xbf, 0xe2, 0x12, 0x98, 0x79, 0x32, 0xc0, 0x0c, 0x78, 0x6f, 0x52,
  0x70, 0x1b, 0x9a, 0xcf, 0xb9, 0x16, 0x47, 0x62, 0x57, 0x38, 0x2d, 0xe6,
  0xd5, 0x4f, 0xe4, 0x79, 0x60, 0x34, 0x98, 0x03, 0x00, 0x2d, 0xaf, 0x68,
  0xb2, 0xf8, 0x08, 0x93, 0xa5, 0xe9, 0x04, 0x19, 0xcc, 0x48, 0xdd, 0x49,
  0x60, 0x1c, 0xc6, 0xc9, 0x7c, 0x66, 0x22, 0xc1, 0xaa, 0x0d, 0x3a, 0x99,
  0x80, 0x74, 0x52, 0x2a, 0x0d, 0x4c, 0x5b, 0xf7, 0x2e, 0xbb, 0x2b, 0xb8,
  0xf6, 0x8e, 0x5f, 0x14, 0x9d, 0xf5, 0xed, 0x01, 0xda, 0x10, 0xdd, 0xf8,
  0x44, 0x38, 0x2f, 0x78, 0x90, 0xbd, 0xe6, 0x64, 0x7a, 0xbe, 0xef, 0xa4,
  0xa7, 0xd5, 0x58, 0x16, 0x34, 0xfc, 0xbb, 0xdf, 0x46, 0xf7, 0x94, 0x0b,
  0xd9, 0xc0, 0x83, 0x38, 0x9d, 0x9a, 0xfc, 0x55, 0x8e, 0xae, 0x45, 0xf1,
  0xd2, 0xab, 0xb1, 0x73, 0xcf, 0x39, 0x28, 0x36, 0xb9, 0x84, 0x92, 0xd4,
  0x31, 0xb3, 0x30, 0x67, 0xf6, 0x5c, 0xf8, 0xdf, 0x7a, 0x09, 0x66, 0x6b,
  0x17, 0xdc, 0x95, 0x2c, 0x8a, 0x30, 0x79, 0x72, 0xb9, 0x59, 0x52, 0x4b,
  0xe6, 0xa0, 0x20, 0x9f, 0x14, 0x5c, 0xa2, 0x1b, 0x35, 0xeb, 0x1f, 0x03,
  0xce, 0xb8, 0x92, 0x4b, 0x6f, 0x05, 0xcd, 0xab, 0x07, 0xc5, 0xf9, 0xa3,
  0xb2, 0x6c, 0x38, 0xe3, 0xe5, 0x00, 0x48, 0xf8, 0x96, 0x6c, 0xcd, 0xdf,
  0xf3, 0xce, 0x29, 0x32, 0x64, 0x55, 0x64, 0x89, 0x9a, 0x76, 0x28, 0x49,
  0xc4, 0xa9, 0xb6, 0xd9, 0x2a, 0x4e, 0x8c, 0x2f, 0xdc, 0x47, 0x1e, 0x8d,
  0xa3, 0x13, 0xd9, 0xed, 0xbc, 0x47, 0x85, 0xf1, 0xed, 0xfb, 0xb1, 0x12,
  0xfd, 0xcd, 0x72, 0x61, 0x2d, 0xbd, 0xf5, 0xac, 0xc2, 0x9a, 0x69, 0xeb,
  0x62, 0x3d, 0x2d, 0x63, 0xb1, 0xf6, 0x36, 0x8f, 0x70, 0x74, 0x3a, 0x3d,
  0xd6, 0x5f, 0x3a, 0x26, 0xed, 0x04, 0xf2, 0x34, 0xa6, 0xc9, 0x9d, 0x8d,
  0xf0, 0xae, 0x7a, 0x1a, 0xf1, 0xb3, 0x97, 0xcb, 0x80, 0xc5, 0x69, 0xda,
  0x19, 0x6f, 0x44, 0x1e, 0x43, 0xe1, 0xa9, 0xb3, 0xe6, 0x39, 0xff, 0xeb,
  0x6e, 0x05, 0x05, 0x6e, 0xcd, 0x61, 0xde, 0xfb, 0xca, 0x0a, 0xb8, 0x28,
  0xfd, 0x1c, 0x49, 0x47, 0xe8, 0xa3, 0xd9, 0xb2, 0xce, 0x28, 0xfd, 0x81,
  0x43, 0x04, 0x41, 0x35, 0xda, 0x50, 0xb2, 0xe5, 0x7e, 0x74, 0x8f, 0x5e,
  0x42, 0x03, 0xa6, 0xa9, 0x74, 0xad, 0x95, 0x9a, 0x99, 0xcb, 0x0b, 0x72,
  0x00, 0x14, 0x9e, 0xf5, 0x9d, 0x53, 0xfb, 0xb6, 0xad, 0x2c, 0xd1, 0xb7,
  0xbf, 0xd4, 0xa5, 0x35, 0xaf, 0xd3, 0x5c, 0xc6, 0x5e, 0xa7, 0x36, 0x0f,
  0x1f, 0x4b, 0xd4, 0xec, 0x28, 0x3d, 0x80, 0x92, 0xcf, 0xca, 0x0d, 0x6d,
  0x58, 0xa6, 0xb3, 0x88, 0x24, 0x59, 0xbc, 0xb4, 0x99, 0xca, 0xa6, 0x01,
  0xb5, 0xc3, 0x36, 0xdc, 0x6f, 0xfe, 0x1e, 0x76, 0xd5, 0x29, 0x6b, 0x23,
  0x1b, 0x9f, 0x80, 0xb2, 0x63, 0x45, 0xda, 0xdb, 0x27, 0x8f, 0xda, 0xa9,
  0x34, 0x03, 0x91, 0x4c, 0xb2, 0x0f, 0x51, 0xce, 0x50, 0x9e, 0xca, 0xac,
  0x8e, 0x18, 0xe6, 0x17, 0xd3, 0xbf, 0xec, 0x7d, 0x0f, 0x9e, 0x9e, 0x0e,
  0x24, 0xe7, 0x2e, 0x62, 0xd1, 0xa8, 0xd2, 0x69, 0x21, 0xc1, 0x90, 0x3f,
  0x09, 0x44, 0x7e, 0x3c, 0x04, 0xff, 0xfe, 0x4d, 0xf0, 0xee, 0x5a, 0x07,
  0x97, 0x97, 0x8c, 0xba, 0xd3, 0x4e, 0x5a, 0x49, 0x91, 0x79, 0x86, 0xf9,
  0x82, 0x25, 0x74, 0x8b, 0xc5, 0x56, 0x17, 0x6f, 0xe7, 0xf2, 0x84, 0x24,
  0xfb, 0x70, 0xc3, 0x25, 0x5f, 0x5f, 0x09, 0xa3, 0x60, 0x0c, 0x1d, 0x2f,
  0x53, 0x87, 0xb6, 0x3d, 0x8b, 0xd9, 0xe1, 0x65, 0x39, 0x41, 0x0e, 0x00,
  0xf9, 0x4f, 0xf1, 0xcc, 0x4f, 0x07, 0xb2, 0xf7, 0x56, 0xaf, 0x5e, 0xed,
  0x9a, 0xd1, 0xf9, 0x6c, 0x4f, 0x71, 0xc7, 0x8a, 0x42, 0x27, 0x91, 0x91,
  0x2f, 0x1a, 0x23, 0x14, 0x32, 0xa6, 0x97, 0xb6, 0xcb, 0x49, 0x14, 0x06,
  0xb1, 0x8e, 0x39, 0xb0, 0x93, 0x4a, 0xe9, 0xd8, 0x4c, 0x63, 0xe1, 0x61,
  0x20, 0xb3, 0xfa, 0x59, 0x28, 0xd2, 0x89, 0x91, 0x94, 0xa8, 0x09, 0x9b,
  0xfe, 0x43, 0x95, 0x0e, 0x90, 0x71, 0x80, 0x7c, 0x55, 0x49, 0x72, 0x25,
  0x80, 0x87, 0xdb, 0x5e, 0x24, 0x19, 0x0a, 0xab, 0xac, 0x12, 0xe5, 0x95,
  0x69, 0x38, 0x25, 0xd0, 0x86, 0xce, 0x17, 0x1e, 0x46, 0xdb, 0xa8, 0x31,
  0x88, 0x96, 0x4d, 0x75, 0x9e, 0xcb, 0xc3, 0x44, 0x04, 0x90, 0xd7, 0xdc,
  0x84, 0x2a, 0x72, 0xa0, 0x33, 0xe5, 0x95, 0x64, 0x06, 0xf9, 0x2e, 0x32,
  0x1a, 0xcc, 0xf3, 0x3a, 0xf7, 0x04, 0xb5, 0xc0, 0x25, 0x3d, 0x88, 0xad,
  0x42, 0x95, 0x11, 0x22, 0x49, 0x94, 0xea, 0x08, 0x36, 0xb5, 0xa0, 0xeb,
  0xdd, 0x23, 0xa8, 0x3b, 0x25, 0x01, 0x75, 0x9c, 0x07, 0xc9, 0xe6, 0x2d,
  0x2f, 0xf1, 0x91, 0x23, 0xb9, 0x01, 0x00, 0x38, 0x3d, 0x32, 0x87, 0x85,
  0x0b, 0x17, 0xca, 0xcc, 0x89, 0xbe, 0x94, 0x95, 0xab, 0x47, 0xe7, 0xdb,
  0x7a, 0x6c, 0xb3, 0xa2, 0x14, 0x52, 0xe5, 0xa2, 0xd4, 0x82, 0x6c, 0x6e,
  0x2f, 0x1d, 0x45, 0xda, 0x39, 0x3b, 0xbd, 0xa4, 0x36, 0x48, 0xab, 0xd6,
  0x62, 0x61, 0x26, 0x28, 0x69, 0xa7, 0x65, 0x4b, 0xbb, 0x94, 0x45, 0x0e,
  0xbf, 0x17, 0x76, 0xb7, 0x80, 0x91, 0xd4, 0xe1, 0x39, 0xaa, 0x00, 0x6c,
  0x2d, 0x1d, 0x63, 0x0c, 0x6e, 0xcd, 0x03, 0xb7, 0xdf, 0xa4, 0xe4, 0xca,
  0x31, 0x89, 0x26, 0x36, 0x9e, 0xd2, 0x46, 0xf3, 0x46, 0x0e, 0xcf, 0xc2,
  0x88, 0x94, 0x00, 0x49, 0x4a, 0x5d, 0xca, 0x6c, 0x5f, 0x63, 0x3b, 0x7a,
  0x5a, 0xba, 0xe1, 0x67, 0x11, 0x64, 0x6c, 0xed, 0x28, 0xc8, 0xd8, 0xb2,
  0xaf, 0xa3, 0x8b, 0xfb, 0x06, 0x09, 0x84, 0xa3, 0x29, 0x74, 0xa6, 0x98,
  0x13, 0x5a, 0x2a, 0x0a, 0x35, 0x1b, 0x45, 0x1e, 0x37, 0xbc, 0x34, 0xe9,
  0x97, 0x37, 0xb5, 0xa3, 0xa8, 0xa1, 0x17, 0x67, 0xd7, 0x16, 0x63, 0xac,
  0x4a, 0x4d, 0xc8, 0xd0, 0x90, 0x00, 0x64, 0x1b, 0xc8, 0xb3, 0xf7, 0xaa,
  0x35, 0x2a, 0x41, 0xb0, 0xdc, 0x9d, 0x5b, 0x54, 0xb9, 0x1a, 0x20, 0x4e,
  0xc6, 0xa5, 0x19, 0x48, 0x67, 0x38, 0x04, 0x29, 0x04, 0xc1, 0x2e, 0x2c,
  0x76, 0x9e, 0x2a, 0xa1, 0x3e, 0x06, 0x6a, 0xa5, 0x70, 0x45, 0xf7, 0xc2,
  0x6d, 0x48, 0x93, 0xa8, 0x70, 0xee, 0x4b, 0x53, 0x10, 0x05, 0xd4, 0x9c,
  0x36, 0x8e, 0xe3, 0x97, 0xd1, 0x1c, 0x37, 0x45, 0x84, 0x54, 0xc2, 0x62,
  0x16, 0x3e, 0x74, 0x2a, 0x99, 0xce, 0x3f, 0x46, 0x78, 0x2c, 0x8e, 0xa4,
  0xe2, 0x29, 0x6c, 0x3b, 0xdc, 0x89, 0x35, 0x3b, 0xbb, 0x9d, 0xbe, 0x13,
  0x4a, 0x5d, 0x28, 0x61, 0x71, 0xe4, 0x60, 0x77, 0x18, 0x3d, 0xc9, 0x20,
  0x92, 0x9c, 0xab, 0xbb, 0x66, 0x36, 0x22, 0x75, 0xd3, 0x51, 0x55, 0x37,
  0x0e, 0x85, 0xa5, 0xa5, 0x78, 0x86, 0x09, 0x90, 0xf2, 0xda, 0x2a, 0x5c,
  0x7b, 0x72, 0x11, 0x96, 0xce, 0xac, 0x44, 0xd8, 0xb2, 0xd1, 0x47, 0x90,
  0xc6, 0x1c, 0x3a, 0xe0, 0x8c, 0x21, 0x0f, 0xc3, 0x02, 0x40, 0x36, 0x24,
  0x08, 0x5a, 0xea, 0x17, 0xb5, 0x42, 0x71, 0xf7, 0x51, 0xc4, 0x94, 0x88,
  0x04, 0xc0, 0x48, 0xa6, 0xab, 0x3c, 0xfd, 0x80, 0x50, 0x5d, 0x69, 0x9f,
  0x20, 0x98, 0xcc, 0x28, 0x0c, 0x7b, 0x15, 0x6a, 0x45, 0xaa, 0xa5, 0x09,
  0xae, 0xd2, 0x51, 0x70, 0xb4, 0x41, 0x86, 0x7a, 0xe5, 0x15, 0xbc, 0x4e,
  0x42, 0x63, 0xca, 0x9b, 0xca, 0x2f, 0xa0, 0x09, 0xa4, 0xc9, 0xb0, 0x12,
  0x74, 0x60, 0x54, 0x29, 0x6a, 0x09, 0xe8, 0x07, 0xa5, 0x0a, 0x1a, 0xdc,
  0x25, 0x96, 0x92, 0x6d, 0x08, 0x9a, 0x88, 0x30, 0x29, 0x92, 0xb4, 0x9f,
  0x4d, 0x56, 0xf6, 0xf1, 0xd0, 0x47, 0x60, 0x83, 0x36, 0x8e, 0xf0, 0x52,
  0xd4, 0xf8, 0x51, 0x1f, 0x6a, 0x44, 0x45, 0x0b, 0x93, 0xa6, 0x5d, 0x3b,
  0x9d, 0x90, 0x79, 0xe3, 0x9b, 0xeb, 0x70, 0x03, 0x0b, 0xa4, 0x53, 0x26,
  0xd6, 0x38, 0xfe, 0x22, 0x5b, 0x65, 0x52, 0xf2, 0xc7, 0x3a, 0xe3, 0xc8,
  0xc3, 0xb0, 0x01, 0x90, 0x8d, 0xad, 0x93, 0x2e, 0x6e, 0x70, 0x6d, 0x7e,
  0x78, 0x82, 0x42, 0x95, 0x15, 0xdc, 0xcc, 0x70, 0x6a, 0x7b, 0xfd, 0x98,
  0x97, 0x6d, 0x24, 0x59, 0x64, 0x0a, 0x07, 0x77, 0x41, 0x9d, 0x76, 0x32,
  0xeb, 0xf5, 0xb5, 0xd0, 0x1a, 0x0e, 0xc1, 0xac, 0x9f, 0xe1, 0x04, 0x2a,
  0x2a, 0x55, 0x15, 0xf5, 0x13, 0xd2, 0x0d, 0x65, 0xdb, 0x92, 0x89, 0x0c,
  0x80, 0x0e, 0x31, 0x33, 0x24, 0xf3, 0x5c, 0x02, 0xc3, 0x71, 0x6e, 0x96,
  0x70, 0xd3, 0x44, 0x92, 0x65, 0x1a, 0xe8, 0x4e, 0x4a, 0x40, 0x15, 0x4c,
  0xa0, 0x46, 0x94, 0x8c, 0xab, 0xc1, 0x99, 0x04, 0xe4, 0xa5, 0xfd, 0xac,
  0x16, 0xb7, 0xc6, 0xf1, 0x78, 0xaf, 0x81, 0xbe, 0x34, 0xde, 0x28, 0x3e,
  0xdc, 0x88, 0x37, 0x18, 0xef, 0x33, 0xa0, 0xcc, 0xd1, 0x4c, 0x3a, 0xed,
  0x45, 0x53, 0x68, 0xc5, 0xdc, 0x6e, 0x33, 0x58, 0x63, 0x04, 0xfd, 0x54,
  0xbe, 0x42, 0x25, 0x9e, 0x7c, 0x6a, 0xae, 0x55, 0x16, 0xfc, 0x5c, 0xa7,
  0x0f, 0xba, 0x70, 0x2f, 0xf9, 0xaf, 0x49, 0x34, 0x56, 0x0e, 0x44, 0xe6,
  0xbd, 0x34, 0x05, 0x3f, 0x35, 0xc1, 0xc7, 0xd5, 0x81, 0x91, 0x9c, 0x24,
  0x29, 0x61, 0x29, 0x79, 0xb5, 0x7e, 0x3a, 0xac, 0xdd, 0x87, 0x18, 0xd1,
  0x31, 0xf8, 0x99, 0x4c, 0x10, 0xda, 0xda, 0x9c, 0xe7, 0x29, 0xbf, 0x1f,
  0xea, 0x81, 0x36, 0x46, 0x6d, 0x5e, 0xc7, 0x17, 0x98, 0x31, 0x7a, 0xf2,
  0x51, 0x54, 0x4d, 0x32, 0x6d, 0x06, 0x2b, 0x10, 0x4a, 0xc4, 0x10, 0x0c,
  0x57, 0xa2, 0x23, 0x62, 0xa2, 0x35, 0x10, 0x75, 0xce, 0xb2, 0x63, 0x81,
  0xa6, 0xc2, 0xc7, 0x12, 0x98, 0xd4, 0x06, 0x49, 0x93, 0x4b, 0x74, 0x47,
  0xad, 0x37, 0x2e, 0x1a, 0x83, 0x7b, 0xa7, 0x97, 0x63, 0x9c, 0x4b, 0xcb,
  0x01, 0x21, 0x9f, 0x17, 0x65, 0x2a, 0xc3, 0x97, 0x16, 0x68, 0x98, 0x3a,
  0x7d, 0x3c, 0xa3, 0x57, 0x57, 0xda, 0x9c, 0xe8, 0x5b, 0xa2, 0x45, 0x02,
  0xbe, 0xe5, 0x5d, 0x39, 0xc1, 0x1f, 0x17, 0x00, 0xb2, 0x9a, 0x22, 0x8a,
  0xe9, 0xb4, 0x32, 0x51, 0xa1, 0x08, 0xd0, 0x6e, 0x03, 0x36, 0xac, 0x5e,
  0x0b, 0x72, 0x19, 0x94, 0x64, 0x51, 0xdd, 0x8d, 0xfa, 0x59, 0xce, 0xb5,
  0xb2, 0x75, 0x17, 0xec, 0x69, 0x53, 0x9c, 0x6b, 0xd1, 0xde, 0x09, 0x4f,
  0xed, 0x0c, 0xd8, 0x0d, 0xeb, 0xb9, 0xef, 0x4f, 0x09, 0x92, 0x92, 0x7d,
  0x41, 0x98, 0xcc, 0x14, 0xdd, 0xf9, 0xa3, 0x11, 0x6b, 0x8f, 0x23, 0xdc,
  0x5e, 0x86, 0xf6, 0xbe, 0x18, 0x3a, 0x23, 0x09, 0xda, 0xb5, 0x8d, 0x08,
  0x31, 0x96, 0x5a, 0x20, 0x29, 0x41, 0xfb, 0xcd, 0x52, 0x31, 0xc1, 0xd8,
  0x42, 0xe9, 0x37, 0x70, 0x15, 0xf8, 0xdc, 0xe4, 0x02, 0xbc, 0xf9, 0xd9,
  0x59, 0xb8, 0x77, 0x6e, 0x35, 0x64, 0x92, 0x94, 0x65, 0x5e, 0x4a, 0xff,
  0x73, 0xa7, 0x73, 0xb7, 0xa8, 0x8a, 0xfb, 0x8c, 0xf1, 0x74, 0x5c, 0x60,
  0xf6, 0xb1, 0x04, 0x37, 0xff, 0x1a, 0x28, 0xca, 0x2d, 0xb9, 0xc1, 0x32,
  0x0a, 0x94, 0x1d, 0xfa, 0xd8, 0xe7, 0xe4, 0x7f, 0xd7, 0x0b, 0x57, 0x73,
  0x77, 0x7a, 0xf9, 0xcb, 0x06, 0x44, 0x54, 0x2d, 0x19, 0x1c, 0x09, 0xbe,
  0x94, 0xf0, 0xe9, 0x88, 0x5f, 0x76, 0x13, 0xb0, 0x6f, 0x33, 0x5c, 0x9b,
  0x5e, 0x85, 0xfe, 0x15, 0x4e, 0xf8, 0xc2, 0xd3, 0x8c, 0x2e, 0xe9, 0xe3,
  0x66, 0x5f, 0x08, 0xfb, 0xb7, 0xbf, 0x85, 0x7d, 0xd1, 0x05, 0xd0, 0xce,
  0x3c, 0x03, 0x91, 0xde, 0x08, 0xbc, 0x54, 0xcf, 0xc0, 0xea, 0xd7, 0x11,
  0x7f, 0x65, 0x33, 0x92, 0x4c, 0x90, 0x82, 0x09, 0x03, 0xbd, 0x32, 0x12,
  0x25, 0xb9, 0x18, 0xf3, 0x67, 0x3d, 0xb9, 0xd4, 0x82, 0x7c, 0xa6, 0xb3,
  0x59, 0xda, 0xd7, 0x1d, 0x45, 0x6b, 0xc8, 0x84, 0xc1, 0x62, 0x48, 0x75,
  0x91, 0x1b, 0x73, 0x6b, 0x8a, 0xa8, 0x25, 0x3e, 0x74, 0xc8, 0x1d, 0x65,
  0x52, 0x8d, 0x97, 0xda, 0xc2, 0xb1, 0x65, 0x79, 0x3d, 0x5b, 0x65, 0x36,
  0xf9, 0x9e, 0xee, 0x7b, 0x37, 0xe9, 0x14, 0x64, 0x5a, 0x95, 0xd8, 0xee,
  0xb8, 0x34, 0xc0, 0x19, 0x59, 0x1e, 0x64, 0x26, 0x56, 0xc5, 0x2d, 0xec,
  0x2a, 0x86, 0x9b, 0x3e, 0xd5, 0x91, 0xbe, 0xa3, 0xfe, 0x64, 0x5e, 0x82,
  0x10, 0xfb, 0xc3, 0x0a, 0xb4, 0xb2, 0x2e, 0xd7, 0x44, 0x7f, 0x17, 0x7b,
  0x75, 0x03, 0x8c, 0xd3, 0x2e, 0x86, 0x68, 0x3c, 0xc2, 0x2c, 0xcd, 0x0d,
  0xb3, 0x8a, 0xab, 0xc1, 0xba, 0xad, 0x0e, 0xf3, 0x72, 0xa8, 0x78, 0x8c,
  0xc8, 0xcc, 0xfc, 0x14, 0x3f, 0x87, 0x31, 0x73, 0xd2, 0x96, 0x8c, 0x4b,
  0x4a, 0x99, 0xa6, 0xb3, 0x9c, 0x49, 0x50, 0xa4, 0x07, 0x8f, 0xc5, 0x2d,
  0xa7, 0x8d, 0xd4, 0x0a, 0x3f, 0xb1, 0xf0, 0x30, 0xed, 0x75, 0x53, 0xdd,
  0xdb, 0x82, 0x06, 0x9e, 0xdf, 0xd5, 0x85, 0x3f, 0xee, 0x6d, 0xc1, 0xc1,
  0x23, 0x87, 0xd1, 0xd3, 0xd1, 0xea, 0xac, 0x18, 0x1d, 0x8d, 0x6d, 0xd0,
  0xa5, 0x60, 0x32, 0x25, 0xf6, 0xe6, 0x0b, 0xae, 0x4d, 0xf4, 0x67, 0x5e,
  0xce, 0x71, 0x14, 0x52, 0xf9, 0xdf, 0x31, 0x48, 0x88, 0xd5, 0x2e, 0x65,
  0xcf, 0x8a, 0x1f, 0x0a, 0x9b, 0x3e, 0x24, 0x4a, 0xd5, 0xec, 0xe4, 0x5e,
  0x5c, 0x84, 0xda, 0xe4, 0xf7, 0xc1, 0x60, 0x12, 0x6f, 0x27, 0x14, 0x04,
  0xf9, 0xa2, 0x5d, 0xdb, 0x43, 0x74, 0x5e, 0x02, 0xe3, 0x7e, 0xfe, 0x6a,
  0x4d, 0x5e, 0x4c, 0xfd, 0x46, 0x42, 0x74, 0x28, 0x6a, 0xf7, 0x41, 0xd6,
  0xeb, 0x05, 0xdc, 0x53, 0x27, 0x20, 0xb1, 0x76, 0x23, 0xdc, 0x4c, 0x59,
  0x53, 0x63, 0xc6, 0x21, 0x11, 0x4f, 0xfb, 0x23, 0x51, 0x32, 0x0a, 0xc9,
  0x9d, 0xfb, 0xd3, 0x51, 0x36, 0xeb, 0x81, 0x32, 0xfa, 0x96, 0x24, 0x55,
  0xdf, 0xb0, 0x58, 0x46, 0x63, 0x3c, 0x6a, 0xdb, 0x29, 0x24, 0x84, 0x8b,
  0x2f, 0xad, 0x31, 0xef, 0x51, 0x29, 0x5d, 0x26, 0x68, 0x8a, 0xf4, 0x41,
  0x1a, 0x9f, 0x2b, 0x08, 0xa4, 0x5c, 0x34, 0x1d, 0xa0, 0x85, 0xfb, 0x0c,
  0x36, 0x73, 0x9e, 0xa9, 0x55, 0x3e, 0xe8, 0xac, 0x34, 0x77, 0x74, 0x07,
  0x10, 0x9c, 0x3c, 0x0b, 0x13, 0x6f, 0xb8, 0x3d, 0x8d, 0x6c, 0x7a, 0x68,
  0xe7, 0x98, 0x73, 0x06, 0xfd, 0xee, 0x0d, 0x79, 0x69, 0xec, 0xda, 0xfb,
  0x5b, 0xad, 0x91, 0x8c, 0x33, 0x12, 0x54, 0xe4, 0x96, 0x36, 0x23, 0x10,
  0x95, 0xc1, 0x5c, 0xac, 0x23, 0xee, 0x64, 0x74, 0xd9, 0x8e, 0xa1, 0x1a,
  0xdd, 0x5e, 0xf0, 0xd0, 0x5a, 0x0d, 0x0f, 0xd5, 0xca, 0x5b, 0xaa, 0xcc,
  0x0a, 0xbb, 0xc3, 0x49, 0xd3, 0x5a, 0xf5, 0x18, 0xf4, 0xfa, 0x2f, 0x41,
  0x99, 0x50, 0x03, 0x6b, 0xd5, 0x5a, 0x68, 0xdc, 0x02, 0x47, 0x65, 0x0d,
  0xc1, 0x0c, 0x40, 0x1f, 0x57, 0x87, 0xd4, 0x59, 0xdc, 0x12, 0x7f, 0x71,
  0x07, 0x92, 0x99, 0x4d, 0xd4, 0xac, 0x85, 0xc9, 0xb3, 0x34, 0x8b, 0x24,
  0x97, 0xd8, 0x3c, 0xcb, 0x80, 0x95, 0x11, 0x5b, 0x1e, 0x4b, 0xe3, 0x49,
  0x02, 0x01, 0x6a, 0x8a, 0xa1, 0xab, 0x8c, 0xda, 0xd2, 0xa6, 0x93, 0xef,
  0xd6, 0x31, 0xb9, 0x2c, 0x6d, 0x0a, 0x92, 0xf9, 0x6d, 0x9e, 0x51, 0x58,
  0x7c, 0xc7, 0xb3, 0x3a, 0xee, 0x1c, 0x6c, 0xf1, 0xc7, 0x67, 0x02, 0x7b,
  0x9e, 0x5a, 0xae, 0x48, 0xc8, 0xca, 0xf9, 0x46, 0x8c, 0x65, 0x04, 0xc3,
  0x61, 0x27, 0x19, 0xe2, 0x2d, 0xcb, 0xd9, 0xfb, 0xe3, 0xd2, 0xcc, 0x60,
  0xe6, 0xec, 0xe7, 0x83, 0x03, 0x34, 0xcb, 0x51, 0xbb, 0x0b, 0xbe, 0x3e,
  0x5a, 0x94, 0xd5, 0x22, 0xb4, 0x76, 0x15, 0xb4, 0xd3, 0x4e, 0x93, 0xc0,
  0xc0, 0xf5, 0xc7, 0xd7, 0x1c, 0xe6, 0xe5, 0x75, 0x9c, 0x15, 0xe0, 0xaa,
  0x53, 0xce, 0x80, 0x31, 0xfb, 0x54, 0x98, 0xe1, 0x84, 0x63, 0xff, 0xf2,
  0x7e, 0x96, 0x24, 0x08, 0x01, 0x2e, 0xb9, 0x12, 0x08, 0xe9, 0x20, 0xe5,
  0x2f, 0xe9, 0xe4, 0x00, 0x69, 0x5f, 0x21, 0xdd, 0x90, 0xfc, 0x49, 0xe6,
  0xc7, 0xfa, 0xd2, 0x72, 0xdd, 0xdf, 0x11, 0xc5, 0xbb, 0x31, 0xc6, 0x01,
  0xd7, 0xdd, 0x32, 0xfa, 0xfd, 0xaa, 0x9f, 0x1d, 0x77, 0xd8, 0x00, 0x88,
  0xd0, 0xbb, 0xe5, 0xae, 0x24, 0xab, 0x42, 0x95, 0x25, 0x4e, 0x71, 0x04,
  0xb2, 0xb4, 0xc6, 0x15, 0x40, 0x63, 0xa0, 0xe1, 0xf1, 0x79, 0x1d, 0x0d,
  0x08, 0xaa, 0x26, 0x55, 0x6d, 0xca, 0x8e, 0xec, 0xe0, 0xfd, 0xcf, 0x15,
  0x75, 0xd3, 0xdb, 0x8a, 0x6e, 0x7c, 0xfc, 0x57, 0xd6, 0xa1, 0x30, 0xe2,
  0x07, 0x9a, 0x61, 0x9c, 0x3c, 0x0e, 0xd1, 0xbd, 0x87, 0x91, 0xf7, 0xc4,
  0x1f, 0x98, 0xfa, 0xa6, 0xcd, 0x40, 0x9e, 0xab, 0xce, 0x99, 0x0d, 0xd7,
  0x8c, 0x19, 0x50, 0x43, 0xf2, 0xbb, 0x98, 0x34, 0x25, 0xb9, 0x93, 0x9a,
  0xfd, 0x85, 0x6d, 0x0d, 0xed, 0xac, 0x1b, 0x4a, 0x30, 0xe4, 0x2f, 0x66,
  0x98, 0x8e, 0xaf, 0x90, 0x2d, 0xb3, 0xbe, 0xa3, 0x89, 0x79, 0xcb, 0xbe,
  0x9e, 0x04, 0xe4, 0x79, 0xfc, 0x37, 0x7f, 0x3a, 0xaf, 0x6e, 0xfe, 0x85,
  0xe9, 0x75, 0x38, 0x33, 0x5e, 0xff, 0xd3, 0x60, 0x9d, 0xe8, 0xff, 0xb4,
  0xdf, 0xb5, 0xfc, 0x94, 0x4d, 0x7e, 0xdd, 0xa1, 0x30, 0x03, 0x94, 0x85,
  0x11, 0x49, 0x8a, 0x9b, 0xa1, 0x2c, 0x97, 0x42, 0xee, 0x59, 0x30, 0x2e,
  0x50, 0xb0, 0x27, 0xae, 0x25, 0xe7, 0xde, 0x1f, 0x48, 0xeb, 0x5e, 0xa6,
  0x6f, 0x74, 0xfb, 0x73, 0x37, 0xa7, 0x5a, 0x77, 0xdf, 0x9c, 0xe7, 0x1e,
  0xdf, 0x90, 0xb7, 0xe0, 0xf2, 0x29, 0x87, 0xdf, 0x7a, 0x35, 0x9e, 0xb8,
  0xe9, 0xf3, 0x6e, 0x7f, 0xcd, 0x38, 0xee, 0x18, 0xed, 0xe1, 0xc7, 0x4f,
  0x51, 0x68, 0x13, 0x6b, 0x11, 0x59, 0xb4, 0x00, 0x29, 0x7f, 0x21, 0x7a,
  0x58, 0x22, 0x93, 0x94, 0xda, 0xb0, 0x16, 0x6d, 0x1b, 0x77, 0x3a, 0xd7,
  0xa6, 0x2c, 0x9f, 0x0d, 0x93, 0xd4, 0x42, 0xfa, 0xa2, 0x90, 0x81, 0xaa,
  0xc9, 0x13, 0x70, 0xd2, 0x35, 0xb7, 0x8d, 0x1d, 0xb3, 0x70, 0x61, 0xf3,
  0x07, 0x75, 0x3d, 0x26, 0x00, 0xb4, 0x5f, 0x9d, 0x1f, 0x38, 0x18, 0xce,
  0xa7, 0x2d, 0xfd, 0x47, 0x4a, 0x70, 0x15, 0xe8, 0x4d, 0x42, 0x3a, 0x71,
  0xae, 0x36, 0xd8, 0x14, 0xd0, 0xcc, 0x79, 0xf7, 0x07, 0x68, 0x14, 0x47,
  0x49, 0x34, 0xad, 0xf3, 0x5a, 0xaf, 0x2c, 0x8f, 0x49, 0xcf, 0x64, 0xb7,
  0x73, 0x2a, 0x9a, 0x87, 0x32, 0xe1, 0x22, 0x1c, 0x69, 0xe9, 0x42, 0xec,
  0xf5, 0x97, 0x18, 0x3f, 0xd0, 0x63, 0x91, 0x2a, 0xbc, 0x02, 0xe1, 0xf2,
  0x72, 0x28, 0x8b, 0x2e, 0x45, 0x77, 0xa5, 0x93, 0x7f, 0xa1, 0xc7, 0xce,
  0x87, 0xeb, 0x60, 0x13, 0x02, 0x1b, 0xd8, 0xae, 0x23, 0xe4, 0x78, 0xfb,
  0x0f, 0x02, 0x22, 0x41, 0xd5, 0xf7, 0x50, 0x1b, 0xe4, 0xb9, 0xe6, 0xdc,
  0x8b, 0xb0, 0xf4, 0xce, 0x07, 0x8f, 0xc9, 0x9b, 0x9c, 0x7b, 0x80, 0xad,
  0xca, 0x1b, 0xfd, 0xc9, 0x88, 0xed, 0xbc, 0x4e, 0xec, 0x5e, 0xfe, 0xb6,
  0xd6, 0xbe, 0x19, 0x4a, 0x21, 0xfd, 0xac, 0x4a, 0xe7, 0x27, 0xe8, 0xf9,
  0xa2, 0xb4, 0x1c, 0xd6, 0x03, 0x64, 0x6f, 0x3d, 0x0f, 0x78, 0xaf, 0x7b,
  0x30, 0xf3, 0x72, 0x9c, 0x1b, 0xaf, 0x9c, 0xf6, 0x0d, 0x57, 0xfb, 0xfa,
  0x45, 0x42, 0xab, 0x86, 0xca, 0x5d, 0x20, 0x21, 0x23, 0xc5, 0xed, 0x6f,
  0xc0, 0xdb, 0xda, 0x4a, 0x30, 0x68, 0xf7, 0x8c, 0xf8, 0xba, 0xc2, 0x54,
  0x7f, 0x6e, 0x79, 0xa7, 0x42, 0x0c, 0x84, 0x1a, 0x76, 0xd1, 0xfb, 0x9b,
  0xe8, 0x29, 0xaa, 0xa4, 0x76, 0x31, 0x7b, 0x2c, 0x29, 0xa2, 0xd3, 0x1c,
  0x8f, 0xb8, 0x9b, 0x1f, 0x47, 0x05, 0xbb, 0xa1, 0xc9, 0xfa, 0x3f, 0x3f,
  0xa1, 0x93, 0xbf, 0x24, 0xa3, 0x78, 0x06, 0x80, 0x39, 0x4a, 0x69, 0x7c,
  0xb7, 0x93, 0xe7, 0xe1, 0xf4, 0x1b, 0x6e, 0x7c, 0xf0, 0xec, 0x6b, 0x6e,
  0x9a, 0x9d, 0x7b, 0x70, 0x8c, 0x8b, 0x21, 0x57, 0x01, 0xb3, 0xf5, 0xb1,
  0x88, 0xb2, 0xf1, 0x62, 0x9f, 0xd2, 0xc3, 0xec, 0x8d, 0xf1, 0xb8, 0xf3,
  0x31, 0x23, 0x79, 0x56, 0x4c, 0xd6, 0xf2, 0xe5, 0xc4, 0x14, 0x54, 0xb4,
  0xd7, 0xc6, 0x6e, 0xeb, 0x24, 0xe3, 0xf4, 0xfb, 0x37, 0x10, 0x86, 0xc1,
  0xe4, 0xf2, 0xf8, 0x97, 0xca, 0x6a, 0x92, 0xb4, 0xf0, 0x94, 0xc6, 0xc2,
  0xa8, 0x87, 0x19, 0x4e, 0x75, 0x35, 0xdc, 0x2d, 0x6e, 0xf8, 0xf4, 0x66,
  0x7e, 0x28, 0x26, 0x9f, 0x28, 0x69, 0x10, 0x64, 0xf7, 0x70, 0x1c, 0xa6,
  0xb1, 0x05, 0x2e, 0xa6, 0xbb, 0xdd, 0x35, 0xd3, 0xe1, 0xaa, 0xaf, 0x45,
  0xaa, 0x78, 0x0c, 0x70, 0x5a, 0x35, 0xf4, 0xd3, 0xce, 0x44, 0xe2, 0xc8,
  0x41, 0xb8, 0x7b, 0xda, 0x61, 0x76, 0xb6, 0xcb, 0xd6, 0xb2, 0xb6, 0xe1,
  0x90, 0x5e, 0x39, 0x0a, 0x53, 0x2f, 0x5a, 0xd6, 0x3a, 0xff, 0xf2, 0xab,
  0x6b, 0xb0, 0xe2, 0x85, 0xcc, 0xdd, 0xe1, 0x9d, 0x86, 0x54, 0x13, 0xeb,
  0xf1, 0x92, 0xb4, 0x67, 0x92, 0x99, 0x56, 0x8a, 0x52, 0xcf, 0x90, 0xe2,
  0x62, 0xf9, 0x39, 0x98, 0x42, 0x63, 0xd8, 0x8b, 0xe4, 0xe8, 0xcb, 0xef,
  0x99, 0xf6, 0x85, 0xff, 0xbc, 0x2e, 0xfb, 0xec, 0xfd, 0x67, 0xa3, 0x69,
  0xdd, 0x3c, 0xf5, 0x95, 0xe5, 0x1b, 0x44, 0xb2, 0x02, 0x56, 0x98, 0x55,
  0x5b, 0x59, 0x27, 0xa0, 0xc4, 0xcd, 0xde, 0x1e, 0xd8, 0x9d, 0xf4, 0xa1,
  0x81, 0x14, 0xfa, 0x18, 0x56, 0x4b, 0x53, 0x68, 0x65, 0x78, 0xdb, 0x39,
  0xef, 0x3c, 0xe8, 0x87, 0xde, 0x73, 0x54, 0x3e, 0xd8, 0x67, 0xc0, 0x53,
  0xce, 0xb0, 0xbb, 0xb6, 0x0e, 0x92, 0x41, 0xa3, 0x8c, 0xbf, 0x82, 0xd2,
  0x01, 0x53, 0xc8, 0x2d, 0xaf, 0xfa, 0xa9, 0xd3, 0x0e, 0xcf, 0x5f, 0xfc,
  0xf7, 0xf5, 0xf4, 0xf2, 0xe9, 0xf7, 0x1d, 0xd0, 0xe2, 0xd8, 0xff, 0x0c,
  0x09, 0x40, 0xf2, 0xd9, 0x85, 0xc2, 0x15, 0xd9, 0x42, 0x1d, 0xcf, 0x0c,
  0x22, 0x81, 0x60, 0x32, 0x62, 0x99, 0x15, 0x68, 0x2b, 0x5b, 0xf6, 0x56,
  0xdd, 0x15, 0x77, 0x9f, 0x73, 0xec, 0xe1, 0xa9, 0xe6, 0x8f, 0xce, 0x17,
  0xee, 0xe8, 0x01, 0x06, 0x31, 0x55, 0xac, 0x36, 0xf3, 0x9b, 0xa0, 0x10,
  0x1d, 0x67, 0x3f, 0x10, 0x8c, 0x48, 0x1c, 0x8d, 0x4d, 0x26, 0xf2, 0xbe,
  0xf9, 0x1f, 0x4b, 0xa7, 0x5f, 0x77, 0xcb, 0x9f, 0xe4, 0x98, 0x7b, 0x57,
  0xbf, 0x50, 0xde, 0xd1, 0xd9, 0xbe, 0x3f, 0xfc, 0xee, 0xfa, 0x62, 0x7b,
  0xcd, 0xca, 0xf4, 0x34, 0x05, 0x47, 0x77, 0x8b, 0x2b, 0xe7, 0x94, 0x43,
  0xfb, 0xec, 0x3d, 0xee, 0x39, 0x73, 0xe6, 0xf0, 0xad, 0x3e, 0x1a, 0x0d,
  0x09, 0x80, 0x2c, 0x1c, 0x06, 0xd7, 0xdd, 0xb1, 0xcd, 0xdd, 0xd3, 0x34,
  0x21, 0x16, 0xc3, 0x91, 0x54, 0x59, 0xf9, 0x9e, 0xaa, 0x0b, 0x6e, 0xf9,
  0x0c, 0x03, 0xaf, 0xe3, 0x42, 0x3a, 0xb2, 0xfd, 0xb9, 0x90, 0xe7, 0xcd,
  0xaf, 0x14, 0xc0, 0xeb, 0xa6, 0x0f, 0x60, 0xf0, 0x40, 0x52, 0xf8, 0x65,
  0xa8, 0x20, 0xe3, 0xb2, 0x60, 0x2a, 0x33, 0xc5, 0xc6, 0xf1, 0xcb, 0x52,
  0x93, 0xee, 0xfc, 0x5d, 0x16, 0xea, 0x1c, 0x47, 0xf1, 0x37, 0x7f, 0x26,
  0x82, 0xbf, 0xfe, 0xb1, 0x13, 0x5b, 0x14, 0xeb, 0x09, 0xd8, 0x3e, 0x46,
  0x7c, 0x8c, 0xf2, 0x64, 0x3c, 0x51, 0xfd, 0xeb, 0xbd, 0x43, 0xbe, 0x7b,
  0x6e, 0x80, 0x61, 0x5c, 0x0c, 0xe9, 0x03, 0x32, 0x2a, 0x35, 0x63, 0x18,
  0x63, 0x7c, 0x60, 0x13, 0xff, 0x8c, 0x4b, 0x0b, 0x83, 0x2f, 0xff, 0x4c,
  0xf8, 0xf6, 0xff, 0x0a, 0x6a, 0xe9, 0x61, 0x2e, 0xa1, 0x3e, 0x58, 0x5c,
  0xaa, 0x64, 0x25, 0xcc, 0xd5, 0x17, 0x47, 0x62, 0xe6, 0xa5, 0x62, 0xd2,
  0x97, 0x9f, 0x1e, 0xc4, 0xbc, 0x1c, 0xd4, 0xd8, 0xb6, 0x79, 0xd0, 0xd8,
  0x12, 0x84, 0x40, 0xa0, 0x71, 0xd0, 0xfd, 0x0f, 0x7b, 0x63, 0xd8, 0x81,
  0xd0, 0x87, 0x9d, 0x40, 0xf6, 0x2b, 0x5a, 0xf2, 0x7d, 0x25, 0x3a, 0xe9,
  0x06, 0x5b, 0xb4, 0xb1, 0x50, 0x92, 0x0a, 0x72, 0x4f, 0xcf, 0x80, 0x5d,
  0x34, 0x06, 0x91, 0x85, 0xdf, 0xea, 0xf4, 0x7d, 0xf9, 0xe9, 0x21, 0xdf,
  0xc1, 0xac, 0xac, 0x72, 0xa6, 0xb5, 0xf2, 0xd2, 0xd2, 0x97, 0x3b, 0x48,
  0x85, 0xce, 0x47, 0x50, 0x4c, 0x43, 0x5e, 0xff, 0xcd, 0x17, 0x3f, 0xca,
  0x3b, 0x65, 0xfb, 0x7e, 0x2c, 0x6a, 0x94, 0x1d, 0x6c, 0x18, 0x67, 0x25,
  0xb2, 0xfd, 0xf1, 0x4a, 0xdf, 0xc9, 0x57, 0x74, 0x0e, 0xc7, 0x69, 0x25,
  0x3a, 0xf6, 0xd5, 0x07, 0x6e, 0x5d, 0xd6, 0x20, 0x12, 0x2d, 0xce, 0xd0,
  0x05, 0x5c, 0x8a, 0x25, 0x35, 0x76, 0x98, 0xf0, 0x5d, 0xf1, 0xdd, 0x70,
  0xdd, 0xc5, 0x37, 0x53, 0x8f, 0x3e, 0x1a, 0x0d, 0x89, 0xfe, 0x47, 0x1b,
  0x76, 0xc8, 0xde, 0xc2, 0x3f, 0xe3, 0x1f, 0x3b, 0x86, 0xc3, 0xbc, 0x1c,
  0xc1, 0x53, 0x35, 0xf9, 0xa0, 0x6b, 0xf9, 0x37, 0x2d, 0x29, 0x79, 0xc9,
  0xbc, 0xf0, 0x71, 0x49, 0xe5, 0x2f, 0x5a, 0xc8, 0x80, 0x8a, 0x5b, 0xb5,
  0x43, 0xce, 0x72, 0x1c, 0x0f, 0x3e, 0x69, 0x00, 0x8e, 0xe3, 0xd5, 0xd2,
  0x4d, 0x2b, 0x16, 0x5c, 0xed, 0x12, 0x9f, 0xf9, 0x9e, 0x90, 0x8c, 0x2b,
  0x32, 0x00, 0xcb, 0x90, 0x7b, 0x2c, 0x03, 0x83, 0xbf, 0x25, 0x8a, 0x1f,
  0x7c, 0x6f, 0x5c, 0xf7, 0x53, 0x5f, 0xb3, 0xe3, 0x77, 0x9f, 0x6b, 0x1f,
  0x79, 0xfe, 0x96, 0x9d, 0x7f, 0x4b, 0xbc, 0x9f, 0xe0, 0xf5, 0x04, 0x02,
  0x27, 0x10, 0x38, 0x81, 0xc0, 0x88, 0x21, 0xf0, 0x7f, 0x0f, 0x37, 0xff,
  0xa5, 0x27, 0xf6, 0xa6, 0xdf, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4e,
  0x44, 0xae, 0x42, 0x60, 0x82
};

const size_t favicon_png_len = sizeof(favicon_png);

#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT 465
const char* emailSenderAccount = "your-email@gmail.com";
const char* emailSenderPassword = "your-app-specific-password";
const char* emailRecipient = "recipient@email.com";
const char* emailSubject = "Aquarium Control Logs";

SMTPSession smtp;

WebServer server(80);

WebSocketsServer webSocket = WebSocketsServer(81);

const int MAX_TEMP_FAILURES = 3;
int consecutiveTempFailures = 0;
bool hasTempError = false;

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

  StaticJsonDocument<2352> doc;
  doc.clear();

  File file = LittleFS.open("/logs.json", "r");
  if (!file) {
    server.send(404, "application/json", "{\"logs\":[]}");
    return;
  }

  DeserializationError error = deserializeJson(doc, file);
  file.close();
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

  StaticJsonDocument<2048> doc;
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
}

void resetWatchdog() {
  lastLoopTime = millis();
}

void checkWatchdog() {
  if (millis() - lastLoopTime > watchdogTimeout) {
    ESP.restart();
  }
}

bool validDateSync = false;

TaskHandle_t networkTask;
TaskHandle_t controlTask;

void attemptTimeSync() {
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    if (!validTimeSync || timeSyncErrorLogged) {
      storeLogEntry("Time and Date sync successful");
    }
    validTimeSync = true;
    validDateSync = true;
    lastNTPSync = millis();
    clearError();
    timeSyncErrorLogged = false;

    setTime(timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec,
            timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_year + 1900);
  } else {
    if (!timeSyncErrorLogged) {
      storeLogEntry("Time sync failed.");
      timeSyncErrorLogged = true;
    }
    indicateError();
  }
}

void onWifiConnected(WiFiEvent_t event, WiFiEventInfo_t info) {
  if (!wifiConnectionErrorLogged) {
    storeLogEntry("Connected to WiFi. IP: " + WiFi.localIP().toString());
    wifiConnectionErrorLogged = false;  // Reset the error flag on successful connection
  }
  attemptTimeSync();
}

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
  pinMode(ONE_WIRE_BUS, INPUT_PULLUP);
  digitalWrite(errorLEDPin, LOW);

  // Serial.begin(115200);
  // delay(2000);

  sensors.begin();

  if (!LittleFS.begin(true)) {
    storeLogEntry("Failed to mount FS");
  } else {
    spiffsInitialized = true;
  }

  wifiConnectHandler = WiFi.onEvent(onWifiConnected, ARDUINO_EVENT_WIFI_STA_CONNECTED);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  unsigned long wifiStartTime = millis();
  const unsigned long wifiTimeout = 20000;

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);

    if (millis() - wifiStartTime > wifiTimeout) {
      storeLogEntry("WiFi connection failed.");
      indicateError();
      break;
    }
  }

  if (WiFi.status() == WL_CONNECTED) {
    storeLogEntry("Connected to WiFi");
    storeLogEntry("IP Address: " + WiFi.localIP().toString());
    clearError();
  }

  server.on("/", HTTP_GET, handleRoot);
  server.on("/favicon.png", HTTP_GET, handleFavicon);
  server.on("/logs", HTTP_GET, handleLogsPage);
  server.on("/logs/data", HTTP_GET, handleGetLogs);
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
  server.on("/temperature/settings", HTTP_GET, handleGetTemperatureSettings);
  server.on("/temperature/save", HTTP_POST, handleSaveTemperatureSettings);
  server.begin();
  EEPROM.begin(EEPROM_SIZE);
  loadSchedulesFromEEPROM();
  loadTemperatureSettings();

  templaunch();

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  resetWatchdog();
  watchdogTicker.attach(1, checkWatchdog);

  xTaskCreatePinnedToCore(
    emailLoop,
    "emailTask",
    8192,
    NULL,
    1,
    &networkTask,
    0);

  xTaskCreatePinnedToCore(
    mainLoop,
    "mainTask",
    8192,
    NULL,
    1,
    &controlTask,
    1);
}

void indicateError() {
  if (!triggerederror) {
    storeLogEntry("Error triggered.");
    digitalWrite(errorLEDPin, HIGH);
    triggerederror = true;
  }
  hasError = true;
}

void clearError() {
  storeLogEntry("Error cleared.");
  digitalWrite(errorLEDPin, LOW);
  hasError = false;
  triggerederror = false;
  timeSyncErrorLogged = false;
  tempErrorLogged = false;
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

void loadTemperatureSettings() {
  if (temperatureData.empty()) {
    TemperatureData data;
    data.minTemp = 24;
    data.maxTemp = 29;
    data.enabled = false;
    temperatureData.push_back(data);
  }

  TemperatureData storedData;
  EEPROM.get(TEMP_SETTINGS_START_ADDR, storedData);

  if (storedData.minTemp >= 20 && storedData.minTemp <= 30 && storedData.maxTemp >= 20 && storedData.maxTemp <= 30 && storedData.minTemp < storedData.maxTemp) {
    temperatureData[0] = storedData;
    storeLogEntry("Temperature settings loaded from EEPROM");
  } else {
    storeLogEntry("Using default temperature settings");
    saveTemperatureSettings();
  }
}

void saveTemperatureSettings() {
  if (!temperatureData.empty()) {
    EEPROM.put(TEMP_SETTINGS_START_ADDR, temperatureData[0]);
    EEPROM.commit();
    storeLogEntry("Temperature settings saved to EEPROM");
  }
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
      storeLogEntry("WebSocket " + String(num) + " Disconnected!");
      break;
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(num);
        storeLogEntry("WebSocket " + String(num) + " Connected from " + ip.toString() + " url: " + String((char*)payload));

        String message = "{\"relay1\":" + String(relay1State || overrideRelay1) + ",\"relay2\":" + String(relay2State || overrideRelay2) + ",\"relay3\":" + String(relay3State || overrideRelay1) + ",\"temperature\":" + String(lastValidTemperature, 1) + ",\"relay1Name\":\"WaveMaker\"" + ",\"relay2Name\":\"Light\"" + ",\"relay3Name\":\"Air Pump\"}";
        webSocket.sendTXT(num, message);
      }
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
  server.sendHeader("Content-Length", String(favicon_png_len));
  server.sendHeader("Cache-Control", "max-age=31536000");
  server.send_P(200, "image/png", (char*)favicon_png, favicon_png_len);
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

        #temperature {
            font-size: 2rem;
            margin: 20px 0;
            text-align: center;
            padding: 20px;
            background-color: var(--card-color);
            border-radius: var(--border-radius);
            box-shadow: var(--shadow);
            color: var(--primary-color);
            transition: var(--transition);
        }

        .buttons {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(180px, 1fr));
            gap: 15px;
            margin-bottom: 30px;
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

        .schedule-table th, .schedule-table td {
            padding: 15px;
            text-align: center;
        }

        .schedule-table th {
            background-color: var(--primary-color);
            color: white;
            font-weight: 500;
        }

        .schedule-table th:first-child {
            border-top-left-radius: var(--border-radius);
        }

        .schedule-table th:last-child {
            border-top-right-radius: var(--border-radius);
        }

        .schedule-table tr:last-child td:first-child {
            border-bottom-left-radius: var(--border-radius);
        }

        .schedule-table tr:last-child td:last-child {
            border-bottom-right-radius: var(--border-radius);
        }

        .schedule-table tr:nth-child(even) {
            background-color: #f9f9f9;
        }

        .schedule-table tr {
            background-color: white;
            transition: var(--transition);
        }

        .schedule-table tr:hover {
            background-color: #f1f1f1;
        }

        .schedule-table td {
            border-bottom: 1px solid #eee;
        }

        .action-button {
            min-width: 100px;
            padding: 8px 12px;
            margin: 5px;
            border: none;
            border-radius: 4px;
            cursor: pointer;
            font-size: 0.9rem;
            font-weight: 500;
            transition: var(--transition);
            box-shadow: 0 2px 5px rgba(0,0,0,0.1);
        }

        .action-button:hover {
            transform: translateY(-2px);
            box-shadow: 0 4px 8px rgba(0,0,0,0.15);
        }

        .action-button:active {
            transform: translateY(1px);
        }

        .action-button.activate {
            background-color: var(--success-color);
            color: white;
        }

        .action-button.deactivate {
            background-color: var(--warning-color);
            color: #333;
        }

        .action-button.delete {
            background-color: var(--error-color);
            color: white;
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
            
            .schedule-table {
                display: block;
                width: 100%;
                overflow-x: auto;
            }
            
            .action-button {
                padding: 8px;
                margin: 3px;
                font-size: 0.8rem;
                min-width: 80px;
            }
        }
        .temp-control {
            background-color: var(--card-color);
            padding: 25px;
            border-radius: var(--border-radius);
            box-shadow: var(--shadow);
            margin-bottom: 25px;
            transition: var(--transition);
        }
        
        .temp-control:hover {
            box-shadow: 0 5px 15px rgba(0,0,0,0.15);
        }
        
        .temp-control h3 {
            color: var(--primary-color);
            margin-bottom: 15px;
            font-size: 1.5rem;
            border-bottom: 2px solid var(--primary-light);
            padding-bottom: 10px;
        }
        
        .temp-control .slider-container {
            margin: 25px 0;
        }
        
        .temp-control .slider-container label {
            display: block;
            margin-bottom: 8px;
            font-weight: 500;
        }
        
        .temp-control .range-values {
            display: flex;
            justify-content: space-between;
            margin-top: 10px;
            color: var(--text-light);
        }
        
        .temp-control .current-value {
            text-align: center;
            font-size: 1.2rem;
            font-weight: bold;
            color: var(--primary-color);
            margin: 10px 0;
        }
        
        .temp-control input[type="range"] {
            width: 100%;
            height: 8px;
            border-radius: 5px;
            background: #ddd;
            outline: none;
            -webkit-appearance: none;
        }
        
        .temp-control input[type="range"]::-webkit-slider-thumb {
            -webkit-appearance: none;
            appearance: none;
            width: 20px;
            height: 20px;
            border-radius: 50%;
            background: var(--primary-color);
            cursor: pointer;
            transition: var(--transition);
        }
        
        .temp-control input[type="range"]::-webkit-slider-thumb:hover {
            background: var(--primary-dark);
            transform: scale(1.1);
        }
        
        .temp-control .toggle-container {
            display: flex;
            align-items: center;
            margin: 20px 0;
        }
        
        .temp-control .toggle-switch {
            position: relative;
            display: inline-block;
            width: 60px;
            height: 34px;
            margin-right: 15px;
        }
        
        .temp-control .toggle-switch input {
            opacity: 0;
            width: 0;
            height: 0;
        }
        
        .temp-control .slider {
            position: absolute;
            cursor: pointer;
            top: 0;
            left: 0;
            right: 0;
            bottom: 0;
            background-color: #ccc;
            transition: .4s;
            border-radius: 34px;
        }
        
        .temp-control .slider:before {
            position: absolute;
            content: "";
            height: 26px;
            width: 26px;
            left: 4px;
            bottom: 4px;
            background-color: white;
            transition: .4s;
            border-radius: 50%;
        }
        
        .temp-control input:checked + .slider {
            background-color: var(--success-color);
        }
        
        .temp-control input:focus + .slider {
            box-shadow: 0 0 1px var(--success-color);
        }
        
        .temp-control input:checked + .slider:before {
            transform: translateX(26px);
        }
        
        .temp-control .toggle-label {
            font-size: 1rem;
            font-weight: 500;
        }
        
        .temp-control .temp-buttons {
            display: flex;
            justify-content: flex-end;
        }
        
        .temp-control .save-button {
            padding: 10px 20px;
            background-color: var(--primary-color);
            color: white;
            border: none;
            border-radius: var(--border-radius);
            cursor: pointer;
            transition: var(--transition);
            font-weight: 500;
        }
        
        .temp-control .save-button:hover {
            background-color: var(--primary-dark);
            transform: translateY(-2px);
        }
        
        .temp-control .temp-values-display {
            display: flex;
            justify-content: space-between;
            margin: 20px 0;
            font-size: 1.1rem;
        }
        
        .temp-control .temp-value-box {
            text-align: center;
            padding: 15px;
            background-color: #f5f7fa;
            border-radius: var(--border-radius);
            flex: 1;
            margin: 0 5px;
            box-shadow: 0 2px 5px rgba(0,0,0,0.05);
        }
        
        .temp-control .temp-value-box span {
            font-weight: bold;
            color: var(--primary-color);
        }
        
        .temp-control .temp-value-box.min-temp {
            border-left: 4px solid var(--warning-color);
        }
        
        .temp-control .temp-value-box.max-temp {
            border-left: 4px solid var(--error-color);
        }
        
        .temp-control .temp-value-box.current-temp {
            border-left: 4px solid var(--success-color);
        }

        .temp-control .heater-status {
            margin: 20px 0;
            padding: 15px;
            background-color: #f5f7fa;
            border-radius: var(--border-radius);
            border-left: 4px solid var(--primary-color);
        }

        .temp-control .status-indicator {
            display: flex;
            align-items: center;
        }

        .temp-control .status-dot {
            display: inline-block;
            width: 12px;
            height: 12px;
            border-radius: 50%;
            margin-right: 10px;
            background-color: #ccc;
        }

        .temp-control .status-dot.on {
            background-color: var(--success-color);
            box-shadow: 0 0 5px var(--success-color);
        }

        .temp-control .status-dot.off {
            background-color: var(--error-color);
            box-shadow: 0 0 5px var(--error-color);
        }

        .temp-control .status-text {
            font-size: 1.1rem;
            font-weight: 500;
        }
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
        <div id="temperature">Temperature: -- °C</div>
        <div class="buttons">
            <button class="button" onclick="toggleRelay(1)" id="btn1">WaveMaker</button>
            <button class="button" onclick="toggleRelay(3)" id="btn3">Air Pump</button>
            <button class="button" onclick="toggleRelay(2)" id="btn2">Light</button>
            <button class="button" onclick="oneClickLight()" id="btnOneClick">Change Light Color</button>
            <button class="button" onclick="showLogs()">Show Logs</button>
        </div>
        <div class="temp-control">
            <h3>Heater Control</h3>

            <div class="heater-status">
                <div class="status-indicator">
                    <span class="status-dot" id="heater-status-dot"></span>
                    <span class="status-text">Heater Status: <span id="heater-status">Unknown</span></span>
                </div>
            </div>
            
            <div class="temp-values-display">
                <div class="temp-value-box min-temp">
                    <div>Min Temperature</div>
                    <span id="min-temp-display">--</span> °C
                </div>
                <div class="temp-value-box max-temp">
                    <div>Max Temperature</div>
                    <span id="max-temp-display">--</span> °C
                </div>
            </div>
            
            <div class="slider-container">
                <label for="min-temp-slider">Minimum Temperature (°C):</label>
                <input type="range" id="min-temp-slider" min="20" max="30" step="0.5" value="24">
                <div class="current-value">
                    <span id="min-temp-value">24</span> °C
                </div>
                <div class="range-values">
                    <span>20°C</span>
                    <span>30°C</span>
                </div>
            </div>
            
            <div class="slider-container">
                <label for="max-temp-slider">Maximum Temperature (°C):</label>
                <input type="range" id="max-temp-slider" min="20" max="30" step="0.5" value="28">
                <div class="current-value">
                    <span id="max-temp-value">28</span> °C
                </div>
                <div class="range-values">
                    <span>20°C</span>
                    <span>30°C</span>
                </div>
            </div>
            
            <div class="toggle-container">
                <label class="toggle-switch">
                    <input type="checkbox" id="temp-control-toggle">
                    <span class="slider"></span>
                </label>
                <div class="toggle-label">Temperature Control</div>
            </div>
            
            <div class="temp-buttons">
                <button class="save-button" onclick="saveTemperatureSettings()">Save Settings</button>
            </div>
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
        <div id="errorSection">
            <p>Error detected!</p>
            <button id="clearErrorBtn" onclick="clearError()">Clear Error</button>
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
        <div class="log-section" id="logSection">
            <h3>Logs</h3>
            <pre id="logs"></pre>
        </div>
        <div id="successDialog">
        <p>Schedule added successfully!</p>
        <button onclick="closeSuccessDialog()">OK</button>
    </div>
    </div>
    <script>
        let relayStates = {
            1: false,
            2: false,
            3: false
        };

        let socket = new WebSocket('ws://' + window.location.hostname + ':81/');

        socket.onopen = () => console.log('WebSocket connected');
        socket.onmessage = (event) => {
            try {
                let data = JSON.parse(event.data);
                
                if (data.relay1Name) window.relay1Name = data.relay1Name;
                if (data.relay2Name) window.relay2Name = data.relay2Name;
                if (data.relay3Name) window.relay3Name = data.relay3Name;
                if (data.relay4Name) window.relay4Name = data.relay4Name;
                
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
                if (data.relay4 !== undefined) {
                    updateHeaterStatus(data.relay4);
                }
                if (data.temperature !== undefined) {
                    document.getElementById('temperature').textContent = 
                        `Temperature: ${data.temperature} °C`;
                    document.getElementById('current-temp-display').textContent = data.temperature;
                }
                if (data.minTemp !== undefined) {
                    document.getElementById('min-temp-display').textContent = data.minTemp;
                }
                if (data.maxTemp !== undefined) {
                    document.getElementById('max-temp-display').textContent = data.maxTemp;
                }
                if (data.tempControlEnabled !== undefined) {
                    document.getElementById('temp-control-status').textContent = 
                        data.tempControlEnabled ? 'Enabled' : 'Disabled';
                }
            } catch (e) {
                console.error('WebSocket error:', e);
            }
        };
        socket.onclose = () => checkErrorStatus();
        socket.onerror = () => checkErrorStatus();

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
                })
                .catch(error => { alert(error.message); checkErrorStatus(); });
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
                alert('Failed to add schedule: ' + error.message); 
                checkErrorStatus(); 
            });
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
                .then(data => { if (data.status === 'success') { loadSchedules(); checkErrorStatus(); } else { throw new Error('Failed to delete schedule'); } })
                .catch(error => { alert('Failed to delete schedule: ' + error.message); checkErrorStatus(); });
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
            .then(() => { loadSchedules(); checkErrorStatus(); })
            .catch(error => { alert('Failed to update schedule: ' + error.message); checkErrorStatus(); });
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

        function updateButtonStyle(relay) {
            const btn = document.getElementById('btn' + relay);
            if (btn) {
                btn.className = 'button ' + (relayStates[relay] ? 'on' : 'off');
                let relayLabel = "Unknown";
                
                if (relay === 1 && window.relay1Name) relayLabel = window.relay1Name;
                else if (relay === 2 && window.relay2Name) relayLabel = window.relay2Name;
                else if (relay === 3 && window.relay3Name) relayLabel = window.relay3Name;
                else if (relay === 1) relayLabel = "WaveMaker";
                else if (relay === 2) relayLabel = "Light";
                else if (relay === 3) relayLabel = "Air Pump";
                
                btn.textContent = `${relayLabel} (${relayStates[relay] ? 'ON' : 'OFF'})`;
            }
        }

        function getInitialStates() {
            fetch('/relay/status')
                .then(response => response.json())
                .then(data => { 
                    relayStates = data; 
                    for(let relay in relayStates) updateButtonStyle(relay);
                    if (data["4"] !== undefined) {
                        updateHeaterStatus(data["4"]);
                    }
                })
                .catch(() => checkErrorStatus());
        }

        function checkErrorStatus() {
            fetch('/error/status')
                .then(response => response.json())
                .then(data => {
                    document.getElementById('errorSection').style.display = data.hasError ? 'block' : 'none';
                })
                .catch(() => {
                    document.getElementById('errorSection').style.display = 'block';
                });
        }

        function clearError() {
            fetch('/error/clear', { method: 'POST' })
                .then(response => response.ok ? response.json() : { status: 'error' })
                .then(data => { if (data.status === 'success') { document.getElementById('errorSection').style.display = 'none'; } else { throw new Error('Failed to clear error'); } })
                .catch(error => { alert('Failed to clear error: ' + error.message); });
        }

        function showLogs() {
            window.location.href = '/logs';
        }

        function oneClickLight() {
            fetch('/relay/oneclick', { method: 'POST' })
            .then(response => response.json().then(data => {
                if (!response.ok) throw new Error(data.error);
                alert('Light colour changed successfully.');
            }))
            .catch(error => alert(error.message));
        }
        
        function updateTemperatureSliders() {
            document.getElementById('min-temp-value').textContent = document.getElementById('min-temp-slider').value;
            document.getElementById('max-temp-value').textContent = document.getElementById('max-temp-slider').value;
        }

        function loadTemperatureSettings() {
            fetch('/temperature/settings')
                .then(response => response.json())
                .then(data => {
                    document.getElementById('min-temp-slider').value = data.minTemp;
                    document.getElementById('max-temp-slider').value = data.maxTemp;
                    document.getElementById('temp-control-toggle').checked = data.enabled;
                    
                    // Update display values
                    document.getElementById('min-temp-value').textContent = data.minTemp;
                    document.getElementById('max-temp-value').textContent = data.maxTemp;
                    document.getElementById('min-temp-display').textContent = data.minTemp;
                    document.getElementById('max-temp-display').textContent = data.maxTemp;
                    document.getElementById('current-temp-display').textContent = data.currentTemp;
                    document.getElementById('temp-control-status').textContent = data.enabled ? 'Enabled' : 'Disabled';
                })
                .catch(error => {
                    console.error('Error loading temperature settings:', error);
                });
        }

        function saveTemperatureSettings() {
            const minTemp = parseInt(document.getElementById('min-temp-slider').value);
            const maxTemp = parseInt(document.getElementById('max-temp-slider').value);
            const enabled = document.getElementById('temp-control-toggle').checked;
            
            if (minTemp >= maxTemp) {
                alert('Minimum temperature must be less than maximum temperature!');
                return;
            }
            
            const settings = {
                minTemp: minTemp,
                maxTemp: maxTemp,
                enabled: enabled
            };
            
            fetch('/temperature/save', {
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
                alert('Temperature settings saved successfully!');
                loadTemperatureSettings(); // Reload to confirm settings
            })
            .catch(error => {
                alert('Failed to save settings: ' + error.message);
            });
        }

        function updateHeaterStatus(isOn) {
            const statusDot = document.getElementById('heater-status-dot');
            const statusText = document.getElementById('heater-status');
            
            if (statusDot && statusText) {
                statusDot.className = 'status-dot ' + (isOn ? 'on' : 'off');
                statusText.textContent = isOn ? 'ON' : 'OFF';
            }
        }
                
        document.getElementById('min-temp-slider').addEventListener('input', updateTemperatureSliders);
        document.getElementById('max-temp-slider').addEventListener('input', updateTemperatureSliders);
        

        setInterval(updateTime, 1000);
        setInterval(checkErrorStatus, 2000);
        updateTime();
        loadSchedules();
        getInitialStates();
        checkErrorStatus();
        loadTemperatureSettings();
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
            
            .header-actions {
                flex-direction: column;
                align-items: stretch;
            }
            
            .button {
                width: 100%;
                margin: 5px 0;
                text-align: center;
            }
            
            .refresh-button {
                float: none;
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
    </style>
</head>
<body>
    <header>
        <h1>System Logs</h1>
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
        function loadLogs() {
            document.getElementById('loading').style.display = 'block';
            
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
                    
                    document.getElementById('loading').style.display = 'none';
                })
                .catch(error => {
                    console.error('Error loading logs:', error);
                    document.getElementById('loading').style.display = 'none';
                });
        }

        function refreshLogs() {
            loadLogs();
        }

        function goBack() {
            window.history.back();
        }

        loadLogs();
        setInterval(loadLogs, 10000);
    </script>
</body>
</html>
)html";

unsigned long lastWifiConnectAttempt = 0;
const unsigned long WIFI_RECONNECT_INTERVAL = 30000;

void handleLogsPage() {
  server.send_P(200, "text/html", logsPage);
}

void loop() {
  vTaskDelete(NULL);
}

void emailLoop(void* parameter) {
  for (;;) {
    handleTemperature();

    if (WiFi.status() != WL_CONNECTED) {
      unsigned long currentMillis = millis();
      if (currentMillis - lastWifiConnectAttempt > WIFI_RECONNECT_INTERVAL) {
        if (!wifiConnectionErrorLogged) {
          storeLogEntry("Attempting to reconnect to WiFi...");
          wifiConnectionErrorLogged = true;
        }
        WiFi.reconnect();
        lastWifiConnectAttempt = currentMillis;
      }
    } else {
      if (wifiConnectionErrorLogged) {
        storeLogEntry("WiFi connection restored");
        wifiConnectionErrorLogged = false;
      }

      if (!validTimeSync) {
        attemptTimeSync();
      }

      if (!startupemail) {
        delay(1500);
        sendEmailWithLogs("Device is powered on");
        startupemail = true;

        struct tm timeinfo;
        if (getLocalTime(&timeinfo)) {
          last90MinCheck = timeinfo.tm_hour * 3600 + timeinfo.tm_min * 60 + timeinfo.tm_sec;
        }
      }

      if (pointemail) {
        sendEmailWithLogs("Status Check");
        pointemail = false;
      }
    }
    delay(600);
  }
}

void mainLoop(void* parameter) {
  for (;;) {
    server.handleClient();
    webSocket.loop();
    resetWatchdog();
    checkoverride1();
    checkoverride2();
    overrideLEDState();
    checkTemperatureControl();

    if (!validTimeSync && WiFi.status() == WL_CONNECTED) {
      unsigned long currentMillis = millis();
      if (currentMillis - lastNtpRetry >= NTP_RETRY_INTERVAL) {
        attemptTimeSync();
        lastNtpRetry = currentMillis;
      }
    }

    static unsigned long lastSecondCheck = 0;
    if (millis() - lastSecondCheck >= 1000) {
      lastSecondCheck = millis();

      if (validTimeSync) {
        checkSchedules();

        struct tm timeinfo;
        if (getLocalTime(&timeinfo)) {
          unsigned long currentSeconds = timeinfo.tm_hour * 3600 + timeinfo.tm_min * 60 + timeinfo.tm_sec;

          // 90 minute check
          if (currentSeconds - last90MinCheck >= CHECK_90MIN_INTERVAL || (last90MinCheck > currentSeconds && currentSeconds >= 0)) {
            String timeStr = String(timeinfo.tm_hour) + ":" + (timeinfo.tm_min < 10 ? "0" : "") + String(timeinfo.tm_min);
            storeLogEntry("Device is powered on at " + timeStr);
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

    if (millis() - lastNTPSync > 43200000) {
      configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
      struct tm timeinfo;
      if (getLocalTime(&timeinfo)) {
        lastNTPSync = millis();
        if (timeSyncErrorLogged) {
          storeLogEntry("Regular time sync successful");
          timeSyncErrorLogged = false;
        }

        setTime(timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec,
                timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_year + 1900);
      } else {
        if (!timeSyncErrorLogged) {
          storeLogEntry("Regular time sync failed.");
          timeSyncErrorLogged = true;
        }
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
      activateRelay(1, false);
      storeLogEntry("Relay 1 activated by startup schedule check");
    } else {
      deactivateRelay(1, false);
      storeLogEntry("Relay 1 deactivated by startup schedule check");
    }

    if (relay3ShouldBeOn) {
      activateRelay(3, false);
      storeLogEntry("Relay 3 activated by startup schedule check");
    } else {
      deactivateRelay(3, false);
      storeLogEntry("Relay 3 deactivated by startup schedule check");
    }
  }

  if (!overrideRelay2) {
    if (relay2ShouldBeOn) {
      activateRelay(2, false);
      storeLogEntry("Relay 2 activated by startup schedule check");
    } else {
      deactivateRelay(2, false);
      storeLogEntry("Relay 2 deactivated by startup schedule check");
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
      storeLogEntry("Relay 1 activated.");
      break;
    case 2:
      toggleLightSequence();
      storeLogEntry("Relay 2 activated with toggle sequence.");
      break;
    case 3:
      digitalWrite(relay3, LOW);
      relay3State = true;
      storeLogEntry("Relay 3 (Air Pump) activated.");
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
      storeLogEntry("Relay 1 deactivated.");
      break;
    case 2:
      digitalWrite(relay2, HIGH);
      relay2State = false;
      storeLogEntry("Relay 2 deactivated.");
      break;
    case 3:
      digitalWrite(relay3, HIGH);
      relay3State = false;
      storeLogEntry("Relay 3 (Air Pump) deactivated.");
      break;
  }
  broadcastRelayStates();
}

void broadcastRelayStates() {
  String message = "{\"relay1\":" + String(relay1State || overrideRelay1) + ",\"relay2\":" + String(relay2State || overrideRelay2) + ",\"relay3\":" + String(relay3State || overrideRelay1) + ",\"relay4\":" + String(relay4State) + ",\"temperature\":" + String(lastValidTemperature, 1);

  if (!temperatureData.empty()) {
    message += ",\"tempControlEnabled\":" + String(temperatureData[0].enabled ? "true" : "false") + ",\"minTemp\":" + String(temperatureData[0].minTemp) + ",\"maxTemp\":" + String(temperatureData[0].maxTemp);
  }

  message += ",\"relay1Name\":\"WaveMaker\"";
  message += ",\"relay2Name\":\"Light\"";
  message += ",\"relay3Name\":\"Air Pump\"";
  message += ",\"relay4Name\":\"Heater\"}";

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
      clearError();
      broadcastRelayStates();
      return;
    }
    indicateError();
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
      clearError();
      broadcastRelayStates();
      return;
    }
    indicateError();
  }
  storeLogEntry("Invalid delete request");
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
        clearError();
        broadcastRelayStates();
        return;
      } else {
        server.send(400, "application/json", "{\"error\":\"Invalid schedule ID\"}");
        storeLogEntry("Invalid schedule update request for ID: " + String(id));
        indicateError();
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

void toggleRelay(int relayPin, bool& relayState) {
  if ((relayPin == relay1 && overrideRelay1) || (relayPin == relay2 && overrideRelay2)) {
    storeLogEntry("Physical override active, ignoring toggle.");
    return;
  }
  relayState = !relayState;
  digitalWrite(relayPin, relayState ? LOW : HIGH);
  storeLogEntry("Relay state changed to: " + String(relayState));
  broadcastRelayStates();
}

void handleRelay1() {
  if (server.method() == HTTP_POST) {
    if (overrideRelay1) {
      server.send(403, "application/json", "{\"error\":\"Physical override active\"}");
      return;
    }
    toggleRelay(relay1, relay1State);
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

    if (!relay2State) {
      toggleLightSequence();
    } else {
      digitalWrite(relay2, HIGH);
      relay2State = false;
      storeLogEntry("Relay 2 deactivated.");
    }

    server.send(200, "application/json", "{\"state\":" + String(relay2State) + "}");
    broadcastRelayStates();
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
    toggleRelay(relay3, relay3State);
    server.send(200, "application/json", "{\"state\":" + String(relay3State) + "}");
  } else if (server.method() == HTTP_GET) {
    server.send(200, "application/json", "{\"state\":" + String(relay3State) + "}");
  }
}

void toggleLightSequence() {
  for (int i = 0; i < TOGGLE_COUNT; i++) {
    digitalWrite(relay2, HIGH);
    delay(TOGGLE_DELAY);
    digitalWrite(relay2, LOW);
    delay(TOGGLE_DELAY);
  }
  digitalWrite(relay2, LOW);
  relay2State = true;
  storeLogEntry("Light relay toggled sequence completed");
  broadcastRelayStates();
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
  json += "\"4\":" + String(relay4State) + "}";
  server.send(200, "application/json", json);
}

void handleClearError() {
  clearError();
  server.send(200, "application/json", "{\"status\":\"success\"}");
}

void handleGetErrorStatus() {
  String json = "{\"hasError\":" + String(hasError ? "true" : "false") + "}";
  server.send(200, "application/json", json);
}

void handleOneClickLight() {
  if (relay2State || overrideRelay2) {
    digitalWrite(relay2, HIGH);
    relay2State = false;
    delay(500);
    digitalWrite(relay2, LOW);
    relay2State = true;
    server.send(200, "application/json", "{\"status\":\"success\"}");
    storeLogEntry("Relay 2 toggled off-on via One Click.");
  } else {
    server.send(403, "application/json", "{\"error\":\"Light is off\"}");
    storeLogEntry("One Click failed: Light is off.");
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
    storeLogEntry("Relay 1 and 3 override activated");
    broadcastRelayStates();
  } else if (!currentReading && overrideRelay1) {
    overrideRelay1 = false;
    if (relay1State) {
      deactivateRelay(1, true);
    }
    if (relay3State) {
      deactivateRelay(3, true);
    }
    storeLogEntry("Relay 1 and 3 override deactivated");
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
    storeLogEntry("Relay 2 override activated");
    broadcastRelayStates();
  } else if (!currentReading && overrideRelay2) {
    overrideRelay2 = false;
    if (relay2State) {
      deactivateRelay(2, true);
    }
    storeLogEntry("Relay 2 override deactivated");
    broadcastRelayStates();
  }
}

void overrideLEDState() {
  bool anyOverrideActive = overrideRelay1 || overrideRelay2;

  if (hasError) {
    return;
  }

  if (anyOverrideActive) {
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
  if (!WiFi.isConnected()) {
    storeLogEntry("Failed to send email: No WiFi connection");
    return;
  }

  if (!LittleFS.exists("/logs.json")) {
    storeLogEntry("Failed to send email: logs.json does not exist");
    return;
  }

  sensors.requestTemperatures();
  float currentTempC = sensors.getTempC(sensorAddress);
  if (currentTempC != DEVICE_DISCONNECTED_C) {
    lastValidTemperature = currentTempC;
  }

  MailClient.networkReconnect(true);
  smtp.debug(0);
  smtp.setTCPTimeout(10000);

  Session_Config config;
  config.server.host_name = SMTP_HOST;
  config.server.port = SMTP_PORT;
  config.login.email = emailSenderAccount;
  config.login.password = emailSenderPassword;
  config.login.user_domain = "";

  SMTP_Message message;
  message.sender.name = "Aquarium Control";
  message.sender.email = emailSenderAccount;
  message.subject = String(emailSubject) + " - " + trigger;
  message.addRecipient("User", emailRecipient);

  struct tm timeinfo;
  String formattedTime = "Unknown";
  if (getLocalTime(&timeinfo)) {
    char timeStr[20];
    sprintf(timeStr, "%02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    formattedTime = String(timeStr);
  }

  String textMsg = "Aquarium Control System Report\n";
  textMsg += "Event: " + trigger + "\n";
  textMsg += "Timestamp: " + formattedTime + "\n\n";
  textMsg += "System Status:\n";
  textMsg += "Temperature: " + String(lastValidTemperature, 1) + " °C\n";
  textMsg += "Relay 1 (WaveMaker): " + String(relay1State ? "ON" : "OFF") + "\n";
  textMsg += "Relay 2 (Light): " + String(relay2State ? "ON" : "OFF") + "\n";
  textMsg += "Relay 3 (Air Pump): " + String(relay3State ? "ON" : "OFF") + "\n";
  textMsg += "Override 1: " + String(overrideRelay1 ? "Active" : "Inactive") + "\n";
  textMsg += "Override 2: " + String(overrideRelay2 ? "Active" : "Inactive") + "\n";
  textMsg += "Error Status: " + String(hasError ? "Error Present" : "No Errors") + "\n\n";
  textMsg += "Full logs are attached as logs.json";

  message.text.content = textMsg.c_str();
  message.text.charSet = "us-ascii";
  message.text.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

  File logsFile = LittleFS.open("/logs.json", "r");
  if (!logsFile) {
    storeLogEntry("Failed to open logs file for email");
    return;
  }

  String logsContent = "";
  while (logsFile.available()) {
    logsContent += (char)logsFile.read();
  }
  logsFile.close();

  if (logsContent.length() == 0) {
    storeLogEntry("Logs file is empty");
    return;
  }

  SMTP_Attachment att;
  att.descr.filename = "logs.json";
  att.descr.mime = "application/json";
  att.descr.transfer_encoding = Content_Transfer_Encoding::enc_base64;
  att.blob.data = (uint8_t*)logsContent.c_str();
  att.blob.size = logsContent.length();
  message.addAttachment(att);

  if (!smtp.connect(&config)) {
    storeLogEntry("Failed to connect to email server");
    return;
  }

  if (!MailClient.sendMail(&smtp, &message)) {
    storeLogEntry("Failed to send email: " + smtp.errorReason());
  } else {
    storeLogEntry("Email sent successfully with logs");
  }

  smtp.closeSession();
}

void handleTemperature() {
  if (millis() - lastTemp >= 5000) {
    sensors.requestTemperatures();
    float tempC = sensors.getTempC(sensorAddress);

    if (tempC != DEVICE_DISCONNECTED_C) {
      currentTemp = tempC;
      lastValidTemperature = tempC;
      broadcastRelayStates();
      consecutiveTempFailures = 0;
      if (hasTempError) {
        clearError();
        hasTempError = false;
        tempErrorLogged = false;
      }
    } else {
      consecutiveTempFailures++;
      if (consecutiveTempFailures >= MAX_TEMP_FAILURES) {
        if (!tempErrorLogged) {
          storeLogEntry("Error: Temperature sensor failed " + String(consecutiveTempFailures) + " times");
          tempErrorLogged = true;
        }
        indicateError();
        hasTempError = true;
      }
    }

    lastTemp = millis();
  }
}

void templaunch() {
  sensors.requestTemperatures();
  float tempC = sensors.getTempC(sensorAddress);

  if (tempC != DEVICE_DISCONNECTED_C) {
    currentTemp = tempC;
    lastValidTemperature = tempC;
    broadcastRelayStates();
    consecutiveTempFailures = 0;
    if (hasTempError) {
      clearError();
      hasTempError = false;
      tempErrorLogged = false;
    }
  }
}

void handleGetTemperatureSettings() {
  String json = "{";
  json += "\"minTemp\":" + String(temperatureData[0].minTemp) + ",";
  json += "\"maxTemp\":" + String(temperatureData[0].maxTemp) + ",";
  json += "\"enabled\":" + String(temperatureData[0].enabled ? "true" : "false") + ",";
  json += "\"currentTemp\":" + String(lastValidTemperature, 1);
  json += "}";
  server.send(200, "application/json", json);
}

void handleSaveTemperatureSettings() {
  if (server.hasArg("plain")) {
    String body = server.arg("plain");
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, body);

    if (!error) {
      int minTemp = doc["minTemp"];
      int maxTemp = doc["maxTemp"];
      bool enabled = doc["enabled"];

      if (minTemp < maxTemp) {
        if (temperatureData.empty()) {
          TemperatureData data;
          temperatureData.push_back(data);
        }

        temperatureData[0].minTemp = minTemp;
        temperatureData[0].maxTemp = maxTemp;
        temperatureData[0].enabled = enabled;

        saveTemperatureSettings();

        server.send(200, "application/json", "{\"status\":\"success\"}");
        storeLogEntry("Temperature control settings updated: Min=" + String(minTemp) + "°C, Max=" + String(maxTemp) + "°C, Enabled=" + String(enabled ? "Yes" : "No"));
        return;
      } else {
        server.send(400, "application/json", "{\"error\":\"Minimum temperature must be less than maximum temperature\"}");
        return;
      }
    }
  }
  server.send(400, "application/json", "{\"error\":\"Invalid request\"}");
}

void checkTemperatureControl() {
  if (temperatureData.empty()) {
    return;
  }

  if (!temperatureData[0].enabled) {
    if (!relay4State) {
      digitalWrite(relay4, LOW);
      relay4State = true;
      storeLogEntry("Heater turned ON - Heater Control Disabled");
      broadcastRelayStates();
    }
    return;
  }

  if (hasTempError) {
    if (!relay4State) {
      digitalWrite(relay4, LOW);
      relay4State = true;
      storeLogEntry("Heater turned ON - Temperature sensor failure failsafe activated");
      broadcastRelayStates();
    }
    return;
  }

  if (lastValidTemperature > temperatureData[0].maxTemp) {
    if (relay4State) {
      digitalWrite(relay4, HIGH);
      relay4State = false;
      storeLogEntry("Heater turned OFF - Temperature " + String(lastValidTemperature, 1) + "°C above maximum " + String(temperatureData[0].maxTemp) + "°C");
      broadcastRelayStates();
    }
  } else if (lastValidTemperature < temperatureData[0].minTemp) {
    if (!relay4State) {
      digitalWrite(relay4, LOW);
      relay4State = true;
      storeLogEntry("Heater turned ON - Temperature " + String(lastValidTemperature, 1) + "°C below minimum " + String(temperatureData[0].minTemp) + "°C");
      broadcastRelayStates();
    }
  }
}