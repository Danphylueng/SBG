/**
  ******************************************************************************
  * @file    bsp_usart1.c
  * @author  fire
  * @version V1.0
  * @date    2013-xx-xx
  * @brief   usart应用bsp(DMA 发送)
  ******************************************************************************
  * @attention
  *
  * 实验平台:野火 iSO STM32 开发板 
  * 论坛    :http://www.chuxue123.com
  * 淘宝    :http://firestm32.taobao.com
  *
  ******************************************************************************
  */ 
#include "bsp_usart2.h"
#include "SBG_seek_frame.h"

static uint8_t g_DMA_rx_buffer[DMA_RECEIVEBUFF_SIZE]; /*!< use to hold the receive data throngh DMA */
static uint8_t g_DMA_tx_buffer[DMA_SENDBUFF_SIZE];		/*!< use to hold the data that use to send    */
__IO u8 DMA_rx_page = 0;															/*!< increase 1 when DMA receive transmit complete */





/// 配置USART1接收中断
void NVIC_Configuration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure; 
	/* Configure the NVIC Preemption Priority Bits */  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
	
	/* Enable the USARTy Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel6_IRQn;	 
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}
/**
  * @brief  USART1 GPIO 配置,工作模式配置。115200 8-N-1
  * @param  无
  * @retval 无
  */
void USART2_Config(void)
{
		GPIO_InitTypeDef GPIO_InitStructure;
		USART_InitTypeDef USART_InitStructure;
		
		/* config USART1 clock */
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
		/* USART1 GPIO config */
		/* Configure USART1 Tx (PA.02) as alternate function push-pull */
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOA, &GPIO_InitStructure);    
		/* Configure USART1 Rx (PA.03) as input floating */
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
		GPIO_Init(GPIOA, &GPIO_InitStructure);
			
		/* USART1 mode config */
		USART_InitStructure.USART_BaudRate = 921600;
		USART_InitStructure.USART_WordLength = USART_WordLength_8b;
		USART_InitStructure.USART_StopBits = USART_StopBits_1;
		USART_InitStructure.USART_Parity = USART_Parity_No ;
		USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
		USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
		USART_Init(USART2, &USART_InitStructure); 	
		//USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);

		USART_DMACmd(USART2, USART_DMAReq_Rx|USART_DMAReq_Tx, ENABLE);
		USART2_rx_DMA_Config();
		USART2_tx_DMA_Config();
		NVIC_Configuration();
		USART_Cmd(USART2, ENABLE);
}


/**
  * @brief  
  * @param  无
  * @retval 无
  */
void USART2_rx_DMA_Config(void)
{
		DMA_InitTypeDef DMA_InitStructure;
		DMA_DeInit(USART2_DMA_CHAN_RX);  
	
		/*开启DMA时钟*/
		RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

		/*设置DMA源：串口数据寄存器地址*/
		DMA_InitStructure.DMA_PeripheralBaseAddr = USART2_DR_Base;	   

		/*内存地址(要传输的变量的指针)*/
		DMA_InitStructure.DMA_MemoryBaseAddr = (u32)g_DMA_rx_buffer;
	
		DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC ;	

		/*传输大小DMA_BufferSize=SENDBUFF_SIZE*/	
		DMA_InitStructure.DMA_BufferSize = DMA_RECEIVEBUFF_SIZE;

		/*外设地址不增*/	    
		DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable; 

		/*内存地址自增*/
		DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;	

		/*外设数据单位*/	
		DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;

		/*内存数据单位 8bit*/
		DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;	 

		/*DMA模式：不断循环*/
		DMA_InitStructure.DMA_Mode = DMA_Mode_Circular ;
		//qDMA_InitStructure.DMA_Mode = DMA_Mode_Circular;	 

		/*优先级：中*/	
		DMA_InitStructure.DMA_Priority = DMA_Priority_Medium    ;  

		/*禁止内存到内存的传输	*/
		DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;

		/*配置DMA1的4通道*/		   
		DMA_Init(USART2_DMA_CHAN_RX, &DMA_InitStructure); 	   
		
		/*使能DMA*/
		DMA_Cmd (USART2_DMA_CHAN_RX, ENABLE);					
		DMA_ITConfig(USART2_DMA_CHAN_RX, DMA_IT_TC, ENABLE);  //配置DMA发送完成后产生中断
}


