#ifndef STATE_H
#define STATE_H

#include <Arduino.h>

extern String bayStatus;

extern float voltage;
extern float current;
extern float power;
extern float energyWh;
extern float temperature;

extern int lastHourOfDay;
extern unsigned long sessionStartMs;
extern float predictedArrivalProb;
extern int predictedDurationMin;

extern bool overloadActive;
extern String loadDecision;
extern int throttleLevel;
extern bool manualOverrideActive;

#endif