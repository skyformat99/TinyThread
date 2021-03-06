/******************************************************************************
 * @file     CDC_Class.c
 * @brief    Nano1xx USB Driver Sample code
 * @version  1.0.1
 * @date     04, September, 2012
 *
 * @note
 * Copyright (C) 2012-2014 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include <string.h>
#include "nano1xx_uart.h"
#include "nano1xx_usb.h"
#include "CDC_Class.h"


/* Define the vendor id and product id */
#define USB_VID		0x0416
#define USB_PID		0x5011

/* Define EP maximum packet size */
#define	MAX_PACKET_SIZE_CTRL        64
#define MAX_PACKET_SIZE_BULK		64
#define MAX_PACKET_SIZE_INT			8

/* Define the interrupt In EP number */
#define BULK_IN_EP_NUM      1
#define BULK_OUT_EP_NUM     2
#define INT_IN_EP_NUM       3   


/********************* Global Variables **********************/
STR_USBD_VCOM_T gVcomInfo;

/********************* Global Variables for UART **********************/
uint8_t volatile comRbuf[RXBUFSIZE];
uint16_t volatile comRbytes = 0;
uint16_t volatile comRhead = 0;
uint16_t volatile comRtail = 0;

uint8_t volatile comTbuf[TXBUFSIZE];
uint16_t volatile comTbytes = 0;
uint16_t volatile comThead = 0;
uint16_t volatile comTtail = 0;

uint8_t gRxBuf[MAX_PACKET_SIZE_BULK] = {0};


/********************************************************/
/*!<USB Device Descriptor */
const uint8_t USB_DeviceDescriptor[] =
{
	LEN_DEVICE,		/* bLength              */
	DESC_DEVICE,	/* bDescriptorType      */
	0x00, 0x02,		/* bcdUSB               */
	0x02,			/* bDeviceClass         */
	0x00,			/* bDeviceSubClass      */
	0x00,			/* bDeviceProtocol      */
	MAX_PACKET_SIZE_CTRL,	/* bMaxPacketSize0 */
	/* idVendor */
	USB_VID & 0x00FF,
	(USB_VID & 0xFF00) >> 8,
	/* idProduct */
	USB_PID & 0x00FF,
	(USB_PID & 0xFF00) >> 8,
	0x00, 0x03,		/* bcdDevice            */
	0x01,			/* iManufacture         */
	0x02,			/* iProduct             */
	0x00,			/* iSerialNumber        */
	0x01			/* bNumConfigurations   */
};


