#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Hash.h>
#include <WiFiUdp.h>
#include "ws2812_i2s.h"
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
#include "config.h"

// Wifi and socket settings
unsigned int localPort = 7777;
char packetBuffer[BUFFER_LEN];

// LED strip
static WS2812 ledstrip;
static Pixel_t pixels[NUM_LEDS];
WiFiUDP port;

void setup() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(wifi_ssid, wifi_password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
    }

    port.begin(localPort);
    ledstrip.init(NUM_LEDS);

    // Port defaults to 8266
    // ArduinoOTA.setPort(8266);

    // Hostname defaults to esp8266-[ChipID]
    ArduinoOTA.setHostname("esp8266");

    // No authentication by default
    ArduinoOTA.setPassword(ota_password);

    ArduinoOTA.begin();
}

uint8_t N = 0;
#if PRINT_FPS
    uint16_t fpsCounter = 0;
    uint32_t secondTimer = 0;
#endif

void loop() {
    ArduinoOTA.handle();

    // Read data over socket
    int packetSize = port.parsePacket();
    // If packets have been received, interpret the command
    if (packetSize) {
        int len = port.read(packetBuffer, BUFFER_LEN);
        for(int i = 0; i < len; i+=4) {
            packetBuffer[len] = 0;
            N = packetBuffer[i];
            pixels[N].R = (uint8_t)packetBuffer[i+1];
            pixels[N].G = (uint8_t)packetBuffer[i+2];
            pixels[N].B = (uint8_t)packetBuffer[i+3];
        }
        ledstrip.show(pixels);
        #if PRINT_FPS
            fpsCounter++;
        #endif
    }
    #if PRINT_FPS
        if (millis() - secondTimer >= 1000U) {
            secondTimer = millis();
            Serial.printf("FPS: %d\n", fpsCounter);
            fpsCounter = 0;
        }
    #endif
}
