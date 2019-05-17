
struct WiFiConnection
{
    const char *ssid;
    const char *password;
};

struct WOL
{
    IPAddress mask;
    IPAddress ip;
    byte *mac;
};

// TODO: need to pul this to config.json
byte mac[] = {0xE0, 0xD5, 0x5E, 0x2A, 0x50, 0x38};