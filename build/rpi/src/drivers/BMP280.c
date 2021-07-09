#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <rtems.h>
#include <dev/i2c/i2c.h>
#include "./BMP280.h"

static void inline delay_sec(int sec)
{
  rtems_task_wake_after(sec*rtems_clock_get_ticks_per_second());
}

static int bmp280_read_register(
  i2c_dev *dev,
  uint8_t reg,
  uint8_t* content
) {
  i2c_msg msg[2] = {
    {
    .addr = dev->address,
    .flags = 0,
    .len = 1,
    .buf = &reg
    },
    {
    .addr = dev->address,
    .flags = I2C_M_RD,
    .len = 1,
    .buf = content
    }
  };

  return i2c_bus_transfer(dev->bus, msg, 2);
}

static int bmp280_write_register(i2c_dev *dev, 
  uint8_t reg, 
  uint8_t val)
{
  uint8_t content[2];

  content[0] = reg;
  content[1] = val;

  i2c_msg msg = {
    .addr = dev->address,
    .flags = 0,
    .len = 2,
    .buf = content
  };

  return i2c_bus_transfer(dev->bus, &msg, 1);
}

int BMP280_READ_TEMPERATURE(i2c_dev *dev, int32_t *Temperature)
{
  int rv = 0;
  uint16_t dig_T1;
  int16_t dig_T2;
  int16_t dig_T3;
  uint8_t dig_T1_msb;
  uint8_t dig_T1_lsb;
  uint8_t dig_T2_msb;
  uint8_t dig_T2_lsb;
  uint8_t dig_T3_msb;
  uint8_t dig_T3_lsb;
  uint8_t Temp_msb;
  uint8_t Temp_lsb;
  uint8_t Temp_xlsb;
  int32_t T_raw;
  int32_t Temp;
  int32_t var1;
  int32_t var2;
  uint32_t t_fine;
  uint8_t temp;

  /* Reset Chip */
  rv = bmp280_write_register(dev, BMP280_SOFT_RESET_ADDR, BMP280_SOFT_RESET_CMD); 
  printf("Chip Reset Status: 0x%08X \n", rv);


  /* Read The Calibration/Compensation Parameters */
  rv = bmp280_read_register(dev, BMP280_DIG_T1_LSB_ADDR, &dig_T1_lsb);
  rv = bmp280_read_register(dev, BMP280_DIG_T1_MSB_ADDR, &dig_T1_msb);
  rv = bmp280_read_register(dev, BMP280_DIG_T2_LSB_ADDR, &dig_T2_lsb);
  rv = bmp280_read_register(dev, BMP280_DIG_T2_LSB_ADDR, &dig_T2_msb);
  rv = bmp280_read_register(dev, BMP280_DIG_T3_LSB_ADDR, &dig_T3_lsb);
  rv = bmp280_read_register(dev, BMP280_DIG_T3_LSB_ADDR, &dig_T3_msb);

  dig_T1 = (uint16_t)((((uint16_t)dig_T1_msb) << 8) | (uint16_t)dig_T1_lsb); 
  dig_T2 = (int16_t)((((uint16_t)dig_T2_msb) << 8) | (uint16_t)dig_T2_lsb); 
  dig_T3 = (int16_t)((((uint16_t)dig_T3_msb) << 8) | (uint16_t)dig_T3_lsb); 

  printf("T1: %d T2: %d T3: %d\n", dig_T1, dig_T2, dig_T3);
 

  /* 
   **Checking Status, and Setting it to Normal Mode
   */
  rv = bmp280_read_register(dev, BMP280_STATUS_ADDR, &temp);
  temp &= 0x09;
  printf("REG Status: %d\n", temp);

  rv = bmp280_read_register(dev, BMP280_CTRL_MEAS_ADDR, &temp);
  temp &= 0x03;
  printf("Mode: [%02X] \n", temp);
  switch (temp) {
    case BMP280_SLEEP_MODE:
      printf("SLEEP\n");
      break;
    case BMP280_FORCED_MODE:
      printf("FORCED\n");
      break;
    case BMP280_NORMAL_MODE:
      printf("NORMAL\n");
      break;
  }
  temp = 0;
  
  rv = bmp280_read_register(dev, BMP280_STATUS_ADDR, &temp);
  uint8_t tempreg = (uint8_t)( temp & ~(0x03));
  rv = bmp280_write_register(dev, BMP280_CTRL_MEAS_ADDR, tempreg | 0x03);

  rv = bmp280_read_register(dev, BMP280_CTRL_MEAS_ADDR, &temp);
  temp &= 0x03;
  printf("Mode: [%02X] \n", temp);
  switch (temp) {
    case BMP280_SLEEP_MODE:
      printf("SLEEP\n");
      break;
    case BMP280_FORCED_MODE:
      printf("FORCED\n");
      break;
    case BMP280_NORMAL_MODE:
      printf("NORMAL\n");
      break;
  }
  temp = 0;

  /*
   * Main Function to read Temperature starts here
   *
   */

  /* Read The Temperature */   
 for(int i = 0; i < 5; i++){
    printf("\n");
    rv = bmp280_read_register(dev, BMP280_TEMP_MSB_ADDR, &Temp_msb);
    rv = bmp280_read_register(dev, BMP280_TEMP_LSB_ADDR, &Temp_lsb);
    rv = bmp280_read_register(dev, BMP280_TEMP_XLSB_ADDR, &Temp_xlsb);

    T_raw = (int32_t)((((uint32_t)Temp_msb) << 12) | (((uint32_t)Temp_lsb) << 4) | (((uint32_t)Temp_xlsb) >> 4));
    printf("Raw Temperature: %d\n", T_raw);
   

    /* Calibrate The Temperature According to the Datasheet */
    var1  = ((((T_raw >> 3) - ((int32_t)dig_T1 << 1))) * ((int32_t)dig_T2)) >> 11;
    var2  = (((((T_raw >> 4) - ((int32_t)dig_T1)) * ((T_raw >> 4) - ((int32_t)dig_T1))) >> 12) * ((int32_t)dig_T3)) >> 14;
    t_fine = var1 + var2; 
    Temp = (t_fine * 5 + 128) >> 8;

    printf("Temperature : %d \n", Temp);
    printf("\n");
    delay_sec(3);
 }
  return(rv);
}

