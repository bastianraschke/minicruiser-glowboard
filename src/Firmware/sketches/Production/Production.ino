#include <ESP8266WiFi.h>
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

/*
 * Neopixel effects
 *
 */

void neopixel_showSingleColorScene()
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

void blinkStatusLED(const int times)
{
    for (int i = 0; i < times; i++)
    {
        // Enable LED
        digitalWrite(PIN_STATUSLED, LOW);
        delay(100);

        // Disable LED
        digitalWrite(PIN_STATUSLED, HIGH);
        delay(100);
    }
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

void setupWifi()
{
    WiFi.mode(WIFI_AP);
    WiFi.softAP(WIFI_SSID, WIFI_PASSWORD);
}

void setup()
{
    Serial.begin(115200);
    delay(250);

    setupPins();
    setupNeopixels();
    setupWifi();

    neopixel_showSingleColorScene();
}

void loop()
{

}
