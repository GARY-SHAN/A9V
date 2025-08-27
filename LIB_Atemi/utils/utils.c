#include "defs.h"
#include "utils.h"

//*****************************************************************************
//
//! Check state debounce.
//!
//! \return True if state is true for over delay time.
//
//*****************************************************************************
bool StateDebounce(bool bState, uint8_t u8Delay, vuint8_t *u8Cntr)
{
    if ( bState )
    {
        if ( (*u8Cntr) >= u8Delay )
        {
            //*u8Cntr = 0;
            return TRUE;
        }
        (*u8Cntr) += 1;
    }
    else
    {
        *u8Cntr = 0;
    }

    return FALSE;
}


bool StateDebounce16(bool bState, uint16_t u16Delay, uint16_t *u16Cntr)
{
    if ( bState )
    {
        if ( ++(*u16Cntr) >= u16Delay )
        {
            *u16Cntr = 0;
            return TRUE;
        }
    }
    else
    {
        *u16Cntr = 0;
    }

    return FALSE;
}
