#include "display_sm.h"
#include "dvi_vdma.h"


static void vTimerCallback(TimerHandle_t xTimer) {
	events sm_event = event_timer_tick;
	display_params * pvParameters = (display_params*)pvTimerGetTimerID(xTimer);

	xQueueSend(pvParameters->statemachine.stateQueue, &sm_event, (TickType_t ) 0);
}

static SM_return Display_idle(display_params *pvParameters,void * event){

	static int stat = 0;

	events *e = (events*) event;
	SM_return ret = state_handled;
	switch (*e) {
	case event_enter:
		ret = state_handled;
		break;
	case event_timer_tick:
		if(stat == 0){
			BSP_clearLED(3);
			stat =1;
		}
		else{
			BSP_setLED(3);
			stat = 0;
		}
		ret = state_handled;
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

SM_return Display_init(display_params *pvParameters,void * event){

	events *e = (events*) event;
	SM_return ret = state_handled;
	switch (*e) {
	case event_init:

		pvParameters->timerCallbackFunc = vTimerCallback;

		pvParameters->timerhandle = xTimerCreate("SM_Timer_display", 3, pdTRUE, pvParameters,
				pvParameters->timerCallbackFunc);

		xTimerStart(pvParameters->timerhandle, 0);

		DVI_initDVI();

		nextState((void*)pvParameters, (void*)Display_idle);
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

