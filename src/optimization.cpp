#include <Arduino.h>
#include "optimization.h"
#include "config.h"
#include "state.h"

#define RELAY_PIN 26

#define GREEN_LED 18
#define YELLOW_LED 19
#define RED_LED 21

void updateLeds()
{
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(RED_LED, LOW);

    if (loadDecision == "DEFER")
    {
        digitalWrite(RED_LED, HIGH);
    }
    else if (loadDecision == "THROTTLE")
    {
        digitalWrite(YELLOW_LED, HIGH);
    }
    else
    {
        digitalWrite(GREEN_LED, HIGH);
    }
}

void runOptimization()
{
    float totalStationPower = power;

    overloadActive = false;

    if (manualOverrideActive)
    {
        updateLeds();
        return;
    }

    if (bayStatus != "CHARGING")
    {
        bool peakHour =
            (lastHourOfDay >= peakTariffStartHr &&
             lastHourOfDay <= peakTariffEndHr);

        if (peakHour && predictedArrivalProb >= predictionThreshold)
        {
            loadDecision = "STANDBY_EXPECTING";
        }
        else if (peakHour)
        {
            loadDecision = "LOW_DEMAND";
        }
        else
        {
            loadDecision = "ALLOW";
        }

        updateLeds();
        return;
    }

    if (current > overloadCurrentA)
    {
        loadDecision = "DEFER";
        throttleLevel = 0;
        overloadActive = true;

        Serial.println("!! Overcurrent -> DEFER - RELAY OFF.");
    }
    else if (current > 12.0 || totalStationPower > maxStationLoadW)
    {
        loadDecision = "THROTTLE";
        throttleLevel = 70;

        Serial.println("!! Charging load high -> THROTTLE @ 70%.");
    }
    else
    {
        loadDecision = "ALLOW";
        throttleLevel = 100;
    }

    updateLeds();
}

void applyRelayDutyCycle()
{
    if (bayStatus != "CHARGING")
    {
        digitalWrite(RELAY_PIN, LOW);
        return;
    }

    if (throttleLevel >= 100)
    {
        digitalWrite(RELAY_PIN, HIGH);
        return;
    }

    if (throttleLevel <= 0)
    {
        digitalWrite(RELAY_PIN, LOW);
        return;
    }

    unsigned long phase = millis() % 1000;
    unsigned long onTime = (1000 * throttleLevel) / 100;

    digitalWrite(RELAY_PIN, phase < onTime ? HIGH : LOW);
}