/*!<USB Configure Descriptor */
const uint8_t USB_ConfigDescriptor[] =
{
	LEN_CONFIG,		/* bLength              */
	DESC_CONFIG,	/* bDescriptorType      */
	0x43, 0x00,		/* wTotalLength         */
	0x02,			/* bNumInterfaces       */
	0x01,			/* bConfigurationValue  */
	0x00,			/* iConfiguration       */
	0xC0,			/* bmAttributes         */
	0x32,			/* MaxPower             */

	/* INTERFACE descriptor */
	LEN_INTERFACE,	/* bLength              */
	DESC_INTERFACE,	/* bDescriptorType      */
	0x00,			/* bInterfaceNumber     */
	0x00,			/* bAlternateSetting    */
	0x01,			/* bNumEndpoints        */
	0x02,			/* bInterfaceClass      */
	0x02,			/* bInterfaceSubClass   */
	0x01,			/* bInterfaceProtocol   */
	0x00,			/* iInterface           */

	/* Communication Class Specified INTERFACE descriptor */
    0x05,           /* Size of the descriptor, in bytes */
    0x24,           /* CS_INTERFACE descriptor type */
    0x00,           /* Header functional descriptor subtype */
    0x10, 0x01,     /* Communication device compliant to the communication spec. ver. 1.10 */
    
	/* Communication Class Specified INTERFACE descriptor */
    0x05,           /* Size of the descriptor, in bytes */
    0x24,           /* CS_INTERFACE descriptor type */
    0x01,           /* Call management functional descriptor */
    0x00,           /* BIT0: Whether device handle call management itself. */
                    /* BIT1: Whether device can send/receive call management information over a Data Class Interface 0 */
    0x01,           /* Interface number of data class interface optionally used for call management */

	/* Communication Class Specified INTERFACE descriptor */
    0x04,           /* Size of the descriptor, in bytes */
    0x24,           /* CS_INTERFACE descriptor type */
    0x02,           /* Abstract control management funcational descriptor subtype */
    0x00,           /* bmCapabilities       */
    
	/* Communication Class Specified INTERFACE descriptor */
    0x05,           /* bLength              */
    0x24,           /* bDescriptorType: CS_INTERFACE descriptor type */
    0x06,           /* bDescriptorSubType   */
    0x00,           /* bMasterInterface     */
    0x01,           /* bSlaveInterface0     */
    
	/* ENDPOINT descriptor */
	LEN_ENDPOINT,	                /* bLength          */
	DESC_ENDPOINT,	                /* bDescriptorType  */
	(EP_INPUT | INT_IN_EP_NUM),     /* bEndpointAddress */
	EP_INT,		                    /* bmAttributes     */
	MAX_PACKET_SIZE_INT, 0x00,	    /* wMaxPacketSize   */
	0x01,	                        /* bInterval        */
			
	/* INTERFACE descriptor */
	LEN_INTERFACE,	/* bLength              */
	DESC_INTERFACE,	/* bDescriptorType      */
	0x01,			/* bInterfaceNumber     */
	0x00,			/* bAlternateSetting    */
	0x02,			/* bNumEndpoints        */
	0x0A,			/* bInterfaceClass      */
	0x00,			/* bInterfaceSubClass   */
	0x00,			/* bInterfaceProtocol   */
	0x00,			/* iInterface           */
			
	/* ENDPOINT descriptor */
	LEN_ENDPOINT,	                /* bLength          */
	DESC_ENDPOINT,	                /* bDescriptorType  */
	(EP_INPUT | BULK_IN_EP_NUM),	/* bEndpointAddress */
	EP_BULK,		                /* bmAttributes     */
	MAX_PACKET_SIZE_BULK, 0x00,	    /* wMaxPacketSize   */
	0x00,			                /* bInterval        */

	/* ENDPOINT descriptor */
	LEN_ENDPOINT,	                /* bLength          */
	DESC_ENDPOINT,	                /* bDescriptorType  */
	(EP_OUTPUT | BULK_OUT_EP_NUM),	/* bEndpointAddress */
	EP_BULK,		                /* bmAttributes     */
	MAX_PACKET_SIZE_BULK, 0x00,     /* wMaxPacketSize   */
	0x00,			                /* bInterval        */
};


/*!<USB Language String Descriptor */
const uint8_t USB_StringLang[4] =
{
	4,				/* bLength */
	DESC_STRING,	/* bDescriptorType */
	0x09, 0x04
};

/*!<USB Vendor String Descriptor */
const uint8_t USB_VendorStringDesc[] =
{
	16,
	DESC_STRING,
	'N', 0, 'u', 0, 'v', 0, 'o', 0, 't', 0, 'o', 0, 'n', 0
};

/*!<USB Product String Descriptor */
const uint8_t USB_ProductStringDesc[] =
{
	32,             /* bLength          */
	DESC_STRING,    /* bDescriptorType  */
	'U', 0, 'S', 0,	'B', 0,	' ', 0,	'V', 0,	'i', 0,	'r', 0,	't', 0,	'u', 0,	'a', 0,	'l', 0,	
	' ', 0,	'C', 0,	'O', 0,	'M', 0
};


/*********************************************************/
/**   
  * @brief  USB_ProcessDescriptor, Process USB Descriptor.
  * @param  None.
  * @retval 1: Standard Request.
  *         0: has some error.
  */
