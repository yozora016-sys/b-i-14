#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"

/* ---------------- Handle ---------------- */
EventGroupHandle_t xEventGroup;

/* ---------------- Bit d?nh nghia ---------------- */
#define LED1_BIT   (1 << 0)
#define LED2_BIT   (1 << 1)
#define LED3_BIT   (1 << 2)

/* ---------------- Prototype ---------------- */
void LED_Config(void);
void Task_Controller(void *pvParameters);
void Task_LED1(void *pvParameters);
void Task_LED2(void *pvParameters);
void Task_LED3(void *pvParameters);

/* ---------------- Main ---------------- */
int main(void)
{
    SystemInit();
    LED_Config();

    xEventGroup = xEventGroupCreate();

    xTaskCreate(Task_Controller, "CTRL", 128, NULL, 2, NULL);
    xTaskCreate(Task_LED1, "LED1", 128, NULL, 1, NULL);
    xTaskCreate(Task_LED2, "LED2", 128, NULL, 1, NULL);
    xTaskCreate(Task_LED3, "LED3", 128, NULL, 1, NULL);

    vTaskStartScheduler();

    while (1);
}

/* ---------------- Task chính ---------------- */
void Task_Controller(void *pvParameters)
{
		(void) pvParameters;
    while (1)
    {
        // Kích ho?t LED1 và LED2 cùng lúc
        xEventGroupSetBits(xEventGroup, LED1_BIT | LED2_BIT);
        vTaskDelay(pdMS_TO_TICKS(3000));

        // T?t LED1, LED2, b?t LED3
        xEventGroupClearBits(xEventGroup, LED1_BIT | LED2_BIT);
        xEventGroupSetBits(xEventGroup, LED3_BIT);
        vTaskDelay(pdMS_TO_TICKS(3000));

        // Kích ho?t d?ng th?i c? 3
        xEventGroupSetBits(xEventGroup, LED1_BIT | LED2_BIT | LED3_BIT);
        vTaskDelay(pdMS_TO_TICKS(3000));

        // T?t t?t c?
        xEventGroupClearBits(xEventGroup, LED1_BIT | LED2_BIT | LED3_BIT);
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

/* ---------------- Task ph? ---------------- */
void Task_LED1(void *pvParameters)
{
		(void) pvParameters;
    EventBits_t uxBits;
    while (1)
    {
        uxBits = xEventGroupWaitBits(xEventGroup, LED1_BIT, pdFALSE, pdFALSE, portMAX_DELAY);
        if (uxBits & LED1_BIT)
            GPIOC->ODR ^= (1 << 13);  // Toggle LED1
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void Task_LED2(void *pvParameters)
{
		(void) pvParameters;
    EventBits_t uxBits;
    while (1)
    {
        uxBits = xEventGroupWaitBits(xEventGroup, LED2_BIT, pdFALSE, pdFALSE, portMAX_DELAY);
        if (uxBits & LED2_BIT)
            GPIOC->ODR ^= (1 << 14);
        vTaskDelay(pdMS_TO_TICKS(700));
    }
}

void Task_LED3(void *pvParameters)
{
		(void) pvParameters;
    EventBits_t uxBits;
    while (1)
    {
        uxBits = xEventGroupWaitBits(xEventGroup, LED3_BIT, pdFALSE, pdFALSE, portMAX_DELAY);
        if (uxBits & LED3_BIT)
            GPIOC->ODR ^= (1 << 15);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

/* ---------------- C?u hình LED ---------------- */
void LED_Config(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    GPIO_InitTypeDef GPIO_InitStruct;

    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOC, &GPIO_InitStruct);

    GPIO_SetBits(GPIOC, GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15);
}
