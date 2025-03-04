/*!
*\file  sht4x.h
*\brief This file holds the macros and low level function prototypes 
*       designed specifically for SHT4X (i.e SHT40, SHT41, SHT45) temperature and Humidity sensor in I2C mode.
*
*       Specs of the sensor. References: Sensirion_Humidity_Sensors_SHT4x_Datasheet_digital.pdf(Product datasheet Version 2)
*         -Measurement Range for Temperature
*          Sensor | Temperature range | Typical,worse accuracy                 | Humidity range | Typical,worse accuracy
*          SHT40  | -40 to 125 'C     | +/- 0.2 'C, +/-0.4 'C (for 0 to 65'C)  | 0 to 100 %     | +/- 2.0%, +/- 4.5% (for 10% to 90%)
*          SHT41  | 0 to 90 'C        | +/- 0.2 'C, +/-0.4 'C (for 0 to 90'C)  | 0 to 100 %     | +/- 2.0%, +/- 2.5% (for 0% to 90%)
*          SHT45  | 20 to 60 'C       | +/- 0.1 'C, +/-0.4 'C (for 20 to 60'C) | 0 to 100 %     | +/- 1.5%, +/- 2.0% (for 0% to 80%)
*
*         -Voltage 1.08 to 3.6Volt
*         -Average supply current idle state (not performing measurement) typical 0.08 uAmp Max 1.0 uAmp @ 25'C RT
*         -Average supply current for high repeatability single shot mode 2.4 uAmp
*/

#ifndef SHT4X_H
#define SHT4X_H
#include <stdint.h>
#include "serial-dev.h"
#include "sensirion/sensirion.h"

typedef struct sht4x {
  uint32_t serial_number;
  uint32_t last_temp_mk;
  uint32_t last_rh_ppm;
  serial_dev_t *sht4x_dev;
} sht4x_t;

/*** Following are the macros specific to SHT4x Temperature Humidity sensor ***/
#define SHT4X_AD1B                          1           /*!< AD1B has 0x44 I2C address while BD1B 0x45 */
#if SHT4X_AD1B
#define SHT4X_I2C_DEFAULT_ADDRESS           (0x44 << 1) /*!< Default SHT4X i2c sensor address value from the data sheet*/
#else /* SHT4X_AD1B */
#define SHT4X_I2C_DEFAULT_ADDRESS           (0x45 << 1) /*!< Default SHT4X i2c sensor address value from the data sheet*/
#endif /* SHT4X_AD1B */
#define SHT4X_I2C_SPEED                     400000      /*!< I2C speed in Hertz for SHT4X from the datasheet */
#define SHT4X_POWER_UP_TIME_MS              2           /*!< SHT4X power up time in milliseconds  */
/* CRC calculation related macros */
#define SHT4X_CRC_OK                        0
#define SHT4X_CRC_FAILED                    1
#define SHT4X_WRONG_DATA_LENGTH             2
#define SHT4X_CRC8_POLYNOMIAL               0x31u
#define SHT4X_CRC8_INITIAL_REMAINDER        0xff
#define SHT4X_CRC8_FINAL_XOR_VALUE          0x00

#define SHT4X_SINGLE_MEASUREMENT_MODE       0  /*!< Value when the sensor is in single shot mode */
#define SHT4X_CONTINUOUS_MEASUREMENT_MODE   1  /*!< Value when the sensor is in continuous mode  */

/***************************Global function prototypes***************************************/
/*!
* \fn     uint8_t sht4x_init(void)
* \brief  Function initializes the sensor by reading serial number into serial number variable
* \param  sht pointer to structure sht4x.
* \return Function returns Exception Value, 0 (i.e NO_ERROR) on success.
*/
uint8_t sht4x_init(sht4x_t *sht);

/*!
* \fn     uint8_t sht4x_take_single_measurement(sht4x_t *sht)
* \brief  Function reads raw values from sht4x and converts and write them into millikelvin and 
*         relative humidity in their respective variables. @note Use this function only when
*         not in continues measurement mode.
* \param  sht pointer to structure sht4x.
* \return Function returns Exception value, 0 (i.e NO_ERROR) on success. 
*/
uint8_t sht4x_take_single_measurement(sht4x_t *sht);

/*!
 * \fn    uint32_t sht4x_get_serial_id(sht4x_t *sht)
 * \brief Function returns serial ID of the sensor. This function should only be called
 *        after a successful execution of init function
 * \param sht pointer to structure sht4x.
 */
uint32_t sht4x_get_serial_id(sht4x_t *sht);
#endif /* SHT4X_H */
