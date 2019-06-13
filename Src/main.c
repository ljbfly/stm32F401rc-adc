/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <RTL.h>    /* RTL kernel functions & defines      */
#include <stdio.h>  /* standard I/O .h-file                */
#include <ctype.h>  /* character functions                 */
#include <string.h> /* string and memory functions         */
#include <stdbool.h>
#include "oled.h"
#include "ugui.h"
#include "SmoothFilt.h"
#include "stdlib.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define KEYNUM  4
#define KMAXCNT 8
#define ShortClick  50  //mS
#define LongCLICK   250 //mS
#define DEFKCOUNT 0x08000000

#define ADC_N 3  //采样点密度(/mS)               //#define ADC_N 1  //采样点密度(/mS)
#define ADC_T 10 //采集周期数 /2 半满中断         //#define ADC_T 5 //采集周期数 /2 半满中断
#define ADC_C_LEN (20*ADC_N*ADC_T)               //#define ADC_C_LEN (20*ADC_N*ADC_T)
#define ADC_C_HLEN (10*ADC_N*ADC_T)              //#define ADC_C_HLEN (10*ADC_N*ADC_T)
                                                 //
#define ADC_V_BASE 1.720                         //#define ADC_V_BASE 1.720
#define FLILE_SAVE_mS 5000                       //#define FLILE_SAVE_mS 5000
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
//u c d l
typedef enum
{
    Key_I =  0,//Idle
    Key_U =  1,//up,
    Key_C =  2,//click,
    Key_D =  3,//double click,
    Key_L =  4//long click,
}KeyEnum;
typedef enum
{
    KeyV_1C =  1,//
    KeyV_1D =  2,//
    KeyV_1L =  3,//
    KeyV_2C =  5,//
    KeyV_2D =  6,//
    KeyV_2L =  7,//
    KeyV_3C =  9,//
    KeyV_3D = 10,//
    KeyV_3L = 11,//
    KeyV_4C = 13,//
    KeyV_4D = 14,//
    KeyV_4L = 15,//
}KeyVEnum;
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

RTC_HandleTypeDef hrtc;

SD_HandleTypeDef hsd;
DMA_HandleTypeDef hdma_sdio;

SPI_HandleTypeDef hspi1;
DMA_HandleTypeDef hdma_spi1_tx;

TIM_HandleTypeDef htim3;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
FILE *f;
UG_GUI gui;

RTC_TimeTypeDef sTime;
RTC_DateTypeDef sDate;

char writebuf[256];
char Guibuf[128];

volatile uint32_t Key_Bvaule[KEYNUM] = {
  DEFKCOUNT,DEFKCOUNT,DEFKCOUNT,DEFKCOUNT
};

uint8_t Key_Count=0;
uint32_t Key_vaules=0;

uint32_t Led_Stm;
uint32_t OLED_Ref_Stm;
uint32_t SDCARD_Clo_Stm;

uint8_t ADC_Flg;
uint8_t Sleep_Flg;