uint32_t USB_ProcessDescriptor(uint8_t *pCtrl, uint32_t CtrlMaxPacketSize)
{
	gUsbCtrl.ReqLen = gUsbCtrl.UsbSetupBuf[6] + ((uint32_t)gUsbCtrl.UsbSetupBuf[7] << 8);
	USBD->CFG0 |= USB_CFG_DSQ_SYNC_DATA1;
            
	switch (gUsbCtrl.UsbSetupBuf[3])
	{
		/* Get Device Descriptor */
		case DESC_DEVICE:
		{
			gUsbCtrl.ReqLen = Minimum (gUsbCtrl.ReqLen, LEN_DEVICE);
	
			if (gUsbCtrl.ReqLen >= CtrlMaxPacketSize)
			{
				my_memcpy(pCtrl, (void *)USB_DeviceDescriptor, CtrlMaxPacketSize);
				gUsbCtrl.pDesc = (uint8_t *)&USB_DeviceDescriptor[CtrlMaxPacketSize];
				gUsbCtrl.EP0_Toggle = 1;
				USBD->MXPLD0 = CtrlMaxPacketSize;
				gUsbCtrl.ReqLen -= CtrlMaxPacketSize;
				gUsbCtrl.StateFlag = FLAG_EP0_DATA_IN;
			}
			else
			{
				my_memcpy(pCtrl, (void *)USB_DeviceDescriptor, gUsbCtrl.ReqLen);
				USBD->MXPLD0 = gUsbCtrl.ReqLen;
				gUsbCtrl.StateFlag = FLAG_OUT_ACK;
			}
			USBD->MXPLD1 = 0;
			break;
		}
		/* Get Configuration Descriptor */
		case DESC_CONFIG:
		{
			gUsbCtrl.ReqLen = Minimum (gUsbCtrl.ReqLen, (USB_ConfigDescriptor[3]<<8)|USB_ConfigDescriptor[2]);
			if (gUsbCtrl.ReqLen >= CtrlMaxPacketSize)
			{
				my_memcpy(pCtrl, (void *)USB_ConfigDescriptor, CtrlMaxPacketSize);
				gUsbCtrl.pDesc = (uint8_t *)&USB_ConfigDescriptor[CtrlMaxPacketSize];
				gUsbCtrl.EP0_Toggle = 1;
				USBD->MXPLD0 = CtrlMaxPacketSize;
				gUsbCtrl.ReqLen -= CtrlMaxPacketSize;
				gUsbCtrl.StateFlag = FLAG_EP0_DATA_IN;
			}
			else
			{
				my_memcpy(pCtrl, (void *)USB_ConfigDescriptor, gUsbCtrl.ReqLen);
				USBD->MXPLD0 = gUsbCtrl.ReqLen;
				gUsbCtrl.StateFlag = FLAG_OUT_ACK;
			}
			break;
		}
		/* Get String Descriptor */
		case DESC_STRING:
		{
			/* Get Language */
			if(gUsbCtrl.UsbSetupBuf[2] == 0)
			{
				gUsbCtrl.ReqLen = Minimum (gUsbCtrl.ReqLen, USB_StringLang[0]);
				my_memcpy(pCtrl, (void *)USB_StringLang, gUsbCtrl.ReqLen);
				USBD->MXPLD0 = gUsbCtrl.ReqLen;
				gUsbCtrl.StateFlag = FLAG_OUT_ACK;
				break;
			}
			/* Get Vendor String Descriptor */
			else if (gUsbCtrl.UsbSetupBuf[2] == 1)
			{
				gUsbCtrl.ReqLen = Minimum (gUsbCtrl.ReqLen, USB_VendorStringDesc[0]);
				if (gUsbCtrl.ReqLen >= CtrlMaxPacketSize)
				{
					my_memcpy(pCtrl, (void *)USB_VendorStringDesc, CtrlMaxPacketSize);
					gUsbCtrl.pDesc = (uint8_t *)&USB_VendorStringDesc[CtrlMaxPacketSize];
					gUsbCtrl.EP0_Toggle = 1;
					USBD->MXPLD0 = CtrlMaxPacketSize;
					gUsbCtrl.ReqLen -= CtrlMaxPacketSize;
					gUsbCtrl.StateFlag = FLAG_EP0_DATA_IN;
				}
				else
				{
					my_memcpy(pCtrl, (void *)USB_VendorStringDesc, gUsbCtrl.ReqLen);
					USBD->MXPLD0 = gUsbCtrl.ReqLen;
					gUsbCtrl.StateFlag = FLAG_OUT_ACK;
				}
				break;
			}
			/* Get Product String Descriptor */
			else if (gUsbCtrl.UsbSetupBuf[2] == 2)
			{
				gUsbCtrl.ReqLen = Minimum (gUsbCtrl.ReqLen, USB_ProductStringDesc[0]);
				if (gUsbCtrl.ReqLen >= CtrlMaxPacketSize)
				{
					my_memcpy(pCtrl, (void *)USB_ProductStringDesc, CtrlMaxPacketSize);
					gUsbCtrl.pDesc = (uint8_t *)&USB_ProductStringDesc[CtrlMaxPacketSize];
					gUsbCtrl.EP0_Toggle = 1;
					USBD->MXPLD0 = CtrlMaxPacketSize;
					gUsbCtrl.ReqLen -= CtrlMaxPacketSize;
					gUsbCtrl.StateFlag = FLAG_EP0_DATA_IN;
				}
				else
				{
					my_memcpy(pCtrl, (void *)USB_ProductStringDesc, gUsbCtrl.ReqLen);
					USBD->MXPLD0 = gUsbCtrl.ReqLen;
					gUsbCtrl.StateFlag = FLAG_OUT_ACK;
				}
				break;
			}
		}
		/* Get HID Report Descriptor */
		case DESC_HID_RPT:
		{
		}
		default:
		    /* Setup error, stall the device */
			USBD->CFG0 |= (USB_CFG_SSTALL | 0x8000);
			USBD->CFG1 |= (USB_CFG_SSTALL | 0x8000);
			return FALSE;
	}
	return TRUE;
}

