#ifndef CONFIG_H
#define CONFIG_H

extern const char* WIFI_SSID;
extern const char* WIFI_PASS;

extern const char* MQTT_SERVER;
extern const int MQTT_PORT;

extern const char* TB_TOKEN;
extern const char* BAY_ID;

extern float maxStationLoadW;
extern float overloadCurrentA;
extern int peakTariffStartHr;
extern int peakTariffEndHr;
extern float predictionThreshold;

#endif