char file_name[64];
uint8_t Cal_R_V = 40;
uint8_t CalculateFlg;
float ADC_V[3],ADC_V_B[3],ADC_V_S[3];
uint16_t ADC_code[ADC_C_LEN][3];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_ADC1_Init(void);
static void MX_TIM3_Init(void);
static void MX_RTC_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_SPI1_Init(void);
static void MX_SDIO_SD_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint32_t ADC_Code_B[3] = {2822,2815,2409};
float ADC_V_C[3];
uint16_t str_idx;
uint16_t end_idx;
uint16_t ADC_sFlg;
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
       {
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */
  

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_TIM3_Init();
  MX_RTC_Init();
  MX_USART1_UART_Init();
  MX_SPI1_Init();
//  MX_SDIO_SD_Init();
  /* USER CODE BEGIN 2 */
    HAL_Delay(200);
    OledInit();
    UG_Init(&gui, ssd1315_pset, 128, 64);
    UG_FillScreen(C_BLACK);
    UG_FontSelect(&FONT_5X12);
    memcpy(writebuf,"KeyV",sizeof("KeyV"));
    UG_PutString(102, 00, writebuf);
    memcpy(writebuf,"File",sizeof("File"));
    UG_PutString(102, 32, writebuf);

    memcpy(writebuf,"Time:",sizeof("Time:"));
    UG_PutString(0, 00, writebuf);
    memcpy(writebuf,"Chn-A:",sizeof("Chn-A:"));
    UG_PutString(0, 16, writebuf);
    memcpy(writebuf,"Chn-B:",sizeof("Chn-B:"));
    UG_PutString(0, 32, writebuf);  
    memcpy(writebuf,"Chn-C:",sizeof("Chn-C:"));
    UG_PutString(0, 48, writebuf);  
    memcpy(writebuf,"A ",3);
    UG_PutString(84, 16, writebuf);
    UG_PutString(84, 32, writebuf);
    UG_PutString(84, 48, writebuf);
    OLED_Refresh_Gram();
    
    finit(NULL);
    HAL_Delay(200);

    HAL_TIM_Base_Start_IT(&htim3);
    HAL_ADC_Start_DMA(&hadc1,(uint32_t *)&ADC_code,ADC_C_LEN*3);
     
    Key_Count = 0;
        
    EventRecorderInitialize(EventRecordAll,1);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  
  while (1)
  {
//if(ADC_sFlg > 0)
//{
//    ADC_sFlg = 0;
//    HAL_ADC_Start_DMA(&hadc1,(uint32_t *)&ADC_code,ADC_C_LEN*3);
//}
    /* RUN */
    if(HAL_GetTick()-Led_Stm >= 1000)
    {
        EventRecord2(EventLevelOp+0,0,0);
        Led_Stm = HAL_GetTick();
        HAL_GPIO_WritePin(Run_GPIO_Port, Run_Pin, GPIO_PIN_SET);
    }
    else if(HAL_GetTick()-Led_Stm >= 980)
    {
        HAL_GPIO_WritePin(Run_GPIO_Port, Run_Pin, GPIO_PIN_RESET);
    }
    
    /* Key */
    if(Key_Count>0)
    {
        uint8_t Key = Key_vaules&0xF;
        switch(Key)
        {
        case 1: 
            CalculateFlg = 0;
            break;
        case 2:
            break;
        case 3:
            CalculateFlg = 1;
            break;

        case 13:
            CalculateFlg = 0;
            break;
        case 14:

            break;
        case 15:
            if(Sleep_Flg)
            {
                Sleep_Flg = 0;
                OledInit();
            }
            else
            {
                Sleep_Flg = 1;
                HAL_GPIO_WritePin(nORST_GPIO_Port,nORST_Pin,GPIO_PIN_RESET);
            }
            break;
        }
          
        sprintf(writebuf, "%02d", (Key_vaules&0xF));
        UG_PutString(108, 16, writebuf);      
        Key_vaules = Key_vaules>>4;
        Key_Count--;
        
    }
    
    /* ADC */
    if(ADC_Flg != 0)
    {
        EventRecord2(EventLevelOp+3,0,0);
        str_idx = (ADC_Flg-1)*ADC_C_HLEN;
        end_idx = (ADC_Flg)*ADC_C_HLEN;        
        ADC_Flg = 0;

        //calculate   
        uint32_t tADC_code[3];
        uint32_t atotal[3],itotal[3];
        atotal[0] = 0;          atotal[1] = 0;          atotal[2] = 0;
        itotal[0] = 0xFFFFFFFF; itotal[1] = 0xFFFFFFFF; itotal[2] = 0xFFFFFFFF;
        uint32_t gatotal[3]={0,0,0},gitotal[3]={0,0,0};

        uint16_t tvidx[3]={0x8000,0x8000,0x8000}, aindex[3]={0,0,0}, iindex[3]={0,0,0}; 
               
        uint32_t ADC_B_code[3]={0,0,0};

        uint16_t index = 0;
        for (uint16_t i=str_idx+2; i<=end_idx-3; i++)
        {
            /* 0 */
            tADC_code[0] = ( -3*(ADC_code[i-2][0]+ADC_code[i+2][0]) + 12*(ADC_code[i-1][0]+ADC_code[i+1][0]) + 17*ADC_code[i][0]) / 35;
            if(tADC_code[0] > atotal[0]) atotal[0] = tADC_code[0];
            if(tADC_code[0] < itotal[0]) itotal[0] = tADC_code[0];  
            if(tADC_code[0]>ADC_Code_B[0])
            { 
                if(tvidx[0] < 0x8000) tvidx[0] = 0x8000;
                tvidx[0] ++;
            }
            else
            {
                if(tvidx[0] > 0x8000) tvidx[0] = 0x8000;
                tvidx[0] --;
            }
            
            if(tvidx[0] == 0x8000+3*ADC_N)
            {
                if(iindex[0]>0)
                    gitotal[0] += itotal[0];
                itotal[0] = 0xFFFFFFFF;
                iindex[0] ++;
            }
            else if(tvidx[0] == 0x8000-3*ADC_N)
            {
                if(aindex[0]>0)
                    gatotal[0] += atotal[0];
                atotal[0] = 0;
                aindex[0] ++;
            }
            
            /* 1 */
            tADC_code[1] = ( -3*(ADC_code[i-2][1]+ADC_code[i+2][1]) + 12*(ADC_code[i-1][1]+ADC_code[i+1][1]) + 17*ADC_code[i][1]) / 35;
            if(tADC_code[1] > atotal[1]) atotal[1] = tADC_code[1];
            if(tADC_code[1] < itotal[1]) itotal[1] = tADC_code[1];  
            if(tADC_code[1]>ADC_Code_B[1])
            { 
                if(tvidx[1] < 0x8000) tvidx[1] = 0x8000;
                tvidx[1] ++;
            }
            else
            {
                if(tvidx[1] > 0x8000) tvidx[1] = 0x8000;
                tvidx[1] --;
            }
            
            if(tvidx[1] == 0x8000+3*ADC_N)
            {
                if(iindex[1]>0)
                    gitotal[1] += itotal[1];
                itotal[1] = 0xFFFFFFFF;
                iindex[1] ++;
            }
            else if(tvidx[1] == 0x8000-3*ADC_N)
            {
                if(aindex[1]>0)
                    gatotal[1] += atotal[1];
                atotal[1] = 0;
                aindex[1] ++;
            }
            
            /* 2 */
            tADC_code[2] = ( -3*(ADC_code[i-2][2]+ADC_code[i+2][2]) + 12*(ADC_code[i-1][2]+ADC_code[i+1][2]) + 17*ADC_code[i][2]) / 35;
            if(tADC_code[2] > atotal[2]) atotal[2] = tADC_code[2];
            if(tADC_code[2] < itotal[2]) itotal[2] = tADC_code[2]; 
            if(tADC_code[2]>ADC_Code_B[2])
            { 
                if(tvidx[2] < 0x8000) tvidx[2] = 0x8000;
                tvidx[2] ++;
            }
            else
            {
                if(tvidx[2] > 0x8000) tvidx[2] = 0x8000;
                tvidx[2] --;
            }
            
            if(tvidx[2] == 0x8000+3*ADC_N)
            {
                if(iindex[2]>0)
                    gitotal[2] += itotal[2];
                itotal[2] = 0xFFFFFFFF;
                iindex[2] ++;
            }
            else if(tvidx[2] == 0x8000-3*ADC_N)
            {
                if(aindex[2]>0)
                    gatotal[2] += atotal[2];
                atotal[2] = 0;
                aindex[2] ++;
            }

            if(CalculateFlg > 0)
            {
                index ++;
                ADC_B_code[0] += tADC_code[0];
                ADC_B_code[1] += tADC_code[1];
                ADC_B_code[2] += tADC_code[2];
            }
        }   

        if(CalculateFlg > 0)
        {
             ADC_Code_B[0] = ADC_B_code[0]/index;
             ADC_Code_B[1] = ADC_B_code[1]/index;
             ADC_Code_B[2] = ADC_B_code[2]/index;
        }
        else
        {
            if(aindex[0]>0)
            aindex[0]--; 
            if(iindex[0]>0)
            iindex[0]--;
            if(aindex[1]>0)
            aindex[1]--; 
            if(iindex[1]>0)
            iindex[1]--;
            if(aindex[2]>0)
            aindex[2]--; 
            if(iindex[2]>0)
            iindex[2]--;
            //*1000*2.5/4096/Cal_R_V/2^-2
            if(aindex[0]>ADC_T/3&&iindex[0]>ADC_T/3)
            {
                if(ADC_V[0]!=0)
                    ADC_V[0] = (ADC_V[0]+((gatotal[0]/aindex[0]-gitotal[0]/iindex[0])*1000)/92681.90002)/2;
                else
                    ADC_V[0] = ((gatotal[0]/aindex[0]-gitotal[0]/iindex[0])*1000)/92681.90002;
            }
            else
                ADC_V[0] = 0;
            if(aindex[1]>ADC_T/3&&iindex[1]>ADC_T/3)
            {
                if( ADC_V[1]!=0)
                    ADC_V[1] = (ADC_V[1]+((gatotal[1]/aindex[1]-gitotal[1]/iindex[1])*1000)/92681.90002)/2;
                else
                    ADC_V[1] = ((gatotal[1]/aindex[1]-gitotal[1]/iindex[1])*1000)/92681.90002;                   
            }
            else
                ADC_V[1] = 0;
            if(aindex[2]>ADC_T/3&&iindex[2]>ADC_T/3)
            {
                ADC_V[2] = 0;//(ADC_V[2]+((gatotal[2]/aindex[2]-gitotal[2]/iindex[2])*1000)/92681.90002)/2;
            }
            else
                ADC_V[2] = 0;
        }
        
//        if(ADC_V[0] < 1.0f)   ADC_V[0] = ADC_V[0]/8;
//        if(ADC_V[1] < 1.0f)   ADC_V[1] = ADC_V[1]/8;
//        if(ADC_V[2] < 1.0f)   ADC_V[2] = 0;
                
        //save
        HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
        HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
        
        sprintf((char *)&writebuf,"%02d%02d.csv",sDate.Date,sTime.Hours);
        if(strstr(file_name,(char *)&writebuf)==NULL)
        {//Creat file
            if(file_name[0] != 0)
            {
              fclose(f);
              f = NULL;         
            }            
            strcpy(file_name,(char *)&writebuf);
            f = fopen(file_name, "a");
            if(f!=NULL)
            {
                memcpy((char *)&writebuf,"Time,Chn-A(v/mA),Ch-B(v/mA),Ch-C(v/mA)\r\n",sizeof("Time,Chn-A(v/mA),Ch-B(v/mA),Ch-C(v/mA)\r\n"));
                fputs((char *)&writebuf, f);              
            }
        }
        
        if(f==NULL)
        {//Open file
            f = fopen(file_name, "a");
        }
        
        if(f!=NULL)
        {//File information Ok
            memcpy(writebuf,"Ok",sizeof("Ok"));
            UG_PutString(108, 48, writebuf);              
        }
        else
        {//File information Error
            memcpy(writebuf,"Err",sizeof("Err"));
            UG_PutString(108, 48, writebuf);  
        }
            
        if(f!=NULL)
        {//Write data           
            char tmpbuf[64];
            sprintf((char *)&tmpbuf, "'%02d:%02d:%03d,%6.3f,%6.3f,%6.3f,\r\n", \
            sTime.Minutes,sTime.Seconds,(1024-sTime.SubSeconds),ADC_V[0],ADC_V[1],ADC_V[2]);
            
            fputs((char *)&tmpbuf, f);

            if(HAL_GetTick()-SDCARD_Clo_Stm >= FLILE_SAVE_mS)
            {//Close file to Save
                SDCARD_Clo_Stm = HAL_GetTick(); 
                fclose(f);
                f = NULL;
            }
        }
        EventRecord2(EventLevelOp+3,1,1);
    }
    
    /* GUI */
    if(Sleep_Flg==0&&HAL_GetTick()-OLED_Ref_Stm >= 250)
    {
        OLED_Ref_Stm = HAL_GetTick();
        HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
        HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

        sprintf((char *)Guibuf, "%02d:%02d:%02d", sTime.Hours, sTime.Minutes, sTime.Seconds);
        UG_PutString(42, 00, (char *)Guibuf);
        sprintf((char *)Guibuf, "%6.2f", ADC_V[0]);
        UG_PutString(48, 16, (char *)Guibuf);
        sprintf((char *)Guibuf, "%6.2f", ADC_V[1]);
        UG_PutString(48, 32, (char *)Guibuf);
        sprintf((char *)Guibuf, "%6.2f", ADC_V[2]);
        UG_PutString(48, 48, (char *)Guibuf);

        OLED_Refresh_Gram();
    }

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Configure the main internal regulator output voltage 
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 192;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */
  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion) 
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = ENABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISING;
  hadc1.Init.ExternalTrigConv = ADC_EXTERNALTRIGCONV_T3_TRGO;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 3;
  hadc1.Init.DMAContinuousRequests = ENABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SEQ_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time. 
  */
  sConfig.Channel = ADC_CHANNEL_11;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_84CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time. 
  */
  sConfig.Channel = ADC_CHANNEL_12;
  sConfig.Rank = 2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time. 
  */
  sConfig.Channel = ADC_CHANNEL_13;
  sConfig.Rank = 3;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */
  /** Initialize RTC Only 
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 31;
  hrtc.Init.SynchPrediv = 1023;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief SDIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_SDIO_SD_Init(void)
{

  /* USER CODE BEGIN SDIO_Init 0 */

  /* USER CODE END SDIO_Init 0 */

  /* USER CODE BEGIN SDIO_Init 1 */

  /* USER CODE END SDIO_Init 1 */
  hsd.Instance = SDIO;
  hsd.Init.ClockEdge = SDIO_CLOCK_EDGE_RISING;
  hsd.Init.ClockBypass = SDIO_CLOCK_BYPASS_DISABLE;
  hsd.Init.ClockPowerSave = SDIO_CLOCK_POWER_SAVE_DISABLE;
  hsd.Init.BusWide = SDIO_BUS_WIDE_1B;
  hsd.Init.HardwareFlowControl = SDIO_HARDWARE_FLOW_CONTROL_DISABLE;
  hsd.Init.ClockDiv = 0;
  if (HAL_SD_Init(&hsd) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_SD_ConfigWideBusOperation(&hsd, SDIO_BUS_WIDE_4B) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SDIO_Init 2 */

  /* USER CODE END SDIO_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_1LINE;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_HIGH;
  hspi1.Init.CLKPhase = SPI_PHASE_2EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 47;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = (1000/ADC_N)-1;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/** 
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void) 
{
  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA2_Stream0_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);
  /* DMA2_Stream3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream3_IRQn);
  /* DMA2_Stream5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream5_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, nORST_Pin|nOCS_Pin|DC_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(Run_GPIO_Port, Run_Pin, GPIO_PIN_SET);

  /*Configure GPIO pins : PC13 PC0 PC4 PC5 
                           PC6 PC7 */
  GPIO_InitStruct.Pin = GPIO_PIN_13|GPIO_PIN_0|GPIO_PIN_4|GPIO_PIN_5 
                          |GPIO_PIN_6|GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PA0 PA1 PA2 PA8 
                           PA11 PA12 PA15 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_8 
                          |GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : nORST_Pin nOCS_Pin DC_Pin */
  GPIO_InitStruct.Pin = nORST_Pin|nOCS_Pin|DC_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB0 PB2 PB10 PB3 
                           PB4 PB5 PB6 PB7 
                           PB8 PB9 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_2|GPIO_PIN_10|GPIO_PIN_3 
                          |GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7 
                          |GPIO_PIN_8|GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : Run_Pin */
  GPIO_InitStruct.Pin = Run_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(Run_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PB12 PB13 PB14 PB15 */
  GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

}

/* USER CODE BEGIN 4 */
void ScanKey(void)
{
  uint8_t i;
  //11,12,13,14
  for(i=0;i<KEYNUM;i++)
  {
    if((GPIOB->IDR>>(12+i)) & 0x1)
    {//up
      if((Key_Bvaule[i]&0x0FFFFFFF) > DEFKCOUNT)
      {
        Key_Bvaule[i] &= 0xF0000000;
        Key_Bvaule[i] |= DEFKCOUNT;
      }
      else if((Key_Bvaule[i]&0x0FFFFFFF) > DEFKCOUNT-LongCLICK)
      {
        Key_Bvaule[i]--;
      }

      if((Key_Bvaule[i]&0x0FFFFFFF) < DEFKCOUNT-ShortClick)
      {
        if(((Key_Bvaule[i]&0x0FFFFFFF) > DEFKCOUNT-LongCLICK)&&(Key_Bvaule[i]>>28)!=Key_U)
        {
          if((Key_Bvaule[i]>>28)==Key_C)
          {          
            Key_Bvaule[i] &= 0x0FFFFFFF;
            Key_Bvaule[i] |= Key_U<<28;
          }
        }
        else if(((Key_Bvaule[i]&0x0FFFFFFF)==DEFKCOUNT-LongCLICK)&&(Key_Bvaule[i]>>28)!=Key_I)
        {
          if((Key_Bvaule[i]>>28)==Key_D)
          {
            Key_vaules |= ((4*i+2)<<(Key_Count*4));//???
            if(Key_Count<KMAXCNT)
            Key_Count++;
          }
          else if((Key_Bvaule[i]>>28)==Key_U)
          {
            Key_vaules |= ((4*i+1)<<(Key_Count*4));//????
            if(Key_Count<KMAXCNT)
              Key_Count++;
          }
          Key_Bvaule[i] &= 0x0FFFFFFF;
          Key_Bvaule[i] |= Key_I<<28;
        }
      }
    }
    else
    {//down
      if ((Key_Bvaule[i]&0x0FFFFFFF) < DEFKCOUNT)
      {
        Key_Bvaule[i] &= 0xF0000000;
        Key_Bvaule[i] |= DEFKCOUNT;
      }
      else if((Key_Bvaule[i]&0x0FFFFFFF) <  DEFKCOUNT+2*LongCLICK)
      {
        Key_Bvaule[i]++;
      }
            
      if((Key_Bvaule[i]&0x0FFFFFFF) > DEFKCOUNT+ShortClick)
      {
        if(((Key_Bvaule[i]&0x0FFFFFFF) < DEFKCOUNT+LongCLICK)&&(Key_Bvaule[i]>>28)!=Key_C)
        {
            if((Key_Bvaule[i]>>28)==Key_U)
            {
              Key_Bvaule[i] &= 0x0FFFFFFF;
              Key_Bvaule[i]|=Key_D<<28;
            }
            else if((Key_Bvaule[i]>>28)==Key_I)            
            {
              Key_Bvaule[i] &= 0x0FFFFFFF;
              Key_Bvaule[i]|=Key_C<<28;  
            }              
        }
        else if(((Key_Bvaule[i]&0x0FFFFFFF)==DEFKCOUNT+2*LongCLICK)&&(Key_Bvaule[i]>>28)!=Key_L)
        {
            Key_Bvaule[i] &= 0x0FFFFFFF;
            Key_Bvaule[i]|=Key_L<<28;  
          
            Key_vaules |= ((4*i+3)<<(Key_Count*4));//????
            if(Key_Count<KMAXCNT)
              Key_Count++;
        }
      }
    }
  }
}

void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef *hadc)
{
  if (hadc == &hadc1)
  {
      EventRecord2(EventLevelOp+1,1,1);
      ADC_Flg = 1;
  }
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
  if (hadc == &hadc1)
  {
//      HAL_ADC_Stop_DMA(&hadc1);
      EventRecord2(EventLevelOp+2,2,2);
      ADC_Flg = 2;
  }
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
