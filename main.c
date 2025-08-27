/*
 * Copyright (c) 2021, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "ti_msp_dl_config.h"
#include "defs.h"
#include "main.h"
#include "utils.h"

#include "Charger_BQ24725A.h"
#include "LED.h"
#include "charger_check.h"

#define JUMP_TO_BOOTLOADER()     {((void (*)()) (*(uint32_t *)(0x00000000 + 4))) ();}
#define BOOTLOADER_MAIN_BASE_ADDRESS 0x00000000
/* ==== ADC data ====*/
#define RESULT_SIZE (5)

volatile bool gCheckADC;
volatile uint16_t gADC_PA27_A0_BAT[RESULT_SIZE];
volatile uint16_t gADC_PA26_A1_IOUT[RESULT_SIZE];
volatile uint16_t gADC_PA25_A2_TS[RESULT_SIZE];
volatile uint16_t gADC_PA24_A3_DC[RESULT_SIZE];
volatile uint16_t gADC_PA22_A5_VCH[RESULT_SIZE];
volatile uint16_t gADC_PA16_A8_MCU[RESULT_SIZE];
/* ==== ADC data ====*/

uint32_t AC_OK_Status = 0;

uint16_t Average_AC_value = 0;
uint16_t Average_BAT_value = 0;
uint16_t Average_IOUT_value = 0;
uint16_t Average_TS_value = 0;
uint16_t Average_CS_value = 0;
uint16_t Average_VCH_value = 0;
uint16_t Average_BUT_value = 0;

// uint16_t PK_temp = 0;

uint8_t timer_10ms_count = 0;
#define TIMER_1_SEC_COUNTS  10

void OneHundredMsHnadler(void);
void OneSecondHandler(void);
static void polling_all_adc(void);

void system_status_check(void);
void disable_charger(void);

vuint8_t g_u8SystemState = 0, g_u8SystemFlags = 0, g_u8SystemErrors = 0, g_u8SysFlags = 0;
vuint8_t g_u8ChrgFlag = 0, g_u8ChrgStatusFlags = 0;

vuint8_t g_u8ACOKCntr = 0, g_u8ACNGCntr = 0, g_u8CapacityUnder25percentageCntr = 0, g_u8CapacityOver25percentageCntr = 0, g_u8CapacityFullCntr = 0;
vuint8_t g_u8BatteryOVPCntr = 0, g_u8BatteryOVPReleaseCntr = 0, g_u8BatteryUVPCntr = 0, g_u8BatteryUVPReleaseCntr = 0, g_u8BatteryBATIMAXCntr = 0, g_u8BatteryBATIMINCntr = 0;
vuint8_t g_u8DCHGOTPCntr = 0, g_u8DCHGOTPReleaseCntr = 0, g_u8CHGOTPCntr = 0, g_u8CHGOTPReleaseCntr = 0, g_u8CHGUTPCntr = 0, g_u8CHGUTPReleaseCntr = 0;
vuint8_t g_u8OCPCntr = 0, g_u8OCPReleaseCntr = 0, g_u8SCPCntr = 0, g_u8SCPReleaseCntr = 0;
vuint8_t g_u8CutoffCntr = 0, g_u8CutoffReleaseCntr = 0;
vuint8_t g_u8IdleCntr = 0, g_u8IdleReleaseCntr = 0;
vuint8_t g_u8HIGHVALUECntr = 0, g_u8HIGHVALUERELEASECntr = 0, g_u8RechargeCntr = 0;
vuint8_t g_u8FastChargeReleaseCntr = 0, g_u8FastChargeCntr = 0;

void func_I2C_RESET()
{
    DL_I2C_reset(I2C_INST);
    DL_I2C_enablePower(I2C_INST);
    SYSCFG_DL_I2C_init();
    delay_cycles(120000);
}

void Disable_bat_mos()
{
    DL_GPIO_clearPins(BAMBOO_GPIO_PORT, BAMBOO_GPIO_CH_EN_PIN | BAMBOO_GPIO_ACOK_PIN);

    DL_GPIO_clearPins(BAMBOO_GPIO_PORT, BAMBOO_GPIO_VCH_PIN);
}

void Enable_bat_mos()
{
    DL_GPIO_setPins(BAMBOO_GPIO_PORT, BAMBOO_GPIO_ACOK_PIN);
    delay_cycles(120000); // 0.05sec
    DL_GPIO_setPins(BAMBOO_GPIO_PORT, BAMBOO_GPIO_CH_EN_PIN);

    DL_GPIO_setPins(BAMBOO_GPIO_PORT, BAMBOO_GPIO_VCH_PIN);
    delay_cycles(120000); // 0.05sec
}

static uint16_t timeout[2] = {0};

uint16_t Dummy_IOUT = 0;
uint16_t Dummy_VBAT = 0;
uint8_t MCU_ID = 1;
//For debug...PK20241008+
// 1 : Enable OCP while cool boot
// 0 : Disable OCP while cool boot
// uint8_t PK_XD = 1;
char test_index = 0;
char bat_in_temp_check = 0;
uint8_t uart_rx = 0;
uint8_t bat_in_count = 0;

char u8_bat_first_in = 0;

int main(void)
{
    static char test_revert = 0;
//    uint16_t Average_AC_value = 0;

#if 0   //Gary test 250217
    //SYSCFG_DL_SYSCTL_init();
#ifdef evm_test
    DL_GPIO_reset(GPIOA);
    DL_GPIO_enablePower(GPIOA);
    delay_cycles(POWER_STARTUP_DELAY);

    DL_GPIO_initDigitalOutput(IOMUX_PINCM23);
    DL_GPIO_clearPins(GPIOA, DL_GPIO_PIN_22);
    DL_GPIO_setPins(GPIOA, DL_GPIO_PIN_22);
    DL_GPIO_enableOutput(GPIOA, DL_GPIO_PIN_22);
    SYSCFG_DL_SYSCTL_init();

    while(1)
    {
        DL_GPIO_clearPins(GPIOA, DL_GPIO_PIN_22);
        delay_cycles(4000000);
        DL_GPIO_setPins(GPIOA, DL_GPIO_PIN_22);
        delay_cycles(4000000);
    }
#else
    SYSCFG_DL_init();
    DL_UART_enableFIFOs(UART_0_INST);
    NVIC_EnableIRQ(UART0_INT_IRQn);
        while(1)
    {
        DL_GPIO_clearPins(BAMBOO_GPIO_PORT, BAMBOO_GPIO_LED_GREEN_PIN | BAMBOO_GPIO_LED_RED_PIN);
        delay_cycles(2000000);
        DL_GPIO_setPins(BAMBOO_GPIO_PORT, BAMBOO_GPIO_LED_GREEN_PIN | BAMBOO_GPIO_LED_RED_PIN);
        delay_cycles(2000000);
    }
#endif

#else
    SYSCFG_DL_init();

    DL_GPIO_clearPins(BAMBOO_GPIO_PORT, BAMBOO_GPIO_LED_GREEN_PIN | BAMBOO_GPIO_LED_RED_PIN);

    //DL_GPIO_clearPins(BAMBOO_GPIO_PORT, BAMBOO_GPIO_LED_GREEN_PIN | BAMBOO_GPIO_LED_RED_PIN |
    //                  BAMBOO_GPIO_CH_EN_PIN | BAMBOO_GPIO_BS_EN_PIN |
    //                  BAMBOO_GPIO_DS_EN_PIN | BAMBOO_GPIO_VCH_PIN);
    DL_UART_enableFIFOs(UART_0_INST);
    DL_GPIO_clearPins(BAMBOO_GPIO_PORT,
                      BAMBOO_GPIO_CH_EN_PIN |
                      BAMBOO_GPIO_VCH_PIN);

    DL_GPIO_setPins(BAMBOO_GPIO_PORT, BAMBOO_GPIO_BS_EN_PIN);

    DL_GPIO_setPins(BAMBOO_GPIO_PORT, BAMBOO_GPIO_CH_EN_PIN);

    DL_GPIO_setPins(BAMBOO_GPIO_PORT, BAMBOO_GPIO_VCH_PIN);

    NVIC_EnableIRQ(UART0_INT_IRQn);

//ADC config
    NVIC_EnableIRQ(ADC12_0_INST_INT_IRQN);
    gCheckADC = false;

    /* Timer setup */
    NVIC_EnableIRQ(TIMER_0_INST_INT_IRQN);

    DL_TimerA_startCounter(TIMER_0_INST);

    SET_BIT(g_u8SysFlags, SYS_FLAG_SLEEP);

    //Init_LED_script();

    //DL_GPIO_setPins(BAMBOO_GPIO_PORT, BAMBOO_GPIO_LED_GREEN_PIN | BAMBOO_GPIO_LED_RED_PIN);

    /* If write and read were successful, toggle LED */
    while (1) {
        OneHundredMsHnadler();
        OneSecondHandler();
//        DL_SYSCTL_resetDevice(SYSCTL_RESETLEVEL_LEVEL_POR);
#if 0
        delay_cycles(12000000);
        if(test_revert)
        {
            test_revert = 0;
            DL_GPIO_clearPins(BAMBOO_GPIO_PORT, BAMBOO_GPIO_LED_GREEN_PIN | BAMBOO_GPIO_LED_RED_PIN);
        }
        else
        {
            test_revert = 1;
            DL_GPIO_setPins(BAMBOO_GPIO_PORT, BAMBOO_GPIO_LED_GREEN_PIN | BAMBOO_GPIO_LED_RED_PIN);
        }
#endif
    }
#endif
}

