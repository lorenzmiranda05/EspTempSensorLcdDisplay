#include <Arduino.h>
#include <Utility.h>

void setup()
{
  Serial.begin(921600);
  TelnetStream.begin();
  loadConfigFile();
  setupOTA();
}

void loop()
{
  if (wm.run() != WL_CONNECTED)
  {
    serialAndTelnetPrintln("WiFi not connected!");
    delay(5000);
  }
  if (wm.run() == WL_CONNECTED)
  {
    ArduinoOTA.handle();
    if (broadcastDeviceDetails == 0)
    {
      // DO SOMETHING
    }
    else
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
      delay(5000);
    }
  }
}