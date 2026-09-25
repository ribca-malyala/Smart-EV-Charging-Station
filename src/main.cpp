#include <Arduino.h>
#include <DHT.h>
#include "state.h"
#include "edge_ai.h"
#include "optimization.h"
#include "network.h"
#include "telemetry.h"

// =========================
// Pin Definitions
// =========================

#define VOLTAGE_PIN 34
#define CURRENT_PIN 35
#define DHT_PIN 15

#define RELAY_PIN 26

#define PLUG_IN_PIN 32
#define PLUG_OUT_PIN 33

#define GREEN_LED 18
#define YELLOW_LED 19
#define RED_LED 21

#define DHT_TYPE DHT22

// =========================
// Sensor / System Settings
// =========================

const float MAX_CURRENT = 16.0;     // Amps
const float MAX_POWER = 6000.0;     // Watts

const float VOLTAGE_MAX = 250.0;    // Simulated voltage range
const float CURRENT_MAX = 20.0;     // Simulated current range

const unsigned long TELEMETRY_INTERVAL = 2000;

// =========================
// Variables
// =========================

DHT dht(DHT_PIN, DHT_TYPE);



bool evPluggedIn = false;

unsigned long lastTelemetry = 0;
unsigned long lastEnergyUpdate = 0;

// =========================
// Setup
// =========================

void setup() {

    Serial.begin(115200);
    connectWiFi();
    connectMQTT();

    pinMode(VOLTAGE_PIN, INPUT);
    pinMode(CURRENT_PIN, INPUT);

    pinMode(RELAY_PIN, OUTPUT);

    pinMode(PLUG_IN_PIN, INPUT_PULLUP);
    pinMode(PLUG_OUT_PIN, INPUT_PULLUP);

    pinMode(GREEN_LED, OUTPUT);
    pinMode(YELLOW_LED, OUTPUT);
    pinMode(RED_LED, OUTPUT);

    digitalWrite(RELAY_PIN, LOW);

    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(RED_LED, LOW);

    dht.begin();

    Serial.println();
    Serial.println("=================================");
    Serial.println(" Smart EV Charging Station");
    Serial.println(" ESP32 Edge Controller Starting");
    Serial.println("=================================");

    lastEnergyUpdate = millis();
}

// =========================
// Read Voltage
// =========================

float readVoltage() {

    int rawValue = analogRead(VOLTAGE_PIN);

    float voltageValue =
        (rawValue / 4095.0) * VOLTAGE_MAX;

    return voltageValue;
}

// =========================
// Read Current
// =========================

float readCurrent() {

    int rawValue = analogRead(CURRENT_PIN);

    float currentValue =
        (rawValue / 4095.0) * CURRENT_MAX;

    return currentValue;
}

// =========================
// Charging Decision
// =========================

void updateChargingDecision() {

    // EV not connected
    if (!evPluggedIn) {

        digitalWrite(RELAY_PIN, LOW);

        digitalWrite(GREEN_LED, LOW);
        digitalWrite(YELLOW_LED, LOW);
        digitalWrite(RED_LED, LOW);

        return;
    }

    // Overload condition
    if (current > MAX_CURRENT || power > MAX_POWER) {

        digitalWrite(RELAY_PIN, LOW);

        digitalWrite(GREEN_LED, LOW);
        digitalWrite(YELLOW_LED, LOW);
        digitalWrite(RED_LED, HIGH);

        Serial.println("DECISION: DEFER - OVERLOAD");

        return;
    }

    // High load condition
    if (current > 12.0 || power > 4500.0) {

        digitalWrite(RELAY_PIN, HIGH);

        digitalWrite(GREEN_LED, LOW);
        digitalWrite(YELLOW_LED, HIGH);
        digitalWrite(RED_LED, LOW);

        Serial.println("DECISION: THROTTLE");

        return;
    }

    // Normal charging
    digitalWrite(RELAY_PIN, HIGH);

    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(RED_LED, LOW);

    Serial.println("DECISION: ALLOW");
}

// =========================
// Energy Calculation
// =========================

void updateEnergy() {

    unsigned long now = millis();

    float elapsedHours =
        (now - lastEnergyUpdate) / 3600000.0;

    if (evPluggedIn && digitalRead(RELAY_PIN) == HIGH) {

        energyWh += (power * elapsedHours) / 1000.0;
    }

    lastEnergyUpdate = now;
}

// =========================
// Main Loop
// =========================

void loop() {
    mqtt.loop();

    // -------------------------
    // Plug-in detection
    // -------------------------

    if (digitalRead(PLUG_IN_PIN) == LOW) {

        evPluggedIn = true;
        bayStatus = "CHARGING";
        loadDecision = "ALLOW";
        throttleLevel = 100;
        sessionStartMs = millis();

        Serial.println("EV PLUGGED IN");

        delay(300);
    }

    // -------------------------
    // Plug-out detection
    // -------------------------

    if (digitalRead(PLUG_OUT_PIN) == LOW) {

        evPluggedIn = false;
        bayStatus = "FREE";
        loadDecision = "ALLOW";
        throttleLevel = 100;

        digitalWrite(RELAY_PIN, LOW);

        Serial.println("EV PLUGGED OUT");

        delay(300);
    }

    // -------------------------
    // Sensor readings
    // -------------------------

    voltage = readVoltage();

    current = readCurrent();

    temperature = dht.readTemperature();

    // Protect against invalid DHT readings

    if (isnan(temperature)) {

        temperature = 0.0;
    }

    // -------------------------
    // Power calculation
    // -------------------------

    power = voltage * current;

    // -------------------------
    // Energy calculation
    // -------------------------

    updateEnergy();
    // -------------------------
    // Edge AI inference
    // -------------------------
    runEdgeAIInference();
    // -------------------------
    // Charging decision
    // -------------------------
    runOptimization();
    applyRelayDutyCycle();
    //updateChargingDecision();

    // -------------------------
    // Serial telemetry
    // -------------------------

    if (millis() - lastTelemetry >= TELEMETRY_INTERVAL) {

        lastTelemetry = millis();

        Serial.println();
        Serial.println("------ EV TELEMETRY ------");

        Serial.print("Voltage: ");
        Serial.print(voltage);
        Serial.println(" V");

        Serial.print("Current: ");
        Serial.print(current);
        Serial.println(" A");

        Serial.print("Power: ");
        Serial.print(power);
        Serial.println(" W");

        Serial.print("Energy: ");
        Serial.print(energyWh);
        Serial.println(" kWh");

        Serial.print("Temperature: ");
        Serial.print(temperature);
        Serial.println(" C");

        Serial.print("EV Plugged: ");
        Serial.println(evPluggedIn ? "YES" : "NO");

        Serial.print("Relay: ");
        Serial.println(
            digitalRead(RELAY_PIN) ? "ON" : "OFF"
        );
        Serial.print("AI Arrival Probability: ");
        Serial.print(predictedArrivalProb * 100.0);
        Serial.println(" %");

        Serial.print("AI Predicted Duration: ");
        Serial.print(predictedDurationMin);
        Serial.println(" min");

        Serial.println("--------------------------");
        publishTelemetry();

    }

    delay(100);
}