static void polling_all_adc(void){
    static uint8_t i = 0;

// Read multi-ADC channel
    //configure the former 4 channels
    ADC12_0_INST->ULLMEM.MEMCTL[0] = 0x00000000;    //A0-VBAT (PA27)
    ADC12_0_INST->ULLMEM.MEMCTL[1] = 0x00000001;    //A1-IOUT (PA26)
    ADC12_0_INST->ULLMEM.MEMCTL[2] = 0x00000002;    //A2-TS (PA25)
    ADC12_0_INST->ULLMEM.MEMCTL[3] = 0x00000003;    //A3-CS (PA24)

    DL_ADC12_enableConversions(ADC12_0_INST);
    DL_ADC12_startConversion(ADC12_0_INST);

    while (false == gCheckADC) {
        __WFE();
    }
    DL_ADC12_disableConversions(ADC12_0_INST);

    gADC_PA27_A0_BAT[i] = DL_ADC12_getMemResult(ADC12_0_INST, DL_ADC12_MEM_IDX_0);
    gADC_PA26_A1_IOUT[i] = DL_ADC12_getMemResult(ADC12_0_INST, DL_ADC12_MEM_IDX_1);
    gADC_PA25_A2_TS[i] = DL_ADC12_getMemResult(ADC12_0_INST, DL_ADC12_MEM_IDX_2);
    gADC_PA24_A3_DC[i] = DL_ADC12_getMemResult(ADC12_0_INST, DL_ADC12_MEM_IDX_3);
    gCheckADC = false;

    //configure the later 1 channels
    //ADC12_0_INST->ULLMEM.MEMCTL[0] = 0x00000005;    //A4-DC (PA22)
    ADC12_0_INST->ULLMEM.MEMCTL[1] = 0x00000008;    //A8-BUT (PA16)

    DL_ADC12_enableConversions(ADC12_0_INST);
    DL_ADC12_startConversion(ADC12_0_INST);

    while (false == gCheckADC) {
        __WFE();
    }

    //gADC_PA22_A5_VCH[i] = DL_ADC12_getMemResult(ADC12_0_INST, DL_ADC12_MEM_IDX_0);
    gADC_PA16_A8_MCU[i] = DL_ADC12_getMemResult(ADC12_0_INST, DL_ADC12_MEM_IDX_1);
    gCheckADC = false;

    i++;
    if (i >= RESULT_SIZE) {
//            __BKPT(0);
        i = 0;
    }

    DL_ADC12_disableConversions(ADC12_0_INST);
}

void disable_charger(void){
    write_charge_reg(BQ24725A_REG_12H_RW_CHARGE_OPTION, CHARGE_OPTION_DISABLE_CHARGE_TEST_7821H);
    delay_cycles(12000);

    if(IS_BIT_SET(g_u8SystemFlags, FLAG_CHARGING)){
        CLR_BIT(g_u8SystemFlags, FLAG_CHARGING);
        //Reload_LED_script();
    }

    CLR_BIT(g_u8SystemFlags, FLAG_CHARGING);

    CLR_BIT(g_u8SystemState, SYS_STAT_CHARGER_ENABLE);
    CLR_BIT(g_u8SystemState, SYS_STAT_CHARGER_INIT);

    Disable_bat_mos();
    //DL_GPIO_clearPins(BAMBOO_GPIO_PORT, BAMBOO_GPIO_CH_EN_PIN);
    //DL_GPIO_clearPins(BAMBOO_GPIO_PORT, BAMBOO_GPIO_BS_EN_PIN);
}

void update_charge_current_by_VBAT(void)
{
    if(IS_BIT_SET(g_u8SystemState, SYS_STAT_FAST_CHARGE))
        write_charge_reg(BQ24725A_REG_14H_RW_CHARGE_CURRENT, CHARGE_CURRENT_2048mA);
    else
        write_charge_reg(BQ24725A_REG_14H_RW_CHARGE_CURRENT, CHARGE_CURRENT_640mA);
    delay_cycles(120000);
}

void Charger_init(void)
{
    read_charge_reg(BQ24725A_REG_FFH_RO_DEVICE_ID);

    read_charge_reg(BQ24725A_REG_FEH_RO_MANUFACTORY_ID);

    read_charge_reg(BQ24725A_REG_12H_RW_CHARGE_OPTION);

    write_charge_reg(BQ24725A_REG_12H_RW_CHARGE_OPTION, CHARGE_OPTION_DISABLE_CHARGE_TEST_7821H);
    delay_cycles(120000);

    update_charge_current_by_VBAT();

    write_charge_reg(BQ24725A_REG_15H_RW_CHARGE_VOLTAGE, CHARGE_VOLTAGE_4096mV);
    delay_cycles(120000);

    write_charge_reg(BQ24725A_REG_3FH_RW_INPUT_CURRENT, CHARGE_CURRENT_2048mA);
    delay_cycles(120000);

    write_charge_reg(BQ24725A_REG_12H_RW_CHARGE_OPTION, CHARGE_OPTION_ENABLE_CHARGE);
    delay_cycles(120000);

    read_charge_reg(BQ24725A_REG_12H_RW_CHARGE_OPTION);
    delay_cycles(120000);
}

