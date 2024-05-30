//*****************************************************************************
//
// Copyright: 2024, 上海交通大学工程实践与科技创新II-47组
// File name: common.h
// Description: 接收端和发射端通用函数
// Author:	上海交通大学工程实践与科技创新II-47组
// Version: 1.0.0.20240506
// Date：2024-05-06
// History：
//
//*****************************************************************************

#ifndef __COMMON_H__
#define __COMMON_H__

#ifdef __cplusplus
extern "C" {
#endif

//*****************************************************************************
//
// 头文件
//
//*****************************************************************************
#include <stdint.h>

//*****************************************************************************
//
// 宏定义
//
//*****************************************************************************
// 编译选项
#define ENABLE_DEBUG                // 使用DEBUG输出到电脑串口，生产环境注释本行

// 时钟相关定义
#define SYSTICK_FREQUENCY   50		// SysTick频率为50Hz，即循环定时周期20ms
#define V_T100ms	        5       // 0.1s=5*20ms
#define V_T1s	            50      // 1s=50*20ms
#define V_T2s               100     // 2s=100*20ms
#define V_T10s              500     // 10s=500*20ms

// 其他宏定义
#ifdef ENABLE_DEBUG
#   define DEBUG(msg)        UARTStringPutNOBlocking(UART0_BASE, (uint8_t*)(msg))
#   define DEBUGN(n)         UARTNumberPutNOBlocking(UART0_BASE, (n))
#else
#   define DEBUG(msg)
#   define DEBUGN(n)
#endif

//*****************************************************************************
//
// 函数原型声明
//
//*****************************************************************************
void GPIOInit(void);            // GPIO初始化

void SysTickInit(void);         // 设置SysTick中断
void ResetClocks(void);         // 重置软时钟

void UARTInit(void);            // UART初始化
void UARTStringPutNOBlocking(uint32_t ui32Base, uint8_t *cMessage); // 向UART发送字符串
void UARTNumberPutNOBlocking(uint32_t ui32Base, int32_t data); // 发送数字

void PWMInit(void);             // PWM初始化
void PWMStart(uint32_t freq);   // 产生频率为freq方波
void PWMStop(void);             // PWM停止产生PWM信号

void BuzzerStart(uint32_t freq);// 蜂鸣器
void BuzzerStop(void);

void ConvertTemperatureToString(unsigned char *buffer, int16_t temp);
void ConvertVoltageToString(unsigned char *buffer, uint32_t voltage);

// 已定义的中断服务程序
void SysTick_Handler(void);
void UART6_Handler(void);

//*****************************************************************************
//
// 变量声明
//
//*****************************************************************************
// 系统时钟频率
extern uint32_t g_ui32SysClock;

// 软件定时器溢出标志
extern volatile uint8_t clock100ms_flag;
extern volatile uint8_t clock1s_flag;
extern volatile uint8_t clock2s_flag;

// 数码管和LED驱动
// 8位数码管显示的数字或字母符号
// 注：板上数码位从左到右序号排列为4、5、6、7、0、1、2、3
extern uint8_t digit[8];
// 8位小数点 1亮  0灭
// 注：板上数码位小数点从左到右序号排列为4、5、6、7、0、1、2、3
extern uint8_t pnt;
// 8个LED指示灯状态，0灭，1亮
// 注：板上指示灯从左到右序号排列为7、6、5、4、3、2、1、0
//     对应元件LED8、LED7、LED6、LED5、LED4、LED3、LED2、LED1
extern uint8_t led[8];

// UART接收内容
extern uint8_t uart_receive[20];
extern uint8_t uart_receive_flag;

#ifdef __cplusplus
}
#endif

#endif
