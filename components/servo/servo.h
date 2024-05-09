#include <stdint.h>
#include "esp_err.h"

#ifndef SERVO_H
#define SERVO_H

void suspendRotation();
void resumeRotation();

void ServoTask(void* pvParameters);

#endif