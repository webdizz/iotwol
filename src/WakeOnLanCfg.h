#include <ArduinoJson.h>
#include <ESP8266WiFi.h>

struct WiFiConnection
{
    const char *ssid;
    const char *password;
};

struct WOL
{
    IPAddress mask;
    IPAddress ip;
};

bool loadConfig(WiFiConnection &wiFiConnection, WOL &wol);