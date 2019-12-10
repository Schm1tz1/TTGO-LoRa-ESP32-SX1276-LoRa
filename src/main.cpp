//
// Created by Roman on 30.11.2019.
//

#include <Arduino.h>

// headers for TTGO/LoRa
#include "PinsTTGO.h"
#include "LoRa.h"

#define USE_DISPLAY

#ifdef USE_DISPLAY
// headers for OLED Display
#include "SpecsOLED.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#endif

#define SERIAL_BAUD 9600

//433E6 for Asia
//866E6 for Europe
//915E6 for North America
#define LORA_BAND 866E6

byte loraLocalAddress = 0xB5;     // address of this device
byte loraDestination = 0xFE;      // destination to send to

unsigned long globalLoraPacketCounter = 0;
unsigned long lastSendTime = 0;

#ifdef USE_DISPLAY
Adafruit_SSD1306 display(OLED_SCREEN_WIDTH, OLED_SCREEN_HEIGHT, &Wire, OLED_RST);
#endif

void initSerial() {
    Serial.begin(SERIAL_BAUD);
    while (!Serial) {
        delay(10);
    }
}

#ifdef USE_DISPLAY

void initOled() {
    //reset OLED display via software
    pinMode(OLED_RST, OUTPUT);
    digitalWrite(OLED_RST, LOW);
    delay(20);
    digitalWrite(OLED_RST, HIGH);

    //initialize OLED
    Wire.begin(OLED_SDA, OLED_SCL);
    if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDRRESS, false, false)) {
        Serial.println(F("SSD1306 allocation failed"));
        for (;;); // Don't proceed, loop forever
    }

    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("LORA TTGO SX1276 TEST");
    display.display();
}

#endif

void initLoRa() {

    // override the default CS, reset, and IRQ pins (optional)
    LoRa.setPins(TTGO_PIN_NSS, TTGO_PIN_RST, TTGO_PIN_DIO0); // set CS, reset, IRQ pin

    if (!LoRa.begin(LORA_BAND)) {         // initialize ratio at 915 MHz
        Serial.println("LoRa init failed. Check your connections.");
        while (true);                   // if failed, do nothing
    }

    // Sync Words will not receive each other's transmissions. This is one way you can filter out radios you want to ignore, without making an addressing scheme.
    //LoRa.setSyncWord(0xF3);           // ranges from 0-0xFF, default 0x34, see API docs

    // Spreading factor affects reliability of transmission at high rates, however, avoid a large spreading factor when you're sending continually.
    //LoRa.setSpreadingFactor(8);           // ranges from 6-12,default 7 see API docs

    //Serial.println("LoRa Dump Registers");
    //LoRa.dumpRegisters(Serial);

    // in receiver mode with callback (or alternatively poll in loop with receivePacket)
    // TODO: still to be testes with ESP32-TTGO OLED. Together with OLED display the handler is not triggered while with seial-only output it works. Needs further debugging.
    //LoRa.onReceive(onReceive);
    //LoRa.receive();
}

void onReceive(int packetSize) {

    if (packetSize) {
        // received a packet
        if (Serial) Serial.print("Received packet '");

        //read packet
        String LoRaData;
        while (LoRa.available()) {
            LoRaData = LoRa.readString();
            Serial.print(LoRaData);
        }

        //print RSSI of packet
        int rssi = LoRa.packetRssi();
        float snr = LoRa.packetSnr();

        // print RSSI of packet
        if (Serial) {
            Serial.print("' with RSSI ");
            Serial.println(rssi);
            Serial.print("' and SNR ");
            Serial.println(snr, 3);
        }

#ifdef USE_DISPLAY
        display.clearDisplay();
        display.setCursor(0, 0);
        display.print("LORA RECEIVER");
        display.setCursor(0, 20);
        display.print("Received packet:");
        display.setCursor(0, 30);
        display.print(LoRaData);
        display.setCursor(0, 40);
        display.print("RSSI:");
        display.setCursor(30, 40);
        display.print(rssi);
        display.display();
        display.print("SNR:");
        display.setCursor(30, 50);
        display.print(snr);
        display.display();
#endif

    }
}

void setup() {
    initSerial();
#ifdef USE_DISPLAY
    initOled();
#endif
    initLoRa();

}

void receivePacket() {
    int packetSize = LoRa.parsePacket();
    onReceive(packetSize);
}

void loop() {
    // do nothing for callback or use polling send/receive
    receivePacket();
}