int check = 0;
void Charger_control(void){
    if(IS_BIT_SET(g_u8SystemState, SYS_STAT_AC_OK) && IS_BIT_CLR(g_u8SystemFlags, FLAG_CAPACITY_FULL) && 
       IS_BIT_CLR(g_u8SystemState, SYS_STAT_CHARGER_ENABLE) && IS_BIT_CLR(g_u8SystemErrors, SYS_ERROR_OCP) && 
       IS_BIT_CLR(g_u8SystemErrors, SYS_ERROR_BATTERY_OVP) && IS_BIT_CLR(g_u8SystemErrors, SYS_ERROR_CHG_OTP) && IS_BIT_CLR(g_u8SystemErrors, SYS_ERROR_CHG_UTP) &&
       IS_BIT_CLR(g_u8SystemErrors, SYS_ERROR_TIME) && IS_BIT_CLR(g_u8SystemState, SYS_STAT_IDLE) &&
       IS_BIT_SET(g_u8SystemState, SYS_STAT_BAT_IN)
       ){
    	Enable_bat_mos();

        read_charge_reg(BQ24725A_REG_FFH_RO_DEVICE_ID);

        read_charge_reg(BQ24725A_REG_FEH_RO_MANUFACTORY_ID);

        read_charge_reg(BQ24725A_REG_12H_RW_CHARGE_OPTION);

        SET_BIT(g_u8SystemState, SYS_STAT_CHARGER_ENABLE);
    }

   if(IS_BIT_SET(g_u8SystemState, SYS_STAT_AC_OK) && IS_BIT_CLR(g_u8SystemFlags, FLAG_CAPACITY_FULL) && 
      IS_BIT_SET(g_u8SystemState, SYS_STAT_CHARGER_ENABLE) && IS_BIT_CLR(g_u8SystemErrors, SYS_ERROR_TIME) &&
      IS_BIT_CLR(g_u8SystemErrors, SYS_ERROR_BATTERY_OVP) && IS_BIT_CLR(g_u8SystemErrors, SYS_ERROR_CHG_OTP) && IS_BIT_CLR(g_u8SystemErrors, SYS_ERROR_CHG_UTP) &&
      IS_BIT_CLR(g_u8SystemErrors, SYS_ERROR_OCP) && IS_BIT_CLR(g_u8SystemState, SYS_STAT_IDLE) &&
      IS_BIT_SET(g_u8SystemState, SYS_STAT_BAT_IN)
      ){
        if(IS_BIT_CLR(g_u8SystemState, SYS_STAT_CHARGER_INIT)){
            check= 1;
            SET_BIT(g_u8SystemState, SYS_STAT_CHARGER_INIT);

            write_charge_reg(BQ24725A_REG_12H_RW_CHARGE_OPTION, CHARGE_OPTION_DISABLE_CHARGE_TEST_7821H);
            delay_cycles(120000);

            // write_charge_reg(BQ24725A_REG_14H_RW_CHARGE_CURRENT, CHARGE_CURRENT_704mA);
            // delay_cycles(12000);
            update_charge_current_by_VBAT();

            write_charge_reg(BQ24725A_REG_15H_RW_CHARGE_VOLTAGE, CHARGE_VOLTAGE_4096mV);
            delay_cycles(120000);

            write_charge_reg(BQ24725A_REG_3FH_RW_INPUT_CURRENT, CHARGE_CURRENT_2048mA);
            delay_cycles(120000);

            write_charge_reg(BQ24725A_REG_12H_RW_CHARGE_OPTION, CHARGE_OPTION_ENABLE_CHARGE);
            delay_cycles(120000);

            read_charge_reg(BQ24725A_REG_12H_RW_CHARGE_OPTION);
            delay_cycles(120000);
        }

        // write_charge_reg(BQ24725A_REG_15H_RW_CHARGE_VOLTAGE, CHARGE_VOLTAGE_8192mV);    // Continuous write to reset WDT
        // delay_cycles(120000);
        //while(1);


        // DL_GPIO_setPins(BAMBOO_GPIO_PORT, BAMBOO_GPIO_LED_GREEN_PIN | BAMBOO_GPIO_LED_RED_PIN);
//            __BKPT(0);
    }

    if(IS_BIT_SET(g_u8SystemFlags, FLAG_CAPACITY_FULL) && IS_BIT_SET(g_u8SystemFlags, FLAG_CHARGING)){
        disable_charger();
    }
}

void system_status_check(void){

#if 0
    // discharge OTP
    if (IS_BIT_CLR(g_u8SystemState, SYS_STAT_AC_OK) && IS_BIT_CLR(g_u8SystemErrors, SYS_ERROR_DCHG_OTP) && StateDebounce((Average_TS_value < TS_DCHG_OTP_60_DEGREE_0P431V), 3 ,&g_u8DCHGOTPCntr)){
        SET_BIT(g_u8SystemErrors, SYS_ERROR_DCHG_OTP);
        //DL_GPIO_clearPins(BAMBOO_GPIO_PORT, BAMBOO_GPIO_DS_EN_PIN | BAMBOO_GPIO_BS_EN_PIN);
    } else if(IS_BIT_CLR(g_u8SystemState, SYS_STAT_AC_OK) && IS_BIT_SET(g_u8SystemErrors, SYS_ERROR_DCHG_OTP) && StateDebounce((Average_TS_value > TS_DCHG_OTP_RELEASE_55_DEGREE_0P496V), 3 ,&g_u8DCHGOTPReleaseCntr)){
        CLR_BIT(g_u8SystemErrors, SYS_ERROR_DCHG_OTP);
    }
#endif
    // charge OTP
    if(IS_BIT_SET(g_u8SystemFlags, FLAG_CHARGING) && IS_BIT_CLR(g_u8SystemErrors, SYS_ERROR_CHG_OTP) &&
        StateDebounce((Average_TS_value < TS_DCHG_OTP_60_DEGREE_0P431V), 3 ,&g_u8CHGOTPCntr))
    {
        SET_BIT(g_u8SystemErrors, SYS_ERROR_CHG_OTP);
        disable_charger();
    } else
    if(IS_BIT_SET(g_u8SystemFlags, FLAG_CHARGING) && IS_BIT_SET(g_u8SystemErrors, SYS_ERROR_CHG_OTP) &&
        StateDebounce((Average_TS_value > TS_CHG_OTP_RELEASE_40_DEGREE_0P744V), 3 ,&g_u8CHGOTPReleaseCntr))
    {
        CLR_BIT(g_u8SystemErrors, SYS_ERROR_CHG_OTP);
    }
    // charge UTP
    if(IS_BIT_SET(g_u8SystemFlags, FLAG_CHARGING) && IS_BIT_CLR(g_u8SystemErrors, SYS_ERROR_CHG_UTP) &&
        StateDebounce((Average_TS_value > TS_CHG_UTP_0_DEGREE), 3 ,&g_u8CHGUTPCntr))
    {
        SET_BIT(g_u8SystemErrors, SYS_ERROR_CHG_UTP);
        disable_charger();
    } else
    if(IS_BIT_SET(g_u8SystemFlags, FLAG_CHARGING) && IS_BIT_SET(g_u8SystemErrors, SYS_ERROR_CHG_UTP) &&
        StateDebounce((Average_TS_value < TS_CHG_UTP_RELEASE_5_DEGREE), 3 ,&g_u8CHGUTPReleaseCntr))
    {
        CLR_BIT(g_u8SystemErrors, SYS_ERROR_CHG_UTP);
    }

    // bat in OTP
    if(IS_BIT_SET(g_u8SystemFlags, FLAG_CHARGING) && IS_BIT_CLR(g_u8SystemErrors, SYS_ERROR_BAT_IN_OTP) &&
        StateDebounce((Average_TS_value > TS_CHG_OTP_45_DEGREE_0P650V), 3 ,&g_u8CHGUTPCntr) && u8_bat_first_in == 1)
    {
        SET_BIT(g_u8SystemErrors, SYS_ERROR_BAT_IN_OTP);
        u8_bat_first_in = 0;
        disable_charger();
    } else
    if(IS_BIT_SET(g_u8SystemFlags, FLAG_CHARGING) && IS_BIT_SET(g_u8SystemErrors, SYS_ERROR_BAT_IN_OTP) &&
        StateDebounce((Average_TS_value < TS_CHG_OTP_45_DEGREE_0P650V-10), 3 ,&g_u8CHGUTPReleaseCntr))
    {
        CLR_BIT(g_u8SystemErrors, SYS_ERROR_BAT_IN_OTP);
    }
    else 
    {
        bat_in_count = (bat_in_count>5)?6:(bat_in_count+1);
        if(bat_in_count >= 5)
        {
            u8_bat_first_in = 0;
        }
    }

    // charge fast timeout
    if(IS_BIT_SET(g_u8SystemFlags, FLAG_CHARGING) && IS_BIT_SET(g_u8SystemState, SYS_STAT_FAST_CHARGE))
    {
        timeout[0]++;
        if(timeout[0] > FAST_TIMEOUT)
        {
            SET_BIT(g_u8SystemErrors, SYS_ERROR_TIME);
            disable_charger();
        }
    }
    else if(IS_BIT_SET(g_u8SystemFlags, FLAG_CHARGING))
    {
        timeout[1]++;
        if(timeout[1] > PRE_TIMEOUT)
        {
            SET_BIT(g_u8SystemErrors, SYS_ERROR_TIME);
            disable_charger();
        }
    }

    // OCP
#if 1
    if(StateDebounce((Average_CS_value >= CS_OCP_0P09V_RELEASE), 3 ,&g_u8OCPCntr)){
        SET_BIT(g_u8SystemErrors, SYS_ERROR_OCP);
        disable_charger();
        


    } else if(IS_BIT_SET(g_u8SystemErrors, SYS_ERROR_OCP) && StateDebounce((Average_CS_value < CS_OCP_0P09V_RELEASE), 3 ,&g_u8OCPReleaseCntr)){
        //CLR_BIT(g_u8SystemErrors, SYS_ERROR_OCP);
        //CLR_BIT(g_u8SystemState, SYS_STAT_AC_OK);

        //if(IS_BIT_SET(g_u8SystemState, SYS_STAT_CUTOFF) || IS_BIT_SET(g_u8SystemErrors, SYS_ERROR_DCHG_OTP))
        //    DL_GPIO_clearPins(BAMBOO_GPIO_PORT, BAMBOO_GPIO_BS_EN_PIN);
        //else{
        //    if(IS_BIT_CLR(g_u8SystemState, SYS_STAT_AC_OK))
        //        DL_GPIO_setPins(BAMBOO_GPIO_PORT, BAMBOO_GPIO_BS_EN_PIN);
        }

#endif
#if 0
    if(IS_BIT_CLR(g_u8SystemState, SYS_STAT_IDLE) && IS_BIT_CLR(g_u8SystemState, SYS_STAT_AC_OK) && StateDebounce(Average_CS_value <= CS_SYSTEM_AWAY_0P06V, 10 ,&g_u8IdleCntr)){
        SET_BIT(g_u8SystemState, SYS_STAT_IDLE);
        // Wake up
        DL_GPIO_clearPins(BAMBOO_GPIO_PORT, BAMBOO_GPIO_BS_EN_PIN);

// Sleep mode ... PK 20241015+
    //    DL_TimerA_reset(TIMER_0_INST);
    //    DL_TimerA_disablePower(TIMER_0_INST);

    //    DL_I2C_reset(I2C_INST);
    //    DL_I2C_disablePower(I2C_INST);

    //    DL_ADC12_reset(ADC12_0_INST);
    //    DL_ADC12_disablePower(ADC12_0_INST);

    //    DL_SYSCTL_setPowerPolicySTANDBY0();

    //    __WFE();

    //    DL_SYSCTL_resetDevice(SYSCTL_RESETLEVEL_LEVEL_POR);

    // } else if(IS_BIT_CLR(g_u8SystemState, SYS_STAT_IDLE) && IS_BIT_SET(g_u8SystemState, SYS_STAT_AC_OK) || StateDebounce(Average_CS_value > CS_SYSTEM_AWAY_0P06V, 1 ,&g_u8IdleReleaseCntr)){
    } else if(IS_BIT_SET(g_u8SystemState, SYS_STAT_IDLE) && IS_BIT_SET(g_u8SystemState, SYS_STAT_AC_OK)){
        CLR_BIT(g_u8SystemState, SYS_STAT_IDLE);
        
        // Ready to sleep
        // DL_GPIO_setPins(BAMBOO_GPIO_PORT, BAMBOO_GPIO_DS_EN_PIN | BAMBOO_GPIO_BS_EN_PIN);        
    }
#endif
}

