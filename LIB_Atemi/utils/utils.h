#ifndef __UTILITY_H__
#define __UTILITY_H__

#include "ti_msp_dl_config.h"

extern bool StateDebounce(bool bState, uint8_t u8Delay, vuint8_t *u8Cntr);
extern bool StateDebounce16(bool bState, uint16_t u16Delay, uint16_t *u16Cntr);

#endif
