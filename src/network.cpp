#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

#include "network.h"
#include "config.h"
#include "rpc.h"

WiFiClient espClient;
PubSubClient mqtt(espClient);

void connectWiFi()
{
    Serial.print("Connecting to WiFi");

    WiFi.begin(WIFI_SSID, WIFI_PASS);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println();
    Serial.println("WiFi connected!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
}

void connectMQTT()
{
    mqtt.setServer(MQTT_SERVER, MQTT_PORT);

    // Allow larger MQTT messages
    mqtt.setBufferSize(1024);

    // Register incoming MQTT callback
    mqtt.setCallback(mqttCallback);

    while (!mqtt.connected())
    {
        Serial.print("Connecting to ThingsBoard...");

        if (mqtt.connect(BAY_ID, TB_TOKEN, nullptr))
        {
            Serial.println("connected!");

            // Standard ThingsBoard RPC
            bool standardRpc =
                mqtt.subscribe("v1/devices/me/rpc/request/+");

            // Short ThingsBoard RPC
            bool shortRpc =
                mqtt.subscribe("v2/r/req/+");

            // Diagnostic: listen to all incoming MQTT topics
            bool debugAll =
                mqtt.subscribe("#");

            Serial.print("Standard RPC subscription: ");
            Serial.println(standardRpc);

            Serial.print("Short RPC subscription: ");
            Serial.println(shortRpc);

            Serial.print("Debug ALL subscription: ");
            Serial.println(debugAll);
        }
        else
        {
            Serial.print("failed, state=");
            Serial.println(mqtt.state());

            delay(2000);
        }
    }
}
void mqttCallback(char* topic, byte* payload, unsigned int length)
{
    Serial.println();
    Serial.println("========== MQTT MESSAGE ==========");

    Serial.print("Topic: ");
    Serial.println(topic);

    Serial.print("Payload: ");

    for (unsigned int i = 0; i < length; i++)
    {
        Serial.print((char)payload[i]);
    }

    Serial.println();
    Serial.println("==================================");

    String topicString = String(topic);

    String requestId = "";

    // ------------------------------------------------
    // Standard RPC topic
    // v1/devices/me/rpc/request/<requestId>
    // ------------------------------------------------

    String standardPrefix =
        "v1/devices/me/rpc/request/";

    if (topicString.startsWith(standardPrefix))
    {
        requestId =
            topicString.substring(standardPrefix.length());

        char message[512];

        if (length >= sizeof(message))
        {
            Serial.println("RPC payload too large.");
            return;
        }

        memcpy(message, payload, length);
        message[length] = '\0';

        handleRpc(requestId, message, false);

        return;
    }

    // ------------------------------------------------
    // Short RPC topic
    // v2/r/req/<requestId>
    // ------------------------------------------------

    String shortPrefix =
        "v2/r/req/";

    if (topicString.startsWith(shortPrefix))
    {
        requestId =
            topicString.substring(shortPrefix.length());

        char message[512];

        if (length >= sizeof(message))
        {
            Serial.println("RPC payload too large.");
            return;
        }

        memcpy(message, payload, length);
        message[length] = '\0';

        handleRpc(requestId, message, true);

        return;
    }

    Serial.println("MQTT message was not an RPC message.");
}