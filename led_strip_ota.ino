/**
 * @file OTA-mDNS-SPIFFS.ino
 * 
 * @author Pascal Gollor (http://www.pgollor.de/cms/)
 * @date 2015-09-18
 * 
 * changelog:
 * 2015-10-22: 
 * - Use new ArduinoOTA library.
 * - loadConfig function can handle different line endings
 * - remove mDNS studd. ArduinoOTA handle it.
 * 
 */

// includes
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <FS.h>
#include <ArduinoOTA.h>
#include <Adafruit_NeoPixel.h>


#define PIN 13

Adafruit_NeoPixel strip = Adafruit_NeoPixel(8, PIN, NEO_GRB + NEO_KHZ800);


/**
 * @brief mDNS and OTA Constants
 * @{
 */
#define HOSTNAME "PARAM-COOL-OTA-LAMP-" ///< Hostename. The setup function adds the Chip ID at the end.
/// @}

/**
 * @brief Default WiFi connection information.
 * @{
 */
const char* ap_default_ssid = "Singilla"; ///< Default SSID.
const char* ap_default_psk = "problemkyahai"; ///< Default PSK.
/// @}

/// Uncomment the next line for verbose output over UART.
//#define SERIAL_VERBOSE

/**
 * @brief Read WiFi connection information from file system.
 * @param ssid String pointer for storing SSID.
 * @param pass String pointer for storing PSK.
 * @return True or False.
 * 
 * The config file have to containt the WiFi SSID in the first line
 * and the WiFi PSK in the second line.
 * Line seperator can be \r\n (CR LF) \r or \n.
 */
bool loadConfig(String *ssid, String *pass)
{
  // open file for reading.
  File configFile = SPIFFS.open("/cl_conf.txt", "r");
  if (!configFile)
  {
    Serial.println("Failed to open cl_conf.txt.");

    return false;
  }

  // Read content from config file.
  String content = configFile.readString();
  configFile.close();
  
  content.trim();

  // Check if ther is a second line available.
  int8_t pos = content.indexOf("\r\n");
  uint8_t le = 2;
  // check for linux and mac line ending.
  if (pos == -1)
  {
    le = 1;
    pos = content.indexOf("\n");
    if (pos == -1)
    {
      pos = content.indexOf("\r");
    }
  }

  // If there is no second line: Some information is missing.
  if (pos == -1)
  {
    Serial.println("Infvalid content.");
    Serial.println(content);

    return false;
  }

  // Store SSID and PSK into string vars.
  *ssid = content.substring(0, pos);
  *pass = content.substring(pos + le);

  ssid->trim();
  pass->trim();

#ifdef SERIAL_VERBOSE
  Serial.println("----- file content -----");
  Serial.println(content);
  Serial.println("----- file content -----");
  Serial.println("ssid: " + *ssid);
  Serial.println("psk:  " + *pass);
#endif

  return true;
} // loadConfig


/**
 * @brief Save WiFi SSID and PSK to configuration file.
 * @param ssid SSID as string pointer.
 * @param pass PSK as string pointer,
 * @return True or False.
 */
bool saveConfig(String *ssid, String *pass)
{
  // Open config file for writing.
  File configFile = SPIFFS.open("/cl_conf.txt", "w");
  if (!configFile)
  {
    Serial.println("Failed to open cl_conf.txt for writing");

    return false;
  }

  // Save SSID and PSK.
  configFile.println(*ssid);
  configFile.println(*pass);

  configFile.close();
  
  return true;
} // saveConfig


/**
 * @brief Arduino setup function.
 */
void setup()
{
  String station_ssid = "Singilla";
  String station_psk = "problemkyahai";

  Serial.begin(115200);
  
  delay(100);

  Serial.println("\r\n");
  Serial.print("Chip ID: 0x");
  Serial.println(ESP.getChipId(), HEX);

  // Set Hostname.
  String hostname(HOSTNAME);
  hostname += String(ESP.getChipId(), HEX);
  WiFi.hostname(hostname);

  // Print hostname.
  Serial.println("Hostname: " + hostname);
  //Serial.println(WiFi.hostname());


  // Initialize file system.
  if (!SPIFFS.begin())
  {
    Serial.println("Failed to mount file system");
    return;
  }

  // Load wifi connection information.
  if (! loadConfig(&station_ssid, &station_psk))
  {
    station_ssid = "";
    station_psk = "";

    Serial.println("No WiFi connection information available.");
  }

  // Check WiFi connection
  // ... check mode
  if (WiFi.getMode() != WIFI_STA)
  {
    WiFi.mode(WIFI_STA);
    delay(10);
  }

  // ... Compare file config with sdk config.
  if (WiFi.SSID() != station_ssid || WiFi.psk() != station_psk)
  {
    Serial.println("WiFi config changed.");

    // ... Try to connect to WiFi station.
    WiFi.begin(station_ssid.c_str(), station_psk.c_str());

    // ... Pritn new SSID
    Serial.print("new SSID: ");
    Serial.println(WiFi.SSID());

    // ... Uncomment this for debugging output.
    //WiFi.printDiag(Serial);
  }
  else
  {
    // ... Begin with sdk config.
    WiFi.begin();
  }

  Serial.println("Wait for WiFi connection.");

  // ... Give ESP 10 seconds to connect to station.
  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < 10000)
  {
    Serial.write('.');
    //Serial.print(WiFi.status());
    delay(500);
  }
  Serial.println();

  // Check connection
  if(WiFi.status() == WL_CONNECTED)
  {
    // ... print IP Address
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }
  else
  {
    Serial.println("Can not connect to WiFi station. Go into AP mode.");
    
    // Go into software AP mode.
    WiFi.mode(WIFI_AP);

    delay(10);

    WiFi.softAP(ap_default_ssid, ap_default_psk);

    Serial.print("IP address: ");
    Serial.println(WiFi.softAPIP());
  }

  // Start OTA server.
  ArduinoOTA.setHostname((const char *)hostname.c_str());
  ArduinoOTA.begin();
  
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'

}


/**
 * @brief Arduino loop function.
 */
void loop()
{
  
  // Handle OTA server.
  ArduinoOTA.handle();
//  yield();
  allFeatures();
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
 
  //rainbow(10);
  rainbowCycle(5);
 // theaterChaseRainbow(10);
}

