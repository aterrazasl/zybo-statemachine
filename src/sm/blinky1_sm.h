#ifndef BLINKY1_H
#define BLINKY1_H

#include "statemachine.h"

typedef struct {
	SM_params statemachine;
	TimerHandle_t timerhandle;
	TimerCallbackFunction_t timerCallbackFunc;

} blinky1_params;

SM_return Blinky1_init(blinky1_params *pvParameters,void * event);



#endif
