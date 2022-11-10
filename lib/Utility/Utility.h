#include <FS.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <ESP8266WiFiMulti.h>
#include <ArduinoOTA.h>
#include <TelnetStream.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define JsonConfigFile "/config.json"
#define Columns 16
#define Rows 4
#define I2CDataPin 4
#define I2CClockPin 5
#define OneWirePin 10

ESP8266WiFiMulti WiFiMulti;
unsigned long previousMillis = 60000;
unsigned long interval = 60000;

char espName[15];
int broadcastDeviceDetails = 1;
unsigned long broadcastMillis = 0;
unsigned long broadcastInterval = 5000;

LiquidCrystal_I2C lcd(PCF8574_ADDR_A21_A11_A01, 4, 5, 6, 16, 11, 12, 13, 14, POSITIVE);
unsigned long lcdAnimationMillis = 0;
unsigned long lcdAnimationInterval = 3000;

OneWire oneWire(OneWirePin);
// Pass oneWire reference to Dallas Temperature sensor
DallasTemperature sensors(&oneWire);
float previousTempC = 0;
float previousTempF = 0;

class Schedule
{
public:
    unsigned long storedMillis;
    unsigned long interval;
    boolean checkMillisSchedule()
    {
        unsigned long millisNow = millis();
        if (millisNow - storedMillis >= interval)
        {
            storedMillis = millisNow;
            return true;
        }

        return false;
    }
};

Schedule broadcastSchedule;
Schedule lcdRefreshSchedule;

void serialAndTelnetPrint(__FlashStringHelper *message)
{
    Serial.print(message);
    TelnetStream.print(message);
}
void serialAndTelnetPrint(const char *message)
{
    Serial.print(message);
    TelnetStream.print(message);
}
void serialAndTelnetPrint(int message)
{
    Serial.print(message);
    TelnetStream.print(message);
}
void serialAndTelnetPrint(float message)
{
    Serial.print(message);
    TelnetStream.print(message);
}
void serialAndTelnetPrint(IPAddress message)
{
    Serial.print(message);
    TelnetStream.print(message);
}
void serialAndTelnetPrint(String message)
{
    Serial.print(message);
    TelnetStream.print(message);
}

void serialAndTelnetPrintln(__FlashStringHelper *message)
{
    Serial.println(message);
    TelnetStream.println(message);
}
void serialAndTelnetPrintln(const char *message)
{
    Serial.println(message);
    TelnetStream.println(message);
}
void serialAndTelnetPrintln(int message)
{
    Serial.println(message);
    TelnetStream.println(message);
}
void serialAndTelnetPrintln(float message)
{
    Serial.println(message);
    TelnetStream.println(message);
}
void serialAndTelnetPrintln(IPAddress message)
{
    Serial.println(message);
    TelnetStream.println(message);
}
void serialAndTelnetPrintln(String message)
{
    Serial.println(message);
    TelnetStream.println(message);
}

bool loadConfigFile()
// Load existing configuration file
{
    // Read configuration from FS json
    serialAndTelnetPrintln(F("Mounting FS"));

    if (LittleFS.begin())
    {
        // The file exists, reading and loading
        serialAndTelnetPrintln(F("Reading config"));
        File configFile = LittleFS.open(JsonConfigFile, "r");
        if (configFile)
        {
            serialAndTelnetPrintln(F("Opened config"));
            StaticJsonDocument<512> json;
            DeserializationError error = deserializeJson(json, configFile);
            if (!error)
            {
                serialAndTelnetPrintln(F("Parsing config"));
                strcpy(espName, json["deviceType"]);
                broadcastDeviceDetails = json["broadcastDeviceDetails"].as<int>();
                WiFiMulti.addAP(json["accessPoint"][0]["ssid"], json["accessPoint"][0]["password"]);
                WiFiMulti.addAP(json["accessPoint"][1]["ssid"], json["accessPoint"][1]["password"]);
                WiFiMulti.addAP(json["accessPoint"][2]["ssid"], json["accessPoint"][2]["password"]);
                IPAddress gateway(192, 168, 1, 1);
                IPAddress subnet(255, 255, 0, 0);
                IPAddress local_IP(json["ipAddress"][0].as<int>(), json["ipAddress"][1].as<int>(), json["ipAddress"][2].as<int>(), json["ipAddress"][3].as<int>());
                WiFi.config(local_IP, gateway, subnet);
                ArduinoOTA.setPassword(json["otaPassword"]);
                return true;
            }
            else
            {
                // Error loading JSON data
                serialAndTelnetPrintln(F("Load config failed"));
            }
        }
    }
    else
    {
        // Error mounting file system
        serialAndTelnetPrintln(F("Mount FS failed"));
    }
    return false;
}