/**   
  * @brief  USB_ClassRequest, Process USB Class Request.
  * @param  None.
  * @retval 1: Standard Request.
  *         0: has some error.
  */
uint32_t USB_ClassRequest(void)
{
	switch(gUsbCtrl.UsbSetupBuf[1])
	{
		case SET_LINE_CODE:
		{
			/* Ready to get next Ctrl out */
			USBD->MXPLD1 = 7;
			USBD->CFG1 |= USB_CFG_DSQ_SYNC_DATA1;
			gUsbCtrl.StateFlag = FLAG_SET_LINE_CODE;
			return TRUE;
		}

		case GET_LINE_CODE:
		{
			my_memcpy((uint8_t *)((uint32_t)USBD_SRAM_BASE + (uint32_t)USBD->BUFSEG0), (void *)&gCdcInfo, 7);
			USBD->CFG0 |= USB_CFG_DSQ_SYNC_DATA1;
			USBD->MXPLD0 = 7;
			gUsbCtrl.StateFlag = FLAG_OUT_ACK;
			return TRUE;
		}

		case SET_CONTROL_LINE_STATE:
		{
			gVcomInfo.CtrlSignal = gUsbCtrl.UsbSetupBuf[3];
			gVcomInfo.CtrlSignal = (gVcomInfo.CtrlSignal << 8) | gUsbCtrl.UsbSetupBuf[2];

			USBD->CFG0 |= USB_CFG_DSQ_SYNC_DATA1;
			USBD->MXPLD0 = 0;
			return TRUE;
		}

		default:
		    /* Setup error, stall the device */
			USBD->CFG0 |= (USB_CFG_SSTALL | 0x8000);
			USBD->CFG1 |= (USB_CFG_SSTALL | 0x8000);
	}
	return FALSE;
}

/**   
  * @brief  USB_EpAck0, Ack Transfer Pipe.
  * @param  None.
  * @retval None.
  */
uint32_t USB_EpAck0(void)
{
	/* Bulk IN */
	gVcomInfo.TxSize = 0;
	return 0;
}

/**   
  * @brief  USB_EpAck1, Ack Transfer Pipe.
  * @param  None.
  * @retval None.
  */
uint32_t USB_EpAck1(void)
{
	/* Bulk OUT */
	gVcomInfo.RxSize = USBD->MXPLD3;
	gVcomInfo.RxBuf = (uint8_t *)((uint32_t)USBD_SRAM_BASE + (uint32_t)USBD->BUFSEG3);

	/* Set a flag to indicate builk out ready */
	gVcomInfo.BulkOutReady = 1;
	return 0;
}


/**   
  * @brief  USB_EpAck2, Ack Transfer Pipe.
  * @param  None.
  * @retval None.
  */
uint32_t USB_EpAck2(void)
{
	/* Interrupt IN */
	uint8_t au8Buf[8] = {0};
	my_memcpy((uint8_t *)((uint32_t)USBD_SRAM_BASE + (uint32_t)USBD->BUFSEG4), au8Buf, MAX_PACKET_SIZE_INT);
	USBD->MXPLD4 = MAX_PACKET_SIZE_INT;
	return 0;
}


/**   
  * @brief  USB_EpAck3, Ack Transfer Pipe.
  * @param  None.
  * @retval None.
  */
uint32_t USB_EpAck3(void)
{
	return 0;
}


/**   
  * @brief  USB_EpAck4, Ack Transfer Pipe.
  * @param  None.
  * @retval None.
  */
uint32_t USB_EpAck4(void)
{
	return 0;
}


/**   
  * @brief  USB_EpAck5, Ack Transfer Pipe.
  * @param  None.
  * @retval None.
  */
uint32_t USB_EpAck5(void)
{
	return 0;
}


