#ifndef STAR_SEARCH_SM_H
#define STAR_SEARCH_SM_H

#include "../sm/statemachine.h"
#include "../display/dvi_vdma.h"
#include "../bsp/bsp.h"


typedef struct {
	SM_params statemachine;
	TimerHandle_t timerhandle;
	TimerCallbackFunction_t timerCallbackFunc;

} ss_params;

SM_return SS_init(ss_params *pvParameters,void * event);



#endif
