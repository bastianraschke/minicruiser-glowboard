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

#define PIN_NEOPIXELS               D1      // GPIO5 = D1
#define NEOPIXELS_COUNT             44
#define NEOPIXELS_BRIGHTNESS        50     // [0..255]

#define FIRMWARE_VERSION            "1.1"

Adafruit_NeoPixel neopixelStrip = Adafruit_NeoPixel(NEOPIXELS_COUNT, PIN_NEOPIXELS, NEO_GRB + NEO_KHZ800);
ESP8266WebServer webServer = ESP8266WebServer(80);

const IPAddress gatewayAddress = IPAddress(10, 0, 0, 1);
const IPAddress broadcastAddress = IPAddress(255, 255, 255, 0);

/*
 * Neopixel effects
 *
 */

void neopixel_off()
{
    for (uint16_t i = 0; i < neopixelStrip.numPixels(); i++)
    {
        neopixelStrip.setPixelColor(i, 0x000000);
    }

    neopixelStrip.show();
}

void neopixel_showStartupAnimation()
{
    const uint16_t neopixelCount = neopixelStrip.numPixels();

    const uint32_t colorNormal = 0xFFFFFF;
    const uint32_t colorOff = 0x000000;

    for (uint16_t i = 0; i < neopixelCount / 2; i++)
    {
        neopixelStrip.setPixelColor(i, colorNormal);
        neopixelStrip.setPixelColor(neopixelCount - i, colorNormal);
        neopixelStrip.show();

        delay(20);
    }

    for (uint16_t i = neopixelCount / 2; i > 0; i--)
    {
        neopixelStrip.setPixelColor(i, colorOff);
        neopixelStrip.setPixelColor(neopixelCount - i, colorOff);
        neopixelStrip.show();

        Serial.println(i);

        delay(20);
    }

    delay(100);
}

void neopixel_showSingleColorScene(const uint32_t colorNormal)
{
    for (uint16_t i = 0; i < neopixelStrip.numPixels(); i++)
    {
        neopixelStrip.setPixelColor(i, colorNormal);
    }

    neopixelStrip.show();
}

void neopixel_showSingleColorSceneWithDifferentWheelColor(const uint32_t colorNormal, const uint32_t colorWheels)
{
    const uint8_t WHEEL_LEDS_COUNT = 8;
    const uint32_t wheelLEDs[WHEEL_LEDS_COUNT] =
    {
        00, 01,
        20, 21,
        22, 23,
        42, 43,
    };

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

void neopixel_showSingleColorSceneWithDifferentWheelColor()
{
    const uint32_t colorNormal = 0x00AEFF;
    const uint32_t colorWheels = 0xA92CCE;

    neopixel_showSingleColorSceneWithDifferentWheelColor(colorNormal, colorWheels);
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

    for (uint16_t i = 0; i < neopixelCount / 2; i++)
    {
        float percentage = _mapPixelCountToPercentage(i, neopixelCount);

        // Calculate the color of this iteration
        // see: https://stackoverflow.com/questions/27532/ and https://stackoverflow.com/questions/22218140/
        const uint8_t r = (color1_r * percentage) + (color2_r * (1 - percentage));
        const uint8_t g = (color1_g * percentage) + (color2_g * (1 - percentage));
        const uint8_t b = (color1_b * percentage) + (color2_b * (1 - percentage));

        const uint32_t currentColor = neopixelStrip.Color(r, g, b);

        neopixelStrip.setPixelColor(i, currentColor);
        neopixelStrip.setPixelColor(neopixelCount - i, currentColor);
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


/*
 * Default logic
 *
 */

void setupNeopixels()
{
    Serial.println("Setup Neopixels...");

    neopixelStrip.begin();
    neopixelStrip.setBrightness(NEOPIXELS_BRIGHTNESS);
}

void setupWifi()
{
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(gatewayAddress, gatewayAddress, broadcastAddress);
    WiFi.softAP(WIFI_SSID, WIFI_PASSWORD);
}

void setupWebserver()
{
    Serial.println("Starting HTTP webserver...");

    webServer.on("/", HTTP_GET, sendDefaultPage);

    webServer.on("/scene/0", HTTP_GET, []() {
        neopixel_off();
        sendDefaultPage();
    });

    webServer.on("/scene/1", HTTP_GET, []() {
        neopixel_showSingleColorSceneWithDifferentWheelColor();
        sendDefaultPage();
    });

    webServer.on("/scene/2", HTTP_GET, []() {

        const uint32_t color1 = 0xA92CCE;
        const uint32_t color2 = 0xCE2C2C;
        neopixel_showGradientScene(color1, color2);

        sendDefaultPage();
    });

    webServer.on("/scene/3", HTTP_GET, []() {

        const uint32_t colorWhite = 0xFFFFFF;
        neopixel_showSingleColorScene(colorWhite);

        sendDefaultPage();
    });

    webServer.on("/scene/4", HTTP_GET, []() {

        neopixel_showStartupAnimation();

        sendDefaultPage();
    });

    webServer.onNotFound(handleNotFound);

    webServer.begin();
}

void setup()
{
    Serial.begin(115200);
    delay(250);

    setupNeopixels();
    setupWifi();
    setupWebserver();

    neopixel_showStartupAnimation();
    neopixel_showSingleColorSceneWithDifferentWheelColor();

    Serial.println("Initialization done.");
}

void loop()
{
    webServer.handleClient();
}




void handleNotFound()
{
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


const String defaultPageContent = R"=====(
<!DOCTYPE html>
<html lang='en'>

<head>
    <meta charset='UTF-8' /><meta name='viewport' content='width=device-width, initial-scale=1.0' />
    <title>Glowboard control</title>


<style type='text/css'>
    
body{
    background-color: #222222;
    font-family: sans-serif;
    color: #DCDCDC;
}

a.lightscene {
    display: block;
    padding: 1.2em;
    color: #FFFFFF;
    
    text-decoration: none;
    margin-bottom: 1em;
}

</style>

</head>

<body>

<h1>Glowboard control</h1>

<a href='/scene/1' class='lightscene' style='background: linear-gradient(135deg, #00AEFF 0%, #A92CCE 100%);'>Scene 1</a>
<a href='/scene/2' class='lightscene' style='background: linear-gradient(135deg, #A92CCE 0%, #CE2C2C 100%);'>Scene 2</a>
<a href='/scene/3' class='lightscene' style='background: linear-gradient(135deg, #FFFFFF 0%, #FFFFFF 100%); color: #000000;'>Flashlight mode</a>
<a href='/scene/0' class='lightscene' style='background: linear-gradient(135deg, #000000 0%, #000000 100%);'>Disable underglow</a>

<p>Connected to )=====" + String(WIFI_SSID) + R"=====(. <br>Firmware version: )=====" + String(FIRMWARE_VERSION) + R"=====(</p>


</body>

</html>

)=====";


void sendDefaultPage()
{
    webServer.send(200, "text/html", defaultPageContent.c_str());
}