void update_battery_capacity(void){
#ifdef BAT_CHECK_IN_ORI
    static char bat_first_in = 0;
    static char count_in = 0;
    static char count_out = 0;
#endif
    // Battery OVP
    if(IS_BIT_SET(g_u8SystemFlags, FLAG_CHARGING) && IS_BIT_CLR(g_u8SystemErrors, SYS_ERROR_BATTERY_OVP) && StateDebounce((Average_BAT_value > VBAT_OVP_1P708V), 30 ,&g_u8BatteryOVPCntr)){
        test_index = 1;
        SET_BIT(g_u8SystemErrors, SYS_ERROR_BATTERY_OVP);
        disable_charger();
    } else if(IS_BIT_SET(g_u8SystemErrors, SYS_ERROR_BATTERY_OVP) && StateDebounce((Average_BAT_value <= VBAT_OVP_RELEASE_1P620V), 3 ,&g_u8BatteryOVPReleaseCntr)){
        //CLR_BIT(g_u8SystemErrors, SYS_ERROR_BATTERY_OVP);
    }
#ifdef BAT_CHECK_IN_ORI
    if((Average_TS_value > TS_AC_IN) && (Average_TS_value < TS_CHG_UTP_0_DEGREE))
    {
        count_in = (count_in<10)?count_in+1:10;
        count_out = 0;
        if(count_in > 5)
        {
            SET_BIT(g_u8SystemState, SYS_STAT_BAT_IN);


            if(bat_first_in == 0)
            {
                bat_first_in = 1;
                DL_GPIO_setPins(BAMBOO_GPIO_PORT, BAMBOO_GPIO_CH_EN_PIN);
                DL_GPIO_setPins(BAMBOO_GPIO_PORT, BAMBOO_GPIO_VCH_PIN);

            }
        }

    }
    else
    {
        count_in = 0;
        count_out = (count_out<10)?count_out+1:10;
        if(count_out > 3)
        {
            if(IS_BIT_SET(g_u8SystemState, SYS_STAT_BAT_IN))
            {
                bat_first_in = 0;
                g_u8HIGHVALUECntr = 0;
                g_u8SystemErrors = 0;
                CLR_BIT(g_u8SystemState, SYS_STAT_FAST_CHARGE); // pre charge
                update_charge_current_by_VBAT();

                disable_charger();
            }
            CLR_BIT(g_u8SystemState, SYS_STAT_BAT_IN);
            g_u8FastChargeCntr = 0;
            g_u8FastChargeReleaseCntr = 0;
            g_u8BatteryBATIMAXCntr = 0;
            g_u8BatteryBATIMINCntr = 0;

            CLR_BIT(g_u8SystemState, SYS_STAT_AC_OK);
            CLR_BIT(g_u8SystemState, SYS_STAT_CHARGER_ENABLE);
            CLR_BIT(g_u8SystemState, SYS_STAT_CHARGER_INIT);
            CLR_BIT(g_u8SystemFlags, FLAG_CAPACITY_FULL);
            CLR_BIT(g_u8SystemFlags, FLAG_CHARGING);
            // disable_charger();
            DL_GPIO_clearPins(BAMBOO_GPIO_PORT, BAMBOO_GPIO_CH_EN_PIN | BAMBOO_GPIO_ACOK_PIN);

            DL_GPIO_clearPins(BAMBOO_GPIO_PORT, BAMBOO_GPIO_VCH_PIN);
        }

    }
#endif
    /*
    // Battery UVP
    if(IS_BIT_CLR(g_u8SystemErrors, SYS_ERROR_BATTERY_UVP) && StateDebounce((Average_BAT_value < VBAT_UVP_0P999V), 3 ,&g_u8BatteryUVPCntr)){
        SET_BIT(g_u8SystemErrors, SYS_ERROR_BATTERY_UVP);
    } else if(IS_BIT_SET(g_u8SystemErrors, SYS_ERROR_BATTERY_UVP) && StateDebounce((Average_BAT_value >= VBAT_UVP_RELEASE_1P195V), 3 ,&g_u8BatteryUVPReleaseCntr)){
        CLR_BIT(g_u8SystemErrors, SYS_ERROR_BATTERY_UVP);
    }        
    
    // Battery SVP
    if(IS_BIT_CLR(g_u8SystemErrors, SYS_ERROR_SVP) && StateDebounce((Average_BAT_value < VBAT_SVP_0P799V), 10 ,&g_u8SVPCntr)){
    //    if(PK_XD == 1){
        SET_BIT(g_u8SystemErrors, SYS_ERROR_SVP);
        // disable_charger();
    //    }
    } else if(IS_BIT_SET(g_u8SystemErrors, SYS_ERROR_BATTERY_UVP) && StateDebounce((Average_BAT_value >= VBAT_SVP_0P799V), 3 ,&g_u8SVPReleaseCntr)){
        CLR_BIT(g_u8SystemErrors, SYS_ERROR_SVP);
    }  
    */
    // Fast charge or not
    if(IS_BIT_SET(g_u8SystemFlags, FLAG_CHARGING)){
        if(StateDebounce((Average_BAT_value >= VBAT_SVP_0P300V_IN_FAST), 2 ,&g_u8FastChargeCntr) && IS_BIT_CLR(g_u8SystemState, SYS_STAT_FAST_CHARGE)){
            SET_BIT(g_u8SystemState, SYS_STAT_FAST_CHARGE); // fast charge
            update_charge_current_by_VBAT();
            g_u8FastChargeReleaseCntr = 0;
        } else if(IS_BIT_SET(g_u8SystemState, SYS_STAT_FAST_CHARGE) && StateDebounce((Average_BAT_value < VBAT_SVP_0P300V_IN_FAST ), 2 ,&g_u8FastChargeReleaseCntr)){
            CLR_BIT(g_u8SystemState, SYS_STAT_FAST_CHARGE); // pre charge
            update_charge_current_by_VBAT();
            g_u8FastChargeCntr = 0;
        }  
    }
#if 0
    // Cutoff or not
    if(IS_BIT_CLR(g_u8SystemState, SYS_STAT_CUTOFF) && StateDebounce((Average_BAT_value < VBAT_CUTOFF_POINT_0P999V), 2 ,&g_u8CutoffCntr)){
        SET_BIT(g_u8SystemState, SYS_STAT_CUTOFF);
        //DL_GPIO_clearPins(BAMBOO_GPIO_PORT, BAMBOO_GPIO_DS_EN_PIN | BAMBOO_GPIO_BS_EN_PIN);
    }

    else if(StateDebounce(IS_BIT_SET(g_u8SystemState, SYS_STAT_CUTOFF) && IS_BIT_SET(g_u8SystemState, SYS_STAT_AC_OK) &&
                            (Average_BAT_value >= VBAT_CUTOFF_POINT_0P999V), 2 ,&g_u8CutoffReleaseCntr)){
        CLR_BIT(g_u8SystemState, SYS_STAT_CUTOFF);

        DL_GPIO_setPins(BAMBOO_GPIO_PORT, BAMBOO_GPIO_BS_EN_PIN);
    }
#endif

    if(IS_BIT_CLR(g_u8SystemErrors, SYS_ERROR_BATTERY_OVP) && IS_BIT_CLR(g_u8SystemErrors, SYS_ERROR_BATTERY_UVP))
    {
#if 0   //Gary
        // if(IS_BIT_CLR(g_u8SystemFlags, FLAG_CAPACITY_UNDER_25_PERCENTAGE) && StateDebounce(Average_BAT_value <= VBAT_25_PERCENTAGE_CAPACITY_1P334V, 3, &g_u8CapacityUnder25percentageCntr)){
        if(StateDebounce(Average_BAT_value <= VBAT_25_PERCENTAGE_CAPACITY_1P334V, 3, &g_u8CapacityUnder25percentageCntr)){
            SET_BIT(g_u8SystemFlags, FLAG_CAPACITY_UNDER_25_PERCENTAGE);
            CLR_BIT(g_u8SystemFlags, FLAG_CAPACITY_OVER_25_PERCENTAG);
            CLR_BIT(g_u8SystemFlags, FLAG_CAPACITY_FULL);
            CLR_BIT(g_u8SystemErrors, SYS_ERROR_BATTERY_OVP);
            CLR_BIT(g_u8SystemErrors, SYS_ERROR_BATTERY_UVP);
    //        __BKPT(0);
        // } else if(IS_BIT_CLR(g_u8SystemFlags, FLAG_CAPACITY_FULL) && IS_BIT_SET(g_u8SystemFlags, FLAG_CHARGING) &&
        //                     StateDebounce((Average_BAT_value >= VBAT_FULL_CAPACITY_1P595V && Average_IOUT_value <= IOUT_TYPE_CURRENT_200mA), 3, &g_u8CapacityFullCntr)){
        } 

        if(StateDebounce( IS_BIT_CLR(g_u8SystemFlags, FLAG_CAPACITY_FULL) && Average_BAT_value > VBAT_25_PERCENTAGE_CAPACITY_1P334V , 3, &g_u8CapacityOver25percentageCntr)){
            CLR_BIT(g_u8SystemFlags, FLAG_CAPACITY_UNDER_25_PERCENTAGE);
            SET_BIT(g_u8SystemFlags, FLAG_CAPACITY_OVER_25_PERCENTAG);
            CLR_BIT(g_u8SystemFlags, FLAG_CAPACITY_FULL);
            CLR_BIT(g_u8SystemErrors, SYS_ERROR_BATTERY_OVP);
            CLR_BIT(g_u8SystemErrors, SYS_ERROR_BATTERY_UVP);
    //        __BKPT(0);
        } 
#endif
        if(IS_BIT_SET(g_u8SystemFlags, FLAG_CHARGING) &&
                            StateDebounce((Average_BAT_value >= VBAT_FULL_CAPACITY_1P595V && Average_IOUT_value <= IOUT_TYPE_CURRENT_100mA), 10, &g_u8CapacityFullCntr)){
            SET_BIT(g_u8SystemFlags, FLAG_CAPACITY_FULL);
        } 
        else if(IS_BIT_SET(g_u8SystemFlags, FLAG_CAPACITY_FULL) &&
                StateDebounce((Average_BAT_value < VBAT_FULL_RELEASE_CAPACITY_1P556V), 5, &g_u8RechargeCntr))
        {
            CLR_BIT(g_u8SystemFlags, FLAG_CAPACITY_FULL);
        }
    }
}
uint32_t addr_start = 0x3f00;
void fun_uart_test()
{
    char rx_byte;
    static char rx_pair_count = 0;
    char status[4] = {0};
    char result;
    char Chk_update_flash;

    uint32_t newAppStackPointer;
    //DL_UART_transmitData(UART0, 'a');
    if(DL_UART_isRXFIFOEmpty(UART_0_INST) == 0)
    {
        rx_byte = DL_UART_receiveData(UART_0_INST);
        if (rx_byte == MCU_ID+0x30) {
            rx_pair_count++;
        }
        else
        {
            rx_pair_count = 0;
        }
        if(rx_pair_count >= 3)
        {
            status[0] = 1;
            status[1] = MCU_ID;

            DL_FlashCTL_unprotectAllMemory(FLASHCTL);

            DL_CORE_configInstruction(CPUSS_CTL_ICACHE_DISABLE,
                CPUSS_CTL_PREFETCH_DISABLE, CPUSS_CTL_LITEN_DISABLE);
            //while(result == 0 && bsl_retry < 3)
            {
                result = DL_FlashCTL_programMemoryBlocking(FLASHCTL, 0x3ffc,
                                        (uint32_t *) &status[0],
                                        1,
                                        DL_FLASHCTL_REGION_SELECT_MAIN);
            }
            Chk_update_flash = (*((volatile uint8_t *) (0x3ff8)));

            DL_CORE_configInstruction(CPUSS_CTL_ICACHE_ENABLE,
                CPUSS_CTL_PREFETCH_ENABLE, CPUSS_CTL_LITEN_ENABLE);

            DL_UART_transmitData(UART0, MCU_ID+0x30);
            DL_UART_transmitData(UART0, 'o');
            DL_UART_transmitData(UART0, 'k');
            rx_pair_count = 0;

            while(DL_UART_isBusy(UART_0_INST) == 1);
            //jump to bootloader
            NVIC_DisableIRQ(UART0_INT_IRQn);
            NVIC_DisableIRQ(ADC12_0_INST_INT_IRQN);
            /* Timer setup */
            NVIC_DisableIRQ(TIMER_0_INST_INT_IRQN);
            DL_TimerA_stopCounter(TIMER_0_INST);

            SCB->VTOR = BOOTLOADER_MAIN_BASE_ADDRESS;

            newAppStackPointer = *((uint32_t *)(BOOTLOADER_MAIN_BASE_ADDRESS));
            __set_MSP(newAppStackPointer);
            JUMP_TO_BOOTLOADER();

            while ((bool) 1) {

            }

        }
    }
}

