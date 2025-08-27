#ifndef __DEFS_H__
#define __DEFS_H__

typedef signed char         int8_t;
//typedef int                 int16_t;
//typedef long                int32_t;

typedef unsigned char       uint8_t;
//typedef unsigned int        uint16_t;
//typedef unsigned long       uint32_t;

typedef unsigned long  const ucint32_t;  /* Read Only */
typedef unsigned short const ucint16_t;  /* Read Only */
typedef unsigned char  const ucint8_t;   /* Read Only */

typedef volatile unsigned long  vuint32_t;
typedef volatile unsigned short vuint16_t;
typedef volatile unsigned char  vuint8_t;

typedef volatile unsigned long  const vucint32_t;  /* Read Only */
typedef volatile unsigned short const vucint16_t;  /* Read Only */
typedef volatile unsigned char  const vucint8_t;   /* Read Only */

//typedef _Bool   bool;

#define     TRUE                    1
#define     FALSE                   0

#define MEM(x,y)   (*(((volatile unsigned char *)(&x))+y))    // get/set the y byte of x

#define SBI(io,bit)     ( io  |=  (unsigned char)(1<<bit) )
//example:SBI(PA_ODR,0);SBI(PA_DDR,0);
#define CBI(io,bit)     ( io  &= (unsigned char)(~(1<<bit)) )
//example:CBI(PA_ODR,0);CBI(PA_DDR,0);
#define GBI(pin ,bit)   ( pin &   (unsigned char)(1<<bit) )
// #define GBIL(pin ,bit)   ( pin &   (unsigned int)(1L<<bit) )
//example: GBI(PA_IDR,0);

typedef union
{
    unsigned char BYTE;
    struct
    {
        unsigned char bit0 :1;
        unsigned char bit1 :1;
        unsigned char bit2 :1;
        unsigned char bit3 :1;
        unsigned char bit4 :1;
        unsigned char bit5 :1;
        unsigned char bit6 :1;
        unsigned char bit7 :1;
    } bit;
} BIT_8;


#define GET_FIELD(addr) (*((volatile  BIT_8 *) (&addr)))
#define BIT(addr,b) GET_FIELD(addr).bit.bit##b

#define MASK(bitpos)        (1L << (bitpos))       // Create mask for selected bit
#define IS_BIT_SET(x,y)     ((x) & (y))           // Test bit in x with mask y
#define IS_BIT_CLR(x,y)     (!((x) & (y)))        // Test bit is 0
#define SET_BIT(a,b)        ((a) |= (b))         // Apply (OR) mask b to UINT8 a
#define CLR_BIT(a,b)        ((a) &= ~(b))         // Apply (AND) mask ~b to UINT8 a
#define SET_BIT_16(a,b)     ((a) |= (uint16_t)(b))         // Apply (OR) mask b to UINT16 a
#define CLR_BIT_16(a,b)     ((a) &= (uint16_t)~(b))         // Apply (AND) mask ~b to UINT16 a
#define IS_BIT_ALLSET(x,y)  (((x)&(y))==(y))
#define IS_BIT_NOTALLSET(x,y)  (((x)&(y))!=(y))
#define TOGGLE_BIT(x,y)     ((x)^=(y))

#endif
