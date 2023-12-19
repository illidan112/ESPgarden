#include <stdint.h>
#ifndef _FAN_H_
#define _FAN_H_

extern const uint8_t BOX_VENT;
extern const uint8_t LAMP_VENT;

void fanInit();

void fanTurnON(uint8_t fan );

void fanTurnOFF(uint8_t fan);

#endif