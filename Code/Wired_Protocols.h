#ifndef WIRED_H
#define WIRED_H

#include <Wire.h>
#include <SPI.h>

#define UART Serial1
static const long BAUD = 9600;

void i2cInit() { Wire.begin(); }

void i2cSend(const String &msg) {
  Wire.beginTransmission(8);
  Wire.write((const uint8_t*)msg.c_str(), msg.length());
  Wire.endTransmission();
}

void spiInit() {
  SPI.begin();
  pinMode(SS, INPUT);
}

String spiRead() {
  String s;
  digitalWrite(SS, LOW);
  while (true) {
    char c = SPI.transfer(0x00);
    if (!c || c=='\n') break;
    s += c;
  }
  digitalWrite(SS, HIGH);
  return s;
}

void uartInit() { UART.begin(BAUD); }

void uartSend(const String &msg) { UART.println(msg); }

#endif
