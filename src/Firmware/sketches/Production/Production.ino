#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Adafruit_NeoPixel.h>

#ifdef __AVR__
    #include <avr/power.h>
#endif


/*
 * Connection configuration
 *
 */

#define WIFI_SSID                   "Bastis Pennyboard"
#define WIFI_PASSWORD               "penny1337"


#define PIN_STATUSLED               LED_BUILTIN

#define PIN_NEOPIXELS               D1      // GPIO5 = D1
#define NEOPIXELS_COUNT             44
#define NEOPIXELS_BRIGHTNESS        255     // [0..255]

Adafruit_NeoPixel neopixelStrip = Adafruit_NeoPixel(NEOPIXELS_COUNT, PIN_NEOPIXELS, NEO_GRB + NEO_KHZ800);
ESP8266WebServer webServer = ESP8266WebServer(80);

/*
 * Neopixel effects
 *
 */

void neopixel_off()
{
    for (uint16_t i = 0; i < neopixelStrip.numPixels(); i++)
    {
        neopixelStrip.setPixelColor(i, 0);
    }

    neopixelStrip.show();
}

void neopixel_showSingleColorSceneWithDifferentWheelColor()
{
    const uint8_t WHEEL_LEDS_COUNT = 8;
    const uint32_t wheelLEDs[WHEEL_LEDS_COUNT] =
    {
        0, 1,
        20, 21,
        22, 23,
        42, 43,
    };

    const uint32_t colorNormal = 0x00AEFF;
    const uint32_t colorWheels = 0xA92CCE;

    for (uint16_t i = 0; i < neopixelStrip.numPixels(); i++)
    {
        neopixelStrip.setPixelColor(i, colorNormal);
    }

    for (uint16_t i = 0; i < WHEEL_LEDS_COUNT; i++)
    {
        neopixelStrip.setPixelColor(wheelLEDs[i], colorWheels);
    }

    neopixelStrip.show();
}

void neopixel_showGradientScene(const uint32_t color1, const uint32_t color2)
{
    const uint16_t neopixelCount = neopixelStrip.numPixels();

    // Split first color to R, B, G parts
    const uint8_t color1_r = (color1 >> 16) & 0xFF;
    const uint8_t color1_g = (color1 >>  8) & 0xFF;
    const uint8_t color1_b = (color1 >>  0) & 0xFF;

    // Split second color to R, B, G parts
    const uint8_t color2_r = (color2 >> 16) & 0xFF;
    const uint8_t color2_g = (color2 >>  8) & 0xFF;
    const uint8_t color2_b = (color2 >>  0) & 0xFF;

    for(uint16_t i = 0; i < neopixelCount; i++)
    {
        float percentage = _mapPixelCountToPercentage(i, neopixelCount);

        // Calculate the color of this iteration
        // see: https://stackoverflow.com/questions/27532/ and https://stackoverflow.com/questions/22218140/
        const uint8_t r = (color1_r * percentage) + (color2_r * (1 - percentage));
        const uint8_t g = (color1_g * percentage) + (color2_g * (1 - percentage));
        const uint8_t b = (color1_b * percentage) + (color2_b * (1 - percentage));

        const uint32_t currentColor = neopixelStrip.Color(r, g, b);
        neopixelStrip.setPixelColor(i, currentColor);
    }

    neopixelStrip.show();
}

float _mapPixelCountToPercentage(uint16_t i, float count)
{
    const float currentPixel = (float) i;
    const float neopixelCount = (float) count;

    const float min = 0.0f;
    const float max = 1.0f;

    return (currentPixel - 0.0f) * (max - min) / (neopixelCount - 0.0f) + min;
}

void setupPins()
{
    pinMode(PIN_STATUSLED, OUTPUT);
}

void setupNeopixels()
{
    Serial.println("Setup Neopixels...");

    neopixelStrip.begin();
    neopixelStrip.setBrightness(NEOPIXELS_BRIGHTNESS);
}



IPAddress gatewayAddress = IPAddress(10, 0, 0, 1);
IPAddress broadcastAddress = IPAddress(255, 255, 255, 0);



void setupWifi()
{
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(gatewayAddress, gatewayAddress, broadcastAddress);
    WiFi.softAP(WIFI_SSID, WIFI_PASSWORD);
}

void setupWebserver()
{
    Serial.println("Starting HTTP webserver...");

    webServer.begin();

    webServer.on("/", sendDefaultPage);
    webServer.onNotFound(handleNotFound);

    webServer.on("/scene/0", []() {
        neopixel_off();
        sendDefaultPage();
    });

    webServer.on("/scene/1", []() {
        neopixel_showSingleColorSceneWithDifferentWheelColor();
        sendDefaultPage();
    });

    webServer.on("/scene/2", []() {

        uint32_t color1 = 0xA92CCE;
        uint32_t color2 = 0xCE2C2C;

        neopixel_showGradientScene(color1, color2);
        sendDefaultPage();
    });

    webServer.begin();
}

void setup()
{
    Serial.begin(115200);
    delay(250);

    setupPins();
    setupNeopixels();
    setupWifi();
    setupWebserver();

    neopixel_showSingleColorSceneWithDifferentWheelColor();
}

void loop()
{
    webServer.handleClient();
}




void handleNotFound(){
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += webServer.uri();
  message += "\nMethod: ";
  message += (webServer.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += webServer.args();
  message += "\n";
  for (uint8_t i=0; i<webServer.args(); i++){
    message += " " + webServer.argName(i) + ": " + webServer.arg(i) + "\n";
  }
  webServer.send(404, "text/plain", message);
}

void sendDefaultPage()
{
    webServer.send(200, "text/html", "<h1>hello from esp8266!</h2>");
}
