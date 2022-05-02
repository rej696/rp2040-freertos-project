/* Read from onboard temperature and Send to serial output */

#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"

static QueueHandle_t xQueue = NULL;

void temp_task(void * pvParameters)
{
  adc_init();
  adc_set_temp_sensor_enabled(1);
  adc_select_input(4);
  uint16_t raw;

  // read temp and add to queue
  while (1)
  {
    raw = adc_read();
    xQueueSend(xQueue, &raw, 0U);
    vTaskDelay(100);
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
    vTaskDelay(100);

    gpio_put(LED_PIN, 0);
    vTaskDelay(100);
  }
}

void usb_task(void *pvParameters)
{
  uint16_t uIReceivedValue;

  while (1)
  {
    xQueueReceive(xQueue, &uIReceivedValue, portMAX_DELAY);

    // calculate temperature
    const float conversion = 3.3f / (1 << 12);
    float voltage = uIRecievedValue * conversion;
    float temperature = 27 - (voltage - 0.706) / 0.001721;

    // print temperature
    printf("Temperature: %f.2 C", temperature);
    }
  }
}

int main()
{
  stdio_init_all();

  xQueue = xQueueCreate(1, sizeof(uint16_t));

  xTaskCreate(led_task, "LED_Task", 256, NULL, 2, NULL);
  xTaskCreate(usb_task, "USB_Task", 256, NULL, 1, NULL);
  xTaskCreate(temp_task, "Temp_Task", 256, NULL, 3, NULL);

  vTaskStartScheduler();

  while(1)
    ;
}
