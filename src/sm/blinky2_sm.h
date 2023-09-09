#ifndef BLINKY2_H
#define BLINKY2_H

#include "statemachine.h"

typedef struct {
	SM_params statemachine;
	TimerHandle_t timerhandle;
	TimerCallbackFunction_t timerCallbackFunc;

} blinky2_params;

SM_return Blinky2_init(blinky2_params *pvParameters,void * event);



#endif
