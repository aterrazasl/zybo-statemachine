#include "dvi_vdma.h"
#include "xil_io.h"
#include "stdlib.h"
#include "xil_cache.h"
#include "GFX.h"

static UINTPTR memptr;
unsigned int videoMem[DVI_HORIZONTAL][DVI_VERTICAL];
static uint16_t x_translate;
static uint16_t y_translate;


static void translateXY(int16_t *x, int16_t *y){
	*x += x_translate; //DVI_HORIZONTAL_CENTER;
	*y = -((*y)- y_translate);
}

void Display_flushMem(void){
	Xil_DCacheFlushRange(memptr, DVI_TOTALMEM);
}

void flushMem(void) {
	Xil_DCacheFlushRange(memptr, DVI_TOTALMEM);
}

static int mapColor(int color){
	int ret=0x00000000;
	switch(color){
	case CBlack:
		ret = 0x00000000;
		break;
	case CWhite:
		ret = 0x00ffffff;
		break;
	case CBlue:
		ret = 0x000000ff;
		break;
	case CGreen:
		ret = 0x0000ff00;
		break;
	case CRed:
		ret = 0x00ff0000;
		break;
	case CGray:
		ret = 0x007f7f7f;
		break;
	case CYellow:
		ret = 0x00ffff00;
		break;
	default:
		break;
	}

	return ret;
}
void Display_fillRect(int16_t x, int16_t y, int16_t width,int16_t height, int color){
	GFX_changePenColor(mapColor(color));
	GFX_fillRect(x, y, width,height);
}

void Display_drawRect(int16_t x, int16_t y, int16_t width,int16_t height, int color){
	GFX_changePenColor(mapColor(color));
	GFX_drawRect(x, y, width,height);
}

void Display_drawRectCenter(int16_t x, int16_t y, int16_t width,int16_t height, int color){
	int16_t x0= x;
	int16_t y0= y;
	translateXY(&x0,&y0);

	GFX_changePenColor(mapColor(color));
	GFX_drawRect(x0-(width), y0-(height), (width*2),(height*2));
}


void DVI_drawOutline(void) {

	GFX_changePenColor(mapColor(CBlack));	// change color to Black
	GFX_fillRect(0, 0, DVI_HORIZONTAL, DVI_VERTICAL);

	GFX_changePenColor(mapColor(CWhite));
	drawLine(0, 0, 0, DVI_VERTICAL - 1);
	drawLine(DVI_HORIZONTAL - 1, 0, DVI_HORIZONTAL - 1, DVI_VERTICAL - 1);

	drawLine(0, 0, DVI_HORIZONTAL - 1, 0);
	drawLine(0, DVI_VERTICAL - 1, DVI_HORIZONTAL - 1, DVI_VERTICAL - 1);

	GFX_changePenColor(mapColor(CWhite));	//change color to White

	Xil_DCacheFlushRange(memptr, DVI_TOTALMEM);

}

static void DVI_ConfigureVDMA(UINTPTR mempoint, int htzl, int vtl,
		int bytes_per_line) {

//    xil_printf("version:0x%08X\n", *((int*)0x4300002c));

	Xil_Out32(XPAR_AXI_VDMA_0_BASEADDR + 0x00, 0x00); 		//Stop DMA
	Xil_Out32(XPAR_AXI_VDMA_0_BASEADDR + 0x00, 0x01); 	//Start DMA in park mode

	Xil_Out32(XPAR_AXI_VDMA_0_BASEADDR + 0x5C, mempoint); 	//Pass DMA pointers
//	Xil_Out32(XPAR_AXI_VDMA_0_BASEADDR + 0x60, mempoint); 	//Pass DMA pointers
//	Xil_Out32(XPAR_AXI_VDMA_0_BASEADDR + 0x64, mempoint); 	//Pass DMA pointers

//    xil_printf("Mem Location:0x%08X\n", *((int*)0x4300005c));

	Xil_Out32(XPAR_AXI_VDMA_0_BASEADDR + 0x58, (htzl) * bytes_per_line); //Configures the htzl
	Xil_Out32(XPAR_AXI_VDMA_0_BASEADDR + 0x54, (htzl) * bytes_per_line);
	Xil_Out32(XPAR_AXI_VDMA_0_BASEADDR + 0x50, vtl); 		//Configures lines

	Xil_Out32(XPAR_AXI_VDMA_0_BASEADDR + 0x28, 0x00); // in park mode, sets the pointer where it will park

//    xil_printf("status:0x%08X\n", *((int*)0x43000004));
}

static int DVI_initVDMA(void) {
//	memptr = (UINTPTR) malloc(
//			DVI_HORIZONTAL * DVI_VERTICAL * DVI_BYTES_PER_LINE);

	memptr = (UINTPTR) &videoMem;
	DVI_ConfigureVDMA(memptr, DVI_HORIZONTAL, DVI_VERTICAL, DVI_BYTES_PER_LINE);

//	xil_printf("\r\nAddress of videoMem = %08X\r\n",&videoMem);
	return 0;
}

int DVI_initDVI(void) {
	DVI_initVDMA();

	GFX_init(memptr, DVI_HORIZONTAL, DVI_VERTICAL);
	DVI_drawOutline();
	return 0;
}

void Display_sendPA(uint32_t const *pa, uint16_t y, uint16_t h) {
	uint32_t i, j, k;
	uint32_t const *p = pa;

	for (i = y; i < DVI_VERTICAL; i += 1) {

		for (j = 0; j < h; j += 32, p += 1) {

			for (k = 0; k < 32; k++) {
				if (*p & (1 << k)) {
					setPixelColor(k + j, i, 0xff);
				} else {
					setPixelColor(k + j, i, 0x00);
				}
			}
		}
	}

}

void Display_sendPA_interleaved(uint32_t const *pa, uint16_t y, uint16_t h) {
	uint32_t i, j, k;
	uint32_t const *p = pa;
	static uint32_t start=1;


	if (start ==0){
		start =1;
		p+=20;
	}
	else
	{
		start =0;
	}
	for (i = y + start ; i < DVI_VERTICAL; i+=2) {

		for (j = 0 ; j < h; j += 32, p+=1) {

			for (k = 0; k < 32; k++) {
				if (*p & (1 << k)) {
					setPixelColor(k + j,i, 0xff);
				} else {
					setPixelColor(k + j,i, 0x00);
				}
			}
		}
		p+=20;
	}

}

void Display_updateXYaxis(int16_t x, int16_t y){


	x_translate = x + DVI_HORIZONTAL_CENTER;
	y_translate = -(y - DVI_VERTICAL_CENTER);
}

void Display_drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1){

	int16_t x0_= x0;
	int16_t y0_= y0;
	translateXY(&x0_,&y0_);

	int16_t x1_= x1;
	int16_t y1_= y1;
	translateXY(&x1_,&y1_);


	drawLine(x0_,y0_,x1_,y1_);

}


void Display_fillCircle(int16_t x0, int16_t y0, int16_t radius){

	int16_t x= x0;
	int16_t y= y0;
	translateXY(&x,&y);
	GFX_fillCircle(x,y,radius);

}

void Display_setPixel(int16_t x0, int16_t y0){

	int16_t x= x0;
	int16_t y= y0;
	translateXY(&x,&y);
	setPixel(x,y);

}

void Display_changePenColor(uint8_t color){
	GFX_changePenColor(mapColor(color));
//	GFX_changePenColor(color);
}

