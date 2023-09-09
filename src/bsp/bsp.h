#ifndef BSP_H
#define BSP_H

#include "xil_types.h"
#include "xparameters.h"
#include "xil_io.h"


#define LEDBASE_DATA 0x41200000 + 8
#define SWBASE_DATA  0x41200000 + 0
#define LED_RED     (0)
#define LED_BLUE    (1)
#define LED_GREEN   (2)
#define LED_DBG    (3)
#define BTN_SW0   (1U << 0)
#define BTN_SW1   (1U << 1)
#define BTN_SW2   (1U << 2)
#define BTN_SW3   (1U << 3)


//Methods

void BSP_setLED(u8 led);
void BSP_clearLED(u8 led);


#endif
