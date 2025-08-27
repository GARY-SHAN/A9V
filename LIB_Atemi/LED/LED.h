#ifndef __LED_H__
#define __LED_H__

#include "ti_msp_dl_config.h"

#define MAX_ACTION_PROFILE_NUM  4

// For LED colors
typedef enum eLEDColors
{
    LED_COLOR_WITHOUT_SET = 0x00,
    LED_COLOR_RED = 0x01,
    LED_COLOR_GREEN = 0x02,
    LED_COLOR_YELLOW = 0x03,
}eLEDColors_t;

// For LED Scenario
// typedef enum eScenario
// {
//     BATTERY_UNKNOWN = 0x00,
//     BATTERY_FULL = 0x01,
//     BATTERY_CAPACITY_OVER_25_PERCENTAG = 0x02,
//     BATTERY_CAPACITY_UNDER_25_PERCENTAG = 0x03,
//     BATTERY_CHARGING = 0x04,
// }eScenario_t;

// Record LED action
typedef struct sActionProfile
{
    uint8_t u8_ActionLED;
    uint8_t u8_LEDONCount;
    uint8_t u8_LEDOFFCount;
    uint8_t u8_ActionRepeatTimes;    
}sActionProfile_t;

// Record LED action
typedef struct sWorkingProfile
{
    uint8_t u8_NowScriptNumber;
    // uint8_t u8_TotalScriptNumber;
    uint8_t u8_ActionLED;
    // uint8_t u8_LEDON_Reload_Count;
    uint8_t u8_LEDONCount;
    // uint8_t u8_LEDOFF_Reload_Count;
    uint8_t u8_LEDOFFCount;
    uint8_t u8_ActionRepeatTimes;    
}sWorkingProfile_t;


// Record LED script
typedef struct sLED_script
{
    uint8_t u8_TotalAction;
    sActionProfile_t ActionProfile[MAX_ACTION_PROFILE_NUM];
}sLED_script_t;

// Record now LED working status
typedef struct sLED_WorkingProfile
{
    // eScenario_t u8_Scenario;
    sWorkingProfile_t WorkingProfile;
}sLED_WorkingProfile_t;

extern void LED_handler(void);
extern void Init_LED_script(void);
extern void Reload_LED_script(void);
#endif
