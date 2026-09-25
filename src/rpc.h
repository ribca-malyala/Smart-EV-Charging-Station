#ifndef RPC_H
#define RPC_H

#include <Arduino.h>

void handleRpc(
    String requestId,
    char* payload,
    bool shortTopic
);

#endif