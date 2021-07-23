#include <stdio.h>
#include <math.h>
#include <rtems.h>
#include "./BMP280.h"
#include <rtems/libi2c.h>
#include <rtems/libio.h>
#include <rtems/bspIo.h>

static rtems_status_code spi_bmp280_write(
  rtems_device_major_number major,
  rtems_device_minor_number minor,
  void *arg
);

static rtems_status_code spi_bmp280_read(
  rtems_device_major_number major,
  rtems_device_minor_number minor,
  void *arg
);

static rtems_libi2c_tfr_mode_t tfr_mode = {
  .baudrate = 100000,
  .bits_per_char = 8,
  .lsb_first = FALSE,
  .clock_inv = TRUE,
  .clock_phs = FALSE
};

/*
 * Main Function to Get Temperature
 * -> Calls, the read and write functions
 */

int spi_bmp280_get_Temperature (
  rtems_device_major_number major,
  rtems_device_minor_number minor,
  void *arg
)
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

  rtems_libio_rw_args_t *rwargs = arg; 
  unsigned char *buf = (unsigned char *)rwargs->buffer;
  
  /* Reset Chip */
  rwargs->count = 1;
  rwargs->offset = BMP280_SOFT_RESET_ADDR;
  *buf = BMP280_SOFT_RESET_CMD;
  rv = spi_bmp280_write(major, minor, arg);
  printf("Chip Reset Status: 0x%08x \n", rv);

  
  /* Read The Calibration/Compensation Parameters */
  rwargs->count = 6;
  rwargs->offset = BMP280_DIG_T1_LSB_ADDR;
  rv = spi_bmp280_read(major, minor, arg); 
  dig_T1_msb = buf[0];
  dig_T1_lsb = buf[1];
  dig_T2_msb = buf[2];
  dig_T2_lsb = buf[3];
  dig_T3_msb = buf[4];
  dig_T3_lsb = buf[5];

  dig_T1 = (uint16_t)((((uint16_t)dig_T1_msb) << 8) | (uint16_t)dig_T1_lsb); 
  dig_T2 = (int16_t)((((uint16_t)dig_T2_msb) << 8) | (uint16_t)dig_T2_lsb); 
  dig_T3 = (int16_t)((((uint16_t)dig_T3_msb) << 8) | (uint16_t)dig_T3_lsb); 
  

  /*
   * These values should be positive integers. A negative value is an error returned. 0 likely means a dead chip or a wrong connection.
   */
  printf("T1: %d T2: %d T3: %d\n", dig_T1, dig_T2, dig_T3);

  /*
   ** Checking Status, And setting it to Normal mode
   */

  rwargs->count = 1;
  rwargs->offset = BMP280_STATUS_ADDR;
  rv = spi_bmp280_read(major, minor, arg);
  temp = buf[0];
  temp &= 0x09;

  rwargs->count = 1;
  rwargs->offset = BMP280_CTRL_MEAS_ADDR;
  rv = spi_bmp280_read(major, minor, arg);
  temp = buf[0];
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

  rwargs->count = 1;
  rwargs->offset = BMP280_STATUS_ADDR;
  rv = spi_bmp280_read(major, minor, arg);
  temp = buf[0];
  uint8_t tempreg = (uint8_t)(temp & ~(0x03));

  /*
   *  Setting the device to its 'Normal' mode.
   */
  rwargs->count = 1;
  rwargs->offset = BMP280_CTRL_MEAS_ADDR;
  *buf = (tempreg | 0x03); 
  rv = spi_bmp280_write(major, minor, arg);
  

  rwargs->count = 1;
  rwargs->offset = BMP280_CTRL_MEAS_ADDR;
  rv = spi_bmp280_read(major, minor, arg);
  temp = buf[0];
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

  return (rv);
}


/*
 * This Function should print whoami = 0x58 (The correct device ID)
 */

int spi_bmp280_whoami(
  rtems_device_major_number major,
  rtems_device_minor_number minor,
  void *arg
) 
{
  int rv = 0;
  rtems_libio_rw_args_t *rwargs = arg;
  unsigned char *buf = (unsigned char *)rwargs->buffer;
  
  rwargs->count = 1;
  rwargs->offset = BMP280_CHIP_ID_ADDR;
  rv = spi_bmp280_read(major, minor, arg);
  printf("BMP280: read rv = 0x%08x, whoami = 0x%x \n", rv, buf[0]);

  return (rv);
}


/*
 * Write Byte(s) to BMP280
 */


