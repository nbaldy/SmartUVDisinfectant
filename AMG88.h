/**
 * Much of this code is from https://www.robot-electronics.co.uk/files/grideyeappnote.pdf
 * 11/03/22
 * Last Modified by Nicole Baldy, 10/21
**/

#ifndef AMG88_H
#define	AMG88_H

#include <xc.h> // include processor files - each processor file is guarded.  

/*******************************************************************************
macro definition
*******************************************************************************/
#define TRUE (1)
#define FALSE (0)

typedef unsigned char bool;
typedef unsigned char uchar;

/* Grid-EYE's I2C slave address */
// Was 0xD0, 0xD2
#define GRIDEYE_ADR_GND (0xD0) /* AD_SELECT pin connect to GND */
#define GRIDEYE_ADR_VDD (0xD2) /* AD_SELECT pin connect to VDD */
#define GRIDEYE_ADR GRIDEYE_ADR_VDD // Pin is pulled up by default

/* Grid-EYE's register address */
#define GRIDEYE_REG_THS00 (0x0E) /* head address of thermistor resister */
#define GRIDEYE_REG_TMP00 (0x80) /* head address of temperature resister */

/* Grid-EYE's register size */
#define GRIDEYE_REGSZ_THS (0x02) /* size of thermistor resister */
#define GRIDEYE_REGSZ_TMP (0x80) /* size of temperature resister */


/* Grid-EYE's number of pixels */
#define SNR_SZ_X (8)
#define SNR_SZ_Y (8)
#define SNR_SZ (SNR_SZ_X * SNR_SZ_Y)

/*******************************************************************************
method definition
*******************************************************************************/
bool bReadTempFromGridEYE( void );
bool bAMG_PUB_I2C_Read( uchar, uchar, uchar, uchar* );
short shAMG_PUB_TMP_ConvThermistor( uchar[2] );
short shAMG_PUB_TMP_ConvTemperature( uchar[2] );
void vAMG_PUB_TMP_ConvTemperature64( uchar*, short* );
short shAMG_PUB_CMN_ConvFtoS( float );
float  fAMG_PUB_CMN_ConvStoF( short );

// t1 expected < t2
int numPixelsInRange(short t1, short t2);
short maxPixel();
#endif	/* AMG88_H */

