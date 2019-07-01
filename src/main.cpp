#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <WakeOnLan.h>
#include <WakeOnLanCfg.h>

#include <ESP8266Ping.h>

#include "FS.h"

#include <ESP8266WebServerSecure.h>
#include <WakeOnLanTLS.h>

boolean wifiConnected = false;

struct WiFiConnection wiFiConnection;
struct WOL wol;

BearSSL::ESP8266WebServerSecure secureServer(443);

WiFiUDP UDP;

//empty methods defined here, for declaration see lower.
boolean connectWifi();
void sendWOL();
String wakeOnLan();
void startSecureServer();

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

  startSecureServer();
}

void loop()
{
  secureServer.handleClient();
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

String wakeOnLan()
{
  if (wifiConnected)
  {
    bool is_ip_online = Ping.ping(wol.ip);
    if (is_ip_online)
    {
      return "Waked up, IP is online.";
    }
    else
    {
      Serial.println("Sending WOL Packet...");
      WakeOnLan::sendWOL(wol.mask, UDP, mac_address, sizeof(mac_address));
      return "Is about to wake up.";
    }
  }
  else
  {
    return "WiFi is not connected";
  }
}

void startSecureServer()
{
  BearSSL::X509List *serverCertList = new BearSSL::X509List(serverCert);
  BearSSL::PrivateKey *serverPrivKey = new BearSSL::PrivateKey(serverKey);
  secureServer.setRSACert(serverCertList, serverPrivKey);

  secureServer.on("/wake", HTTP_POST, []() {
    String message = wakeOnLan();
    secureServer.send(200, "text/plain", message);
  });

  secureServer.begin();
  Serial.println("HTTPS server started");
}