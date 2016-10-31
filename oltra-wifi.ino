#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <WiFiManager.h>

#include <PubSubClient.h>
#include <ArduinoOTA.h>
#include <Adafruit_NeoPixel.h>

#define PIXEL_DATA_PIN 13
#define sec 1000

//#define API_KEY "edf4e5f5-d108-4117-bcc9-6e27fae35fa8"
//#define PROJECT_ID 8173
//#define DEVICE_UUID "ee844dc5-cfea-4571-b6e3-25ccef680b1b"
//#define ACTUATOR_NAME "lightstrip"

char clientId[] = "OLTRA Lamp";
char actuatorTopic[] = "/a/edf4e5f5-d108-4117-bcc9-6e27fae35fa8/p/8173/d/ee844dc5-cfea-4571-b6e3-25ccef680b1b/actuator/lightstrip/state";
char server[] = "mqtt.devicehub.net";
char message_buffer[150];

// handles message arrived on subscribed topic
void callback(char* topic, byte* payload, unsigned int length)
{
  Serial.println("callback");
  // handles extracting the actuator value from payload
  int i = 0;
  int j = 0;
  int k = 0;
  for (i = 0; i < length; i++) {
    message_buffer[i] = payload[i];
  }
  message_buffer[i] = '\0';
  for (i = 0; i < length; i++) {
    if (!(strncmp(&message_buffer[i], "s", 1) || strncmp(&message_buffer[i + 3], "t", 1))) {
      break;
    }
  }
  for (j = i + 9; j < length; j++) {
    if (!(strncmp(&message_buffer[j], "\"", 1))) {
      break;
    }
  }
  String string_actuator_value = "";
  for (k = i + 9; k < j; k++) {
    string_actuator_value += message_buffer[k];
  }
  int actuator_value = string_actuator_value.toInt();

  // here is the value from the actuator
  Serial.print(topic);
  Serial.print(" value: ");
  Serial.println(actuator_value);
}

Adafruit_NeoPixel strip = Adafruit_NeoPixel(8, PIXEL_DATA_PIN, NEO_GRB + NEO_KHZ800);
WiFiManager wifiManager;
WiFiClient Client;
PubSubClient client(server, 1883, callback, Client);

void setup()
{
  wifiManager.autoConnect();
  ArduinoOTA.begin();

  strip.begin();
  strip.show();

//  client.setServer(server, 1883);
//  client.setCallback(callback);
//
//  if (client.connect(clientId)) {
//    //add or remove based on the number of actuators
//    if (client.subscribe(actuatorTopic)) {
//      Serial.println("Ok to subscrib");
//    } else {
//      Serial.println("Erro no subscrib");
//    }
//
//    Serial.println("Successfuly connected and running!");
//  } else {
//    Serial.println("Connection problem");
//  }
}

void loop()
{
  ArduinoOTA.handle();
  allFeatures();

//  //if client it's not connected or disconnects here we try to reconnect
//  if (!client.connected()) {
//    Serial.println("reconnecting ...");
//    client.connect(clientId);
//    delay(3 * sec);
//    client.subscribe(actuator1Topic);
//  }
//  // MQTT client loop processing
//  client.loop();
//  if (client.loop()) {
//    //Serial.println(client.state());
//  } else {
//    Serial.println("NAO Conectado");
//  }
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

//Theatre-style crawling lights.
void theaterChase(uint32_t c, uint8_t wait) {
  for (int j=0; j<10; j++) {  //do 10 cycles of chasing
    for (int q=0; q < 3; q++) {
      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, c);    //turn every third pixel on
      }
      strip.show();

      delay(wait);

      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait) {
  for (int j=0; j < 256; j++) {     // cycle all 256 colors in the wheel
    for (int q=0; q < 3; q++) {
      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, Wheel( (i+j) % 255));    //turn every third pixel on
      }
      strip.show();

      delay(wait);

      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

void allFeatures() {
 
  // rainbow(10);
  rainbowCycle(5);
  // theaterChaseRainbow(10);
}

bool isWiFiConnected() {
  return (WiFi.status() == WL_CONNECTED);
}

