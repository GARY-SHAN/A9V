#ifndef __CHARGE_BQ24725A_H__
#define __CHARGE_BQ24725A_H__

typedef enum eCharger_BQ24725A_Cmds
{
    BQ24725A_REG_12H_RW_CHARGE_OPTION 		= 0x12,
    BQ24725A_REG_14H_RW_CHARGE_CURRENT 		= 0x14,
    BQ24725A_REG_15H_RW_CHARGE_VOLTAGE 		= 0x15,    
    BQ24725A_REG_3FH_RW_INPUT_CURRENT 		= 0x3F,     
    BQ24725A_REG_FEH_RO_MANUFACTORY_ID 		= 0xFE,     // fix reply 0x0040
    BQ24725A_REG_FFH_RO_DEVICE_ID 		    = 0xFF,     // fix reply 0x000B
}eCharger_BQ24725A_Cmds_t;

#define CHARGE_OPTION_DISABLE_CHARGE_TEST_7821H     0x7821 //0x7C21 //0x7E21//0x7821  // WatchDog 175sec, Falling Threshold = 70.97% of voltage regulation limit(~2.981V/cell),
                                                            //    IOUT is the 20x charge current amplifier output

// #define CHARGE_OPTION_DISABLE_CHARGE_TEST_7801H     0x7801  // WatchDog 175sec, Falling Threshold = 70.97% of voltage regulation limit(~2.981V/cell), 
                                                            // IOUT is the 20x input current amplifier output

#define CHARGE_OPTION_DISABLE_CHARGE_TEST_4001H     0x4001  // WatchDog 88sec, Falling Threshold = 62.65% of voltage regulation limit(~2.631V/cell)
#define CHARGE_OPTION_ENABLE_CHARGE             0x18A2 //0x1CA2 //0x1EA2//0x18A2      // IOUT is the 20x charge current amplifier output
                                                            //    IOUT is the 20x charge current amplifier output
                                                            // Disable WDT

// #define CHARGE_OPTION_ENABLE_CHARGE             0x7800      // IOUT is the 20x input current amplifier output
                                                            // IOUT is the 20x input current amplifier output
#define CHARGE_CURRENT_2048mA        0x0800
#define CHARGE_CURRENT_1792mA        0x0700
#define CHARGE_CURRENT_1024mA        0x0400
#define CHARGE_CURRENT_576mA         0x0240
#define CHARGE_CURRENT_640mA         0x0280
#define CHARGE_CURRENT_704mA         0x02C0
#define CHARGE_CURRENT_384mA         0x0180
#define CHARGE_CURRENT_320mA         0x0140
#define CHARGE_CURRENT_256mA         0x0100
#define CHARGE_CURRENT_192mA         0x00C0
#define CHARGE_CURRENT_128mA         0x0080

#define CHARGE_VOLTAGE_10496mV       0x2900
#define CHARGE_VOLTAGE_8192mV        0x2000
#define CHARGE_VOLTAGE_4096mV        0x1060

#define INPUT_CURRENT_1024mA         0x0400
void read_charge_reg(uint8_t reg_addr);
void write_charge_reg(uint8_t reg_addr, uint16_t reg_data);

extern volatile uint8_t I2C_timeout_counter;
#endif
