#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <WakeOnLan.h>
#include <WakeOnLanCfg.h>

#include <Ticker.h>

#include <ESP8266Ping.h>

#include "FS.h"
#include <ArduinoJson.h>

boolean wifiConnected = false;

struct WiFiConnection wiFiConnection;
struct WOL wol;

WiFiUDP UDP;

//empty methods defined here, for declaration see lower.
boolean connectWifi();
void sendWOL();
void wakeOnLan();

Ticker wakeTicker(wakeOnLan, 5000);

bool loadConfig()
{
  const char *filename = "/config.json";
  File configFile = SPIFFS.open(filename, "r");
  if (!configFile)
  {
    Serial.println("Failed to open config file");
    return false;
  }

  // Deserialize the JSON document
  StaticJsonDocument<512> json;
  DeserializationError error = deserializeJson(json, configFile);
  if (error)
  {
    Serial.println(F("Failed to read file, using default configuration"));
  }

  // Copy values from the JSON document
  wiFiConnection.ssid = json["wifi"]["ssid"].as<char *>();
  wiFiConnection.password = json["wifi"]["password"].as<char *>();

  IPAddress ip;
  if (ip.fromString(json["wol"]["ip"].as<char *>()))
  {
    wol.ip = ip;
  }

  IPAddress mask;
  if (mask.fromString(json["wol"]["mask"].as<char *>()))
  {
    wol.mask = mask;
  }

  return true;
}

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
  if (!loadConfig())
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
      WakeOnLan::sendWOL(wol.mask, UDP, mac, sizeof mac);
    }
  }
}