char light_g = 0;
int light_syn = 0;
char light_r = 0;
char led_status = 0;


uint32_t w_addr = 0x2000;
uint32_t addr = 0;
void OneSecondHandler(void){
    static char revert = 0;
    if ( IS_BIT_CLR(g_u8SystemFlags, FLAG_1S) )
        return;
#if 0
    if(revert)
    {
        revert = 0;
        DL_GPIO_clearPins(BAMBOO_GPIO_PORT, BAMBOO_GPIO_LED_GREEN_PIN | BAMBOO_GPIO_LED_RED_PIN);
    }
    else
    {
        revert = 1;
        DL_GPIO_setPins(BAMBOO_GPIO_PORT, BAMBOO_GPIO_LED_GREEN_PIN | BAMBOO_GPIO_LED_RED_PIN);
    }
#endif



    //fun_uart_test();
    CLR_BIT(g_u8SystemFlags, FLAG_1S);

    update_battery_capacity();

    system_status_check();
}

char Check_Error()
{
    return 0;
}

char Check_ID(uint16_t count)
{
    if(count <= 483)
    {
        return 1;
    }
    else if(count <= 629)
    {
        return 2;
    }
    else if(count <= 777)
    {
        return 3;
    }
    return 4;
}

void Cal_ADC(void)
{
    uint8_t i = 0;
    // Calculate average BAT value
    /*
    for(i=0;i<RESULT_SIZE;i++){
        if(i==0)
            Average_BAT_value = gADC_PA27_A0_BAT[i];
        else
            Average_BAT_value += gADC_PA27_A0_BAT[i];        
    }
    Average_BAT_value = Average_BAT_value / RESULT_SIZE;
    */
    Average_BAT_value = gADC_PA27_A0_BAT[i];

    // Calculate average IOUT value
    /*
    for(i=0;i<RESULT_SIZE;i++){
        if(i==0)

        else
            Average_IOUT_value += gADC_PA26_A1_IOUT[i];        
    }
    Average_IOUT_value = Average_IOUT_value / RESULT_SIZE;
    */
    Average_IOUT_value = gADC_PA26_A1_IOUT[i];

    // Calculate average DC value
    /*
    for(i=0;i<RESULT_SIZE;i++){
        if(i==0)
            Average_AC_value = gADC_PA24_A3_DC[i];
        else
            Average_AC_value += gADC_PA24_A3_DC[i];
    }
    Average_AC_value = Average_AC_value / RESULT_SIZE;
    */
    Average_AC_value = gADC_PA24_A3_DC[i];

    // Calculate average TS value
    /*
    for(i=0;i<RESULT_SIZE;i++){
        if(i==0)
            Average_TS_value = gADC_PA25_A2_TS[i];
        else
            Average_TS_value += gADC_PA25_A2_TS[i];        
    }
    Average_TS_value = Average_TS_value / RESULT_SIZE;
    */
    Average_TS_value = gADC_PA25_A2_TS[i];

    // Average_TS_value = PK_temp;
    // Calculate average MCU value
    /*
    for(i=0;i<RESULT_SIZE;i++){
        if(i==0)
            Average_BUT_value = gADC_PA16_A8_MCU[i];
        else
            Average_BUT_value += gADC_PA16_A8_MCU[i];
    }
    Average_BUT_value = Average_BUT_value / RESULT_SIZE;
    */
    Average_BUT_value = gADC_PA16_A8_MCU[i];

    MCU_ID = Check_ID(Average_BUT_value);
}

