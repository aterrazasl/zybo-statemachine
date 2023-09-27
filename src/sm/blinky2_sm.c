#include "blinky2_sm.h"
#include "../bsp/bsp.h"


static SM_return Blinky2_on(blinky2_params *pvParameters,void * event);
static SM_return Blinky2_off(blinky2_params *pvParameters,void * event);


static void vTimerCallback(TimerHandle_t xTimer) {
	events sm_event = event_timer_tick;
	blinky2_params * pvParameters = (blinky2_params*)pvTimerGetTimerID(xTimer);

	xQueueSend(pvParameters->statemachine.stateQueue, &sm_event, (TickType_t ) 0);
}

static SM_return Blinky2_on(blinky2_params *pvParameters,void * event) {
	events *e = (events*) event;
	SM_return ret = state_handled;
	switch (*e) {
	case event_enter:
		xTimerChangePeriod(pvParameters->timerhandle, 90, 0);
		xTimerStart(pvParameters->timerhandle, 0);
		BSP_setLED(1);
		ret = state_handled;
		break;
	case event_timer_tick:
		SM_nextState((void*)pvParameters, (void*)Blinky2_off);
		ret = state_transition;
		break;
	case event_exit:
		ret = state_handled;
		break;
	default:
		ret = state_error;
		break;
	}
	return ret;
}

static SM_return Blinky2_off(blinky2_params *pvParameters,void * event) {
	events *e = (events*) event;
	SM_return ret = state_handled;
	switch (*e) {
	case event_enter:
		xTimerChangePeriod(pvParameters-> timerhandle, 10, 0);
		xTimerStart(pvParameters->timerhandle, 0);
		BSP_clearLED(1);
		ret = state_handled;
		break;
	case event_timer_tick:
		SM_nextState((void*)pvParameters, (void*)Blinky2_on);
		ret = state_transition;
		break;
	case event_exit:
		ret = state_handled;
		break;
	default:
		ret = state_error;
		break;
	}
	return ret;
}


SM_return Blinky2_init(blinky2_params *pvParameters,void * event) {
	events *e = (events*) event;
	SM_return ret = state_handled;
	switch (*e) {
	case event_init:
		pvParameters->timerCallbackFunc = vTimerCallback;

		pvParameters->timerhandle = xTimerCreate("SM_Timer", 100, pdFALSE, pvParameters,
				pvParameters->timerCallbackFunc);
		SM_nextState((void*)pvParameters, (void*)Blinky2_on);
		ret = state_transition;
		break;
	case event_exit:
		ret = state_handled;
		break;
	default:
		ret = state_error;
		break;
	}
	return ret;
}


SM_return Blinky2_init_old(blinky2_params *pvParameters,void * event) {
	pvParameters->timerCallbackFunc = vTimerCallback;

	pvParameters->timerhandle = xTimerCreate("SM_Timer", 100, pdFALSE, pvParameters,
			pvParameters->timerCallbackFunc);
	SM_nextState((void*)pvParameters, (void*)Blinky2_on);

	return state_transition;
}

