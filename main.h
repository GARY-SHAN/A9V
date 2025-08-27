#ifndef __MAIN_H__
#define __MAIN_H__

#define FW_Version_Major 1
#define FW_Version_Minor 1

//#define BAT_CHECK_IN_ORI
//#define VERSION_1_0
#define VERSION_1_1

#define DC_OK_HIGH_THRESHOLD_2P946V        635 //2.946v (divider) -> 13.5v (Original)
#define DC_OK_LOW_THRESHOLD_2P46V         423 //2.46v (divider) -> 11.36v (Original)
#define DC_OK_REMOVE_THRESHOLD_1P400V     745 //1.400v (divider) -> 11v (Original)

#define VBAT_FULL_CAPACITY_1P595V              254 //1.595v (divider) -> 8v (Original)
#define VBAT_FULL_RELEASE_CAPACITY_1P556V      247 //1.556v (divider) -> 7.8v (Original)
#define VBAT_25_PERCENTAGE_CAPACITY_1P334V     414 //1.334v (divider) -> 6.69v (Original)

#define VBAT_OVP_1P708V             273 //1.708v (divider) -> 8.56v (Original)
#define VBAT_OVP_RELEASE_1P620V     262 //1.620v (divider) -> 8.12v (Original)

// #define VBAT_UVP_0P999V             297 //0.957v (divider) -> 4.8v (Original)
#define VBAT_UVP_0P999V             310 //0.999v (divider) -> 5v (Original)
#define VBAT_UVP_RELEASE_1P195V     371 //1.195v (divider) -> 6v (Original)
#define VBAT_FAST_CHARGE_POINT_1P195V     371 //1.195v (divider) -> 6v (Original)
#define VBAT_CUTOFF_POINT_0P999V    310 //0.999v (divider) -> 5v (Original)
#define VBAT_SVP_0P799V             248 //0.799v (divider) -> 4v (Original)
#define VBAT_SVP_0P280V_PRE    173 //0.999v (divider) -> 5v (Original)

//Gary
#define IOUT_TYPE_CURRENT_100mA      22 // 300mA (23 = 0.069v(0.0032*23) = I * 0.01 ohm * 20) => I = 345mA close to 200mA (small current have 10% ~ 20% tolerance)
#define VBAT_FULL_CAPACITY_1P410V              253 //1.595v (divider) -> 4.1v (Original)
#define VBAT_SVP_0P300V_IN_FAST    185 //0.999v (divider) -> 5v (Original)
#define VBAT_HIGH_CAPACITY          253
#define VBAT_HIGH_CAPACITY_torlerance   235

#define IOUT_TYPE_CURRENT_1700mA      105 // 200mA (23 = 0.069v(0.0032*23) = I * 0.01 ohm * 20) => I = 345mA close to 200mA (small current have 10% ~ 20% tolerance)
#define IOUT_TYPE_CURRENT_1500mA      93 // 200mA (23 = 0.069v(0.0032*23) = I * 0.01 ohm * 20) => I = 345mA close to 200mA (small current have 10% ~ 20% tolerance)
#define IOUT_TYPE_CURRENT_200mA      23 // 200mA (23 = 0.069v(0.0032*23) = I * 0.01 ohm * 20) => I = 345mA close to 200mA (small current have 10% ~ 20% tolerance)
#define IOUT_TYPE_MINIMUM_CHARGING_CURRENT_32mA      5 //150mA (5 = 0.0544v(0.0032*23) = I * 0.01 ohm * 20) => I = 272mA close to 150mA (small current have 10% ~ 20% tolerance)

// charge OTP
#define TS_DCHG_OTP_60_DEGREE_0P431V           186 // 60 degree TH1 resitance : 3021 ohm, 0.250v(divider) -> 3.3v (Original)
#define TS_DCHG_OTP_RELEASE_55_DEGREE_0P496V   218 // 55 degree TH1 resitance : 3538 ohm, 0.279v(divider) -> 3.3v (Original)
// discharge OTP
#define TS_CHG_OTP_45_DEGREE_0P650V            297 // 45 degree TH1 resitance : 4913 ohm, 0.344v(divider) -> 3.3v (Original)
#define TS_CHG_OTP_RELEASE_40_DEGREE_0P744V    343 // 40 degree TH1 resitance : 5828 ohm, 0.381v(divider) -> 3.3v (Original)
// discharge UTP
#define TS_CHG_UTP_0_DEGREE            799 // 0 degree TH1 resitance : 27640 ohm, 0.737v(divider) -> 3.3v (Original)
#define TS_CHG_UTP_RELEASE_5_DEGREE    748 // 5 degree TH1 resitance : 22270 ohm, 0.693v(divider) -> 3.3v (Original)

