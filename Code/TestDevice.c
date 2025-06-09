#include <SPI.h> 
const char* FRAME_START = "=";
const char* FRAME_END   = "=";
const char* MSG_START   = "-";
const char* MSG_END     = "-";
const char* ACK         = "=--=";

const int SS_PIN = 10;  

String readSerialLine() {
  String s = "";
  while (true) {
    if (Serial.available()) {
      char c = Serial.read();
      if (c == '\n' || c == '\r') {
        if (s.length() > 0) break;
        else continue;
      }
      s += c;
    }
  }
  s.trim();
  return s;
}

void sendUART(uint8_t dest, const String &msg) {
  Serial.print(FRAME_START);
  Serial.print(dest);
  Serial.print(FRAME_END);
  Serial.print(MSG_START);
  Serial.print(msg);
  Serial.print(MSG_END);
}

void sendSPI(uint8_t dest, const String &msg) {
  digitalWrite(SS_PIN, LOW);
  SPI.transfer(FRAME_START[0]);
  SPI.transfer('0' + dest);
  SPI.transfer(FRAME_END[0]);
  SPI.transfer(MSG_START[0]);
  for (char c : msg) SPI.transfer(c);
  SPI.transfer(MSG_END[0]);
  digitalWrite(SS_PIN, HIGH);
}


bool awaitAck(Stream &bus) {
  unsigned long t0 = millis();
  while (millis() - t0 < 500) {
    if (bus.available()) {
      String r = bus.readStringUntil('\n');
      r.trim();
      if (r == ACK) return true;
    }
  }
  return false;
}

void setup() {
  Serial.begin(115200);
  while (!Serial);  
  Serial.println("UNO two-bus (UART+SPI) client ready");

  pinMode(SS_PIN, OUTPUT);
  SPI.begin();
}

void loop() {

  Serial.println(F("Choose source: u=UART, s=SPI"));
  String src = readSerialLine();

  char c = src.length() ? src.charAt(0) : '\0';
  if (c!='u' && c!='s') {
    Serial.println("Invalid source");
    delay(100); 
    return;
  }

  Serial.print("Dest (5=UART,6=SPI)? ");
  uint8_t dest = (uint8_t)readSerialLine().toInt();

  Serial.print("Msg: ");
  String msg = readSerialLine();


  if (c == 'u') {
    sendUART(dest, msg);
    Serial.println("UART: ACK received");
  } 
  else {
    sendSPI(dest, msg);
    Serial.println("SPI: message sent");

  }


  if (Serial.available()) {
    String in = readSerialLine();
    Serial.print("UART<- "); Serial.println(in);
  }


  digitalWrite(SS_PIN, LOW);
  char c0 = SPI.transfer(0);
  digitalWrite(SS_PIN, HIGH);
  if (c0 == FRAME_START[0]) {
    String in; in += c0;
    while (true) {
      digitalWrite(SS_PIN, LOW);
      char x = SPI.transfer(0);
      digitalWrite(SS_PIN, HIGH);
      in += x;
      if (x == MSG_END[0]) break;
    }
    Serial.print("SPI<- "); Serial.println(in);
  }
  Serial.println(msg);
}
