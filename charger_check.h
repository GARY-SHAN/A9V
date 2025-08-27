#ifndef __CHARGER_CHECK_H__
#define __CHARGER_CHECK_H__

//define
#define DEFINE_DC_OK_HIGH_THRESHOLD        914 //2.946v (divider) -> 13.5v (Original)
#define DEFINE_DC_OK_LOW_THRESHOLD         423 //2.46v (divider) -> 10v (Original)

#define DEFINE_BAT_IN                      914 //
#define DEFINE_BAT_OUT                     914 //

#define DEFINE_VBAT_FULL                    253 //
#define DEFINE_IBAT_FULL                    13  //

#define DEFINE_VBAT_RECHARGE                250 //

#define VBAT_IN_FAST                        185  //

//function
char Flag_shift(char shift);
char Check_DC_OK(uint16_t Average_AC_value);
char Check_DC_NG(uint16_t Average_AC_value);
char Check_BAT_IN(uint16_t Average_BAT_value);
char Check_BAT_OUT(uint16_t Average_BAT_value);
char Check_BAT_Full(uint16_t Average_BAT_value, uint16_t Average_BAT_Current);
char Check_BAT_Recharge(uint16_t Average_BAT_value);
char Check_BAT_FastCharge(uint16_t Average_BAT_value);

#endif
