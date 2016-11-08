/*
Copyright (c) 2016 Ubidots.

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Original Maker: Mateo Velez - Metavix for Ubidots Inc
Modified by: Jose Garcia

*/

#include "UbidotsESPMQTT.h"

Ubidots::Ubidots(char* token, char* clientName) {
    _token = token;  // This is to get the token
    _clientName = clientName;
    currentValue = 0;
    val = (Value *)malloc(MAX_VALUES*sizeof(Value));
}


void Ubidots::begin(void (*callback)(char*,uint8_t*,unsigned int)) {
    this->callback = callback;
    _client.setServer(SERVER, MQTT_PORT);
    _client.setCallback(callback);
}


bool Ubidots::add(char* sourceLabel, char* variableLabel, float value, char *context) {
    (val+currentValue)->_variableLabel = variableLabel;
    (val+currentValue)->_sourceLabel = sourceLabel;
    (val+currentValue)->_value = value;
    (val+currentValue)->_context = context;    
    currentValue++;
    if (currentValue > MAX_VALUES) {
        Serial.println(F("You are sending more than the maximum of consecutive variables"));
        currentValue = MAX_VALUES;
    }
    return true;
}


bool Ubidots::ubidotsSubscribe(char* deviceLabel, char* variableLabel) {
    char topic[150];
    sprintf(topic, "%s%s/%s/lv", FIRST_PART_TOPIC, deviceLabel, variableLabel);
    Serial.println(topic);
    if (!_client.connected()) {
        reconnect();
    }
    return _client.subscribe(topic);
}


bool Ubidots::ubidotsPublish() {
    char topic[150];
    char payload[500];
    String str;
    sprintf(payload, "{");
    for (int i = 0; i <= currentValue; ) {
        sprintf(topic, "%s%s", FIRST_PART_TOPIC, (val+i)->_sourceLabel);
        str = String((val+i)->_value, 2);
        Serial.println(str);
        sprintf(payload, "%s\"%s\": %s", payload, (val+i)->_variableLabel, str.c_str());
        i++;
        if (i >= currentValue) {
            sprintf(payload, "%s}", payload);
            break;
        } else {
            sprintf(payload, "%s, ", payload);
        }
    }
    Serial.print("TOPIC: ");
    Serial.println(topic);
    Serial.print("JSON dict: ");
    Serial.println(payload);
    currentValue = 0;
    return _client.publish(topic, payload);
}


bool Ubidots::connected(){
    return _client.connected();
}


void Ubidots::reconnect() {
    while (!_client.connected()) {
        Serial.print("Attempting MQTT connection...");
        if (_client.connect(_clientName, _token, NULL)) {
            Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(_client.state());
      Serial.println(" try again in 3 seconds");
      delay(3000);
    }
  }
}


bool Ubidots::loop() {
    if (!_client.connected()) {
        reconnect();
    }
    return _client.loop();
}


bool Ubidots::wifiConnection(char* ssid, char* pass) {
    WiFi.begin(ssid, pass);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println(F("WiFi connected"));
    Serial.println(F("IP address: "));
    Serial.println(WiFi.localIP());
}