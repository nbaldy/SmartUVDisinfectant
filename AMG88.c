/**
 * Much of this code is from https://www.robot-electronics.co.uk/files/grideyeappnote.pdf
 * 11/03/22
 * Last Modified by Nicole Baldy, 10/21
**/

#include "AMG88.h"
#include "peripherals.h"
/*******************************************************************************
variable value definition
*******************************************************************************/

short g_shThsTemp; /* thermistor temperature */
short g_ashRawTemp[SNR_SZ]; /* temperature of 64 pixels */

/*******************************************************************************
method
******************************************************************************/

/*------------------------------------------------------------------------------
Read temperature from Grid-EYE.
------------------------------------------------------------------------------*/
bool bReadTempFromGridEYE( void )
{
    uchar aucThsBuf[GRIDEYE_REGSZ_THS];
    uchar aucTmpBuf[GRIDEYE_REGSZ_TMP];
    /* Get thermistor register value. */
    if( FALSE == bAMG_PUB_I2C_Read( GRIDEYE_ADR, GRIDEYE_REG_THS00, GRIDEYE_REGSZ_THS, aucThsBuf ) )
    {
        return( FALSE );
    }
    /* Convert thermistor register value. */
    g_shThsTemp = shAMG_PUB_TMP_ConvThermistor( aucThsBuf );
    /* Get temperature register value. */
    if( FALSE == bAMG_PUB_I2C_Read( GRIDEYE_ADR, GRIDEYE_REG_TMP00, GRIDEYE_REGSZ_TMP, aucTmpBuf ) )
    {
        return( FALSE );
    }
    /* Convert temperature register value. */
    vAMG_PUB_TMP_ConvTemperature64( aucTmpBuf, g_ashRawTemp );
    return( TRUE );
}

/*------------------------------------------------------------------------------
Read data form I2C bus.
------------------------------------------------------------------------------*/
bool bAMG_PUB_I2C_Read( uchar ucI2cAddr, uchar ucRegAddr, uchar ucSize, uchar* ucDstAddr )
{
    /* ucI2cAddr : I2C Address ( In the case of the connection with GND of AD_SELECT PIN ) */
    /* ucRegAddr : Source address of Grid-EYE */
    /* ucSize : Data Size */
    /* ucDstAddr : Destination address of MCU */
    /* return : TRUE: success, FALSE: failure */
    /* This function is only interface definition. */
    I2Csendbyte(ucI2cAddr); // Send addr
    us_delay(100);
    I2Csendbyte(ucRegAddr); // Send the register to read
    us_delay(100);
    
    uchar *arr_ptr = ucDstAddr;
    int i;
    for(i = 0; i < ucSize; i++)
        *arr_ptr = I2Cgetbyte();
        arr_ptr += i;
    return( TRUE );
}

/*------------------------------------------------------------------------------
Convert thermistor register value.
------------------------------------------------------------------------------*/
short shAMG_PUB_TMP_ConvThermistor( uchar aucRegVal[2] )
{
    /* Convert to 16 bit Two's complement */
    /* bit15 : sign bit */
    /* bit14-8 : integral number bits */
    /* bit7-0 : fixed-point numbers bits */
    short shVal = ((short)(aucRegVal[1] & 0x07) << 8) | aucRegVal[0];
    if( 0 != (0x08 & aucRegVal[1]) )
    {
        shVal *= -1;
    }
    shVal *= 16;
    return( shVal );
}

/*------------------------------------------------------------------------------
Convert temperature register value for 1 pixel.
------------------------------------------------------------------------------*/
short shAMG_PUB_TMP_ConvTemperature( uchar aucRegVal[2] )
{
    /* Convert to 16 bit Two's complement */
    /* bit15 : sign bit */
    /* bit14-8 : integral number bits */
    /* bit7-0 : fixed-point numbers bits */
    short shVal = ((short)(aucRegVal[1] & 0x07) << 8) | aucRegVal[0];
    if( 0 != (0x08 & aucRegVal[1]) )
    {
        shVal -= 2048;
    }
    shVal *= 64;
    return( shVal );
}

/*------------------------------------------------------------------------------
Convert temperature register value for 64 pixel.
------------------------------------------------------------------------------*/
void vAMG_PUB_TMP_ConvTemperature64( uchar* pucRegVal, short* pshVal )
{
    uchar ucCnt;
    for( ucCnt = 0u; ucCnt < SNR_SZ; ucCnt++ )
    {
        pshVal[ucCnt] = shAMG_PUB_TMP_ConvTemperature( pucRegVal + (ucCnt * 2u) );
    }
}

/*------------------------------------------------------------------------------
Convert value.
------------------------------------------------------------------------------*/
short shAMG_PUB_CMN_ConvFtoS( float fVal )
{
    return( ( fVal > 0 ) ? ((short)((fVal * 256) + 0.5)) : ((short)((fVal * 256) - 0.5)) );
}

/*------------------------------------------------------------------------------
Convert value.
------------------------------------------------------------------------------*/
float fAMG_PUB_CMN_ConvStoF( short shVal )
{
    return( (float)shVal / 256 );
}