/**
  ******************************************************************************
  * @file    main.c
  * @author  fire
  * @version V1.0
  * @date    2013-xx-xx
  * @brief   用3.5.0版本库建的工程模板
  ******************************************************************************
  * @attention
  *
  * 实验平台:野火 iSO STM32 开发板 
  * 论坛    :http://www.chuxue123.com
  * 淘宝    :http://firestm32.taobao.com
  *
  ******************************************************************************
  */
  
#include "stm32f10x.h"
#include "bsp_usart1.h"
#include "bsp_usart2.h"
#include "SBG_seek_frame.h"

extern u8 _g_receive_buffer[RECEIVE_BUFFER_LEN];								/* a buffer to receive the data */
extern u8 *p;																										/* a ponter point to the buffer to save data */
extern u32 DMA_page;
extern uint8_t _g_DMA_buffer[DMA_RECEIVEBUFF_SIZE];

/**
  * @brief  主函数
  * @param  无
  * @retval 无
  */
int main(void)
{
	u16 frame_length = 0;
	u32 i;
	u16 length;

	u32 timestick = 0;																							/* use to hold the timestick from frame */
	u32 timestick_0x03, timestick_0x06, timestick_0x24, timestick_0x08;
	u8 buffer[255];
	
	sbg_data sbg_frame_data;									/* use to hold a frame except ETX SYN1 SYN2 CRC */
#ifdef __DEBUG__
	u16 recenum = 0;
#endif
	
	USART1_Config();
	USART2_Config();
	printf("test \n");
	
	sbg_frame_data.sbg_msg = 0x01;
	sbg_frame_data.sbg_class = 0x02;
	sbg_frame_data.sbg_len = 0x0A;
	for (i = 0; i<0x0A; i++) {
		
		sbg_frame_data.sbg_data[i] = i;
	}
	length = 0xff;
	if (create_a_frame(buffer, sbg_frame_data, &length)) 
	{
		printf("create a frame success \r\n");
	}
	for (i = 0; i < 100; i++) {
		while(uart2_send_data(buffer,length) != 1);
	}

	
	
	
	i = 0;
	
	while (1) {
		
		/**********************************
		 *move the data from DMAbuffer to receivebuffer.
		 ***********************************/
#ifdef __DEBUG__
		recenum = read_from_DMAbuffer(p, _g_receive_buffer + RECEIVE_BUFFER_LEN - p);
		if(recenum > 800){
			DE_PRINTF("receive num more than 800\n");
		}
		p += recenum;
#else
		p += read_from_DMAbuffer(p, _g_receive_buffer + RECEIVE_BUFFER_LEN - p);  
#endif

		do {
				if (receive_a_frame(&frame_length)) {																							/* judge there is a frame in the buffer or not */
					
					i++;
					
//				printf("receive a frame frame length %d \n", frame_length);
		
					move_frame_from_buffer(&sbg_frame_data);																						/* remove the data to the structure , */
//				printf("SBG msg is %x, class is %x, len is %x,data section list as below :\n", sbg_frame_data.sbg_msg, sbg_frame_data.sbg_class, sbg_frame_data.sbg_len);
//				for (i = 0; i < sbg_frame_data.sbg_len; i++) {
//					printf("%x ", sbg_frame_data.sbg_data[i]);
//				}
//				printf("\n");
					
					timestick = 0;
					timestick |= sbg_frame_data.sbg_data[3] << 24;
					timestick |= sbg_frame_data.sbg_data[2] << 16;
					timestick |= sbg_frame_data.sbg_data[1] << 8;
					timestick |= sbg_frame_data.sbg_data[0];
					
					switch (sbg_frame_data.sbg_msg) {
					
					case 0x03:																																	/* the time interval is 10000 us when the msg == 0x03 */
						if (timestick_0x03 + 10000 != timestick && timestick_0x03 != 0) {					/* judge the time is corret or not expect timestick_0x03 ==0 */									
							printf("03lost\n");
						}
						timestick_0x03 = timestick;
						break;
					
					case 0x06: 																																	/* the time interval is 10000 us when the msg == 0x06 */
						if (timestick_0x06 + 10000 != timestick && timestick_0x06 != 0) {					/* judge the time is corret or not expect timestick_0x06 ==0 */	
							printf("06lost\n");					
						}
						timestick_0x06 = timestick;
						break;
					
					case 0x24: 																																	/* the time interval is 20000 us when the msg == 0x24 */
						if (timestick_0x24 + 20000 != timestick && timestick_0x24 != 0) {					/* judge the time is corret or not expect timestick_0x24 ==0 */	
							printf("24lost\n");
						}
						timestick_0x24 = timestick;						
						break;
					
					case 0x08: 																																	/* the time interval is 40000 us when the msg == 0x08 */
						if (timestick_0x08 + 40000 != timestick && timestick_0x08 != 0) {					/* judge the time is corret or not expect timestick_0x08 ==0 */	
							printf("08lost\n");
						}
						timestick_0x08 = timestick;						
						break;
					
					default:
					
						break;
					}
			}
		} while (p - _g_receive_buffer > 200);
	}
		
}

/*********************************************END OF FILE**********************/

