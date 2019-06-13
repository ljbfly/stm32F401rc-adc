#include "stdint.h"
#include "stm32f4xx_hal.h"
#include "ugui.h"

#ifndef __OLED_H__
#define __OLED_H__
extern SPI_HandleTypeDef hspi1;    

extern void OledInit(void);
extern void OLED_Refresh_Gram(void);
extern void ssd1315_pset ( UG_S16 x , UG_S16 y , UG_COLOR c );

#endif
