#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <bsp.h> 
#include <bsp/gpio.h>
#include <rtems/bspIo.h>

rtems_gpio_irq_state handle_irq(void *gpio_pin);

static void inline delay_sec(int sec)
{
  rtems_task_wake_after(sec*rtems_clock_get_ticks_per_second());
}

int irq_test_cmd(int argc, char* argv[])
{
  rtems_status_code sc;
  uint32_t led22, ACT_LED, led3, sw20;
  led22 = 22;
  sw20 = 20;
  led3 = 3;
  ACT_LED = 47;

  printf("Initializing GPIO\n");
  sc = rtems_gpio_initialize();
  assert(sc == RTEMS_SUCCESSFUL);

  printf("Calling rtems_gpio_request_pin for the Breadboard LED at GPIO 22\n");
  sc = rtems_gpio_request_pin(
    led22, DIGITAL_OUTPUT, false, false, NULL);
  if (sc != RTEMS_SUCCESSFUL )
  {
     printf("Failed to request GPIO 22\n");
     return(sc);
  }
  
  printf("Calling rtems_gpio_request_pin for the Activity LED at GPIO 47\n");
  sc = rtems_gpio_request_pin(
    ACT_LED, DIGITAL_OUTPUT, false, false, NULL);
  if (sc != RTEMS_SUCCESSFUL )
  {
     printf("Failed to request GPIO 47\n");
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

  printf("Calling rtems_gpio_request_pin for the Breadboard Switch at GPIO 20\n");
  sc = rtems_gpio_request_pin(
    sw20, DIGITAL_INPUT, false, false, NULL);
  if (sc != RTEMS_SUCCESSFUL )
  {
     printf("Failed to request GPIO 20\n");
     return(sc);
  }

  /* Enable the internal pull up resistor on the switch */
  printf("Enabling the Pull up resistor on the switch\n");
  sc = rtems_gpio_resistor_mode(sw20, PULL_UP);
  assert(sc == RTEMS_SUCCESSFUL);

  /* Enable the Interrupts and the function that is called to handle the interrupt */   
  printf("\nEnabling Interrupt on switch at GPIO 20\n"); 
  sc = rtems_gpio_enable_interrupt(
      sw20, FALLING_EDGE, UNIQUE_HANDLER, true, handle_irq, &sw20);
  if (sc != RTEMS_SUCCESSFUL )
  {
     printf("\nFailed to Enable Interrupt\n");
     return(sc);
  }
  
  printf("\nPress the switch (GPIO 20) in the next 10 seconds.\n");
  
  delay_sec(10);
  

  rtems_gpio_clear(led22);
  rtems_gpio_clear(led3);
  rtems_gpio_set(ACT_LED);
  rtems_gpio_release_pin(led22);
  rtems_gpio_release_pin(led3);
  rtems_gpio_release_pin(ACT_LED);
  printf("GPIO Out pins Released\n");
  
  sc = rtems_gpio_release_pin(sw20);
  assert(sc == RTEMS_SUCCESSFUL);
  printf("GPIO In pin Released\n");
  
  printf("GPIO Test Completed\n");
  
  return(0);
}


rtems_gpio_irq_state handle_irq(void *gpio_pin)
{
  rtems_status_code sc;
  uint32_t led22, ACT_LED, led3;
  led22 = 22;
  led3 = 3;
  ACT_LED = 47;

  int value;
  uint32_t pin = *((uint32_t*) gpio_pin);
  value = rtems_gpio_get_value(pin);
  
  if (value == 0){
    sc = rtems_gpio_set(led22);
    assert(sc == RTEMS_SUCCESSFUL);
    sc = rtems_gpio_clear(ACT_LED);
    assert(sc == RTEMS_SUCCESSFUL);
    sc = rtems_gpio_set(led3);
    assert(sc == RTEMS_SUCCESSFUL);
    printf("Switch Pressed, LED will light up before test ends\n");
  }
  else{
    sc = rtems_gpio_clear(led22);
    assert(sc == RTEMS_SUCCESSFUL);
    sc = rtems_gpio_clear(led3);
    assert(sc == RTEMS_SUCCESSFUL);
    sc = rtems_gpio_set(ACT_LED);
    assert(sc == RTEMS_SUCCESSFUL);
  }

  return IRQ_HANDLED;
}
