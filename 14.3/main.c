#include "stm32f10x.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_pwr.h"
#include "stm32f10x_rtc.h"
#include "stm32f10x_bkp.h"
#include "stm32f10x_exti.h"

/* ---------------- Function Prototypes ---------------- */
void DelayMs(uint32_t ms);
void UART1_Init(void);
void UART1_SendString(const char *str);
void RTC_Config(void);
void EXTI17_Config(void);

/* ---------------- Delay Function ---------------- */
void DelayMs(uint32_t time) {
    for (uint32_t i = 0; i < time; i++) {
        for (uint32_t j = 0; j < 7200; j++);
    }
}

/* ---------------- UART Config ---------------- */
void UART1_Init(void) {
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1, ENABLE);

    // PA9 = TX
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // PA10 = RX (not used but needed)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate = 9600;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx;
    USART_Init(USART1, &USART_InitStructure);

    USART_Cmd(USART1, ENABLE);
}

/* ---------------- UART Send String ---------------- */
void UART1_SendString(const char *str) {
    while (*str) {
        USART_SendData(USART1, *str++);
        while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
    }
    while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
}

/* ---------------- RTC Config (LSI) ---------------- */
void RTC_Config(void) {
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
    PWR_BackupAccessCmd(ENABLE);

    BKP_DeInit();

    RCC_LSICmd(ENABLE);
    while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET);

    RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);
    RCC_RTCCLKCmd(ENABLE);

    RTC_WaitForSynchro();
    RTC_WaitForLastTask();

    RTC_SetPrescaler(40000 - 1); // LSI ~40kHz -> 1Hz
    RTC_WaitForLastTask();

    RTC_ITConfig(RTC_IT_ALR, ENABLE);
    RTC_WaitForLastTask();
}

/* ---------------- EXTI Line17 (RTC Alarm) ---------------- */
void EXTI17_Config(void) {
    EXTI_InitTypeDef EXTI_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

    EXTI_ClearITPendingBit(EXTI_Line17);
    EXTI_InitStructure.EXTI_Line = EXTI_Line17;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);
}

/* ---------------- MAIN ---------------- */
int main(void) {
    SystemInit();
    UART1_Init();
    EXTI17_Config();

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
    PWR_BackupAccessCmd(ENABLE);

    if (PWR_GetFlagStatus(PWR_FLAG_SB) != RESET) {
        /* Wakeup from STANDBY */
        UART1_Init(); // Re-init UART because it resets
        UART1_SendString("Wake from STANDBY!\r\n");

        PWR_ClearFlag(PWR_FLAG_SB);
        RTC_ClearFlag(RTC_FLAG_ALR);
        EXTI_ClearITPendingBit(EXTI_Line17);
        RTC_WaitForSynchro();
        DelayMs(1000);
    } else {
        /* Normal power-on */
        UART1_SendString("Normal startup\r\n");
        RTC_Config();
    }

    RTC_WaitForLastTask();
    RTC_SetAlarm(RTC_GetCounter() + 5);
    RTC_WaitForLastTask();

    UART1_SendString("Entering STANDBY mode...\r\n");
    DelayMs(1000);

    PWR_ClearFlag(PWR_FLAG_WU);
    PWR_EnterSTANDBYMode();

    while (1);
}

/* ---------------- RTC IRQ Handler ---------------- */
void RTC_IRQHandler(void) {
    if (RTC_GetITStatus(RTC_IT_ALR) != RESET) {
        RTC_ClearITPendingBit(RTC_IT_ALR);
        EXTI_ClearITPendingBit(EXTI_Line17);
    }
}
