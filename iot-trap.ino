#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>

const char* ssid = "xxxx";
const char* password = "xxxx";
const char* Host = "hooks.slack.com";
const String SlackHookUrl = "xxxx";
const String SlackIcon = ":rat:";
const String SlackMessageSet = "@xxxx, your trap is set :slightly_smiling_face:";
const String SlackMessageOff = "@xxxx, your trap has gone off :skull_and_crossbones:";
const String SlackUsername = "xxxx";
const int httpsPort = 443;
const int TripWire = 5;
const int SleepTimeSeconds = 10;
long *rtcMemory = (long *)0x60001200;

void setup() {
    Serial.begin(115200);
    Serial.setTimeout(2000);
    pinMode(TripWire, INPUT_PULLUP);

    // Wait for serial to initialize.
    while (!Serial) { }

    // read the input pin:
    int TrapState = digitalRead(TripWire);
    Serial.println(TrapState);
  
    if (TrapState == 1 && *rtcMemory == 1){
        int CallItIn;
        CallItIn = FunctionPhoneHome(SlackMessageOff);
        Serial.println("Trap has gone off");
        *rtcMemory = 0;
    }
    
    if (TrapState == 0 && *rtcMemory != 1){
        int CallItIn;
        CallItIn = FunctionPhoneHome(SlackMessageSet);
        Serial.println("Trap has been set");
        *rtcMemory = 1;
    }

    Serial.println("Going into deep sleep");
    ESP.deepSleep(SleepTimeSeconds * 1000000);
}

void loop() {
}

int FunctionPhoneHome(String Message){
  Serial.print("connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Connecting to ");
  Serial.println(Host);

  WiFiClientSecure client;

  if (!client.connect(Host, httpsPort)) {
    Serial.println("Connection failed :-(");
  }

  Serial.print("Posting to URL: ");
  Serial.println(SlackHookUrl);

  String PostData="payload={\"link_names\": 1, \"icon_emoji\": \"" + SlackIcon + "\", \"username\": \"" + SlackUsername + "\", \"text\": \"" + Message + "\"}";

  client.print(String("POST ") + SlackHookUrl + " HTTP/1.1\r\n" +
               "Host: " + Host + "\r\n" +
               "Content-Type: application/x-www-form-urlencoded\r\n" +
               "Connection: close" + "\r\n" +
               "Content-Length:" + PostData.length() + "\r\n" +
               "\r\n" + PostData);
  Serial.println("Request sent");
  String Line = client.readStringUntil('\n');
  Serial.printf("Response code was: ");
  Serial.println(Line);
  Serial.println("closing connection");
  WiFi.disconnect();
  delay(100);
  WiFi.mode(WIFI_OFF);
}
