#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <bsp.h> 
#include <bsp/gpio.h>

static void inline delay_sec(int sec)
{
  rtems_task_wake_after(sec*rtems_clock_get_ticks_per_second());
}

int test_gpio_cmd(int argc, char *argv[])
{
  int i, val;
  uint32_t led22, ACT_LED, led3, sw20;
  rtems_status_code sc;

  led22 = 22;
  sw20 = 20;
  led3 = 3;
  ACT_LED = 47;
  
  printf("Starting GPIO testing\n");
  rtems_gpio_initialize (); 
  assert(sc == RTEMS_SUCCESSFUL);

  printf("Calling rtems_gpio_request_pin function for the ACT_LED\n");
 
  /*
   * This Fails
  sc = rtems_gpio_request_pin(
    ACT_LED, DIGITAL_OUTPUT, false, false, NULL);
  if (sc != RTEMS_SUCCESSFUL )
  {
     printf("Failed to request ACT_LED\n");
     return(sc);
  }
  */
  
  printf("Calling rtems_gpio_request_pin for the Breadboard LED at GPIO 22\n");
  sc = rtems_gpio_request_pin(
    led22, DIGITAL_OUTPUT, false, false, NULL);
  if (sc != RTEMS_SUCCESSFUL )
  {
     printf("Failed to request GPIO 22\n");
     return(sc);
  }
  
  printf("Calling rtems_gpio_request_pin for the Breadboard LED at GPIO 3\n");
  sc = rtems_gpio_request_pin(
    led3, DIGITAL_OUTPUT, false, false, NULL);
  if (sc != RTEMS_SUCCESSFUL )
  {
     printf("Failed to request GPIO 3\n");
     return(sc);
  }
  

  for(i=0; i<2; i++)
  {
    rtems_gpio_set(led22);
    delay_sec(2);
    rtems_gpio_clear(led22);
    delay_sec(2);
    
    rtems_gpio_set(led3);
    delay_sec(2);
    rtems_gpio_clear(led3);
    delay_sec(2);
  }

  printf("Calling rtems_gpio_request_pin for the Breadboard Switch at GPIO 20\n");
  sc = rtems_gpio_request_pin(
    sw20, DIGITAL_INPUT, false, false, NULL);
  if (sc != RTEMS_SUCCESSFUL )
  {
     printf("Failed to request GPIO 20\n");
     return(sc);
  }

  /* Enable the internal pull up resistor on the switch */
  sc = rtems_gpio_resistor_mode(sw20, PULL_UP);
  assert(sc == RTEMS_SUCCESSFUL); 
 
  /* Testing GPIO Input, and Lighting up an LED*/
  printf("Press the switch\n");
  while (1){
    val = rtems_gpio_get_value(sw20);
    if (val == 0){
      sc = rtems_gpio_set(led22);
      assert(sc == RTEMS_SUCCESSFUL);
      printf("Switch Pressed, LED will light up for 5 seconds before Test ends\n");
      delay_sec(5);
      break;
    }
    else{
      sc = rtems_gpio_clear(led22);
      assert(sc == RTEMS_SUCCESSFUL);
    }
  }


  /*
  for (i=0; i<54; i++){
    // Rpi has 0-53 GPIOs available, although only 17 are physically available via the GPIO Headers (Including the UART Alt pins 14, 15) 
    if (i<=13 || i>=16){
      
      sc = rtems_gpio_request_pin(i, DIGITAL_OUTPUT, false, false, NULL);
      if(sc != RTEMS_SUCCESSFUL) printf("GPIO Output failed for pin %d\n", i);
      rtems_gpio_release_pin(i);
      
      sc = rtems_gpio_request_pin(i, DIGITAL_INPUT, false, false, NULL);
      if(sc != RTEMS_SUCCESSFUL) printf("GPIO Input failed for pin %d\n", i);
      rtems_gpio_release_pin(i);
    
    }
    printf("GPIO Test Done for %d\n\n",i);
  }
  */

  rtems_gpio_clear(led22);
  rtems_gpio_release_pin(led22);
  rtems_gpio_release_pin(led3);
  rtems_gpio_release_pin(sw20);
  printf("GPIO Test Completed\n");
  return(0);
}


/*
** Initialize GPIO Pin
*/
int init_gpio_cmd(int argc, char *argv[])
{
  rtems_status_code sc;

  printf("Initialize GPIO 22\n");
  
  /* Initializes the GPIO API */
  printf("Calling rtems_gpio_initialize\n");
  rtems_gpio_initialize ();

  printf("Calling rtems_gpio_request_pin function 22\n");
  sc = rtems_gpio_request_pin(
    22, DIGITAL_OUTPUT, false, false, NULL);
  if (sc != RTEMS_SUCCESSFUL )
  {
     printf("Failed to request GPIO 22\n");
     return(sc);
  }
  return(0);
}

/*
** Toggle GPIO on, use only after init_gpio
*/
int toggle_gpio_on_cmd( int argc, char *argv[])
{
  printf("Toggle GPIO 22 on\n");
  rtems_gpio_set(22);

  return(0);
}

/*
** Toggle GPIO off, use only after init_gpio
*/
int toggle_gpio_off_cmd( int argc, char *argv[])
{
  printf("Toggle GPIO 22 off\n");
  rtems_gpio_clear(22);

  return(0);
}

/*
** Toggle GPIO off, use only after init_gpio
*/
int release_gpio_cmd( int argc, char *argv[])
{
  printf("Release GPIO 22 pin\n");
  rtems_gpio_release_pin(22);
  
  return(0);
}
