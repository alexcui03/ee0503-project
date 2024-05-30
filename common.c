//*****************************************************************************
//
// Copyright: 2024, �Ϻ���ͨ��ѧ����ʵ����Ƽ�����II-47��
// File name: common.c
// Description: ���ն˺ͷ����ͨ�ú���
// Author:	�Ϻ���ͨ��ѧ����ʵ����Ƽ�����II-47��
// Version: 1.0.0.20240506
// Date��2024-05-06
// History��
//
//*****************************************************************************

//*****************************************************************************
//
// ͷ�ļ�
//
//*****************************************************************************
#include "common.h"
#include "ui.h"
#include <stdbool.h>
#include "inc/hw_ints.h"          // ���ж��йصĺ궨��
#include "driverlib/interrupt.h"  // NVIC Interrupt Controller Driver ԭ��
#include "driverlib/pwm.h"        // ��PWM�йصĺ���ԭ��
#include "driverlib/systick.h"    // SysTick Driver ԭ��
#include "driverlib/uart.h"       // ��UART�йصĺ궨��ͺ���ԭ��
#include "tm1638.h"               // �����TM1638оƬ�йصĺ���
#include "JLX12864G.h"            // LCD��ʾ��

//*****************************************************************************
//
// ��������
//
//*****************************************************************************
uint32_t g_ui32SysClock;

volatile uint8_t clock100ms_flag = 0;
volatile uint8_t clock1s_flag = 0;
volatile uint8_t clock2s_flag = 0;

static volatile uint8_t clock100ms = 0;
static volatile uint8_t clock1s = 0;
static volatile uint8_t clock2s = 0;

// 8λ�������ʾ�����ֻ���ĸ����
// ע����������λ�������������Ϊ4��5��6��7��0��1��2��3
uint8_t digit[8]={' ',' ',' ',' ','_',' ','_',' '};
// 8λС���� 1��  0��
// ע����������λС����������������Ϊ4��5��6��7��0��1��2��3
uint8_t pnt = 0x04;
// 8��LEDָʾ��״̬��0��1��
// ע������ָʾ�ƴ������������Ϊ7��6��5��4��3��2��1��0
//     ��ӦԪ��LED8��LED7��LED6��LED5��LED4��LED3��LED2��LED1
uint8_t led[] = {1, 1, 1, 1, 1, 1, 1, 0};

uint8_t uart_receive[20];
uint8_t uart_receive_flag = 0;

//*****************************************************************************
//
// ����ԭ�ͣ�void GPIOInit(void)
// �������ܣ�GPIO��ʼ����
//          PK4>TM1638STB / PK5>TM1638DIO / PM0>TM1638CLK
// ������������
// ��������ֵ����
//
//*****************************************************************************
void GPIOInit(void) {
    //����TM1638оƬ�ܽ�
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOK);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOK));
    
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOM);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOM));
    
    // PK4 > STB, PK5 > DIO
    GPIOPinTypeGPIOOutput(GPIO_PORTK_BASE, GPIO_PIN_4 | GPIO_PIN_5);
    // PM0 > CLK
    GPIOPinTypeGPIOOutput(GPIO_PORTM_BASE, GPIO_PIN_0);	
        
    //����JLX12864G��Ļ�ܽ�
    LCD_PORT_init();
}

//*****************************************************************************
// 
// ����ԭ�ͣ�void SysTickInit(void)
// �������ܣ�����SysTick�ж�
// ������������
// ��������ֵ����
//
//*****************************************************************************
void SysTickInit(void) {
    SysTickPeriodSet(g_ui32SysClock / SYSTICK_FREQUENCY); // ������������,��ʱ����20ms
    SysTickEnable();  			// SysTickʹ��
    SysTickIntEnable();			// SysTick�ж�����
}

//*****************************************************************************
// 
// ����ԭ�ͣ�void ResetClocks(void)
// �������ܣ����������ʱ��
// ������������
// ��������ֵ����
//
//*****************************************************************************
void ResetClocks(void) {
    clock100ms = clock1s = clock2s = 0;
    clock100ms_flag = clock1s_flag = clock2s_flag = 0;
}

