#include <Arduino.h>
#include <ArduinoJson.h>

#include "telemetry.h"
#include "network.h"
#include "state.h"
#include "config.h"

void publishTelemetry()
{
    JsonDocument doc;

    doc["bayId"] = BAY_ID;
    doc["voltage"] = voltage;
    doc["current"] = current;
    doc["power"] = power;
    doc["energy"] = energyWh;
    doc["temperature"] = temperature;
    doc["evPlugged"] = (bayStatus == "CHARGING");
    doc["bayStatus"] = bayStatus;
    doc["loadDecision"] = loadDecision;
    doc["throttleLevel"] = throttleLevel;
    doc["overloadActive"] = overloadActive;
    doc["arrivalProbability"] = predictedArrivalProb;
    doc["predictedDurationMin"] = predictedDurationMin;

    char buffer[512];
    serializeJson(doc, buffer);

    bool published = mqtt.publish("v1/devices/me/telemetry", buffer);
    Serial.print("Telemetry publish: ");
    Serial.println(published);
    Serial.println("MQTT connected: " + String(mqtt.connected()));
} 