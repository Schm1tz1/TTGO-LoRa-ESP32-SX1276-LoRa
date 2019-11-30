//
// Created by Roman on 30.11.2019.
//

#include <Arduino.h>
#include "PinsTTGO.h"
#include "LoRa.h"

#define LORA_BAND    868E6  //you can set band here directly,e.g. 868E6,915E6,433E6

int counter = 0;

void setup() {
    Serial.begin(9600);
    while (!Serial) {};

    Serial.println("LoRa Dump Registers");

    // override the default CS, reset, and IRQ pins (optional)
    LoRa.setPins(TTGO_PIN_NSS, TTGO_PIN_RST, TTGO_PIN_DIO0); // set CS, reset, IRQ pin

    if (!LoRa.begin(LORA_BAND)) {         // initialize ratio at 915 MHz
        Serial.println("LoRa init failed. Check your connections.");
        while (true);                   // if failed, do nothing
    }

    LoRa.dumpRegisters(Serial);
}

void receivePacket() {
    // try to parse packet
    int packetSize = LoRa.parsePacket();
    if (packetSize) {
        // received a packet
        Serial.print("Received packet '");

        // read packet
        while (LoRa.available()) {
            Serial.print((char) LoRa.read());
        }

        // print RSSI of packet
        Serial.print("' with RSSI ");
        Serial.println(LoRa.packetRssi());
        Serial.print("' and SNR ");
        Serial.println(LoRa.packetSnr(), 3);
    }
}

void sendPacket(int waitInMillis) {

    while (LoRa.beginPacket() == 0) {
        Serial.print("waiting for radio ... ");
        delay(waitInMillis);
    }

    Serial.print("Sending packet non-blocking: ");
    Serial.println(counter);

    // send packet
    LoRa.beginPacket();
    LoRa.print("Hello LoRa #");
    LoRa.print(counter);
    LoRa.endPacket(true);

    counter++;
}

void loop() {
    //sendPacket(100);
    receivePacket();
}