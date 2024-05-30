//*****************************************************************************
//
// Copyright: 2024, 上海交通大学工程实践与科技创新II-47组
// File name: common.c
// Description: 接收端和发射端通用函数
// Author:	上海交通大学工程实践与科技创新II-47组
// Version: 1.0.0.20240506
// Date：2024-05-06
// History：
//
//*****************************************************************************

//*****************************************************************************
//
// 头文件
//
//*****************************************************************************
#include "common.h"
#include "ui.h"
#include <stdbool.h>
#include "inc/hw_ints.h"          // 与中断有关的宏定义
#include "driverlib/interrupt.h"  // NVIC Interrupt Controller Driver 原型
#include "driverlib/pwm.h"        // 与PWM有关的函数原型
#include "driverlib/systick.h"    // SysTick Driver 原型
#include "driverlib/uart.h"       // 与UART有关的宏定义和函数原型
#include "tm1638.h"               // 与控制TM1638芯片有关的函数
#include "JLX12864G.h"            // LCD显示屏

//*****************************************************************************
//
// 变量定义
//
//*****************************************************************************
uint32_t g_ui32SysClock;

volatile uint8_t clock100ms_flag = 0;
volatile uint8_t clock1s_flag = 0;
volatile uint8_t clock2s_flag = 0;

static volatile uint8_t clock100ms = 0;
static volatile uint8_t clock1s = 0;
static volatile uint8_t clock2s = 0;

// 8位数码管显示的数字或字母符号
// 注：板上数码位从左到右序号排列为4、5、6、7、0、1、2、3
uint8_t digit[8]={' ',' ',' ',' ','_',' ','_',' '};
// 8位小数点 1亮  0灭
// 注：板上数码位小数点从左到右序号排列为4、5、6、7、0、1、2、3
uint8_t pnt = 0x04;
// 8个LED指示灯状态，0灭，1亮
// 注：板上指示灯从左到右序号排列为7、6、5、4、3、2、1、0
//     对应元件LED8、LED7、LED6、LED5、LED4、LED3、LED2、LED1
uint8_t led[] = {1, 1, 1, 1, 1, 1, 1, 0};

uint8_t uart_receive[20];
uint8_t uart_receive_flag = 0;

//*****************************************************************************
//
// 函数原型：void GPIOInit(void)
// 函数功能：GPIO初始化。
//          PK4>TM1638STB / PK5>TM1638DIO / PM0>TM1638CLK
// 函数参数：无
// 函数返回值：无
//
//*****************************************************************************
void GPIOInit(void) {
    //配置TM1638芯片管脚
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOK);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOK));
    
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOM);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOM));
    
    // PK4 > STB, PK5 > DIO
    GPIOPinTypeGPIOOutput(GPIO_PORTK_BASE, GPIO_PIN_4 | GPIO_PIN_5);
    // PM0 > CLK
    GPIOPinTypeGPIOOutput(GPIO_PORTM_BASE, GPIO_PIN_0);	
        
    //配置JLX12864G屏幕管脚
    LCD_PORT_init();
}

//*****************************************************************************
// 
// 函数原型：void SysTickInit(void)
// 函数功能：设置SysTick中断
// 函数参数：无
// 函数返回值：无
//
//*****************************************************************************
void SysTickInit(void) {
    SysTickPeriodSet(g_ui32SysClock / SYSTICK_FREQUENCY); // 设置心跳节拍,定时周期20ms
    SysTickEnable();  			// SysTick使能
    SysTickIntEnable();			// SysTick中断允许
}

//*****************************************************************************
// 
// 函数原型：void ResetClocks(void)
// 函数功能：重置软件定时器
// 函数参数：无
// 函数返回值：无
//
//*****************************************************************************
void ResetClocks(void) {
    clock100ms = clock1s = clock2s = 0;
    clock100ms_flag = clock1s_flag = clock2s_flag = 0;
}

//*****************************************************************************
//
// 函数原型：void UARTInit(void)
// 函数功能：UART初始化。使能UART0，设置PA0,PA1为UART0 RX,TX引脚；
//          设置波特率及帧格式。
// 函数参数：无
// 函数返回值：无
//
//*****************************************************************************
void UARTInit(void) {
    // 引脚配置
    
    // PA0 > U0RX, PA1 > U0TX
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA));

    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);     

	// 设置端口为UART引脚
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);   

    // 波特率及帧格式设置
    UARTConfigSetExpClk(UART0_BASE, 
	                    g_ui32SysClock,
	                    115200,                  // 波特率：115200
	                    (UART_CONFIG_WLEN_8 |    // 数据位：8
	                     UART_CONFIG_STOP_ONE |  // 停止位：1
	                     UART_CONFIG_PAR_NONE)); // 校验位：无
	
    UARTFIFOLevelSet(UART0_BASE, UART_FIFO_TX1_8, UART_FIFO_RX7_8);
    IntEnable(INT_UART0); // UART 中断允许
    UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT); // 使能UART RX,RT 中断
    
    // PP0 > U6RX, PP1 > U6TX
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART6);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOP);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOP));

    GPIOPinConfigure(GPIO_PP0_U6RX);
    GPIOPinConfigure(GPIO_PP1_U6TX);     

	// 设置端口为UART引脚
    GPIOPinTypeUART(GPIO_PORTP_BASE, GPIO_PIN_0 | GPIO_PIN_1);   

    // 波特率及帧格式设置
