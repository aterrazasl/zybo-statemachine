#ifndef DVI_VDMA_H_
#define DVI_VDMA_H_

#include <xil_types.h>

#define DVI_HORIZONTAL (640-0)
#define DVI_VERTICAL (480-0)

#define DVI_HORIZONTAL_CENTER DVI_HORIZONTAL/2
#define DVI_VERTICAL_CENTER   DVI_VERTICAL/2

#define DVI_BYTES_PER_LINE 3
#define DVI_TOTALMEM DVI_HORIZONTAL * DVI_VERTICAL * DVI_BYTES_PER_LINE

enum{
	CBlack,CWhite,CRed,CGreen,CBlue,CGray,CYellow
};


int DVI_initDVI(void);
void Display_sendPA(uint32_t const *pa, uint16_t y, uint16_t h);
void Display_sendPA_interleaved(uint32_t const *pa, uint16_t y, uint16_t h);
void Display_fillCircle(int16_t x0, int16_t y0, int16_t radius);
void Display_drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1);
void Display_changePenColor(uint8_t color);
void Display_flushMem(void);
void Display_updateXYaxis(int16_t x, int16_t y);
void Display_drawRectCenter(int16_t x, int16_t y, int16_t width,int16_t height, int color);
void Display_drawRect(int16_t x, int16_t y, int16_t width,int16_t height, int color);
void Display_fillRect(int16_t x, int16_t y, int16_t width,int16_t height, int color);
void Display_setPixel(int16_t x0, int16_t y0);
void DVI_drawOutline(void);

#endif
