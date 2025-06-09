#include <Arduino.h>
#include "wireless.h"
#include "wired.h"

const String FRAME_START = "=";
const String FRAME_END   = "=";
const String MSG_START   = "-";
const String MSG_END     = "-";
const String ACK         = "=--=";

bool busy = false;
String btDeviceAddress = "";

String readStream(Stream &s) {
  String r;
  while (s.available()) { r += char(s.read()); delay(1); }
  r.trim();
  return r;
}

void routeTo(const String &dest, const String &body) {
  int n = dest.substring(4).toInt();  
  if (n <= 2)      wifiSend(body);
  else if (n == 3) btSend(body);
  else if (n == 4) i2cSend(body);
  else if (n == 5) uartSend(body);
  else if (n == 6) { String x=body+"\n"; SPI.beginTransaction(SPISettings()); digitalWrite(SS, LOW);
                     for(char c:x) SPI.transfer(c); digitalWrite(SS, HIGH); }
}

void handleFrame(const String &f, Stream &src, const String &srcAddr) {
  if (!f.startsWith(FRAME_START)) return;
  int idx = f.indexOf(FRAME_END,1);
  String da = f.substring(1, idx);
  String payload = f.substring(idx+1);
  if (!payload.startsWith(MSG_START) || !payload.endsWith(MSG_END)) {
    Serial.println("Bad frame"); src.println("ERR"); return;
  }
  String msg = payload.substring(1, payload.length()-1);
  src.println(ACK);
  String destAddr = "dev_" + da;
  Stream *dst = nullptr;
  if (da.toInt() <= 2)      dst = &wifiClients[da.toInt()];
  else if (da.toInt()==3)   dst = &BTSerial;
  else if (da.toInt()==5)   dst = &UART;
  unsigned long t = millis();
  while (millis() - t < 5) {
    dst->println(ACK);
    if (dst->available() && readStream(*dst)==ACK) break;
  }
  dst->println(MSG_START + msg + MSG_END);
  Serial.print(srcAddr + "→" + destAddr + ": "); Serial.println(msg);
}

void setup() {
  Serial.begin(115200);
  wifiInit("ESP32_AP","12345678");
  btInit("ESP32_BT");
  i2cInit();
  spiInit();
  uartInit();
}

void loop() {
  handleWiFiClients();
  if (BTSerial.available()) {
    if (btDeviceAddress=="") {
      btDeviceAddress="dev_3";
      Serial.println("Assigned " + btDeviceAddress);
    }
    String b = readStream(BTSerial);
    handleFrame(b, BTSerial, btDeviceAddress);
  }
  if (UART.available()) {
    String u = readStream(UART);
    handleFrame(u, UART, "dev_5");
  }
  Wire.requestFrom(8, 32);
  if (Wire.available()) {
    String d; while (Wire.available()) d += char(Wire.read());
    handleFrame(d+"\n", Serial, "dev_4");
  }
  String s = spiRead();
  if (s.length()) handleFrame(s, Serial, "dev_6");
  delay(100);
}