/**
  * @brief  
  * @param  无
  * @retval 无
  */
void USART2_tx_DMA_Config(void)
{
		DMA_InitTypeDef DMA_InitStructure;
		DMA_DeInit(USART2_DMA_CHAN_TX);      
	
		/*开启DMA时钟*/
		RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

		/*设置DMA源：串口数据寄存器地址*/
		DMA_InitStructure.DMA_PeripheralBaseAddr = USART2_DR_Base;	   

		/*内存地址(要传输的变量的指针)*/
		DMA_InitStructure.DMA_MemoryBaseAddr = (u32)g_DMA_tx_buffer;
	
		DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST ;	

		/*传输大小DMA_BufferSize=SENDBUFF_SIZE*/	
		DMA_InitStructure.DMA_BufferSize = 1;

		/*外设地址不增*/	    
		DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable; 

		/*内存地址自增*/
		DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;	

		/*外设数据单位*/	
		DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;

		/*内存数据单位 8bit*/
		DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;	 

		/*DMA模式：不断循环*/
		DMA_InitStructure.DMA_Mode = DMA_Mode_Normal ;
		//qDMA_InitStructure.DMA_Mode = DMA_Mode_Circular;	 

		/*优先级：中*/	
		DMA_InitStructure.DMA_Priority = DMA_Priority_Medium ;  

		/*禁止内存到内存的传输	*/
		DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;

		/*配置DMA1的4通道*/		   
		DMA_Init(USART2_DMA_CHAN_TX, &DMA_InitStructure); 	   
		
		/*使能DMA*/
		DMA_Cmd (USART2_DMA_CHAN_TX, ENABLE);					
		//DMA_ITConfig(USART2_DMA_CHAN_TX, DMA_IT_TC, ENABLE);  //配置DMA发送完成后产生中断
}

/**
 * \brief    : this fuction use to move data from DMA_buffer to receive_buffer,in this project we have to increase p afer this fuction
 * \detail   :
 * 
* \param[in]	:			len					: the max space in the receive_buffer to save data
* \param[out]	: 			*p 				:where to save the data .
 *
 * \retval :  how many datas we has move;
 */