int BMP280_READ_WHOAMI(i2c_dev *dev, uint8_t *whoami)
{
  int rv = 0;

  rv = bmp280_read_register(dev, BMP280_CHIP_ID_ADDR, whoami);
  printf("BMP280: i2c_bus_transfer rv = 0x%08x, whoami=0x%x \n", rv, *whoami);

  return (rv);
}


static int BMP280_linux_ioctl(i2c_dev *dev,
                              ioctl_command_t command,
                              void* arg)
{
  int rv = 0;
  uint8_t *whoami = NULL;
  int32_t *Temperature = NULL;

  switch( command )
  {
    case BMP280_read_whoami:
      whoami = (uint8_t *)arg;
      rv = BMP280_READ_WHOAMI(dev, whoami);
      break;
    case BMP280_read_Temperature:
      Temperature = (int32_t *)arg;
      rv = BMP280_READ_TEMPERATURE(dev, Temperature);
      break;
    case BMP280_read_Press:
      break;
  }
  return rv;  
}

/*
 ** Register The Device
 */
int i2c_dev_register_BMP280(const char *bus_path,
                            const char *dev_path,
                            uint32_t address) 
{
    i2c_dev *dev;
    dev = i2c_dev_alloc_and_init(sizeof(*dev), bus_path, address);
    if (dev == NULL) 
    {
        printf("BMP280: Error calling i2c_dev_alloc_and_init\n");
        return -1;
    }

    dev->ioctl = BMP280_linux_ioctl;
    return i2c_dev_register(dev, dev_path);
}
