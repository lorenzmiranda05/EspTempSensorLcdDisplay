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
      serialAndTelnetPrintln("");
      serialAndTelnetPrint("Device Name: ");
      serialAndTelnetPrintln(espName);
      serialAndTelnetPrint("WiFi Connection: ");
      serialAndTelnetPrintln(WiFi.SSID());
      serialAndTelnetPrint("MAC Address: ");
      serialAndTelnetPrintln(WiFi.macAddress());
      serialAndTelnetPrint("IP Address: ");
      serialAndTelnetPrintln(WiFi.localIP());
      serialAndTelnetPrintln("Web Server: OFF");
      delay(5000);
    }
  }

  /*DO SOME NON-WIFI RELATED STUFF*/
}