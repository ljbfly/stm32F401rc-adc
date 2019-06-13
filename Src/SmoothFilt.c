/**********************************************************************
*                   WLKJ D3-μcosIII-STM32F107
*
* Filename      : app.c
* Version       : V2.00
* Programmer(s) : 李建波
**********************************************************************/

#include "stdint.h"

typedef struct {
uint8_t FILTER_NUM;
uint8_t DataSum;
uint8_t *Filter_Para;
}SMFILT_Tp;

static uint8_t  Filter_Para0[10]={1,0,1,1,1,1,1,1,0,1}; //加权系数，权值和2^3
static uint8_t  Filter_Para1[15]={1,0,1,1,1,2,1,2,1,2,1,1,1,0,1}; //加权系数，权值和2^4
static uint8_t  Filter_Para2[21]={1,0,1,1,1,2,2,2,2,3,2,3,2,2,2,2,1,1,1,0,1}; //加权系数，权值和2^5
static uint8_t  Filter_Para3[28]={1,0,1,1,1,2,2,3,2,4,3,4,4,4,4,4,4,3,4,2,3,2,2,1,1,1,0,1 }; //加权系数，权值和2^6
static uint8_t  Filter_Para4[36]={1,0,1,1,1,2,2,3,3,4,4,5,5,6,6,7,6,7,7,6,7,6,6,5,5,4,4,3,3,2,2,1,1,1,0,1}; //加权系数，权值和2^7
	
SMFILT_Tp SmFilt[5] = {	{10,3,Filter_Para0},
						{15,4,Filter_Para1},
						{21,5,Filter_Para2},
						{28,6,Filter_Para3},
						{36,7,Filter_Para4} };

uint8_t filter_storecount[3]={0};	//存储计数
uint32_t array_filter[3][36]={0};	//滤波队列
uint32_t SmoothFilter(uint8_t ch, uint32_t pre_value, uint8_t filter_order)
{	
	uint32_t sum=0;
	uint8_t para_count=0;	//参数计数
	uint8_t i=0,j=0;

    if(filter_order == 0)
    {
        for(i=0;i<SmFilt[4].FILTER_NUM;i++)
            array_filter[ch][filter_storecount[ch]++] = pre_value;
    }
    
	array_filter[ch][filter_storecount[ch]++] = pre_value;
	
	filter_storecount[ch] %= SmFilt[4].FILTER_NUM;
    
	for(i=filter_storecount[ch];i<SmFilt[4].FILTER_NUM;i++)
	{
		sum = sum + array_filter[ch][i]*(SmFilt[4].Filter_Para)[para_count++];
	}

	for(j=0;j<filter_storecount[ch];j++)
	{
		sum = sum + array_filter[ch][j]*(SmFilt[4].Filter_Para)[para_count++];
	}

	sum = sum >> SmFilt[4].DataSum;
	
	return sum; 
}

/* 五点三次平滑滤波
    69    4   -6    4   -1   /70
     2   27   12   -8    2   /35
    -3   12   17   12   -3   /35
     2   -8   12   27    2   /35
    -1    4   -6    4   69   /70

*/

void linearSmooth5 ( int16_t *in, float *out, int N )
{
  int i;
  if ( N < 5 )
  {
    for (i=0; i<=N-1; i++)
    {
      out[i] = in[i];
    }
  }
  else
  {
    out[0] = ( 69*in[0] + 4*(in[1]+in[3]) - 6*in[2] - in[4] ) / 70.0;
    out[1] = ( 2*(in[0]+in[4]) + 27*in[1] + 12*in[2] - 8*in[3]) / 35.0;
    for (i=2; i<=N-3; i++)
    {
      out[i] = ( -3*(in[i-2]+in[i+2]) + 12*(in[i-1]+in[i+1]) + 17*in[i]) / 35.0;
    }
      out[N-2] = ( 2*(in[N-5]+in[N-1]) - 8*in[N-4] + 12*in[N-3] + 27*in[N-2] ) / 35.0;
      out[N-1] = ( -1*in[N-5] + 4*(in[N-4]+in[N-2]) - 6*in[N-3] + 69*in[N-1]) / 70.0;
  }
}

