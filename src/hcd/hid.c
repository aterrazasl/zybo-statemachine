#include "hid.h"
#include "xil_printf.h"
#include "hcd_usbCh9.h"


static void hid_createGetReportRequest(hcd_t *hcdPtr);

static hcd_endpoint0  ep1;
static char hid_data[8];

void hid_printLine(u8* data, u32 size, char* comment){
	int i;
	for(i = 0; i < size; i++){
		xil_printf("%02X", *(data+i));
	}
	xil_printf(" // %s\r", comment);
}

static u8 evaluateDiff(u8* new){
	int i;
	for(i =0;i<8;i++){
		if(hid_data[i]!= new[i]){
			return 1;
		}
	}
	return 0;
}

char* readHID_Data(void){
	return hid_data;
}

void hid_callbackHandler(void *CallBackRef, u32 Mask){
	hcd_t *hcdPtr = (hcd_t*)CallBackRef;
	if(evaluateDiff(((u8*)(hcdPtr->periodicqTD[1]->buffer[0]&0xfffff000)))){
		hid_printLine((u8*)(hcdPtr->periodicqTD[1]->buffer[0]&0xfffff000), 0x0008, "Keyboard Report");
		memcpy((void*)hid_data, (void*)(hcdPtr->periodicqTD[1]->buffer[0]&0xfffff000) , 8);
	}
}

static void hid_createGetReportRequest(hcd_t *hcdPtr){

	hcd_endpoint0 *epPtr = hcdPtr->ep1;

	epPtr->setupData.bmRequestType = DEVICE_TO_HOST | CLASS_TYPE | INTERFACE_RECIPIENT;
	epPtr->setupData.bRequest 		= GET_REPORT;
	epPtr->setupData.wValue 		= hcd_swap_uint16(REPORT_INPUT);
	epPtr->setupData.wIndex 		= hcd_swap_uint16(0x0000);
	epPtr->setupData.wLength 		= 0x0008;

	epPtr->address = 1;
	epPtr->maxPacketSize = 64;
	epPtr->expectReply = 1;

	return;
}

void hid_requestReport(hcd_t *hcdPtr){
	hcdPtr->ep1 = &ep1;
	hid_createGetReportRequest(hcdPtr);
	hcd_enquePeriodicQH(hcdPtr);

}
