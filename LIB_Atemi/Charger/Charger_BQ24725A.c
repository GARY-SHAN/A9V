#include "ti_msp_dl_config.h"

#include "Charger_BQ24725A.h"
#include "defs.h"
#include "main.h"

typedef enum eCharger_BQ24725A_Lens
{
    BQ24725A_TX_READ_FRAME_LEN 		= 0x01,
    BQ24725A_TX_WRITE_FRAME_LEN     = 0x03,
    BQ24725A_RX_READ_LEN 		    = 0x02,
}eCharger_BQ24725A_Lens_t;

/* Data sent to the Target */
uint8_t gTxPacket[BQ24725A_TX_WRITE_FRAME_LEN] = {0x00};

/* Data received from Target */
uint8_t gRxPacket[BQ24725A_RX_READ_LEN];

/* I2C Target address */
#define CHARGER_BQ24725A_I2C_8_BITS_ADDR (0x09)

// I2C timeout detection...PK20240924+
#define I2C_TIMEOUT_VALUE_2S    20
volatile uint8_t I2C_timeout_counter = 0;

volatile uint8_t I2C_Timeout = 0;
void read_charge_reg(uint8_t reg_addr){
// Send I2C command
    I2C_Timeout = 0;

    // I2C timeout detection...PK20240924+
    I2C_timeout_counter = I2C_TIMEOUT_VALUE_2S;

    gTxPacket[0] = reg_addr;

    DL_I2C_fillControllerTXFIFO(I2C_INST, &gTxPacket[0], BQ24725A_TX_READ_FRAME_LEN);

    /* Wait for I2C to be Idle */
    while (!(DL_I2C_getControllerStatus(I2C_INST) & DL_I2C_CONTROLLER_STATUS_IDLE)){
        if(I2C_timeout_counter == 0){
            I2C_Timeout = 1;
            return ;
        }
    }

    /* Send the packet to the controller.
        * This function will send Start + Stop automatically.
        */
    DL_I2C_startControllerTransfer(I2C_INST, CHARGER_BQ24725A_I2C_8_BITS_ADDR,
        DL_I2C_CONTROLLER_DIRECTION_TX, BQ24725A_TX_READ_FRAME_LEN);

    /* Poll until the Controller writes all bytes */
    while (DL_I2C_getControllerStatus(I2C_INST) & DL_I2C_CONTROLLER_STATUS_BUSY_BUS){
        if(I2C_timeout_counter == 0){
            I2C_Timeout = 1;
            return ;
        }    
    }

    /* Trap if there was an error */
    if (DL_I2C_getControllerStatus(I2C_INST) &
        DL_I2C_CONTROLLER_STATUS_ERROR) {
        /* LED will remain high if there is an error */
        __BKPT(0);
    }

    /* Wait for I2C to be Idle */
    while (!(DL_I2C_getControllerStatus(I2C_INST) & DL_I2C_CONTROLLER_STATUS_IDLE)){
        if(I2C_timeout_counter == 0){
            I2C_Timeout = 1;
            return ;
        }
    }

    /* Add delay between transfers */
    delay_cycles(1000);

    /* Send a read request to Target */
    DL_I2C_startControllerTransfer(I2C_INST, CHARGER_BQ24725A_I2C_8_BITS_ADDR,
        DL_I2C_CONTROLLER_DIRECTION_RX, BQ24725A_RX_READ_LEN);

    /*
        * Receive all bytes from target. LED will remain high if not all bytes
        * are received
        */
    for (uint8_t i = 0; i < BQ24725A_RX_READ_LEN; i++) {
        while (DL_I2C_isControllerRXFIFOEmpty(I2C_INST)){
            if(I2C_timeout_counter == 0){
                I2C_Timeout = 1;
                return ;
            }
        }
        
        gRxPacket[i] = DL_I2C_receiveControllerData(I2C_INST);
        
        I2C_timeout_counter = 0;
    }
}

void write_charge_reg(uint8_t reg_addr, uint16_t reg_data){
    I2C_Timeout = 0;
// Send I2C command
    I2C_timeout_counter = I2C_TIMEOUT_VALUE_2S;

    gTxPacket[0] = reg_addr;
    gTxPacket[1] = (uint8_t) reg_data;  // low data byte
    gTxPacket[2] = (uint8_t) (reg_data >> 8);  // low data byte

    DL_I2C_fillControllerTXFIFO(I2C_INST, &gTxPacket[0], BQ24725A_TX_WRITE_FRAME_LEN);

    /* Wait for I2C to be Idle */
    while (!(DL_I2C_getControllerStatus(I2C_INST) & DL_I2C_CONTROLLER_STATUS_IDLE)){
        if(I2C_timeout_counter == 0){
            I2C_Timeout = 1;
            return ;
        }
    }

    /* Send the packet to the controller.
        * This function will send Start + Stop automatically.
        */
    DL_I2C_startControllerTransfer(I2C_INST, CHARGER_BQ24725A_I2C_8_BITS_ADDR,
        DL_I2C_CONTROLLER_DIRECTION_TX, BQ24725A_TX_WRITE_FRAME_LEN);

    /* Poll until the Controller writes all bytes */
    while (DL_I2C_getControllerStatus(I2C_INST) & DL_I2C_CONTROLLER_STATUS_BUSY_BUS){
        if(I2C_timeout_counter == 0){
            I2C_Timeout = 1;
            return ;
        }
    }

    /* Trap if there was an error */
    if (DL_I2C_getControllerStatus(I2C_INST) &
        DL_I2C_CONTROLLER_STATUS_ERROR) {
        /* LED will remain high if there is an error */
        __BKPT(0);
    }

    /* Wait for I2C to be Idle */
    while (!(DL_I2C_getControllerStatus(I2C_INST) & DL_I2C_CONTROLLER_STATUS_IDLE)){
        if(I2C_timeout_counter == 0){
            I2C_Timeout = 1;
            return ;
        }
    }

    /* Add delay between transfers */
    delay_cycles(1000);

    I2C_timeout_counter = 0;
}
