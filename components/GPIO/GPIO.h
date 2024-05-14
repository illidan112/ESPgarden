#ifndef _LIGHTING_H_
#define _LIGHTING_H_

void lightingInit();
void fanInit();

void lightingTurnON();

void lightingTurnOFF();

void fanTurnON(fangpio_t fan);

void fanTurnOFF(fangpio_t fan);

void EnableButton();

typedef enum {
    outFan = 14,
    movingFan = 15, // надо поменять
    inFan,
} fangpio_t;

#endif