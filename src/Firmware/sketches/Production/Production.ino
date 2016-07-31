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

#define WIFI_DEBUG_MODE_ENABLED     true
#define WIFI_DEBUG_SSID             ""
#define WIFI_DEBUG_PASSWORD         ""


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

void neopixel_showSingleColorWithDifferentWheelColorScene()
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
    #if WIFI_DEBUG_MODE_ENABLED == true

        Serial.printf("Connecting to %s\n", WIFI_SSID);

        // Disable Wifi access point mode
        WiFi.mode(WIFI_STA);

        WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

        while (WiFi.status() != WL_CONNECTED)
        {
            // Blink 2 times when connecting
            blinkStatusLED(2);

            delay(500);
            Serial.print(".");
        }

        Serial.println();
        Serial.println("WiFi connected.");

        Serial.print("Obtained IP address: ");
        Serial.println(WiFi.localIP());

    #else

        Serial.printf("Starting access point %s\n", WIFI_SSID);

        WiFi.mode(WIFI_AP);
        WiFi.softAP(WIFI_SSID, WIFI_PASSWORD);

    #endif
}

void setupWebserver()
{
    webServer.begin();

    webServer.on( "/", handleRoot );
    webServer.on( "/test.svg", drawGraph );
    webServer.on( "/inline", []() {
        webServer.send ( 200, "text/plain", "this works as well" );
    } );
    webServer.onNotFound ( handleNotFound );
    webServer.begin();

    Serial.println ("HTTP webserver started.");
}

void setup()
{
    Serial.begin(115200);
    delay(250);

    setupPins();
    setupNeopixels();
    setupWifi();
    setupWebserver();

    neopixel_showSingleColorWithDifferentWheelColorScene();
}

void loop()
{
    webServer.handleClient();
}






void drawGraph() {
    String out = "";
    char temp[100];
    out += "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" width=\"400\" height=\"150\">\n";
    out += "<rect width=\"400\" height=\"150\" fill=\"rgb(250, 230, 210)\" stroke-width=\"1\" stroke=\"rgb(0, 0, 0)\" />\n";
    out += "<g stroke=\"black\">\n";
    int y = rand() % 130;
    for (int x = 10; x < 390; x+= 10) {
        int y2 = rand() % 130;
        sprintf(temp, "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" stroke-width=\"1\" />\n", x, 140 - y, x + 10, 140 - y2);
        out += temp;
        y = y2;
    }
    out += "</g>\n</svg>\n";

    webServer.send ( 200, "image/svg+xml", out);
}

void handleRoot() {
  webServer.send(200, "text/plain", "hello from esp8266!");
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