#define TS_AC_IN    186
// OCP
#define CS_OCP_0P09V                    217 // 1.8A = 0.09V (R=0.05 ohm)
#define CS_OCP_0P09V_RELEASE            155 // 1.8A = 0.09V (R=0.05 ohm)
#define CS_SYSTEM_AWAY_0P06V            1 // 3.3v/1024 * 1(count) / 0.05 (R=0.05 ohm) = 0.06v

#define FAST_TIMEOUT    25200
#define PRE_TIMEOUT    3600

extern vuint8_t g_u8SystemState;
enum eSystemState
{
    SYS_STAT_AC_OK              = MASK(0),
    SYS_STAT_CHARGER_ENABLE     = MASK(1),
    SYS_STAT_CHARGER_INIT       = MASK(2),
    SYS_STAT_FAST_CHARGE        = MASK(3),
    SYS_STAT_CUTOFF             = MASK(4),
    SYS_STAT_IDLE               = MASK(5),
    SYS_STAT_BAT_IN             = MASK(6),
    SYS_STAT_RESERVE_1          = MASK(7)
};

extern vuint8_t g_u8SystemFlags;
enum eSystemFlags
{
    FLAG_100MS                  = MASK(0),
    FLAG_1S                     = MASK(1),
    FLAG_CAPACITY_FULL                  = MASK(2),
    FLAG_CAPACITY_OVER_25_PERCENTAG     = MASK(3),
    FLAG_CAPACITY_UNDER_25_PERCENTAGE   = MASK(4),
    FLAG_CHARGING               = MASK(5),
    FLAG_RESERVE_2              = MASK(6),
    FLAG_RESERVE_1              = MASK(7)
};

#if 1   //Gary
extern vuint8_t g_u8SysFlags;
enum eSysFlags
{
    SYS_FLAG_SLEEP                  = MASK(0),
    SYS_FLAG_STANDBY                = MASK(1),
    SYS_FLAG_ERROR                  = MASK(2),
    SYS_FLAG_CHARGE                 = MASK(3),
    SYS_FLAG_RESERVE_4              = MASK(4),
    SYS_FLAG_RESERVE_3              = MASK(5),
    SYS_FLAG_RESERVE_2              = MASK(6),
    SYS_FLAG_RESERVE_1              = MASK(7)
};

extern vuint8_t g_u8ChrgFlags;
enum eChrgFlags
{
    CHRG_FLAG_ERROR                  = MASK(0),
    CHRG_FLAG_FULL                   = MASK(1),
    CHRG_FLAG_FAST_CHARGE            = MASK(2),
    CHRG_FLAG_PRE_CHARGE             = MASK(3),
    CHRG_FLAG_RESERVE_4              = MASK(4),
    CHRG_FLAG_RESERVE_3              = MASK(5),
    CHRG_FLAG_RESERVE_2              = MASK(6),
    CHRG_FLAG_RESERVE_1              = MASK(7)
};

extern vuint8_t g_u8ChrgSTATUSFlags;
enum eChrgStatusFlags
{
    CHRG_STATUS_FLAG_INIT                   = MASK(0),
    CHRG_STATUS_FLAG_OTP                    = MASK(1),
    CHRG_STATUS_FLAG_UTP                    = MASK(2),
    CHRG_STATUS_FLAG_OVP                    = MASK(3),
    CHRG_STATUS_FLAG_UVP                    = MASK(4),
    CHRG_STATUS_FLAG_OCP                    = MASK(5),
    CHRG_STATUS_FLAG_RESERVE_2              = MASK(6),
    CHRG_STATUS_FLAG_RESERVE_1              = MASK(7)
};
#endif
extern vuint8_t g_u8SystemErrors;
enum eSystemErrors
{
    SYS_ERROR_AC_NG             = MASK(0),
    SYS_ERROR_BATTERY_OVP       = MASK(1),
    SYS_ERROR_BATTERY_UVP       = MASK(2),
    SYS_ERROR_BAT_IN_OTP        = MASK(3),
    SYS_ERROR_CHG_OTP           = MASK(4),
    SYS_ERROR_OCP               = MASK(5),
    SYS_ERROR_TIME              = MASK(6),
    SYS_ERROR_CHG_UTP           = MASK(7)
};

#endif
