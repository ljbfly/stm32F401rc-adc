#include "main.h"
#include "stm32f4xx_hal.h"
#include "Oled.h"
#include "ugui.h"
#include "stdbool.h"
#include "string.h"
#include <RTL.h>

typedef unsigned char u8;
u8 OLED_GRAM[128][8] __attribute__((at(0x20001000)));

uint8_t Oledbuf[32];

#define OledCsH()   nOCS_GPIO_Port->BSRR    = nOCS_Pin
#define OledCsL()   nOCS_GPIO_Port->BSRR    = nOCS_Pin << 16
#define OledRstH()  nORST_GPIO_Port->BSRR   = nORST_Pin
#define OledRstL()  nORST_GPIO_Port->BSRR   = nORST_Pin << 16
#define OledDat()   DC_GPIO_Port->BSRR      = DC_Pin
#define OledCmd()   DC_GPIO_Port->BSRR      = DC_Pin << 16

#define MAX_CHAR_POSX 117
#define MAX_CHAR_POSY 42 

#define BASEADDR    0x22020000
const uint8_t OledInitInfc[8]={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
const uint8_t OledInitCode[]={0xAE,0xD5,0x80,0xA8,0x3F,0xD3,0x00,0x40,0x8D,0x14,0x20,0x01,
                                0xA1,0xC0,0xDA,0x12,0x81,0xAF,0xD9,0xF1,0xDB,0x30,0xA4,0xAF};

void OledInit(void)
{    
    OledCsL();
    OledRstL(); 
    HAL_Delay(10);
    OledRstH();
    OledDat();

    HAL_SPI_Transmit(&hspi1, (uint8_t *)&OledInitInfc, 8, 8);
   
    /* ¸´Î»³õÊ¼»¯ÆÁÄ» */
    OledRstL(); 
    HAL_Delay(10);
    OledRstH();
    
    OledCmd();   
    HAL_SPI_Transmit(&hspi1, (uint8_t *)&OledInitCode, sizeof(OledInitCode), sizeof(OledInitCode));
    OledDat();  
}

uint8_t freshflg = false;
void OLED_Refresh_Gram(void)
{
//    if(freshflg++ > 0)
//        return;
    OledCsL();
    OledDat();
    HAL_SPI_Transmit_DMA(&hspi1,&OLED_GRAM[0][0],128*8);
}

void ssd1315_pset ( UG_S16 x , UG_S16 y , UG_COLOR c )
{
   uint32_t * p = (uint32_t *)BASEADDR;
   *(p+64*x+63-y) = c;
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
    if(hspi == &hspi1)
    {
//        if(freshflg > 0)
//        {
//            HAL_SPI_Transmit_DMA(&hspi1,&OLED_GRAM[0][0],128*8);
//        }
//        else
            OledCsH();
        freshflg = 0;
    }
}

