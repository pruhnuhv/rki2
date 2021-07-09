
#include <rtems.h>
#include <dev/i2c/i2c.h>

#define BMP280_I2C_ADDR                      0x76
#define BMP280_I2C_ADDR_SEC                  0x77
#define DEVICE_ID                            0x58
#define BMP280_SOFT_RESET_CMD                0xB6

#define BMP280_CHIP_ID_ADDR                  0xD0
#define BMP280_SOFT_RESET_ADDR               0xE0
#define BMP280_STATUS_ADDR                   0xF3
#define BMP280_CTRL_MEAS_ADDR                0xF4
#define BMP280_CONFIG_ADDR                   0xF5
#define BMP280_PRES_MSB_ADDR                 0xF7
#define BMP280_PRES_LSB_ADDR                 0xF8
#define BMP280_PRES_XLSB_ADDR                0xF9
#define BMP280_TEMP_MSB_ADDR                 0xFA
#define BMP280_TEMP_LSB_ADDR                 0xFB
#define BMP280_TEMP_XLSB_ADDR                0xFC

#define BMP280_SLEEP_MODE                    0x00
#define BMP280_FORCED_MODE                   0x01
#define BMP280_NORMAL_MODE                   0x03

/*! @name Calibration parameter register addresses*/
#define BMP280_DIG_T1_LSB_ADDR               UINT8_C(0x88)
#define BMP280_DIG_T1_MSB_ADDR               UINT8_C(0x89)
#define BMP280_DIG_T2_LSB_ADDR               UINT8_C(0x8A)
#define BMP280_DIG_T2_MSB_ADDR               UINT8_C(0x8B)
#define BMP280_DIG_T3_LSB_ADDR               UINT8_C(0x8C)
#define BMP280_DIG_T3_MSB_ADDR               UINT8_C(0x8D)
#define BMP280_DIG_P1_LSB_ADDR               UINT8_C(0x8E)
#define BMP280_DIG_P1_MSB_ADDR               UINT8_C(0x8F)
#define BMP280_DIG_P2_LSB_ADDR               UINT8_C(0x90)
#define BMP280_DIG_P2_MSB_ADDR               UINT8_C(0x91)
#define BMP280_DIG_P3_LSB_ADDR               UINT8_C(0x92)
#define BMP280_DIG_P3_MSB_ADDR               UINT8_C(0x93)
#define BMP280_DIG_P4_LSB_ADDR               UINT8_C(0x94)
#define BMP280_DIG_P4_MSB_ADDR               UINT8_C(0x95)
#define BMP280_DIG_P5_LSB_ADDR               UINT8_C(0x96)
#define BMP280_DIG_P5_MSB_ADDR               UINT8_C(0x97)
#define BMP280_DIG_P6_LSB_ADDR               UINT8_C(0x98)
#define BMP280_DIG_P6_MSB_ADDR               UINT8_C(0x99)
#define BMP280_DIG_P7_LSB_ADDR               UINT8_C(0x9A)
#define BMP280_DIG_P7_MSB_ADDR               UINT8_C(0x9B)
#define BMP280_DIG_P8_LSB_ADDR               UINT8_C(0x9C)
#define BMP280_DIG_P8_MSB_ADDR               UINT8_C(0x9D)
#define BMP280_DIG_P9_LSB_ADDR               UINT8_C(0x9E)
#define BMP280_DIG_P9_MSB_ADDR               UINT8_C(0x9F)


typedef enum
{
  BMP280_read_whoami,
  BMP280_read_Temperature,
  BMP280_read_Press,
} BMP280_cmd;

int i2c_dev_register_BMP280(const char* bus_path,
                            const char* dev_path,
                            uint32_t address);
