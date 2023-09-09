/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
/* Xilinx includes. */
#include "xil_printf.h"
#include "xparameters.h"

#include "bsp/bsp.h"


typedef enum{
	event_init,
	event_enter,
	event_timer_tick,
	event_exit
}events;

typedef enum {
	state_handled, state_transition, state_error
} SM_return;

typedef struct {
	TimerHandle_t timerhandle;
	TimerCallbackFunction_t timerCallbackFunc;
	QueueHandle_t stateQueue;
	SM_return (*stateFun_ptr)(void*,void*);
} SM_params;

static SM_return SM_init(SM_params *pvParameters,void * event);
static SM_return SM_blink_off(SM_params *pvParameters,void * event);
static SM_return SM_blink_on(SM_params *pvParameters,void * event);


SM_params sm_params;

// task handles
TaskHandle_t sm_dispatcher_handle;

static void nextState(SM_params *pvParameters, void* sm_state) {

	pvParameters->stateFun_ptr = sm_state;
}

static void vTimerCallback(TimerHandle_t xTimer) {
	events sm_event = event_timer_tick;
	SM_params * pvParameters = (SM_params*)pvTimerGetTimerID(xTimer);

	xQueueSend(pvParameters->stateQueue, &sm_event, (TickType_t ) 0);
}

static SM_return SM_blink_on(SM_params *pvParameters,void * event) {
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
		nextState(pvParameters, SM_blink_off);
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

static SM_return SM_blink_off(SM_params *pvParameters,void * event) {
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
		nextState(pvParameters, SM_blink_on);
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

static SM_return SM_init(SM_params *pvParameters,void * event) {
	pvParameters->timerCallbackFunc = vTimerCallback;

	pvParameters->timerhandle = xTimerCreate("SM_Timer", 100, pdFALSE, pvParameters,
			pvParameters->timerCallbackFunc);
	nextState(pvParameters, SM_blink_on);

	return state_transition;
}

static void SM_dispatcher(void* params) {
	SM_params* pvParameters = (SM_params*)params;
	SM_return sm_ret;
	events sm_event;

	pvParameters->stateQueue = xQueueCreate(10, sizeof(events));

	nextState(pvParameters, SM_init);
	sm_event = event_init;
	xQueueSend(pvParameters->stateQueue,  &sm_event, (TickType_t ) 0);

	while (1) {

		xQueueReceive(pvParameters->stateQueue, &sm_event,(TickType_t) portMAX_DELAY);

		sm_ret = (*pvParameters->stateFun_ptr)(pvParameters, &sm_event);

		switch (sm_ret) {
		case state_handled:
			break;
		case state_transition:
			sm_event =event_enter;
			xQueueSend(pvParameters->stateQueue,  &sm_event, (TickType_t ) 0);
			break;
		case state_error:
			break;
		default:
			break;

		}
	}
}

int main(void) {

	xTaskCreate(SM_dispatcher, (const char *) "Dispatcher",
	configMINIMAL_STACK_SIZE, &sm_params, tskIDLE_PRIORITY,
			&sm_dispatcher_handle);

	vTaskStartScheduler();

	for (;;);
}