//*****************************************************************************
//
// ����ԭ�ͣ�void UARTInit(void)
// �������ܣ�UART��ʼ����ʹ��UART0������PA0,PA1ΪUART0 RX,TX���ţ�
//          ���ò����ʼ�֡��ʽ��
// ������������
// ��������ֵ����
//
//*****************************************************************************
void UARTInit(void) {
    // ��������
    
    // PA0 > U0RX, PA1 > U0TX
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA));

    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);     

	// ���ö˿�ΪUART����
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);   

    // �����ʼ�֡��ʽ����
    UARTConfigSetExpClk(UART0_BASE, 
	                    g_ui32SysClock,
	                    115200,                  // �����ʣ�115200
	                    (UART_CONFIG_WLEN_8 |    // ����λ��8
	                     UART_CONFIG_STOP_ONE |  // ֹͣλ��1
	                     UART_CONFIG_PAR_NONE)); // У��λ����
	
    UARTFIFOLevelSet(UART0_BASE, UART_FIFO_TX1_8, UART_FIFO_RX7_8);
    IntEnable(INT_UART0); // UART �ж�����
    UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT); // ʹ��UART RX,RT �ж�
    
    // PP0 > U6RX, PP1 > U6TX
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART6);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOP);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOP));

    GPIOPinConfigure(GPIO_PP0_U6RX);
    GPIOPinConfigure(GPIO_PP1_U6TX);     

	// ���ö˿�ΪUART����
    GPIOPinTypeUART(GPIO_PORTP_BASE, GPIO_PIN_0 | GPIO_PIN_1);   

    // �����ʼ�֡��ʽ����
#ifdef CODE_TRANSMITTER
    UARTConfigSetExpClk(UART6_BASE, 
	                    g_ui32SysClock,
	                    38400,                  // �����ʣ�38400
	                    (UART_CONFIG_WLEN_8 |    // ����λ��8
	                     UART_CONFIG_STOP_ONE |  // ֹͣλ��1
	                     UART_CONFIG_PAR_NONE)); // У��λ����
#else
    UARTConfigSetExpClk(UART6_BASE, 
	                    g_ui32SysClock,
	                    9600,                  // �����ʣ�38400
	                    (UART_CONFIG_WLEN_8 |    // ����λ��8
	                     UART_CONFIG_STOP_ONE |  // ֹͣλ��1
	                     UART_CONFIG_PAR_NONE)); // У��λ����
#endif
	
    UARTFIFOLevelSet(UART6_BASE, UART_FIFO_TX1_8, UART_FIFO_RX7_8);
    IntEnable(INT_UART6); // UART �ж�����
    UARTIntEnable(UART6_BASE, UART_INT_RX | UART_INT_RT); // ʹ��UART RX,RT �ж�
}

//*****************************************************************************
//
// ����ԭ�ͣ�void UARTStringPut(uint32_t ui32Base, const char *cMessage)
// �������ܣ���UARTģ�鷢���ַ���
// ����������ui32Base - UARTģ��
//          cMessage - �������ַ���  
// ��������ֵ����
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
// ����ԭ�ͣ�UARTNumberPutNOBlocking(uint32_t ui32Base, int32_t data)
// �������ܣ���UARTģ�鷢������
// ����������ui32Base - UARTģ��
//          data - ����������
// ��������ֵ����
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
// ����ԭ�ͣ�void PWMInit(void)
// �������ܣ���ʼ��PWM0��PG0>PWM4 / PG1>PWM5 / PF3>PWM3
// ������������
// ��������ֵ����
//
//*******************************************************************************************************
void PWMInit(void) {
    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);     // PWM0ʹ��   
    
    PWMOutputState(PWM0_BASE, PWM_OUT_3_BIT | PWM_OUT_4_BIT | PWM_OUT_5_BIT, true); // ʹ��(����)PWM0_4�����
    
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOG);    // ʹ��GPIOG
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);    // ʹ��GPIOF
    GPIOPinConfigure(GPIO_PG0_M0PWM4);              // �������Ÿ���
    GPIOPinConfigure(GPIO_PG1_M0PWM5);
    GPIOPinConfigure(GPIO_PF3_M0PWM3);
    GPIOPinTypePWM(GPIO_PORTG_BASE, GPIO_PIN_0 | GPIO_PIN_1);   // ����ӳ��
    GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_3);
    
    PWMGenConfigure(PWM0_BASE, PWM_GEN_1, PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC);
    PWMGenConfigure(PWM0_BASE, PWM_GEN_2, PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC);   //����PWM������
}

//*******************************************************************************************************
// 
// ����ԭ�ͣ�void PWMStart(uint32_t freq)
// �������ܣ�����Ƶ�ʷ�����ռ�ձ�Ϊ50%��PWM
// ����������freq - ��Ҫ�����ķ�����Ƶ��
// ��������ֵ����
//
//*******************************************************************************************************
void PWMStart(uint32_t freq) {
    PWMGenDisable(PWM0_BASE, PWM_GEN_2);     //ʹ��PWM0ģ���2�ŷ�����(��Ϊ4��PWM��2�ŷ�����������)   
    
    PWMGenPeriodSet(PWM0_BASE, PWM_GEN_2, g_ui32SysClock / freq); // ����Freq_Hz����PWM����
    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_4,(PWMGenPeriodGet(PWM0_BASE, PWM_GEN_2)/ 2)); //����ռ�ձ�Ϊ50%
    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_5,(PWMGenPeriodGet(PWM0_BASE, PWM_GEN_2)/ 2)); //����ռ�ձ�Ϊ50%
    
    PWMGenEnable(PWM0_BASE, PWM_GEN_2);     //ʹ��PWM0ģ���2�ŷ�����(��Ϊ4��PWM��2�ŷ�����������)   
}

