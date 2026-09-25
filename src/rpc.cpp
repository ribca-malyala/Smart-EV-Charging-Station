#include <Arduino.h>
#include <ArduinoJson.h>

#include "rpc.h"
#include "network.h"
#include "state.h"
#include "config.h"

#define RELAY_PIN 26

void handleRpc(
    String requestId,
    char* payload,
    bool shortTopic
)
{
    Serial.println();
    Serial.println("========== RPC REQUEST ==========");
    Serial.print("Request ID: ");
    Serial.println(requestId);
    Serial.print("Payload: ");
    Serial.println(payload);

    JsonDocument doc;

    DeserializationError error =
        deserializeJson(doc, payload);

    if (error)
    {
        Serial.print("RPC JSON error: ");
        Serial.println(error.c_str());
        return;
    }

    String method =
        doc["method"] | "";

    JsonObject params =
        doc["params"].as<JsonObject>();

    Serial.print("RPC method: ");
    Serial.println(method);

    JsonDocument response;

    // ------------------------------------------------
    // setRelayState
    // ------------------------------------------------

    if (method == "setRelayState")
    {
        bool state =
            params["state"] | false;

        manualOverrideActive = true;

        throttleLevel =
            state ? 100 : 0;

        loadDecision =
            state ? "MANUAL_ON" : "MANUAL_OFF";

        // Immediately control relay
        digitalWrite(
            RELAY_PIN,
            state ? HIGH : LOW
        );

        response["success"] = true;
        response["method"] = "setRelayState";
        response["relayState"] = state;
        response["throttleLevel"] = throttleLevel;

        Serial.print("Relay manually set to: ");
        Serial.println(state ? "ON" : "OFF");
    }

    // ------------------------------------------------
    // setThrottle
    // ------------------------------------------------

    else if (method == "setThrottle")
    {
        int level =
            params["level"] | 100;

        level =
            constrain(level, 0, 100);

        manualOverrideActive = true;

        throttleLevel = level;

        loadDecision =
            "MANUAL_THROTTLE";

        response["success"] = true;
        response["method"] = "setThrottle";
        response["throttleLevel"] = throttleLevel;

        Serial.print("Manual throttle set to: ");
        Serial.print(throttleLevel);
        Serial.println("%");
    }

    // ------------------------------------------------
    // clearManualOverride
    // ------------------------------------------------

    else if (method == "clearManualOverride")
    {
        manualOverrideActive = false;

        loadDecision = "ALLOW";
        throttleLevel = 100;

        response["success"] = true;
        response["method"] = "clearManualOverride";

        Serial.println("Manual override cleared.");
    }

    // ------------------------------------------------
    // Unknown RPC
    // ------------------------------------------------

    else
    {
        response["success"] = false;
        response["error"] = "unknown method";
        response["method"] = method;

        Serial.print("Unknown RPC method: ");
        Serial.println(method);
    }

    char buffer[512];

    serializeJson(
        response,
        buffer,
        sizeof(buffer)
    );

    // ------------------------------------------------
    // Send response using matching topic format
    // ------------------------------------------------

    String responseTopic;

    if (shortTopic)
    {
        responseTopic =
            "v2/r/res/" + requestId;
    }
    else
    {
        responseTopic =
            "v1/devices/me/rpc/response/" +
            requestId;
    }

    bool published =
        mqtt.publish(
            responseTopic.c_str(),
            buffer
        );

    Serial.print("RPC response topic: ");
    Serial.println(responseTopic);

    Serial.print("RPC response: ");
    Serial.println(buffer);

    Serial.print("RPC response published: ");
    Serial.println(published);

    Serial.println("=================================");
}