void wifiReconnect()
{
    /*
        millis()
        - a function that returns the amount
        of milliseconds that have passed
        since program start
        - will overflow and restart at 0 after approximately 50 days
        (0 to 4,294,967,295 milliseconds ~ 49.71 days)
    */
    unsigned long currentMillis = millis();
    if ((WiFi.status() != WL_CONNECTED) && (currentMillis - previousMillis >= interval))
    {
        WiFiMulti.run(); // reconnect to WiFi
        previousMillis = currentMillis;
    }
}

void setupOTA()
{
    // Create unique ota host name by appending MAC Address
    uint16_t maxlen = strlen(espName) + 8;
    char *deviceName = new char[maxlen];
    uint8_t mac[6];
    WiFi.macAddress(mac);
    snprintf(deviceName, maxlen, "%s-%02x%02x%02x", espName, mac[3], mac[4], mac[5]);
    strcpy(espName, deviceName);
    ArduinoOTA.setHostname(deviceName);
    delete[] deviceName;

    ArduinoOTA.onStart([]()
                       {
                            serialAndTelnetPrint(F(""));
                            serialAndTelnetPrintln(F("Update start")); });

    ArduinoOTA.onEnd([]()
                     { serialAndTelnetPrintln(F("\nEnd")); });

    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
                          { Serial.printf("Progress: %u%%\r", (progress / (total / 100))); });

    ArduinoOTA.onError([](ota_error_t error)
                       {
                            Serial.printf("Error[%u]: ", error);
                            if (error == OTA_AUTH_ERROR) serialAndTelnetPrintln(F("\nAuth Failed"));
                            else if (error == OTA_BEGIN_ERROR) serialAndTelnetPrintln(F("\nBegin Failed"));
                            else if (error == OTA_CONNECT_ERROR) serialAndTelnetPrintln(F("\nConnect Failed"));
                            else if (error == OTA_RECEIVE_ERROR) serialAndTelnetPrintln(F("\nReceive Failed"));
                            else if (error == OTA_END_ERROR) serialAndTelnetPrintln(F("\nEnd Failed")); });

    ArduinoOTA.begin();
    serialAndTelnetPrintln(F("ESPOTA READY"));
}

void createLcdCustomCharacters()
{
    /* Custom Characters made with https://maxpromer.github.io/LCD-Character-Creator/ */
    byte temperatureIcon[] = {
        B01100,
        B01100,
        B01100,
        B01100,
        B11110,
        B11110,
        B01100,
        B00000};
    byte degreesCelcius[] = {
        B01000,
        B10100,
        B01000,
        B00011,
        B00100,
        B00100,
        B00011,
        B00000};
    byte degreesFahrenheit[] = {
        B01000,
        B10100,
        B01000,
        B00111,
        B00100,
        B00110,
        B00100,
        B00000};
    lcd.createChar(0, temperatureIcon);
    lcd.createChar(1, degreesCelcius);
    lcd.createChar(2, degreesFahrenheit);
}

float sensorDataCelsius()
{
    // Call sensors.requestTemperatures() to issue a global temperature and Requests to all devices on the bus
    sensors.requestTemperatures();
    float tempC = sensors.getTempCByIndex(0);

    // Suppress no reading (-127)
    if (tempC == -127)
    {
        tempC = previousTempC;
    }
    else
    {
        previousTempC = tempC;
    }

    serialAndTelnetPrint("Temperature in Celsius: ");
    serialAndTelnetPrintln(tempC);
    return tempC;
}

float sensorDataFahrenheit()
{
    // Call sensors.requestTemperatures() to issue a global temperature and Requests to all devices on the bus
    sensors.requestTemperatures();
    float tempF = sensors.getTempFByIndex(0);

    // Suppress no reading (-127)
    if (tempF == -196)
    {
        tempF = previousTempF;
    }
    else
    {
        previousTempF = tempF;
    }
    serialAndTelnetPrint("Temperature in Fahrenheit: ");
    serialAndTelnetPrintln(tempF);
    return tempF;
}