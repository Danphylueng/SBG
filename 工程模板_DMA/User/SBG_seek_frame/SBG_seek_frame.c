/**
 * \file       SBG_seek_frame.c
 * \brief      this file is used to find a complete frame.
 *
 * \version    1.0
 * \date       2017-5-24
 * \author     Danphy
 */
 
 
 
#include "SBG_seek_frame.h"
#include "sbgCrc.h"


u8 _g_receive_buffer[RECEIVE_BUFFER_LEN];								/*!< a buffer to receive the data */ 
u8 *p = _g_receive_buffer;															/*!< a ponter point to the buffer to save data */ 			
	

/**
 * \brief    :to judge the frame have receive or not 
 * \param[out]: use to hold the frame length
 * \retval : 0:there is no frame at all
 * \retval : 1:there is have a frame in receive buffer 
 */
u8 receive_a_frame(u16 *frame_length)
{
	if(seek_frame_head()){				/* we have receive a frame head */
		
		if (p - _g_receive_buffer > 8) {/* the smalllest frame is 8 */
			
			*frame_length = *(_g_receive_buffer + 4) | *(_g_receive_buffer + 5)<<8; /* to find tht frame_length */
			*frame_length += 9;     																					/* add frame head and frame tail */
			
			if ((p - _g_receive_buffer) >= (*frame_length)) {
													/* there is not complete data */
				
				if (_g_receive_buffer[*frame_length - 1] == ETX && 
						sbgCrc16Compute(_g_receive_buffer + 2, *frame_length - 5) == 
													(u16)(_g_receive_buffer[*frame_length - 2] << 8 | _g_receive_buffer[*frame_length - 3]) ){ /* ETX and crc corret */
							
					return 1;
				} else {																																							/* have find a wrong data head */
					
										
					memmove(_g_receive_buffer, _g_receive_buffer+2, p - _g_receive_buffer - 2);
					p -= 2;																					/* change the p pointer */					
					*frame_length = 0;  
					return 0;
				}

			} else {
				if (*frame_length > 0xFF) {
				
					memmove(_g_receive_buffer, _g_receive_buffer+2, p - _g_receive_buffer - 2);
					p -= 2;																					/* change the p pointer */

					*frame_length = 0;
				}
				return 0;
			}
			
		} else {										/* there is less than 8 bytes in buffer */
			
			return 0;
		}
		
	} else {											/* do not have frame head in the receive buffer */
		
		return 0;
		
	}
	return 0;
}


/**
 * \brief    :use to move SYN1 and SYN2 to receive buffer;
 * \detail   :use to find the frame head ,and move the frame head to the buffer head.When we change the pointer p , we have to disable the USART2_IT_RXNE. in case of sonethinf happen that we do not expected
 *						this fuction operation on the goble varity
 * \retval 	: 1: we have find the head and move it
 * \retval 	: 0: we could not find the head 
 */
static u8 seek_frame_head()
{
	u8 *p_head;																							/* a temp pointer to find head */
	
	for (p_head = _g_receive_buffer; p_head < p - 2; p_head++) {
		
		if(*p_head == SYNC1 && *(p_head +1) == SYNC2) {
			break;
		}		
	}
	if (p_head == _g_receive_buffer) { /* there is two condiction  one: no data two :head alread in the top */
		
		if (p - _g_receive_buffer > 1) {	/* this case is alread in the head of the buffer */
			
			return 1;
			
		} else {													/* this case is no data in the buffer */
		
			return 0;
		}
	}
	
	if (p_head == p - 2) {			/* no frame head in receive buffer */
		//USART_ITConfig(USART2, USART_IT_RXNE, DISABLE); /* in case of the conflict about p */	
		memmove(_g_receive_buffer, p_head, p - p_head);
		p -= (p_head - _g_receive_buffer);									/* change the p pointer */
		//USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
		
		return 0;
	}
	//USART_ITConfig(USART2, USART_IT_RXNE, DISABLE); /* in case of the conflict about p */	
	memmove(_g_receive_buffer, p_head, p - p_head);
	p -= (p_head - _g_receive_buffer);									/* change the p pointer */
	//USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
	return 1;
}


/**
 * \brief    :move the frame to the sbg_data structure,and replace the frame.
 * \detail   :receive the frame put it into sbg_frame structure, disable the USART2 interrupt and try to repalce the _g_buffer.
 *
 * \param[out]:sbg_data structure is used to hold a compelte data.
 *
 */
 void move_frame_from_buffer(sbg_data *sbg_frame)
{
	
	/* to fill the structure */
	memset(sbg_frame, 0, sizeof(sbg_data));
	sbg_frame->sbg_msg 	 = *(_g_receive_buffer + 2);
	sbg_frame->sbg_class = *(_g_receive_buffer + 3);
	sbg_frame->sbg_len	 = *(_g_receive_buffer + 4);
	sbg_frame->sbg_len	|= *(_g_receive_buffer + 5) << 8;
	
	memcpy(sbg_frame->sbg_data, _g_receive_buffer + 6, sbg_frame->sbg_len); /* fill the structure data section */
	
	//USART_ITConfig(USART2, USART_IT_RXNE, DISABLE); /* in case of the conflict about p */	
	DE_PRINTF("start to memmove, receive %d datas \n", sbg_frame->sbg_len + 9);
	
	memmove(_g_receive_buffer, _g_receive_buffer + sbg_frame->sbg_len + 9, 
					p - _g_receive_buffer - sbg_frame->sbg_len - 9);									/* move other frame to the head of the buffer */
	
	DE_PRINTF("memmove %d to %d ,totally %d \n", (u32)_g_receive_buffer,
						(u32)_g_receive_buffer + sbg_frame->sbg_len + 9, p - _g_receive_buffer - sbg_frame->sbg_len - 9);
	DE_PRINTF("before p decrease p = %d\n", (u32)p);
	p -= sbg_frame->sbg_len + 9;									/* change the p pointer */
	DE_PRINTF("p decrease  p=%d \n", (u32)p);
	
	//USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
	
}


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
u8 create_a_frame(u8 *buffer, sbg_data sbg_senddata, u16 *buffer_len)    
{
	u16 crc; 																						/* calculate crc */
	if (*buffer_len > sbg_senddata.sbg_len + 9) {												/* there is enough place to save a frame */
		
		*buffer 	= SYNC1;																                  /* fill the frame head */
		buffer[1] = SYNC2;
		buffer[2] = sbg_senddata.sbg_msg;
		buffer[3] = sbg_senddata.sbg_class;
		buffer[4] = sbg_senddata.sbg_len & 0xFF;
		buffer[5] = sbg_senddata.sbg_len >> 8;
		
		if (sbg_senddata.sbg_len) {																				  /* length greater than zero */
			
			memcpy(buffer + 6, sbg_senddata.sbg_data, sbg_senddata.sbg_len);
			
		}
		
		crc = sbgCrc16Compute(buffer + 2, sbg_senddata.sbg_len + 4);
		
		buffer[6 + sbg_senddata.sbg_len] = crc & 0xFF;											/* fill the crc */
		buffer[7 + sbg_senddata.sbg_len] = crc >> 8;
		
		buffer[8 + sbg_senddata.sbg_len] = ETX;													  	/* fill the ETX */
		*buffer_len = sbg_senddata.sbg_len + 9;														  /* change the frame length */
		
		return 1;																					/* return 1 to sign create frame success */
		
		
	} else {																						/* there is no enough place to save a frame */
		
		return 0;
	}
}