#ifdef CODE_TRANSMITTER
    UARTConfigSetExpClk(UART6_BASE, 
	                    g_ui32SysClock,
	                    38400,                  // 波特率：38400
	                    (UART_CONFIG_WLEN_8 |    // 数据位：8
	                     UART_CONFIG_STOP_ONE |  // 停止位：1
	                     UART_CONFIG_PAR_NONE)); // 校验位：无
#else
    UARTConfigSetExpClk(UART6_BASE, 
	                    g_ui32SysClock,
	                    9600,                  // 波特率：38400
	                    (UART_CONFIG_WLEN_8 |    // 数据位：8
	                     UART_CONFIG_STOP_ONE |  // 停止位：1
	                     UART_CONFIG_PAR_NONE)); // 校验位：无
#endif
	
    UARTFIFOLevelSet(UART6_BASE, UART_FIFO_TX1_8, UART_FIFO_RX7_8);
    IntEnable(INT_UART6); // UART 中断允许
    UARTIntEnable(UART6_BASE, UART_INT_RX | UART_INT_RT); // 使能UART RX,RT 中断
}

//*****************************************************************************
//
// 函数原型：void UARTStringPut(uint32_t ui32Base, const char *cMessage)
// 函数功能：向UART模块发送字符串
// 函数参数：ui32Base - UART模块
//          cMessage - 待发送字符串  
// 函数返回值：无
//
//*****************************************************************************
void UARTStringPutNOBlocking(uint32_t ui32Base, uint8_t *cMessage) {
    bool TXFIFO_free = 0;
    
	while(*cMessage != '\0') {
        TXFIFO_free = UARTCharPutNonBlocking(ui32Base, *(cMessage));
        if(TXFIFO_free) {  
            cMessage++;
        }
        TXFIFO_free = 0;
    }
}

//*****************************************************************************
//
// 函数原型：UARTNumberPutNOBlocking(uint32_t ui32Base, int32_t data)
// 函数功能：向UART模块发送数字
// 函数参数：ui32Base - UART模块
//          data - 待发送数字
// 函数返回值：无
//
//*****************************************************************************
void UARTNumberPutNOBlocking(uint32_t ui32Base, int32_t data) {
    static uint8_t buffer[15];
    uint8_t flag = 0;
    uint8_t cur = 14;
    
    if (data == 0) {
        UARTStringPutNOBlocking(ui32Base, (uint8_t *)"0");
        return;
    }
    
    if (data < 0) {
        flag = 1;
        data = -data;
    }
    
    buffer[14] = '\0';
    while (cur > 0 && data) {
        --cur;
        buffer[cur] = data % 10 + '0';
        data /= 10;
    }
    
    if (flag) {
        buffer[--cur] = '-';
    }
    
    UARTStringPutNOBlocking(ui32Base, buffer + cur);
}

//*******************************************************************************************************
// 
// 函数原型：void PWMInit(void)
// 函数功能：初始化PWM0，PG0>PWM4 / PG1>PWM5 / PF3>PWM3
// 函数参数：无
// 函数返回值：无
//
//*******************************************************************************************************
void PWMInit(void) {
    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);     // PWM0使能   
    
    PWMOutputState(PWM0_BASE, PWM_OUT_3_BIT | PWM_OUT_4_BIT | PWM_OUT_5_BIT, true); // 使能(允许)PWM0_4的输出
    
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOG);    // 使能GPIOG
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);    // 使能GPIOF
    GPIOPinConfigure(GPIO_PG0_M0PWM4);              // 配置引脚复用
    GPIOPinConfigure(GPIO_PG1_M0PWM5);
    GPIOPinConfigure(GPIO_PF3_M0PWM3);
    GPIOPinTypePWM(GPIO_PORTG_BASE, GPIO_PIN_0 | GPIO_PIN_1);   // 引脚映射
    GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_3);
    
    PWMGenConfigure(PWM0_BASE, PWM_GEN_1, PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC);
    PWMGenConfigure(PWM0_BASE, PWM_GEN_2, PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC);   //配置PWM发生器
}

//*******************************************************************************************************
// 
// 函数原型：void PWMStart(uint32_t freq)
// 函数功能：产生频率方波，占空比为50%的PWM
// 函数参数：freq - 需要产生的方波的频率
// 函数返回值：无
//
//*******************************************************************************************************
void PWMStart(uint32_t freq) {
    PWMGenDisable(PWM0_BASE, PWM_GEN_2);     //使能PWM0模块的2号发生器(因为4号PWM是2号发生器产生的)   
    
    PWMGenPeriodSet(PWM0_BASE, PWM_GEN_2, g_ui32SysClock / freq); // 根据Freq_Hz设置PWM周期
    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_4,(PWMGenPeriodGet(PWM0_BASE, PWM_GEN_2)/ 2)); //设置占空比为50%
    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_5,(PWMGenPeriodGet(PWM0_BASE, PWM_GEN_2)/ 2)); //设置占空比为50%
    
    PWMGenEnable(PWM0_BASE, PWM_GEN_2);     //使能PWM0模块的2号发生器(因为4号PWM是2号发生器产生的)   
}

