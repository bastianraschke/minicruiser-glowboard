#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
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

#define MDNS_HOST                   "pennyboard"


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
    const uint32_t colorOff = 0x000000;

    for (int i = 0; i < neopixelStrip.numPixels(); i++)
    {
        neopixelStrip.setPixelColor(i, colorOff);
    }

    neopixelStrip.show();
}

void neopixel_showSlideAnimation()
{
    const uint16_t neopixelCount = neopixelStrip.numPixels();
    const uint16_t neopixelSideCount = neopixelCount / 2;

    const uint32_t colorNormal = 0xFFFFFF;
    const uint32_t colorOff = 0x000000;

    for (int i = 0; i < neopixelSideCount; i++)
    {
        neopixelStrip.setPixelColor(i, colorNormal);
        neopixelStrip.setPixelColor(neopixelCount - i, colorNormal);
        neopixelStrip.show();

        delay(20);
    }

    for (int i = neopixelSideCount - 1; i >= 0; i--)
    {
        neopixelStrip.setPixelColor(i, colorOff);
        neopixelStrip.setPixelColor(neopixelCount - i, colorOff);
        neopixelStrip.show();

        delay(20);
    }

    delay(100);
}

void neopixel_showSingleColorScene(const uint32_t colorNormal)
{
    for (int i = 0; i < neopixelStrip.numPixels(); i++)
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

    for (int i = 0; i < neopixelStrip.numPixels(); i++)
    {
        neopixelStrip.setPixelColor(i, colorNormal);
    }

    for (int i = 0; i < WHEEL_LEDS_COUNT; i++)
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

    for (int i = 0; i < neopixelCount / 2; i++)
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

void setupMDNS()
{
    MDNS.begin(MDNS_HOST);
}

void setupWebserver()
{
    Serial.println("Starting HTTP webserver...");

    webServer.on("/", HTTP_GET, _sendDefaultPage);
    webServer.onNotFound(_sendDefaultPage);

    webServer.on("/scene/0", HTTP_GET, []() {
        neopixel_off();
        _sendDefaultPage();
    });

    webServer.on("/scene/1", HTTP_GET, []() {
        neopixel_showSingleColorSceneWithDifferentWheelColor();
        _sendDefaultPage();
    });

    webServer.on("/scene/2", HTTP_GET, []() {
        const uint32_t color1 = 0xA92CCE;
        const uint32_t color2 = 0xCE2C2C;
        neopixel_showGradientScene(color1, color2);

        _sendDefaultPage();
    });

    webServer.on("/scene/3", HTTP_GET, []() {
        const uint32_t colorWhite = 0xFFFFFF;
        neopixel_showSingleColorScene(colorWhite);

        _sendDefaultPage();
    });

    webServer.on("/effect/slide", HTTP_GET, []() {
        neopixel_showSlideAnimation();
        _sendDefaultPage();
    });

    webServer.begin();
}

void setup()
{
    Serial.begin(115200);
    delay(250);

    setupNeopixels();
    setupWifi();
    setupMDNS();
    setupWebserver();

    // Initial effects
    neopixel_showSlideAnimation();
    neopixel_showSingleColorSceneWithDifferentWheelColor();

    Serial.println("Initialization done.");
}

void loop()
{
    webServer.handleClient();
}





const String defaultPageContent = R"=====(

<!DOCTYPE html>
<html lang='en'>

<head>
    <meta charset='UTF-8' />
    <meta name='viewport' content='width=device-width, initial-scale=1.0' />

    <title>Glowboard control</title>

    <style type='text/css'>
        
        body
        {
            background: radial-gradient(#4ceefb 0%, #000000 90%) center center fixed;
            background-repeat: no-repeat;
            background-size: cover;
            color: #FFF;
            font-family: sans-serif;
        }

        .content
        {
            background-color: rgba(34, 34, 34, 0.4);
            padding: 0.5em;
            position: absolute;
            top: 0.5em;
            bottom: 0.5em;
            left: 0.5em;
            right: 0.5em;
        }

        a.lightscene
        {
            color: #FFF;
            display: block;
            margin-bottom: 1.0em;
            padding: 1.2em;
            text-decoration: none;
        }

    </style>

</head>

<body>

    <div class="content">

        <h1>Glowboard control</h1>

        <a href='/scene/1' class='lightscene' style='background: linear-gradient(135deg, #00AEFF 0%, #A92CCE 100%);'>Scene 1</a>
        <a href='/scene/2' class='lightscene' style='background: linear-gradient(135deg, #A92CCE 0%, #CE2C2C 100%);'>Scene 2</a>
        <a href='/scene/3' class='lightscene' style='background: rgba(255, 255, 255, 0.75); color: #000000;'>Flashlight mode</a>
        <a href='/scene/0' class='lightscene' style='background: rgba(10, 10, 10, 0.75);'>Turn off</a>

        <p>Connected to )=====" + String(WIFI_SSID) + R"=====(. <br>Firmware version: )=====" + String(FIRMWARE_VERSION) + R"=====(</p>

    </div>

</body>
</html>

)=====";

void _sendDefaultPage()
{
    webServer.send(200, "text/html", defaultPageContent.c_str());
}
