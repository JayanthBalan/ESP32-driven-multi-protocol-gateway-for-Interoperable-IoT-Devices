#ifndef WIRELESS_H
#define WIRELESS_H

#include <WiFi.h>
#include <WiFiMulti.h>
#include <BluetoothSerial.h>

static const int MAX_MESSAGE_LENGTH = 50;

#define MAX_WIFI_CLIENTS 1
WiFiMulti wifiMulti;
WiFiServer wifiServer(23);
WiFiClient wifiClients[MAX_WIFI_CLIENTS];
String wifiAddr[MAX_WIFI_CLIENTS];

BluetoothSerial BTSerial;
String btAddr = "";

void wifiInit(const char* ssid, const char* pass) {
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, pass);
  Serial.print("WiFi AP IP: "); Serial.println(WiFi.softAPIP());
  wifiServer.begin();
}

void handleWiFiClients() {
  if (wifiServer.hasClient()) {
    for (int i = 0; i < MAX_WIFI_CLIENTS; i++) {
      if (!wifiClients[i] || !wifiClients[i].connected()) {
        if (wifiClients[i]) wifiClients[i].stop();
        wifiClients[i] = wifiServer.accept();
        wifiAddr[i] = "dev_0";
        Serial.print("New WiFi device: ");
        Serial.print(wifiClients[i].remoteIP());
        Serial.print(" → "); Serial.println(wifiAddr[i]);
        break;
      }
    }
  }
  for (int i = 0; i < MAX_WIFI_CLIENTS; i++) {
    if (wifiClients[i] && !wifiClients[i].connected()) {
      Serial.print("WiFi disconnected: "); Serial.println(wifiAddr[i]);
      wifiAddr[i] = "";
      wifiClients[i].stop();
    }
  }
}

void wifiSend(const String &msg) {
  String m = msg.substring(0, MAX_MESSAGE_LENGTH);
  for (int i = 0; i < MAX_WIFI_CLIENTS; i++)
    if (wifiClients[i] && wifiClients[i].connected())
      wifiClients[i].println(m);
}

void btInit(const char* name) {
  BTSerial.begin(name);
  Serial.print("Bluetooth started: "); Serial.println(name);
}

void btSend(const String &msg) {
  String m = msg.substring(0, MAX_MESSAGE_LENGTH);
  BTSerial.println(m);
}

#endif
