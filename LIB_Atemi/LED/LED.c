#include "LED.h"
#include "defs.h"

#include "main.h"

sLED_script_t g_sLED_script;
sWorkingProfile_t g_sWorkingProfile;

uint8_t LED_display_counter = 0;
#define LED_DISPLAY_PERIOD  40

void Init_LED_script(void){
    uint8_t i;

    LED_display_counter = 0;

    g_sLED_script.u8_TotalAction = 0;
    
    for(i=0; i< MAX_ACTION_PROFILE_NUM; i++){
        g_sLED_script.ActionProfile[i].u8_ActionLED = LED_COLOR_WITHOUT_SET;
        g_sLED_script.ActionProfile[i].u8_LEDONCount = 0;
        g_sLED_script.ActionProfile[i].u8_LEDOFFCount = 0;
        g_sLED_script.ActionProfile[i].u8_ActionRepeatTimes = 0;
    }

    g_sWorkingProfile.u8_ActionLED = LED_COLOR_WITHOUT_SET;
    g_sWorkingProfile.u8_LEDONCount = 0;
    g_sWorkingProfile.u8_LEDOFFCount = 0;
    g_sWorkingProfile.u8_ActionRepeatTimes = 0;
}

void Reload_LED_script(void){
    if(IS_BIT_SET(g_u8SystemState, SYS_STAT_IDLE)){
        g_sLED_script.u8_TotalAction = 1;
        g_sLED_script.ActionProfile[0].u8_ActionLED = LED_COLOR_GREEN;
        g_sLED_script.ActionProfile[0].u8_LEDONCount = 0;
        g_sLED_script.ActionProfile[0].u8_LEDOFFCount = 40;
        g_sLED_script.ActionProfile[0].u8_ActionRepeatTimes = 1;    
    } else if(IS_BIT_SET(g_u8SystemErrors, SYS_ERROR_AC_NG) || IS_BIT_SET(g_u8SystemErrors, SYS_ERROR_BATTERY_OVP) ||
       IS_BIT_SET(g_u8SystemErrors, SYS_ERROR_CHG_OTP) || IS_BIT_SET(g_u8SystemErrors, SYS_ERROR_OCP)){
        g_sLED_script.u8_TotalAction = 1;
        g_sLED_script.ActionProfile[0].u8_ActionLED = LED_COLOR_RED;
        g_sLED_script.ActionProfile[0].u8_LEDONCount = 10;
        g_sLED_script.ActionProfile[0].u8_LEDOFFCount = 0;
        g_sLED_script.ActionProfile[0].u8_ActionRepeatTimes = 1;
    } else if(IS_BIT_SET(g_u8SystemFlags, FLAG_CAPACITY_FULL)){
        g_sLED_script.u8_TotalAction = 1;
        g_sLED_script.ActionProfile[0].u8_ActionLED = LED_COLOR_GREEN;
        g_sLED_script.ActionProfile[0].u8_LEDONCount = 10;
        g_sLED_script.ActionProfile[0].u8_LEDOFFCount = 0;
        g_sLED_script.ActionProfile[0].u8_ActionRepeatTimes = 1;
    } else if(IS_BIT_SET(g_u8SystemFlags, FLAG_CHARGING)){
        g_sLED_script.u8_TotalAction = 1;
        g_sLED_script.ActionProfile[0].u8_ActionLED = LED_COLOR_GREEN;
        g_sLED_script.ActionProfile[0].u8_LEDONCount = 50;
        g_sLED_script.ActionProfile[0].u8_LEDOFFCount = 50;
        g_sLED_script.ActionProfile[0].u8_ActionRepeatTimes = 1;        
    } else if(IS_BIT_SET(g_u8SystemFlags, FLAG_CAPACITY_OVER_25_PERCENTAG)){
        g_sLED_script.u8_TotalAction = 1;
        g_sLED_script.ActionProfile[0].u8_ActionLED = LED_COLOR_GREEN;
        g_sLED_script.ActionProfile[0].u8_LEDONCount = 10;
        g_sLED_script.ActionProfile[0].u8_LEDOFFCount = 10;
        g_sLED_script.ActionProfile[0].u8_ActionRepeatTimes = 1;        
    } else if(IS_BIT_SET(g_u8SystemFlags, FLAG_CAPACITY_UNDER_25_PERCENTAGE)){
        g_sLED_script.u8_TotalAction = 1;
        g_sLED_script.ActionProfile[0].u8_ActionLED = LED_COLOR_RED;
        g_sLED_script.ActionProfile[0].u8_LEDONCount = 10;
        g_sLED_script.ActionProfile[0].u8_LEDOFFCount = 10;
        g_sLED_script.ActionProfile[0].u8_ActionRepeatTimes = 1;        
    }
    // else if(IS_BIT_SET(g_u8SystemFlags, FLAG_CAPACITY_OVER_25_PERCENTAG)){
    //     if(IS_BIT_SET(g_u8SystemFlags, FLAG_CHARGING))
    //         g_sLED_script.u8_TotalAction = 2;
    //     else
    //         g_sLED_script.u8_TotalAction = 1;

    //     // 1st action profile
    //     g_sLED_script.ActionProfile[0].u8_ActionLED = LED_COLOR_YELLOW;
    //     g_sLED_script.ActionProfile[0].u8_LEDONCount = 10;
    //     g_sLED_script.ActionProfile[0].u8_LEDOFFCount = 10;
        
    //     if(IS_BIT_SET(g_u8SystemFlags, FLAG_CHARGING))
    //         g_sLED_script.ActionProfile[0].u8_ActionRepeatTimes = 1;
    //     else
    //         g_sLED_script.ActionProfile[0].u8_ActionRepeatTimes = 2;

    //     // 2nd action profile
    //     if(IS_BIT_SET(g_u8SystemFlags, FLAG_CHARGING)){
    //         g_sLED_script.ActionProfile[1].u8_ActionLED = LED_COLOR_GREEN;
    //         g_sLED_script.ActionProfile[1].u8_LEDONCount = 5;
    //         g_sLED_script.ActionProfile[1].u8_LEDOFFCount = 5;
    //         g_sLED_script.ActionProfile[1].u8_ActionRepeatTimes = 2;            
    //     }
    // } else if(IS_BIT_SET(g_u8SystemFlags, FLAG_CAPACITY_UNDER_25_PERCENTAGE)){
    //     if(IS_BIT_SET(g_u8SystemFlags, FLAG_CHARGING))
    //         g_sLED_script.u8_TotalAction = 2;
    //     else
    //         g_sLED_script.u8_TotalAction = 1;

    //     // 1st action profile
    //     g_sLED_script.ActionProfile[0].u8_ActionLED = LED_COLOR_RED;
    //     g_sLED_script.ActionProfile[0].u8_LEDONCount = 10;
    //     g_sLED_script.ActionProfile[0].u8_LEDOFFCount = 10;
        
    //     if(IS_BIT_SET(g_u8SystemFlags, FLAG_CHARGING))
    //         g_sLED_script.ActionProfile[0].u8_ActionRepeatTimes = 1;
    //     else
    //         g_sLED_script.ActionProfile[0].u8_ActionRepeatTimes = 2;

    //     // 2nd action profile
    //     if(IS_BIT_SET(g_u8SystemFlags, FLAG_CHARGING)){
    //         g_sLED_script.ActionProfile[1].u8_ActionLED = LED_COLOR_GREEN;
    //         g_sLED_script.ActionProfile[1].u8_LEDONCount = 5;
    //         g_sLED_script.ActionProfile[1].u8_LEDOFFCount = 5;
    //         g_sLED_script.ActionProfile[1].u8_ActionRepeatTimes = 2;            
    //     }
    // }

    // Setup working profile
    g_sWorkingProfile.u8_NowScriptNumber = 1;
    // g_sWorkingProfile.u8_TotalScriptNumber = g_sLED_script.u8_TotalAction;
    g_sWorkingProfile.u8_ActionLED = g_sLED_script.ActionProfile[0].u8_ActionLED;
    // g_sWorkingProfile.u8_LEDON_Reload_Count = g_sLED_script.ActionProfile[0].u8_LEDONCount;
    g_sWorkingProfile.u8_LEDONCount = g_sLED_script.ActionProfile[0].u8_LEDONCount;
    // g_sWorkingProfile.u8_LEDOFF_Reload_Count = g_sLED_script.ActionProfile[0].u8_LEDOFFCount;
    g_sWorkingProfile.u8_LEDOFFCount = g_sLED_script.ActionProfile[0].u8_LEDOFFCount;
    g_sWorkingProfile.u8_ActionRepeatTimes = g_sLED_script.ActionProfile[0].u8_ActionRepeatTimes;
}


