#ifndef __SBG_SEEK_FRAME_H
#define __SBG_SEEK_FRAME_H

/**
 * \file       SBG_seek_frame.h
 * \brief      SBG_seek_frame.c head file
 *
 * \version    1.0
 * \date       2017-5-24
 * \author     Danphy
 */
 
 #define RECEIVE_BUFFER_LEN 1024
 #define SYNC1 0xFF
 #define SYNC2 0x5A
 #define ETX 0x33

#include "stm32f10x.h"
#include <string.h>

/**
 * \brief    the structure indicate the frame msg,class,len data, except crc ETC SYN1 SYN2.
 *
 */
typedef struct _sbgdata{
	u8 sbg_msg; 									/*!< sbg frame message */
	u8 sbg_class;									/*!< sbg frame class*/
	u16 sbg_len;									/*!< sbg frame length*/
	u8 sbg_data[0xFF];						/*!< sbg frame data section*/
} sbg_data;	


//#define __DEBUG__

#ifdef __DEBUG__
#define DE_PRINTF(format,...) printf("File:"__FILE__",Line: %05d: "format"\n",__LINE__,##__VA_ARGS__)
#else
#define DE_PRINTF(format,...)
#endif


/**
 * \brief    :to judge the frame have receive or not 
 *
 * \retval : 0:there is no frame at all
 * \retval : 1:there is have a frame in receive buffer 
 */
u8 receive_a_frame(u16 *frame_length);

/**
 * \brief    :use to move SYN1 and SYN2 to receive buffer;
 * \detail   :use to find the frame head ,and move the frame head to the buffer.When we change the pointer p , we have to disable the USART2_IT_RXNE. in case of sonethinf happen that we do not expected
 *
 * \retval 	: 1: we have find the head and move it
 * \retval 	: 0: we could not find the head 
 */
static u8 seek_frame_head(void);

/**
 * \brief    :move the frame to the sbg_data structure,and replace the frame.
 * \detail   :receive the frame put it into sbg_frame structure, disable the USART2 interrupt and try to repalce the _g_buffer.
 *
 * \param[out]:sbg_data structure is used to hold a compelte data.
 *
 */
 void move_frame_from_buffer(sbg_data *sbg_frame);
 
 /**
 * \brief    :create the frame
 * \detail   :create the frame and return the length in the buffer
 *
 * \param[out]: *buffer : use to save a frame 
 * \param[in]:		sbg_senddata   : the sbg_data structure that we want to send data.
 * \param[in\out]:  *len : deliever the max length frame in the buffer
														and return the frame length 
* \retval     :    1:create the frame success
									 0:create the frame failure maybe the frame length is greater than the len.
 */
u8 create_a_frame(u8 *buffer, sbg_data sbg_senddata, u16 *buffer_len);

#endif
