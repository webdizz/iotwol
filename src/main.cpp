#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <WakeOnLan.h>
#include <WakeOnLanCfg.h>

#include <Ticker.h>

#include <ESP8266Ping.h>

#include "FS.h"
#include <ArduinoJson.h>

boolean wifiConnected = false;

const char *ssid;
const char *passwd;

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
  ssid = json["wifi"]["ssid"];
  passwd = json["wifi"]["passwd"];

  Serial.print("Loaded SSID: ");
  Serial.println(ssid);
  Serial.print("Loaded passwd: ");
  Serial.println(passwd);
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

  wakeTicker.start();
  //start UDP client, not sure if really necessary.
  UDP.begin(9);
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
  WiFi.begin(ssid, passwd);
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
    Serial.print("Connected to ");
    Serial.println(ssid);
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
    bool is_ip_online = Ping.ping(ip);
    if (is_ip_online)
    {
      Serial.println("Waked up, IP is online.");
    }
    else
    {
      Serial.println("Sending WOL Packet...");
      WakeOnLan::sendWOL(networkMask, UDP, mac, sizeof mac);
    }
  }
}