void Update_LED(void){
    if(g_sWorkingProfile.u8_LEDONCount > 0){
        switch(g_sWorkingProfile.u8_ActionLED){
            case LED_COLOR_RED:
            DL_GPIO_setPins(BAMBOO_GPIO_PORT, BAMBOO_GPIO_LED_RED_PIN);
            DL_GPIO_clearPins(BAMBOO_GPIO_PORT, BAMBOO_GPIO_LED_GREEN_PIN);
            break;

            case LED_COLOR_GREEN:
            DL_GPIO_setPins(BAMBOO_GPIO_PORT, BAMBOO_GPIO_LED_GREEN_PIN);
            DL_GPIO_clearPins(BAMBOO_GPIO_PORT, BAMBOO_GPIO_LED_RED_PIN);
            break;

            case LED_COLOR_YELLOW:
            DL_GPIO_setPins(BAMBOO_GPIO_PORT, BAMBOO_GPIO_LED_GREEN_PIN | BAMBOO_GPIO_LED_RED_PIN);
            break;
        }
        g_sWorkingProfile.u8_LEDONCount--;
    } else if(g_sWorkingProfile.u8_LEDOFFCount > 0){
        DL_GPIO_clearPins(BAMBOO_GPIO_PORT, BAMBOO_GPIO_LED_GREEN_PIN | BAMBOO_GPIO_LED_RED_PIN);

        g_sWorkingProfile.u8_LEDOFFCount--;
    } else if (g_sWorkingProfile.u8_ActionRepeatTimes >= 1){
        g_sWorkingProfile.u8_LEDONCount = g_sLED_script.ActionProfile[g_sWorkingProfile.u8_NowScriptNumber - 1].u8_LEDONCount;
        g_sWorkingProfile.u8_LEDOFFCount = g_sLED_script.ActionProfile[g_sWorkingProfile.u8_NowScriptNumber - 1].u8_LEDOFFCount;

        g_sWorkingProfile.u8_ActionRepeatTimes--;
    } else if(g_sWorkingProfile.u8_NowScriptNumber != g_sLED_script.u8_TotalAction){
        // Update ScriptNumber
        g_sWorkingProfile.u8_NowScriptNumber++;
        g_sWorkingProfile.u8_ActionLED = g_sLED_script.ActionProfile[g_sWorkingProfile.u8_NowScriptNumber - 1].u8_ActionLED;
        g_sWorkingProfile.u8_LEDONCount = g_sLED_script.ActionProfile[g_sWorkingProfile.u8_NowScriptNumber - 1].u8_LEDONCount;
        g_sWorkingProfile.u8_LEDOFFCount = g_sLED_script.ActionProfile[g_sWorkingProfile.u8_NowScriptNumber - 1].u8_LEDOFFCount;
        g_sWorkingProfile.u8_ActionRepeatTimes = g_sLED_script.ActionProfile[g_sWorkingProfile.u8_NowScriptNumber - 1].u8_ActionRepeatTimes;        
    }
}


void LED_handler(void){
    // if((LED_display_counter % LED_DISPLAY_PERIOD) == 0){
    if(g_sWorkingProfile.u8_ActionRepeatTimes == 0)
        Reload_LED_script();
    // }


    //Update_LED();

    // LED_display_counter = LED_display_counter % LED_DISPLAY_PERIOD;
    // LED_display_counter ++;    
}
