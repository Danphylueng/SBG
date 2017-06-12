#ifndef __USART2_H
#define	__USART2_H

#include "stm32f10x.h"
#include <stdio.h>

#define USART2_DR_Base  0x40004404		// 0x40013800 + 0x04 = 0x40013804

#define DMA_RECEIVEBUFF_SIZE   255
#define DMA_SENDBUFF_SIZE 255

#define USART2_DMA_CHAN_TX DMA1_Channel7
#define USART2_DMA_CHAN_RX DMA1_Channel6

void USART2_Config(void);
void USART2_tx_DMA_Config(u16 len);
void USART2_rx_DMA_Config(void);

/**
 * \brief    : this fuction use to move data from DMA_buffer to receive_buffer,in this project we have to increase p afer this fuction
 * \detail   :
 *
* \param[in]	:			len					: the max space in the receive_buffer to save data
* \param[out]	: 			*p 				:where to save the data .

 *
 * \retval :  how many datas we has move;
 */

u16 read_from_DMAbuffer( u8 *p, u32 len);

/**
 * \brief    : this fuction is used to send data,
 * \detail   : this fuction is used to send data,config usart dma transmit. once transmit complete. set the gobal param to 1.
 *

* \param[in]	:			*buffer					: a pointer point to the data that we want to send
* \param[in]	:			len							: buffer length.


 *
* \retval :  0:dma is busy 
*						 1:success
*						-1:dma buffer is too samll to hold the data
 */
char uart2_send_data(u8 *buffer, u16 len);

#endif /* __USART2_H */