u16 read_from_DMAbuffer(u8 *p, u32 len)
{
	
	static u8 old_page = 0; 													/* old page we use it to figure out whether the page has change or not */
	static u16 read_offset = 0; 											/* offset from DMA_buffer use to read from DMA_buffer */
	u16 num;
	
	
	DE_PRINTF("old = %d new = %d", old_page, DMA_rx_page);
	if(DMA_rx_page == old_page ){												 /* this case is in the same section */
	
		num = DMA_GetCurrDataCounter(USART2_DMA_CHAN_RX) + read_offset;
		
		if(num > DMA_RECEIVEBUFF_SIZE){																/* the page change num > DMA_RECEIVEBUFF_SIZE */
			
			u16 retval = 0;
			retval = DMA_RECEIVEBUFF_SIZE - read_offset;
			memcpy(p,  g_DMA_rx_buffer + read_offset, retval);
			read_offset = 0;
			old_page = DMA_rx_page;
			return retval;			
		} else {
			
			num = DMA_RECEIVEBUFF_SIZE - num ;
		}
		if (num > len) {																	/*this case is the data we can receive bigger than len*/
			
			memcpy(p, g_DMA_rx_buffer + read_offset, len);
			read_offset += len;
			
			DE_PRINTF("new == old num > len return len = %d \n", len);
			return len;
		} else {																					/* this case is we can receive all the data we can get */
			
			memcpy(p, g_DMA_rx_buffer + read_offset, num);
			read_offset += num;
			return num;
		}
	} else if ( (u8)(old_page + 1) == DMA_rx_page) {						/*case is large than one page, in some case , we can still read the data.*/
		
		if (read_offset + DMA_GetCurrDataCounter(USART2_DMA_CHAN_RX) < DMA_RECEIVEBUFF_SIZE - 1) { /* this case is ,data is not value */
			
			read_offset = 0;
			
			num = DMA_RECEIVEBUFF_SIZE - DMA_GetCurrDataCounter(USART2_DMA_CHAN_RX) - read_offset;					/* calculate how many data we can read in g_DMA_rx_buffer */
		
			if (num > len) {																	/*this case is the data we can receive bigger than len*/
			
				memcpy(p, g_DMA_rx_buffer + read_offset, len);
				read_offset += len;
				old_page = DMA_rx_page;														/* updata the page */
				DE_PRINTF("the data has been coverted, return len = %d \n", len);
				return len;
			} else {																					/* this case is we can receive all the data we can get */
			
				memcpy(p, g_DMA_rx_buffer + read_offset, num);
				read_offset += num;
				old_page = DMA_rx_page;														/* updata the page */
				DE_PRINTF("the data has been coverted, return num = %d \n", num);
				return num;
			}
			
		} else {																						/* this case is page has change ,but the data haven`t coverted */
			
			int num1, num2;																		/* num1 use to count how many in the end, num2 use to count how many in the top */
			num1 = DMA_RECEIVEBUFF_SIZE - read_offset;
			num2 = DMA_RECEIVEBUFF_SIZE - DMA_GetCurrDataCounter(USART2_DMA_CHAN_RX);
			
			if (len >= num1 +num2) {													/* have enough space to store the data */
				
				memcpy(p, g_DMA_rx_buffer + read_offset, num1);
				memcpy(p + num1, g_DMA_rx_buffer, num2);
				read_offset = num2;
				old_page = DMA_rx_page;														/* updata the page */
				DE_PRINTF("the data hasn't been coverted, return num1 + num2 = %d \n", num1 +num2);
				return num1 + num2;
				
			} else if(len >= num1 && len < num2 +num1){				/* don't have enough space but can move read_offet to the head of the g_DMA_rx_buffer */
				
				memcpy(p, g_DMA_rx_buffer + read_offset, num1);
				memcpy(p + num1, g_DMA_rx_buffer, len - num1);
				read_offset = len - num1;
				old_page = DMA_rx_page;														/* updata the page */
				DE_PRINTF("the data hasn't been coverted, but cross page,return len = %d \n", len);
				return len;
				
			} else{																						/* don't have enough space ,can't move read_offset to the head */
				
				memcpy(p, g_DMA_rx_buffer + read_offset, len);
				read_offset += len;
				old_page = DMA_rx_page;														/* updata the page */	
				DE_PRINTF("the data hasn't been coverted, return len = %d \n", len);				
				return len ;
			}
		}
	} else {																							/* page have change more than one page */
		
		read_offset = 0;
		
		num = DMA_RECEIVEBUFF_SIZE - DMA_GetCurrDataCounter(USART2_DMA_CHAN_RX) - read_offset;					/* calculate how many data we can read in g_DMA_rx_buffer */
		
		if (num > len) {																	/*this case is the data we can receive bigger than len*/
			
			memcpy(p, g_DMA_rx_buffer + read_offset, len);
			read_offset += len;
			old_page = DMA_rx_page;														/* updata the page */
			DE_PRINTF("new - old > 1, return len = %d \n", len);
			return len;
		} else {																					/* this case is we can receive all the data we can get */
			
			memcpy(p, g_DMA_rx_buffer + read_offset, num);
			read_offset += num;
			old_page = DMA_rx_page;														/* updata the page */
			DE_PRINTF("new - old > 1, , return num = %d \n", num);
			return num;
		}
		
	}
	
}

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
char uart2_send_data(u8 *buffer, u16 len)         
{
	
	if (DMA_GetFlagStatus(DMA1_FLAG_TC7) != RESET) { 																	/* dma is busy, return 0 */
		
		DMA_ClearFlag(DMA1_FLAG_TC7);																											/* clear dma transmit complete flag */ 
		if (len < DMA_SENDBUFF_SIZE) {
			
			memcpy(g_DMA_tx_buffer, buffer, len);										/* copy the send data to the g_DMA_tx_buffer */
			DMA_Cmd (USART2_DMA_CHAN_TX, DISABLE);	
			DMA_SetCurrDataCounter(USART2_DMA_CHAN_TX, len);
			DMA_Cmd (USART2_DMA_CHAN_TX, ENABLE);	
			
			return 1;
		} else {
			
			return (char)-1;
		}
	} else {
		return 0;
	}
	

}

/*********************************************END OF FILE**********************/
