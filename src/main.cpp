#include <Arduino.h>
#include <Utility.h>

void setup()
{
  Serial.begin(921600);
  TelnetStream.begin();
  loadConfigFile();
  setupOTA();

  broadcastSchedule.storedMillis = 0;
  broadcastSchedule.interval = 5000;
  lcdRefreshSchedule.storedMillis = 0;
  lcdRefreshSchedule.interval = 1000;

  /* colums - 16, rows - 2, cell resolution - 5x8, SDA - GPIO4, SCL - GPIO5 */
  lcd.begin(Columns, Rows, LCD_5x8DOTS, I2CDataPin, I2CClockPin);
  lcd.setCursor(0, 0);
  lcd.print(F("ESP-12F LCD 1602"));
  lcd.setCursor(0, 1);
  lcd.print(F("Starting..."));
  delay(2000);

  createLcdCustomCharacters();
  lcd.clear();
}

void loop()
{
  wifiReconnect();
  if (WiFi.status() == WL_CONNECTED)
  {
    ArduinoOTA.handle();
    if (broadcastDeviceDetails == 0)
    {
      /*DO SOME WIFI RELATED STUFF*/
    }
    else
    {
      if (broadcastSchedule.checkMillisSchedule())
      {
        serialAndTelnetPrintln("");
        serialAndTelnetPrint("Device Name: ");
        serialAndTelnetPrintln(espName);
        serialAndTelnetPrint("WiFi Connection: ");
        serialAndTelnetPrintln(WiFi.SSID());
        serialAndTelnetPrint("MAC Address: ");
        serialAndTelnetPrintln(WiFi.macAddress());
        serialAndTelnetPrint("IP Address: ");
        serialAndTelnetPrintln(WiFi.localIP());
      }
    }
  }

  /*DO SOME NON-WIFI RELATED STUFF*/
  if (lcdRefreshSchedule.checkMillisSchedule())
  {
    // 1 LCD 16x2 Frame
    lcd.home();
    lcd.write(byte(0));
    lcd.print(sensorDataCelsius());
    lcd.write(byte(1));
    lcd.print(F(" "));
    lcd.print(F(" "));
    lcd.print(F(" "));
    lcd.setCursor(0, 1);
    lcd.write(byte(0));
    lcd.print(sensorDataFahrenheit());
    lcd.write(byte(2));
    lcd.print(F(" "));
    lcd.print(F(" "));
    lcd.print(F(" "));
  }
}