#include "ti_msp_dl_config.h"
#include "defs.h"
#include "main.h"
#include "utils.h"
#include "charger_check.h"

vuint8_t g_u8DCOKCntr = 0, g_u8DCNGCntr = 0, g_u8BATINCntr = 0, g_u8BATOUTCntr = 0;
vuint8_t g_u8BATFULLCntr = 0, g_u8FASTCHARGECntr = 0, g_u8BATRECHARGECntr = 0;

char Flag_shift(char shift)
{
    return (1 << shift);
}

char Check_result = 0;
char Check_DC_OK(uint16_t Average_AC_value)
{
    Check_result = StateDebounce((Average_AC_value < DEFINE_DC_OK_HIGH_THRESHOLD && Average_AC_value > DEFINE_DC_OK_LOW_THRESHOLD), 10, &g_u8DCOKCntr);
    if(Check_result)
    {
        g_u8DCNGCntr = 0;
    }
    return Check_result;
}

char Check_DC_NG(uint16_t Average_AC_value)
{
    Check_result = StateDebounce((Average_AC_value > DEFINE_DC_OK_HIGH_THRESHOLD || Average_AC_value < DEFINE_DC_OK_LOW_THRESHOLD), 10, &g_u8DCNGCntr);
    if(Check_result)
    {
        g_u8DCOKCntr = 0;
    }
    return Check_result;
}

char Check_BAT_IN(uint16_t Average_BAT_value)
{
    Check_result = StateDebounce((Average_BAT_value > DEFINE_BAT_IN), 10, &g_u8BATINCntr);
    if(Check_result)
    {
        g_u8BATOUTCntr = 0;
    }
    return Check_result;
}

char Check_BAT_OUT(uint16_t Average_BAT_value)
{
    Check_result = StateDebounce((Average_BAT_value < DEFINE_BAT_OUT), 10, &g_u8BATOUTCntr);
    if(Check_result)
    {
        g_u8BATINCntr = 0;
    }
    return Check_result;
}

char Check_BAT_Full(uint16_t Average_BAT_value, uint16_t Average_BAT_Current)
{
    Check_result = StateDebounce((Average_BAT_value >= DEFINE_VBAT_FULL && Average_BAT_Current < DEFINE_IBAT_FULL), 10, &g_u8BATFULLCntr);
    if(Check_result)
    {
        g_u8BATRECHARGECntr = 0;
    }
    return Check_result;
}

char Check_BAT_Recharge(uint16_t Average_BAT_value)
{
    Check_result = StateDebounce((Average_BAT_value < DEFINE_VBAT_RECHARGE), 10, &g_u8BATRECHARGECntr);
    if(Check_result)
    {
        g_u8BATFULLCntr = 0;
    }
    return Check_result;
}

char Check_BAT_FastCharge(uint16_t Average_BAT_value)
{
    return StateDebounce((Average_BAT_value >= VBAT_IN_FAST), 2 ,&g_u8FASTCHARGECntr);
}