//*******************************************************************************************************
// 
// 函数原型：void PWMStop()
// 函数功能：M0PWM4(PG0)停止产生PWM信号
// 函数参数：无
// 函数返回值：无
//
//*******************************************************************************************************
void PWMStop() {
    PWMGenDisable(PWM0_BASE, PWM_GEN_2);   // M0PWM4(PG0)停止产生PWM信号
}

//*******************************************************************************************************
// 
// 函数原型：void BuzzerStart(uint32_t freq)
// 函数功能：蜂鸣器输出PWM信号
// 函数参数：freq - PWM频率
// 函数返回值：无
//
//*******************************************************************************************************
void BuzzerStart(uint32_t freq) {
    PWMGenPeriodSet(PWM0_BASE, PWM_GEN_1, g_ui32SysClock / freq);
    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_3, PWMGenPeriodGet(PWM0_BASE, PWM_GEN_1) / 2);
    PWMGenEnable(PWM0_BASE, PWM_GEN_1);
}

//*******************************************************************************************************
// 
// 函数原型：void BuzzerStop()
// 函数功能：停止蜂鸣器输出
// 函数参数：无
// 函数返回值：无
//
//*******************************************************************************************************
void BuzzerStop() {
    PWMGenDisable(PWM0_BASE, PWM_GEN_1);
}

//*****************************************************************************
// 
// 函数原型：void ConvertTemperatureToString(unsigned char *buffer, int16_t temp)
// 函数功能：将温度转换为字符串
// 函数参数：buffer - 字符串缓冲区
//          temp - 温度值
// 函数返回值：无
//
//*****************************************************************************
void ConvertTemperatureToString(unsigned char *buffer, int16_t temp) {
    if (temp < 0) {
        buffer[0] = '-';
        temp = -temp;
    } else {
        buffer[0] = ' ';
    }
    
    buffer[1] = temp / 100 % 10 + '0';  // 计算十位数
    buffer[2] = temp / 10 % 10 + '0'; 	// 计算个位数
    buffer[3] = '.';
    buffer[4] = temp % 10 + '0';        // 计算十分位数
    buffer[5] = CHAR_CELSIUS / 256;
    buffer[6] = CHAR_CELSIUS % 256;
    buffer[7] = '\0';
}

//*****************************************************************************
// 
// 函数原型：void ConvertTemperatureToString(unsigned char *buffer, int16_t temp)
// 函数功能：将温度转换为字符串
// 函数参数：buffer - 字符串缓冲区
//          voltage - 电压值
// 函数返回值：无
//
//*****************************************************************************
void ConvertVoltageToString(unsigned char *buffer, uint32_t voltage) {
    buffer[0] = (voltage / 100) % 10 + '0'; // 显示电压值个位数
    buffer[1] = '.';
    buffer[2] = (voltage / 10) % 10 + '0';  // 显示电压值十分位数
    buffer[3] = voltage % 10 + '0';         // 显示电压值百分位数
    buffer[4] = 'V';
    buffer[5] = '\0';
}

//*****************************************************************************
// 
// 函数原型：void SysTick_Handler(void)
// 函数功能：SysTick中断服务程序
// 函数参数：无
// 函数返回值：无
//
//*****************************************************************************
void SysTick_Handler(void) {
    // 软件定时器计数
    if (++clock100ms >= V_T100ms) {
        clock100ms_flag = 1;
        clock100ms = 0;
    }

    if (++clock1s >= V_T1s) {
        clock1s_flag = 1;
        clock1s = 0;
    }

    if (++clock2s >= V_T2s) {
        clock2s_flag = 1;
        clock2s = 0;
    }

    // 刷新全部数码管和LED指示灯
    TM1638_RefreshDIGIandLED(digit, pnt, led);

    // 检查当前键盘输入
    DetectKey(TM1638_Readkeyboard());
}

//*****************************************************************************
// 
// 函数原型：void UART6_Handler(void)
// 函数功能：UART6中断服务程序
// 函数参数：无
// 函数返回值：无
//
//*****************************************************************************
void UART6_Handler(void) {
    int32_t uart_int_status;
    uint8_t uart_receive_char;
    static uint8_t uart_receive_len = 0;
    volatile uint32_t i;
    
    uart_int_status = UARTIntStatus(UART6_BASE, true); // 取中断状态
    UARTIntClear(UART6_BASE, uart_int_status); // 清中断标志
  
    while(UARTCharsAvail(UART6_BASE)) { // 重复从接收FIFO 读取字符
        uart_receive_char = UARTCharGetNonBlocking(UART6_BASE); // 读入一个字符
        
        if(uart_receive_char != '\n') { // 接收未结束
            uart_receive[uart_receive_len++] = uart_receive_char;
        } else {
            uart_receive[uart_receive_len] = '\0';
            uart_receive_flag = 1;
            uart_receive_len = 0;
        }
    }
}
