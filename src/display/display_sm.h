#ifndef DISPLAY_SM_H_
#define DISPLAY_SM_H_

#include "../sm/statemachine.h"
#include "dvi_vdma.h"
#include "../bsp/bsp.h"


typedef struct {
	SM_params statemachine;
	TimerHandle_t timerhandle;
	TimerCallbackFunction_t timerCallbackFunc;

} display_params;

SM_return Display_init(display_params *pvParameters,void * event);



#endif
