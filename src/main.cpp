#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <WakeOnLan.h>
#include <WakeOnLanCfg.h>

#include <Ticker.h>

#include <ESP8266Ping.h>

#include "FS.h"

boolean wifiConnected = false;

struct WiFiConnection wiFiConnection;
struct WOL wol;
// TODO: need to pul this to config.json
byte mac_address[] = {0xE0, 0xD5, 0x5E, 0x2A, 0x50, 0x38};

WiFiUDP UDP;

//empty methods defined here, for declaration see lower.
boolean connectWifi();
void sendWOL();
void wakeOnLan();

Ticker wakeTicker(wakeOnLan, 5000);

void setup()
{
  // Initialise Serial connection
  Serial.begin(9600);
  if (!SPIFFS.begin())
  {
    Serial.println("Failed to mount file system");
    return;
  }

  delay(500);
  if (!loadConfig(wiFiConnection, wol))
  {
    Serial.println("Failed to load config");
  }
  else
  {
    Serial.println("Config loaded");
  }

  // Initialise wifi connection
  wifiConnected = connectWifi();

  //start UDP client, not sure if really necessary.
  UDP.begin(9);

  delay(500);
  wakeTicker.start();
}

void loop()
{
  wakeTicker.update();
}

// connect to wifi – returns true if successful or false if not
boolean connectWifi()
{
  boolean state = true;
  int i = 0;
  WiFi.begin(wiFiConnection.ssid, wiFiConnection.password);
  Serial.println("");
  Serial.println("Connecting to WiFi");

  // Wait for connection
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
    if (i > 20)
    {
      state = false;
      break;
    }
    i++;
  }
  if (state)
  {
    Serial.println("");
    Serial.print("Connected to: ");
    Serial.println(wiFiConnection.ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }
  else
  {
    Serial.println("");
    Serial.println("Connection failed.");
  }
  return state;
}

void wakeOnLan()
{
  if (wifiConnected)
  {
    bool is_ip_online = Ping.ping(wol.ip);
    if (is_ip_online)
    {
      Serial.println("Waked up, IP is online.");
    }
    else
    {
      Serial.println("Sending WOL Packet...");
      WakeOnLan::sendWOL(wol.mask, UDP, mac_address, sizeof(mac_address));
    }
  }
}