char count_set = 0;
uint8_t addr_round;
void OneHundredMsHnadler(void){
    char status[4] = {0};
    static char record = 0;
    int loc = 0;
    static char error_count = 0;
    static char first_chans = 0;
    static char charge_count[2] = {0};
#ifndef BAT_CHECK_IN_ORI
    static char bat_first_in = 0;
    static char count_in = 0;
    static char count_out = 0;
#endif
    if ( IS_BIT_CLR(g_u8SystemFlags, FLAG_100MS) )
        return;

    CLR_BIT(g_u8SystemFlags, FLAG_100MS);

//    Just for timer stability ... PK 20240923+
//    DL_GPIO_togglePins(BAMBOO_GPIO_PORT, BAMBOO_GPIO_LED_GREEN_PIN);

    polling_all_adc();
    Cal_ADC();

#if 0 //Gary
    if(IS_BIT_SET(g_u8SysFlags, SYS_FLAG_SLEEP) || IS_BIT_SET(g_u8SysFlags, SYS_FLAG_ERROR))
    {
        if(Check_DC_OK(Average_AC_value))
        {
            g_u8SysFlags = 0;
            SET_BIT(g_u8SysFlags, SYS_FLAG_STANDBY);
        }
    }
    if(Check_DC_NG(Average_AC_value))
    {
        g_u8SysFlags = 0;
        SET_BIT(g_u8SysFlags, SYS_FLAG_ERROR);
        CLR_BIT(g_u8ChrgStatusFlags, CHRG_STATUS_FLAG_INIT);
    }
    if(IS_BIT_SET(g_u8SysFlags, SYS_FLAG_STANDBY) && Check_BAT_IN(Average_BAT_value))
    {
        g_u8SysFlags = 0;
        SET_BIT(g_u8SysFlags, SYS_FLAG_CHARGE);
    }
    else if(IS_BIT_SET(g_u8SysFlags, SYS_FLAG_CHARGE))
    {
        if(IS_BIT_CLR(g_u8ChrgFlag, CHRG_FLAG_FULL))
        {
            if(Check_BAT_OUT(Average_BAT_value))
            {
                g_u8SysFlags = 0;
                SET_BIT(g_u8SysFlags, SYS_FLAG_STANDBY);
                CLR_BIT(g_u8ChrgStatusFlags, CHRG_STATUS_FLAG_INIT);
            }
            else if(Check_Error())
            {

            }
            else if(Check_BAT_Full(Average_BAT_value,Average_IOUT_value))
            {
                g_u8ChrgFlag = 0;
                SET_BIT(g_u8SysFlags, CHRG_FLAG_FULL);
            }
            else
            {
                update_charge_current_by_VBAT();
            }
        }
    }

    if(IS_BIT_SET(g_u8SysFlags, SYS_FLAG_ERROR))
    {
        g_u8ChrgStatusFlags = 0;
        g_u8ChrgFlag = 0;
        Disable_bat_mos();
    }
    else if(IS_BIT_SET(g_u8ChrgFlag, CHRG_FLAG_FULL))
    {
        if(Check_BAT_Recharge(Average_BAT_value))
        {
            g_u8SysFlags = 0;
            SET_BIT(g_u8SysFlags, SYS_FLAG_STANDBY);
        }
        disable_charger();
        CLR_BIT(g_u8ChrgStatusFlags, CHRG_STATUS_FLAG_INIT);
    }
    else if(IS_BIT_SET(g_u8SysFlags, SYS_FLAG_STANDBY))
    {
        if(IS_BIT_CLR(g_u8ChrgStatusFlags, CHRG_STATUS_FLAG_INIT))
        {
            SET_BIT(g_u8ChrgStatusFlags, CHRG_STATUS_FLAG_INIT);
            //init
            Charger_init();

            Enable_bat_mos();
        }
    }
    else if(IS_BIT_SET(g_u8SysFlags, SYS_FLAG_CHARGE))
    {
        if(g_u8ChrgStatusFlags & 0xfe)
        {
            disable_charger();
        }
        else
        {

        }
    }

#else
    if( IS_BIT_CLR(g_u8SystemState, SYS_STAT_AC_OK) &&
        StateDebounce((Average_AC_value < DC_OK_HIGH_THRESHOLD_2P946V && Average_AC_value > DC_OK_LOW_THRESHOLD_2P46V), 30, &g_u8ACOKCntr) &&
        IS_BIT_SET(g_u8SystemState, SYS_STAT_BAT_IN)
        ){
    // AC OK...PK 20240919+
        g_u8ACNGCntr = 0;
        SET_BIT(g_u8SystemState, SYS_STAT_AC_OK);
        CLR_BIT(g_u8SystemState, SYS_STAT_CHARGER_ENABLE);
        CLR_BIT(g_u8SystemState, SYS_STAT_CHARGER_INIT);
        CLR_BIT(g_u8SystemErrors, SYS_ERROR_AC_NG);

        if(IS_BIT_SET(g_u8SystemState, SYS_STAT_IDLE))
            CLR_BIT(g_u8SystemState, SYS_STAT_IDLE);

        if(IS_BIT_CLR(g_u8SystemErrors, SYS_ERROR_OCP)){
            //DL_GPIO_clearPins(BAMBOO_GPIO_PORT, BAMBOO_GPIO_DS_EN_PIN);
            DL_GPIO_setPins(BAMBOO_GPIO_PORT, BAMBOO_GPIO_ACOK_PIN);
            delay_cycles(120000); // 0.05sec
            DL_GPIO_setPins(BAMBOO_GPIO_PORT, BAMBOO_GPIO_CH_EN_PIN);

            DL_GPIO_setPins(BAMBOO_GPIO_PORT, BAMBOO_GPIO_VCH_PIN);

            delay_cycles(1200000); // 0.05sec
        }
//                __BKPT(0);
    }
    else if(StateDebounce((Average_AC_value > DC_OK_HIGH_THRESHOLD_2P946V || Average_AC_value < DC_OK_LOW_THRESHOLD_2P46V), 10, &g_u8ACNGCntr)){
    // AC NG...PK 20240919+
        g_u8ACOKCntr = 0;
        CLR_BIT(g_u8SystemState, SYS_STAT_AC_OK);
        CLR_BIT(g_u8SystemState, SYS_STAT_CHARGER_ENABLE);
        CLR_BIT(g_u8SystemState, SYS_STAT_CHARGER_INIT);
        // For customer request ... 20241015+
        if(IS_BIT_SET(g_u8SystemFlags, FLAG_CAPACITY_FULL)){
            //CLR_BIT(g_u8SystemFlags, FLAG_CAPACITY_FULL);
            //Reload_LED_script();
        }

        CLR_BIT(g_u8SystemFlags, FLAG_CAPACITY_FULL);



        if(Average_AC_value > DC_OK_REMOVE_THRESHOLD_1P400V)
            SET_BIT(g_u8SystemErrors, SYS_ERROR_AC_NG);

        // disable_charger();
        DL_GPIO_clearPins(BAMBOO_GPIO_PORT, BAMBOO_GPIO_CH_EN_PIN | BAMBOO_GPIO_ACOK_PIN);

        DL_GPIO_clearPins(BAMBOO_GPIO_PORT, BAMBOO_GPIO_VCH_PIN);
#if 0
        if(IS_BIT_SET(g_u8SystemState, SYS_STAT_CUTOFF) || IS_BIT_SET(g_u8SystemErrors, SYS_ERROR_DCHG_OTP) ||
           IS_BIT_SET(g_u8SystemErrors, SYS_ERROR_OCP)){
            //DL_GPIO_clearPins(BAMBOO_GPIO_PORT, BAMBOO_GPIO_DS_EN_PIN | BAMBOO_GPIO_BS_EN_PIN);
        } else {
            if(IS_BIT_CLR(g_u8SystemState, SYS_STAT_IDLE)){
                //DL_GPIO_setPins(BAMBOO_GPIO_PORT, BAMBOO_GPIO_DS_EN_PIN | BAMBOO_GPIO_BS_EN_PIN);
            }
        }
#endif
//            __BKPT(0);
    }
    // if (IS_BIT_SET(g_u8SystemErrors, SYS_ERROR_OCP)){
    //     CLR_BIT(g_u8SystemState, SYS_STAT_CHARGER_ENABLE);
    //     CLR_BIT(g_u8SystemState, SYS_STAT_CHARGER_INIT);
    //     CLR_BIT(g_u8SystemErrors, SYS_ERROR_AC_NG);

    //     DL_GPIO_clearPins(BAMBOO_GPIO_PORT, BAMBOO_GPIO_DS_EN_PIN | BAMBOO_GPIO_BS_EN_PIN);
    //     DL_GPIO_clearPins(BAMBOO_GPIO_PORT, BAMBOO_GPIO_CH_EN_PIN | BAMBOO_GPIO_ACOK_PIN);    
    // }

    if(IS_BIT_SET(g_u8SystemState, SYS_STAT_AC_OK) &&
       IS_BIT_SET(g_u8SystemState, SYS_STAT_CHARGER_ENABLE) &&
       IS_BIT_SET(g_u8SystemState, SYS_STAT_BAT_IN)  &&
       IS_BIT_CLR(g_u8SystemFlags, FLAG_CAPACITY_FULL))
    {
        if(charge_count[0] > 5)
        {
            SET_BIT(g_u8SystemFlags, FLAG_CHARGING);
        }
        charge_count[0]++;
        charge_count[1] = 0;
    }
    else
    {
        if(charge_count[1] > 200 && Average_BAT_value > VBAT_FULL_CAPACITY_1P595V+1)
        {
            CLR_BIT(g_u8SystemFlags, FLAG_CHARGING);
            SET_BIT(g_u8SystemFlags, FLAG_CAPACITY_FULL);
        }
        charge_count[1] = (charge_count[1]>100)?101:charge_count[1]++;
        charge_count[0] = 0;
    }

#ifndef BAT_CHECK_IN_ORI
    if((Average_TS_value > TS_AC_IN-30) && (Average_TS_value < TS_CHG_UTP_0_DEGREE+30))
    {
        count_in = (count_in<30)?count_in+1:10;
        count_out = 0;
        if(count_in > 10)
        {
            SET_BIT(g_u8SystemState, SYS_STAT_BAT_IN);

            bat_in_temp_check = 0;

            if(bat_first_in == 0)
            {
                func_I2C_RESET();

                charge_count[0] = 0;
                charge_count[1] = 0;

                timeout[0] = 0;
                timeout[1] = 0;

                g_u8BatteryOVPCntr = 0;
                g_u8BatteryOVPReleaseCntr = 0;
                g_u8FastChargeCntr = 0;
                g_u8FastChargeReleaseCntr = 0;
                g_u8CapacityFullCntr = 0;
                g_u8RechargeCntr = 0;
                g_u8CHGOTPCntr = 0;
                g_u8CHGOTPReleaseCntr = 0;
                g_u8CHGUTPCntr = 0;
                g_u8CHGUTPReleaseCntr = 0;

                bat_first_in = 1;
                u8_bat_first_in = 1;
                bat_in_count = 0;
                Enable_bat_mos();

            }
        }

    }
    else
    {
        count_in = 0;
        count_out = (count_out<30)?count_out+1:10;
        if(count_out > 5)
        {
            if(IS_BIT_SET(g_u8SystemState, SYS_STAT_BAT_IN))
            {
                bat_first_in = 0;
                u8_bat_first_in = 0;
                g_u8HIGHVALUECntr = 0;
                g_u8SystemErrors = 0;
                CLR_BIT(g_u8SystemState, SYS_STAT_FAST_CHARGE); // pre charge
                update_charge_current_by_VBAT();

                disable_charger();
            }
            CLR_BIT(g_u8SystemState, SYS_STAT_BAT_IN);
            timeout[0] = 0;
            timeout[1] = 0;
            g_u8BatteryOVPCntr = 0;
            g_u8BatteryOVPReleaseCntr = 0;
            g_u8FastChargeCntr = 0;
            g_u8FastChargeReleaseCntr = 0;
            g_u8CapacityFullCntr = 0;
            g_u8RechargeCntr = 0;
            g_u8CHGOTPCntr = 0;
            g_u8CHGOTPReleaseCntr = 0;
            g_u8CHGUTPCntr = 0;
            g_u8CHGUTPReleaseCntr = 0;
            g_u8BatteryBATIMAXCntr = 0;
            g_u8BatteryBATIMINCntr = 0;

            CLR_BIT(g_u8SystemState, SYS_STAT_AC_OK);
            CLR_BIT(g_u8SystemState, SYS_STAT_CHARGER_ENABLE);
            CLR_BIT(g_u8SystemState, SYS_STAT_CHARGER_INIT);
            CLR_BIT(g_u8SystemFlags, FLAG_CAPACITY_FULL);
            CLR_BIT(g_u8SystemFlags, FLAG_CHARGING);
            // disable_charger();
            DL_GPIO_clearPins(BAMBOO_GPIO_PORT, BAMBOO_GPIO_CH_EN_PIN | BAMBOO_GPIO_ACOK_PIN);

            DL_GPIO_clearPins(BAMBOO_GPIO_PORT, BAMBOO_GPIO_VCH_PIN);
        }

    }
#endif

    Charger_control();
#endif
    //LED_handler();

    if(g_u8SystemErrors)
    {
        error_count = (error_count > 10)?error_count:error_count+1;

    }
    else
    {
        record = 0;
        error_count = 0;
    }

    if(g_u8SystemErrors && (error_count >= 10))
    {
        if(record == 0)
        {
            addr_round = 0;
            while(addr_round != 0xff && addr_start < 0x3fff)
            {
                addr_round = (*((volatile uint8_t *) (addr_start)));
                addr_start += 4;
            }

            status[0] = g_u8SystemErrors&0xff;

            DL_FlashCTL_unprotectAllMemory(FLASHCTL);

            DL_CORE_configInstruction(CPUSS_CTL_ICACHE_DISABLE,
                CPUSS_CTL_PREFETCH_DISABLE, CPUSS_CTL_LITEN_DISABLE);
            //while(result == 0 && bsl_retry < 3)
            {
                DL_FlashCTL_programMemoryBlocking(FLASHCTL, addr_start,
                                        (uint32_t *) &status[0],
                                        1,
                                        DL_FLASHCTL_REGION_SELECT_MAIN);
            }
            //Chk_update_flash = (*((volatile uint8_t *) (0x3ff8)));

            DL_CORE_configInstruction(CPUSS_CTL_ICACHE_ENABLE,
                CPUSS_CTL_PREFETCH_ENABLE, CPUSS_CTL_LITEN_ENABLE);

            record = 1;
        }


        first_chans = 0;
        //ERROR
        DL_GPIO_setPins(BAMBOO_GPIO_PORT, BAMBOO_GPIO_LED_RED_PIN);
#ifdef VERSION_1_0
        led_status = 3;
        if(light_syn%10 <= 5)
        {
            light_g++;
            light_r = 0;
            DL_GPIO_setPins(BAMBOO_GPIO_PORT, BAMBOO_GPIO_LED_GREEN_PIN);
        }
        else if (light_syn%10 <= 10)
        {
            light_r++;
            light_g = 0;
            DL_GPIO_clearPins(BAMBOO_GPIO_PORT, BAMBOO_GPIO_LED_GREEN_PIN);
        }
#else
        DL_GPIO_clearPins(BAMBOO_GPIO_PORT, BAMBOO_GPIO_LED_GREEN_PIN);
#endif
    }
    else

    if(IS_BIT_CLR(g_u8SystemState, SYS_STAT_BAT_IN))
    {
        count_set = 0;
        first_chans = 0;
        //AC OK
        DL_GPIO_setPins(BAMBOO_GPIO_PORT, BAMBOO_GPIO_LED_GREEN_PIN);
#ifdef VERSION_1_0
        led_status = 3;
        if(light_syn%10 <= 5)
        {
            light_g++;
            light_r = 0;
            DL_GPIO_setPins(BAMBOO_GPIO_PORT, BAMBOO_GPIO_LED_RED_PIN);
        }
        else if (light_syn%10 <= 10)
        {
            light_r++;
            light_g = 0;
            DL_GPIO_clearPins(BAMBOO_GPIO_PORT, BAMBOO_GPIO_LED_RED_PIN);
        }
#else
        DL_GPIO_setPins(BAMBOO_GPIO_PORT, BAMBOO_GPIO_LED_RED_PIN);
#endif
    }
    else if(IS_BIT_SET(g_u8SystemFlags, FLAG_CAPACITY_FULL))
    {
        DL_GPIO_setPins(BAMBOO_GPIO_PORT, BAMBOO_GPIO_LED_GREEN_PIN);
        DL_GPIO_clearPins(BAMBOO_GPIO_PORT, BAMBOO_GPIO_LED_RED_PIN);
    }
    else if(IS_BIT_CLR(g_u8SystemFlags, FLAG_CHARGING))
    {
        //BAT IN & NO CHARGING
        first_chans = 0;
        led_status = 1;

#ifndef VERSION_1_1
        if(count_set == 0)
        {
            DL_GPIO_setPins(BAMBOO_GPIO_PORT, BAMBOO_GPIO_LED_GREEN_PIN);
            DL_GPIO_clearPins(BAMBOO_GPIO_PORT, BAMBOO_GPIO_LED_RED_PIN);

            count_set = (count_set>20)?20:count_set+1;
        }
#else
        DL_GPIO_setPins(BAMBOO_GPIO_PORT, BAMBOO_GPIO_LED_GREEN_PIN);
        if(light_syn%10 <= 5)
        {
            light_g++;
            light_r = 0;
            DL_GPIO_setPins(BAMBOO_GPIO_PORT, BAMBOO_GPIO_LED_RED_PIN);
        }
        else if (light_syn%10 <= 10)
        {
            light_r++;
            light_g = 0;
            DL_GPIO_clearPins(BAMBOO_GPIO_PORT, BAMBOO_GPIO_LED_RED_PIN);
        }
#endif
    }
    else
    {
#ifdef VERSION_1_0
        if((Average_TS_value > TS_AC_IN) && (Average_TS_value < TS_CHG_UTP_0_DEGREE))
        {
            if(first_chans == 0)
            {
                first_chans = 1;
                DL_GPIO_setPins(BAMBOO_GPIO_PORT, BAMBOO_GPIO_LED_RED_PIN);
                DL_GPIO_clearPins(BAMBOO_GPIO_PORT, BAMBOO_GPIO_LED_GREEN_PIN);
            }
            //BAT IN & FULL
            if(StateDebounce((Average_BAT_value >= VBAT_HIGH_CAPACITY && Average_IOUT_value <= IOUT_TYPE_CURRENT_1500mA), 10, &g_u8HIGHVALUECntr))
            {
                g_u8HIGHVALUERELEASECntr = 0;
                DL_GPIO_clearPins(BAMBOO_GPIO_PORT, BAMBOO_GPIO_LED_GREEN_PIN);
                DL_GPIO_clearPins(BAMBOO_GPIO_PORT, BAMBOO_GPIO_LED_RED_PIN);
            }
            else if(StateDebounce((Average_BAT_value < VBAT_HIGH_CAPACITY_torlerance && Average_IOUT_value > IOUT_TYPE_CURRENT_1700mA), 10, &g_u8HIGHVALUERELEASECntr))
            {
                g_u8HIGHVALUECntr = 0;
                DL_GPIO_setPins(BAMBOO_GPIO_PORT, BAMBOO_GPIO_LED_RED_PIN);
                DL_GPIO_clearPins(BAMBOO_GPIO_PORT, BAMBOO_GPIO_LED_GREEN_PIN);
            }

            led_status = 2;
        }
#else
        DL_GPIO_setPins(BAMBOO_GPIO_PORT, BAMBOO_GPIO_LED_GREEN_PIN);
        if(light_syn%10 <= 5)
        {
            light_g++;
            light_r = 0;
            DL_GPIO_setPins(BAMBOO_GPIO_PORT, BAMBOO_GPIO_LED_RED_PIN);
        }
        else if (light_syn%10 <= 10)
        {
            light_r++;
            light_g = 0;
            DL_GPIO_clearPins(BAMBOO_GPIO_PORT, BAMBOO_GPIO_LED_RED_PIN);
        }
#endif
    }
}

