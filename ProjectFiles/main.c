#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <stdio.h>
#include <semphr.h>
#include "pico/stdlib.h"

#define DELAY_MULT 5
#define DELAY_MOD 5

static QueueHandle_t xQueue = NULL;
static SemaphoreHandle_t mutex;
static uint counter = 0;
static uint print_delay = 0;

void led_task(void *pvParameters)
{
  const uint LED_PIN = PICO_DEFAULT_LED_PIN;
  uint uIValueToSend = 0;
  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);
  while (1)
  {
    gpio_put(LED_PIN, 1);
    uIValueToSend = 1;
    xQueueSend(xQueue, &uIValueToSend, 0U);
    vTaskDelay(100);

    gpio_put(LED_PIN, 0);
    uIValueToSend = 0;
    xQueueSend(xQueue, &uIValueToSend, 0U);
    vTaskDelay(100);
  }
}

void usb_task(void *pvParameters)
{
  uint uIReceivedValue;

  while (1)
  {
    xQueueReceive(xQueue, &uIReceivedValue, portMAX_DELAY);
    if (xSemaphoreTake(mutex, 0) == pdTRUE)
    {
      printf("\n%s", (uIReceivedValue ? "LED is ON!" : "LED is OFF!"));
      counter = ++counter % DELAY_MOD;
      print_delay = (counter + 1) * DELAY_MULT;
      xSemaphoreGive(mutex);
    }
  }
}

void low_priority_task(void *pvParameters)
{
  while (1)
  {
    if (xSemaphoreTake(mutex, 0) == pdTRUE)
    {
      printf(".");
      xSemaphoreGive(mutex);
    }
    vTaskDelay(print_delay);
  }
}

void mutex_task1()
{
  char ch = '1';
  while (1)
  {
    for (int i = 1; i < 10; ++i)
      putchar(ch);

    putchar('\n');
  }
}

void mutex_task2()
{
  char ch = '2';
  while (1)
  {
    for (int i = 1; i < 10; ++i)
      putchar(ch);

    putchar('\n');
  }
}

int main()
{
  stdio_init_all();

  xQueue = xQueueCreate(1, sizeof(uint));
  mutex = xSemaphoreCreateMutex();

  xTaskCreate(led_task, "LED_Task", 256, NULL, 3, NULL);
  xTaskCreate(usb_task, "USB_Task", 256, NULL, 2, NULL);
  xTaskCreate(low_priority_task, "Low_Priority_Task", 256, NULL, 1, NULL);

  vTaskStartScheduler();

  while(1)
    ;
}
