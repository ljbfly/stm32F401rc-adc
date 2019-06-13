#include "stdint.h"

#ifndef __SMOOTHFILT_H__
#define __SMOOTHFILT_H__

uint32_t SmoothFilter(uint8_t ch, uint32_t pre_value, uint8_t filter_order);
void linearSmooth5 ( int16_t *in, float *out, int N );

#endif