void TIMER_0_INST_IRQHandler(void)
{
    switch (DL_TimerA_getPendingInterrupt(TIMER_0_INST)) {
        case DL_TIMERA_IIDX_REPEAT_COUNT:

            timer_10ms_count++;
            light_syn++;
            SET_BIT(g_u8SystemFlags, FLAG_100MS);

            if(timer_10ms_count == TIMER_1_SEC_COUNTS){
                SET_BIT(g_u8SystemFlags, FLAG_1S);
                timer_10ms_count = 0;
            }

            if(I2C_timeout_counter != 0)
                I2C_timeout_counter--;

            break;

        default:
            break;
    }
}

int uart_count = 0;
void ADC12_0_INST_IRQHandler(void)
{
    switch (DL_ADC12_getPendingInterrupt(ADC12_0_INST)) {
        case DL_ADC12_IIDX_MEM3_RESULT_LOADED:
            gCheckADC = true;
            break;
        default:
            break;
    }
}

void UART_0_INST_IRQHandler(void)
{
    static uint16_t checksum = 0;
    static uint16_t data_checksum = 0;
    static uint8_t gData = 0;
    switch (DL_UART_Main_getPendingInterrupt(UART_0_INST)) {
        case DL_UART_MAIN_IIDX_RX:
            fun_uart_test();
#if 0
            gData = DL_UART_Main_receiveData(UART_0_INST);
            if (gData == MCU_ID+0x31) {
                uart_count++;
            }
            if(uart_count >= 3)
            {
                DL_UART_transmitData(UART0, 'a');
                DL_UART_transmitData(UART0, 'b');
                DL_UART_transmitData(UART0, 'c');
            }
#endif
            break;
        default:
            break;
    }
}