static rtems_status_code spi_bmp280_write(
  rtems_device_major_number major,
  rtems_device_minor_number minor,
  void *arg
) {
  rtems_status_code sc = RTEMS_SUCCESSFUL;
  rtems_libio_rw_args_t *rwargs = arg;
  int cnt = rwargs->count;
  unsigned char *buf = (unsigned char *)rwargs->buffer;
  int bytes_sent = 0;
  int curr_cnt;
  unsigned char cmdbuf[4];
  int ret_cnt = 0;
  int cmd_size;

  while ( cnt > bytes_sent ) {
    curr_cnt = cnt - bytes_sent;

    /* Start the bus */
    sc = rtems_libi2c_send_start(minor);

    if ( sc != RTEMS_SUCCESSFUL ) {
      return sc;
    }

    /* Set transfer mode */
    sc = rtems_libi2c_ioctl(minor, RTEMS_LIBI2C_IOCTL_SET_TFRMODE, &tfr_mode);

    if ( sc != RTEMS_SUCCESSFUL ) {
      return sc;
    }

    /* Address device */
    sc = rtems_libi2c_send_addr(minor,TRUE);

    if ( sc != RTEMS_SUCCESSFUL ) {
      return sc;
    }

    /* Start the bus */
    sc = rtems_libi2c_send_start(minor);

    if ( sc != RTEMS_SUCCESSFUL ) {
      return sc;
    }

    /* Address device */
    sc = rtems_libi2c_send_addr(minor,TRUE);

    if ( sc != RTEMS_SUCCESSFUL ) {
      return sc;
    }

    /* Send write instruction and address. */

    cmdbuf[0] = (rwargs->offset) & ~(1<<7);

    cmd_size= 1;

    ret_cnt = rtems_libi2c_write_bytes(minor,cmdbuf,cmd_size);

    if ( ret_cnt < 0 ) {
      return RTEMS_IO_ERROR;
    }

    /* Send data */
    ret_cnt = rtems_libi2c_write_bytes(minor,buf,curr_cnt);

    if ( ret_cnt < 0 ) {
      return RTEMS_IO_ERROR;
    }

    /* Terminate transfer */
    sc = rtems_libi2c_send_stop(minor);

    if ( sc != RTEMS_SUCCESSFUL ) {
      return sc;
    }

    /* Adjust bytecount to be sent and pointers. */
    bytes_sent += curr_cnt;
    rwargs->offset += curr_cnt;
    buf += curr_cnt;
  }

  rwargs->bytes_moved = bytes_sent;

  return sc;
}



/*
 * Read Byte(s) from BMP280.
 */

static rtems_status_code spi_bmp280_read(
  rtems_device_major_number major,
  rtems_device_minor_number minor,
  void *arg
) {
  rtems_status_code sc = RTEMS_SUCCESSFUL;
  rtems_libio_rw_args_t *rwargs = arg;
  int cnt = rwargs->count;
  unsigned char *buf = (unsigned char *)rwargs->buffer;
  unsigned char cmdbuf;
  int ret_cnt = 0;
  int cmd_size;

  /* Start the bus */
  sc = rtems_libi2c_send_start(minor);

  if ( sc != RTEMS_SUCCESSFUL ) {
    return sc;
  }

  /* Set transfer mode */
  sc = rtems_libi2c_ioctl(minor, RTEMS_LIBI2C_IOCTL_SET_TFRMODE, &tfr_mode);

  if ( sc != RTEMS_SUCCESSFUL ) {
    return sc;
  }

  /* Address device */
  sc = rtems_libi2c_send_addr(minor,TRUE);

  if ( sc != RTEMS_SUCCESSFUL ) {
    return sc;
  }

  /* Send read command and address. */

  cmdbuf = (rwargs->offset) | (1<<7);
  printf("Register being read: 0x%X \n", cmdbuf);
  cmd_size  = 1;
  ret_cnt = rtems_libi2c_write_bytes(minor, &cmdbuf, cmd_size);
  printf("Return count in spi_read - write_bytes: %d \n", ret_cnt);   
  if ( ret_cnt < 0 ) {
    return RTEMS_IO_ERROR;
  }

  /* Fetch data */
  ret_cnt = rtems_libi2c_read_bytes (minor,buf,cnt);
  printf("BUFFER READ: 0x%x \n", buf[0]);
  printf("Return count in spi_read - read_bytes: %d \n", ret_cnt);   
  if ( ret_cnt < 0 ) {
    return RTEMS_IO_ERROR;
  }

  /* Terminate transfer */
  
  sc = rtems_libi2c_send_stop(minor);
  if ( sc != RTEMS_SUCCESSFUL ) {
    return sc;
  }
  
  rwargs->bytes_moved = (sc == RTEMS_SUCCESSFUL) ? ret_cnt : 0;
  
  printf("Exiting SPI Read, Bytes Moved = %d \n\n", rwargs->bytes_moved);
  return 1;
}


static rtems_device_driver spi_bmp280_ioctl(rtems_device_major_number major,
                              rtems_device_minor_number minor,
                              void* arg
                              )
{
  int rv = 0;
  rtems_libio_ioctl_args_t *ioarg = (rtems_libio_ioctl_args_t *) arg;
  if (!ioarg) {
    return RTEMS_INVALID_NAME;
  }

  switch( ioarg->command )
  {
    case SPI_BMP280_read_whoami:
      rv = spi_bmp280_whoami(major, minor, arg);
      break;
    case SPI_BMP280_read_Temperature:
      rv = spi_bmp280_get_Temperature(major, minor, arg);
      break;
    case SPI_BMP280_read_Press:
      break;
  }
  return rv;  
}


static rtems_driver_address_table spi_bmp280_rw_ops = {
  .read_entry = spi_bmp280_read,
  .write_entry = spi_bmp280_write,
  .control_entry = spi_bmp280_ioctl
};


static rtems_libi2c_drv_t spi_bmp280_rw_drv_t = {
  .ops = &spi_bmp280_rw_ops,
  .size = sizeof(spi_bmp280_rw_drv_t)
};


int spi_libi2c_register_bmp280(unsigned spi_bus)
{
  printf("Device Registered\n");
  return rtems_libi2c_register_drv(
           "BMP280-SPI",
           &spi_bmp280_rw_drv_t,
           spi_bus,
            0x00
         );
}
