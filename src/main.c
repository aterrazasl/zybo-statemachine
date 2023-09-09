/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "sm/blinky1_sm.h"
#include "sm/blinky2_sm.h"

blinky1_params blinky1;
blinky1_params blinky2;
// task handles
TaskHandle_t blinky1_handle;
TaskHandle_t blinky2_handle;

int main(void) {


	blinky1.statemachine.init_ptr = Blinky1_init;

	xTaskCreate(SM_dispatcher, (const char *) "Blinky1",
	configMINIMAL_STACK_SIZE, &blinky1, tskIDLE_PRIORITY,
			&blinky1_handle);

	blinky2.statemachine.init_ptr = Blinky2_init;
	xTaskCreate(SM_dispatcher, (const char *) "Blinky2",
	configMINIMAL_STACK_SIZE, &blinky2, tskIDLE_PRIORITY,
			&blinky2_handle);


	vTaskStartScheduler();

	for (;;);
}
