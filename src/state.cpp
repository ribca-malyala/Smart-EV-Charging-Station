#include "state.h"

String bayStatus = "FREE";

float voltage = 0.0;
float current = 0.0;
float power = 0.0;
float energyWh = 0.0;
float temperature = 0.0;

int lastHourOfDay = 12;
unsigned long sessionStartMs = 0;
float predictedArrivalProb = 0.0;
int predictedDurationMin = 0;

bool overloadActive = false;
String loadDecision = "ALLOW";
int throttleLevel = 100;
bool manualOverrideActive = false;