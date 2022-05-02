/* Read from onboard temperature and Send to serial output */

#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"

#define DELAY 100

static QueueHandle_t xQueue = NULL;

void temp_task(void * pvParameters)
{
  adc_init();
  adc_set_temp_sensor_enabled(1);
  adc_select_input(4);

  // read temp and add to queue
  while (1)
  {
    uint16_t raw = adc_read();
    xQueueSend(xQueue, &raw, 10U);
    vTaskDelay(DELAY);
  }
}


void led_task(void *pvParameters)
{
  const uint LED_PIN = PICO_DEFAULT_LED_PIN;
  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);
  while (1)
  {
    gpio_put(LED_PIN, 1);
    vTaskDelay(DELAY);

    gpio_put(LED_PIN, 0);
    vTaskDelay(DELAY);
  }
}

void usb_task(void *pvParameters)
{
  uint16_t raw;

  while (1)
  {
    xQueueReceive(xQueue, &raw, portMAX_DELAY);

    // calculate temperature
    const float conversion = 3.3f / (1 << 12);
    float voltage = raw * conversion;
    float temperature = 27 - (voltage - 0.706) / 0.001721;

    // print temperature
    printf("Temperature: %.2f C\n", temperature);
  }
}

int main()
{
  stdio_init_all();

  xQueue = xQueueCreate(1, sizeof(uint16_t));

  xTaskCreate(led_task, "LED_Task", 256, NULL, 1, NULL);
  xTaskCreate(usb_task, "USB_Task", 256, NULL, 2, NULL);
  xTaskCreate(temp_task, "Temp_Task", 256, NULL, 3, NULL);

  vTaskStartScheduler();

  while(1)
    ;
}