/**   
  * @brief  USB_DeviceStart, Start USB Device Transfer.
  * @param  None.
  * @retval None.
  */
void USB_DeviceStart(void)
{
	USBD->MXPLD3 = MAX_PACKET_SIZE_BULK;
	USBD->MXPLD2 = 0;
}

/*********************************************************/
/**   
  * @brief  VCOM_MainProcess, VCOM main process.
  * @param  None.
  * @retval None.
  */
void VCOM_MainProcess(void)
{
	int32_t i, i32Len;
	STR_USBD_T sParam;

	/* Enable UART Interrupt */
	UART_EnableInt(UART0, DRVUART_RDAINT);

	/* Initial USB */
	USBD_Init();

	sParam.UsbDeviceClass = USB_CLASS_CDC;
	sParam.u32CtrlMaxPacketSize = MAX_PACKET_SIZE_CTRL;
	sParam.u32EndpointCount = 3;
	sParam.EP[0].u32EpNum = BULK_IN_EP_NUM;
	sParam.EP[0].u32MaxPacketSize = MAX_PACKET_SIZE_BULK;
	sParam.EP[0].IsDirIN = TRUE;
	sParam.EP[0].IsIsoTransfer = FALSE;
	sParam.EP[1].u32EpNum = BULK_OUT_EP_NUM;
	sParam.EP[1].u32MaxPacketSize = MAX_PACKET_SIZE_BULK;
	sParam.EP[1].IsDirIN = FALSE;
	sParam.EP[1].IsIsoTransfer = FALSE;
	sParam.EP[2].u32EpNum = INT_IN_EP_NUM;
	sParam.EP[2].u32MaxPacketSize = MAX_PACKET_SIZE_INT;
	sParam.EP[2].IsDirIN = TRUE;
	sParam.EP[2].IsIsoTransfer = FALSE;
	USB_Open(&sParam);

	memset((void *)&gVcomInfo, 0, sizeof(STR_USBD_VCOM_T));

	while(1)
	{
		/* Check if any data to send to USB & USB is ready to send them out */
		if(comRbytes && (gVcomInfo.TxSize == 0))
		{
			i32Len = comRbytes;
			if(i32Len > MAX_PACKET_SIZE_BULK)
				i32Len = MAX_PACKET_SIZE_BULK;

			for(i=0;i<i32Len;i++)
			{
				gRxBuf[i] = comRbuf[comRhead++];
				if(comRhead >= RXBUFSIZE)
					comRhead = 0;
			}

			NVIC_DisableIRQ(UART0_IRQn);
			comRbytes -= i32Len;
			NVIC_EnableIRQ(UART0_IRQn);

			gVcomInfo.TxSize = i32Len;
			my_memcpy((uint8_t *)((uint32_t)USBD_SRAM_BASE + (uint32_t)USBD->BUFSEG2), (void *)gRxBuf, i32Len);
			USBD->MXPLD2 = i32Len;
		}


		/* Process the Bulk out data when bulk out data is ready. */
		if(gVcomInfo.BulkOutReady && (gVcomInfo.RxSize <= TXBUFSIZE - comTbytes))
		{
			for(i=0;i<gVcomInfo.RxSize;i++)
			{
				comTbuf[comTtail++] = *(gVcomInfo.RxBuf+i);
				if(comTtail >= TXBUFSIZE)
					comTtail = 0;
			}

			NVIC_DisableIRQ(UART0_IRQn);
			comTbytes += gVcomInfo.RxSize;
			NVIC_EnableIRQ(UART0_IRQn);

			gVcomInfo.RxSize = 0;
			gVcomInfo.BulkOutReady = 0; /* Clear bulk out ready flag */

			/* Ready to get next BULK out */
			USBD->MXPLD3 = MAX_PACKET_SIZE_BULK;
		}

		/* Process the software Tx FIFO */
		if(comTbytes)
		{
			/* Check if Tx is working */
			if (!(UART0->IER & UART_IER_THRE_IE))
			{
				/* Send one bytes out */
				UART0->THR = comTbuf[comThead++];
				if(comThead >= TXBUFSIZE)
					comThead = 0;

				NVIC_DisableIRQ(UART0_IRQn);
				comTbytes--;
				NVIC_EnableIRQ(UART0_IRQn);

				/* Enable Tx Empty Interrupt. (Trigger first one) */
				UART0->IER |= UART_IER_THRE_IE;
			}
		}
	}
}

/*** (C) COPYRIGHT 2012 Nuvoton Technology Corp. ***/




