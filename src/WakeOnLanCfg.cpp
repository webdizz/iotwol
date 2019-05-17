#include <WakeOnLanCfg.h>

#include "FS.h"
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>

void populateWiFiConnection(WiFiConnection &wiFiConnection, StaticJsonDocument<512> &json)
{
    // Copy values from the JSON document
    wiFiConnection.ssid = json["wifi"]["ssid"].as<char *>();
    wiFiConnection.password = json["wifi"]["password"].as<char *>();
}

void populateWol(WOL &wol, StaticJsonDocument<512> &json)
{
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
}

bool loadConfig(WiFiConnection &wiFiConnection, WOL &wol)
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

    populateWiFiConnection(wiFiConnection, json);
    populateWol(wol, json);

    return true;
}