//*******************************************************************************************************
// 
// ����ԭ�ͣ�void PWMStop()
// �������ܣ�M0PWM4(PG0)ֹͣ����PWM�ź�
// ������������
// ��������ֵ����
//
//*******************************************************************************************************
void PWMStop() {
    PWMGenDisable(PWM0_BASE, PWM_GEN_2);   // M0PWM4(PG0)ֹͣ����PWM�ź�
}

//*******************************************************************************************************
// 
// ����ԭ�ͣ�void BuzzerStart(uint32_t freq)
// �������ܣ����������PWM�ź�
// ����������freq - PWMƵ��
// ��������ֵ����
//
//*******************************************************************************************************
void BuzzerStart(uint32_t freq) {
    PWMGenPeriodSet(PWM0_BASE, PWM_GEN_1, g_ui32SysClock / freq);
    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_3, PWMGenPeriodGet(PWM0_BASE, PWM_GEN_1) / 2);
    PWMGenEnable(PWM0_BASE, PWM_GEN_1);
}

//*******************************************************************************************************
// 
// ����ԭ�ͣ�void BuzzerStop()
// �������ܣ�ֹͣ���������
// ������������
// ��������ֵ����
//
//*******************************************************************************************************
void BuzzerStop() {
    PWMGenDisable(PWM0_BASE, PWM_GEN_1);
}

//*****************************************************************************
// 
// ����ԭ�ͣ�void ConvertTemperatureToString(unsigned char *buffer, int16_t temp)
// �������ܣ����¶�ת��Ϊ�ַ���
// ����������buffer - �ַ���������
//          temp - �¶�ֵ
// ��������ֵ����
//
//*****************************************************************************
void ConvertTemperatureToString(unsigned char *buffer, int16_t temp) {
    if (temp < 0) {
        buffer[0] = '-';
        temp = -temp;
    } else {
        buffer[0] = ' ';
    }
    
    buffer[1] = temp / 100 % 10 + '0';  // ����ʮλ��
    buffer[2] = temp / 10 % 10 + '0'; 	// �����λ��
    buffer[3] = '.';
    buffer[4] = temp % 10 + '0';        // ����ʮ��λ��
    buffer[5] = CHAR_CELSIUS / 256;
    buffer[6] = CHAR_CELSIUS % 256;
    buffer[7] = '\0';
}

//*****************************************************************************
// 
// ����ԭ�ͣ�void ConvertTemperatureToString(unsigned char *buffer, int16_t temp)
// �������ܣ����¶�ת��Ϊ�ַ���
// ����������buffer - �ַ���������
//          voltage - ��ѹֵ
// ��������ֵ����
//
//*****************************************************************************
void ConvertVoltageToString(unsigned char *buffer, uint32_t voltage) {
    buffer[0] = (voltage / 100) % 10 + '0'; // ��ʾ��ѹֵ��λ��
    buffer[1] = '.';
    buffer[2] = (voltage / 10) % 10 + '0';  // ��ʾ��ѹֵʮ��λ��
    buffer[3] = voltage % 10 + '0';         // ��ʾ��ѹֵ�ٷ�λ��
    buffer[4] = 'V';
    buffer[5] = '\0';
}

//*****************************************************************************
// 
// ����ԭ�ͣ�void SysTick_Handler(void)
// �������ܣ�SysTick�жϷ������
// ������������
// ��������ֵ����
//
//*****************************************************************************
void SysTick_Handler(void) {
    // �����ʱ������
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

    // ˢ��ȫ������ܺ�LEDָʾ��
    TM1638_RefreshDIGIandLED(digit, pnt, led);

    // ��鵱ǰ��������
    DetectKey(TM1638_Readkeyboard());
}

//*****************************************************************************
// 
// ����ԭ�ͣ�void UART6_Handler(void)
// �������ܣ�UART6�жϷ������
// ������������
// ��������ֵ����
//
//*****************************************************************************
void UART6_Handler(void) {
    int32_t uart_int_status;
    uint8_t uart_receive_char;
    static uint8_t uart_receive_len = 0;
    volatile uint32_t i;
    
    uart_int_status = UARTIntStatus(UART6_BASE, true); // ȡ�ж�״̬
    UARTIntClear(UART6_BASE, uart_int_status); // ���жϱ�־
  
    while(UARTCharsAvail(UART6_BASE)) { // �ظ��ӽ���FIFO ��ȡ�ַ�
        uart_receive_char = UARTCharGetNonBlocking(UART6_BASE); // ����һ���ַ�
        
        if(uart_receive_char != '\n') { // ����δ����
            uart_receive[uart_receive_len++] = uart_receive_char;
        } else {
            uart_receive[uart_receive_len] = '\0';
            uart_receive_flag = 1;
            uart_receive_len = 0;
        }
    }
}
