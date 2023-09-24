#include "blinky1_sm.h"
#include "../bsp/bsp.h"


static SM_return Blinky1_on(blinky1_params *pvParameters,void * event);
static SM_return Blinky1_off(blinky1_params *pvParameters,void * event);


static void vTimerCallback(TimerHandle_t xTimer) {
	events sm_event = event_timer_tick;
	blinky1_params * pvParameters = (blinky1_params*)pvTimerGetTimerID(xTimer);

	xQueueSend(pvParameters->statemachine.stateQueue, &sm_event, (TickType_t ) 0);
}

static SM_return Blinky1_on(blinky1_params *pvParameters,void * event) {
	events *e = (events*) event;
	SM_return ret = state_handled;
	switch (*e) {
	case event_enter:
		xTimerChangePeriod(pvParameters->timerhandle, 10, 0);
		xTimerStart(pvParameters->timerhandle, 0);
		BSP_setLED(0);
		ret = state_handled;
		break;
	case event_timer_tick:
		nextState((void*)pvParameters, (void*)Blinky1_off);
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

static SM_return Blinky1_off(blinky1_params *pvParameters,void * event) {
	events *e = (events*) event;
	SM_return ret = state_handled;
	switch (*e) {
	case event_enter:
		xTimerChangePeriod(pvParameters-> timerhandle, 90, 0);
		xTimerStart(pvParameters->timerhandle, 0);
		BSP_clearLED(0);
		ret = state_handled;
		break;
	case event_timer_tick:
		nextState((void*)pvParameters, (void*)Blinky1_on);
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

SM_return Blinky1_init(blinky1_params *pvParameters,void * event) {
	events *e = (events*) event;
	SM_return ret = state_handled;
	switch (*e) {
	case event_init:
		pvParameters->timerCallbackFunc = vTimerCallback;

		pvParameters->timerhandle = xTimerCreate("SM_Timer", 100, pdFALSE, pvParameters,
				pvParameters->timerCallbackFunc);
		nextState((void*)pvParameters, (void*)Blinky1_on);

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

SM_return Blinky1_init_old(blinky1_params *pvParameters,void * event) {
	pvParameters->timerCallbackFunc = vTimerCallback;

	pvParameters->timerhandle = xTimerCreate("SM_Timer", 100, pdFALSE, pvParameters,
			pvParameters->timerCallbackFunc);
	nextState((void*)pvParameters, (void*)Blinky1_on);

	return state_transition;
}

