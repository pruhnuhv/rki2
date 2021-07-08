#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <bsp.h> 
#include <bsp/i2c.h>
#include "drivers/BMP280.h"

static int first_run = 0;

/*
 ** BMP280 test console command
 */

int BMP280_test_cmd(int argc, char* argv[])
{
  int rv = 0;         //return variable
  int fd = 0;
  int32_t Temperature = 0;
  uint8_t whoami = 0;

  if(first_run == 0) 
  {
    
    first_run = 1;

    rv = rpi_setup_i2c_bus ();
    if(rv < 0)
    {
      printf("\nrpi_setup_i2c_bus failed\n");
      return(0);
    }
 
    rv = i2c_dev_register_BMP280("/dev/i2c", "/dev/i2c.BMP280", BMP280_I2C_ADDR);
   
    if(rv < 0)
    {
      printf("\ni2c_dev_register_BMP280 failed\n");
      return(rv);
    }

    printf("Registered BMP280 device! \n");
  }
  
  fd = open("/dev/i2c.BMP280", O_RDWR);
  if (rv < 0)
  {
    printf("Error opening BMP280 device\n");
    return(rv);
  }

  printf("Opened BMP 280, Reading Temperature and Pressure...\n");
  
  rv = ioctl(fd, BMP280_read_whoami, (void *)&whoami);
  if (rv<0)
  {
    printf("BMP280 init failed\n");
    return(rv);
  }
  else
  {
    printf("Whoami Main Loop Successful");
  }
  
  rv = ioctl(fd, BMP280_read_Temperature, (void *)&Temperature);
  if (rv<0)
  {
    printf("BMP280 init failed\n");
    return(rv);
  }
  else
  {
    printf("Temperature Main Loop = %d\n", Temperature);
  }

  close(fd);
  return(rv);

}

