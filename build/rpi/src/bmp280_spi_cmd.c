
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <bsp.h> 
#include <bsp/spi.h> 
#include "drivers/BMP280.h"
#include <rtems/libi2c.h>
#include <rtems/libio.h>

static void inline delay_sec(int sec)
{
  rtems_task_wake_after(sec*rtems_clock_get_ticks_per_second());
}

int SPI_BMP280_test_cmd(int argc, char* argv[])
{
  int rv = 0;         //return variable
  int fd = 0;
  int spi_bus;
  int32_t Temperature = 0;
  uint8_t whoami = 0;
  
  delay_sec(2);

  spi_bus = rpi_spi_init(false);
  if ( rv < 0 )
  {
    printf("Error Initializing SPI bus\n");
    return(-1);
  }
  else
  {
    printf("\n\nSPI Initialized\n");
  }

  rv = spi_libi2c_register_bmp280(spi_bus);
  if (rv < 0)
  {
    printf("Error Registering SPI device \n");
  }
  else
  {
    printf("SPI-libi2c registered\n");
  }
  

  fd = open("/dev/spi.BMP280-SPI", O_RDWR);
  if (fd < 0)
  {
    printf("Error opening BMP280 device\n");
    return(fd);
  }

  printf("Opened BMP 280, Reading Temperature and Pressure...\n\n");
 
  printf("\n\n\nWHOAMI:\n"); 
  rv = ioctl(fd, SPI_BMP280_read_whoami, &whoami);
  if (rv < 0)
  {
    printf("BMP280 init failed, rv = %d\n", rv);
    return(rv);
  }
  else
  {
    printf("Whoami Successful, Device ID: 0x%X\n", whoami);
  }
  
  printf("\n\n\nTEMPERATURE:\n");
  rv = ioctl(fd, SPI_BMP280_read_Temperature, &Temperature);
  if (rv < 0)
  {
    printf("BMP280 init failed, rv = %d\n", rv);
    return(rv);
  }
  else
  {
    printf("Temperature = %d\n", Temperature);
  }

  rv = close(fd);
  return(rv);

}

