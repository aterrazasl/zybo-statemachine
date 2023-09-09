#include "bsp.h"


void BSP_setLED(u8 led) {
	Xil_Out32(LEDBASE_DATA, Xil_In32(LEDBASE_DATA) | (1 << led));
}
void BSP_clearLED(u8 led) {
	Xil_Out32(LEDBASE_DATA, Xil_In32(LEDBASE_DATA) & ~(1 << led));
}
