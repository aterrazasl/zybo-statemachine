/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "sm/blinky1_sm.h"
#include "sm/blinky2_sm.h"
#include "hcd/hcd.h"
#include "display/display_sm.h"
#include "xil_mmu.h"

static blinky1_params blinky1;
static blinky1_params blinky2;
static hcd_params hcd;
static display_params disp;


// task handles
static TaskHandle_t blinky1_handle;
static TaskHandle_t blinky2_handle;
static TaskHandle_t hcd_handle;
static TaskHandle_t display_handle;

extern SM_params *signal_pool[10];
extern int pool_count;


int main(void) {

	blinky1.statemachine.init_ptr = Blinky1_init;
	xTaskCreate(SM_dispatcher, (const char *) "Blinky1",
	configMINIMAL_STACK_SIZE, &blinky1, tskIDLE_PRIORITY,
			&blinky1_handle);

	blinky2.statemachine.init_ptr = Blinky2_init;
	xTaskCreate(SM_dispatcher, (const char *) "Blinky2",
	configMINIMAL_STACK_SIZE, &blinky2, tskIDLE_PRIORITY,
			&blinky2_handle);

	disp.statemachine.init_ptr = Display_init;
	xTaskCreate(SM_dispatcher, (const char *) "DVI",
	configMINIMAL_STACK_SIZE, &disp, tskIDLE_PRIORITY+1,
			&display_handle);

	hcd.statemachine.init_ptr = hcd_init;
	xTaskCreate(SM_dispatcher, (const char *) "HCD",
	configMINIMAL_STACK_SIZE, &hcd, tskIDLE_PRIORITY+2,
			&hcd_handle);

	vTaskStartScheduler();

	for (;;);
}
