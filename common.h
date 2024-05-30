//*****************************************************************************
//
// Copyright: 2024, �Ϻ���ͨ��ѧ����ʵ����Ƽ�����II-47��
// File name: common.h
// Description: ���ն˺ͷ����ͨ�ú���
// Author:	�Ϻ���ͨ��ѧ����ʵ����Ƽ�����II-47��
// Version: 1.0.0.20240506
// Date��2024-05-06
// History��
//
//*****************************************************************************

#ifndef __COMMON_H__
#define __COMMON_H__

#ifdef __cplusplus
extern "C" {
#endif

//*****************************************************************************
//
// ͷ�ļ�
//
//*****************************************************************************
#include <stdint.h>

//*****************************************************************************
//
// �궨��
//
//*****************************************************************************
// ����ѡ��
#define ENABLE_DEBUG                // ʹ��DEBUG��������Դ��ڣ���������ע�ͱ���

// ʱ����ض���
#define SYSTICK_FREQUENCY   50		// SysTickƵ��Ϊ50Hz����ѭ����ʱ����20ms
#define V_T100ms	        5       // 0.1s=5*20ms
#define V_T1s	            50      // 1s=50*20ms
#define V_T2s               100     // 2s=100*20ms
#define V_T10s              500     // 10s=500*20ms

// �����궨��
#ifdef ENABLE_DEBUG
#   define DEBUG(msg)        UARTStringPutNOBlocking(UART0_BASE, (uint8_t*)(msg))
#   define DEBUGN(n)         UARTNumberPutNOBlocking(UART0_BASE, (n))
#else
#   define DEBUG(msg)
#   define DEBUGN(n)
#endif

//*****************************************************************************
//
// ����ԭ������
//
//*****************************************************************************
void GPIOInit(void);            // GPIO��ʼ��

void SysTickInit(void);         // ����SysTick�ж�
void ResetClocks(void);         // ������ʱ��

void UARTInit(void);            // UART��ʼ��
void UARTStringPutNOBlocking(uint32_t ui32Base, uint8_t *cMessage); // ��UART�����ַ���
void UARTNumberPutNOBlocking(uint32_t ui32Base, int32_t data); // ��������

void PWMInit(void);             // PWM��ʼ��
void PWMStart(uint32_t freq);   // ����Ƶ��Ϊfreq����
void PWMStop(void);             // PWMֹͣ����PWM�ź�

void BuzzerStart(uint32_t freq);// ������
void BuzzerStop(void);

void ConvertTemperatureToString(unsigned char *buffer, int16_t temp);
void ConvertVoltageToString(unsigned char *buffer, uint32_t voltage);

// �Ѷ�����жϷ������
void SysTick_Handler(void);
void UART6_Handler(void);

//*****************************************************************************
//
// ��������
//
//*****************************************************************************
// ϵͳʱ��Ƶ��
extern uint32_t g_ui32SysClock;

// �����ʱ�������־
extern volatile uint8_t clock100ms_flag;
extern volatile uint8_t clock1s_flag;
extern volatile uint8_t clock2s_flag;

// ����ܺ�LED����
// 8λ�������ʾ�����ֻ���ĸ����
// ע����������λ�������������Ϊ4��5��6��7��0��1��2��3
extern uint8_t digit[8];
// 8λС���� 1��  0��
// ע����������λС����������������Ϊ4��5��6��7��0��1��2��3
extern uint8_t pnt;
// 8��LEDָʾ��״̬��0��1��
// ע������ָʾ�ƴ������������Ϊ7��6��5��4��3��2��1��0
//     ��ӦԪ��LED8��LED7��LED6��LED5��LED4��LED3��LED2��LED1
extern uint8_t led[8];

// UART��������
extern uint8_t uart_receive[20];
extern uint8_t uart_receive_flag;

#ifdef __cplusplus
}
#endif

#endif
