#include <Arduino.h>
#include <EtherCard.h>

static byte mac[] = {0x74, 0x69, 0x11, 0x30, 0x98, 0x61};
byte Ethernet::buffer[700];

void setup()
{
  Serial.begin(57600);
  Serial.println(F("[testDHCP]"));

  Serial.print("MAC: ");

  for (byte i = 0; i < 6; ++i)
  {
    Serial.print(mac[i], HEX);
    if (i < 5)
    {
      Serial.print(':');
    }
  }
  Serial.println();

  if (ether.begin(sizeof Ethernet::buffer, mac, 55) == 0)
  {
    Serial.println(F("Failed to access Ethernet cotroller"));
  }

  Serial.println(F("Setting up DHCP"));
  if (!ether.dhcpSetup())
  {
    Serial.println(F("DHCP failed"));
  }

  ether.printIp("My IP: ", ether.myip);
  ether.printIp("Netmask: ", ether.netmask);
  ether.printIp("GW IP", ether.gwip);
  ether.printIp("DNS IP: ", ether.dnsip);
}

void loop()
{
  ether.packetLoop(ether.packetReceive());
}
