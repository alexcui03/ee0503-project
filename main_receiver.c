//*****************************************************************************
//
// Copyright: 2024, �Ϻ���ͨ��ѧ����ʵ����Ƽ�����II-47��
// File name: main_receiver.c
// Description: �ۺ�ʵ��������ն�
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
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"        // ��ַ�궨��
#include "inc/hw_types.h"         // �������ͺ궨�壬�Ĵ������ʺ���
#include "inc/hw_timer.h"         // �붨ʱ���йصĺ궨��
#include "inc/hw_ints.h"          // ���ж��йصĺ궨��
#include "driverlib/debug.h"      // ������
#include "driverlib/gpio.h"       // ͨ��IO�ں궨��
#include "driverlib/pin_map.h"    // TM4Cϵ��MCU��Χ�豸�ܽź궨��
#include "driverlib/pwm.h"        // ��Timer�йصĺ���ԭ��
#include "driverlib/sysctl.h"     // ϵͳ���ƶ���
#include "driverlib/systick.h"    // SysTick Driver ԭ��
#include "driverlib/interrupt.h"  // NVIC Interrupt Controller Driver ԭ��
#include "driverlib/uart.h"       // ��UART�йصĺ궨��ͺ���ԭ��
#include "driverlib/timer.h"      // ��Timer�йصĺ���ԭ��  
#include "JLX12864G.h"            // LCD��ʾ��
#include "LM75BD.h"               // �¶ȴ�����
#include "ADC.h"                  // ADC
#include "tm1638.h"               // TM1638
#include "ui.h"
#include "common.h"
#include "music.h"

//*****************************************************************************
//
// �궨��
//
//*****************************************************************************
#define SAMPLING_FREQ       48000

//*****************************************************************************
//
// ����ԭ������
//
//*****************************************************************************
void DevicesInit(void);
void GPIOL_Handler(void);       // PL4���ش�����δʹ��
void TemperatureModeInit(void); // ��ʼ��GPIOΪ�¶���ʾ����
void MusicModeInit(void);       // ��ʼ��GPIOΪ���ֲ��Ź���
void FMSetFreq(uint32_t freq);
void ShowFrequency(uint32_t freq);
void ShowTemperature(int16_t temp);
void ShowMusicPlayer(void);
void SetSelect(int ele, int new_select);

//*****************************************************************************
//
// ��������
//
//*****************************************************************************
// 1s����������־
volatile uint8_t g_ui8INTStatus = 0;

// ������һ��TIMER0���ؼ���ֵ
volatile uint32_t g_ui32TPreCount = 0;

// ���汾��TIMER0���ؼ���ֵ
volatile uint32_t g_ui32TCurCount = 0;

// �ز�Ƶ�ʸ�λ��ȷ����Ӧ�����ID
const uint8_t freq_digit_map[5] = {
    UI_ELE_DIGIT1, UI_ELE_DIGIT2, UI_ELE_DIGIT3, UI_ELE_DIGIT4, UI_ELE_CONFIRM
};

// ������ҳ״̬
uint8_t player_play = 0, player_record = 0, player_mode = 0;
uint16_t player_current_sec = 0, player_record_sec = 0;

// ¼����ر���
int32_t counter = 0; // �����ؼ������
int32_t note_counter = 0; // �����������

// ������ر���
uint32_t current_note = 0;

uint8_t freq_digit[4] = { 0, 8, 9, 5 }; // �ز�Ƶ�ʸ�λ����
uint32_t freq = 0;                      // �ز�Ƶ����ֵ
int16_t temp = 0;                       // �¶�
uint8_t select = 0, old_select = 0xff;  // �ز�Ƶ��ҳѡ���λ��ȷ�ϰ�ť

//*****************************************************************************
//
// ������
//
//*****************************************************************************
int main(void) {
    DevicesInit();  //  MCU������ʼ��
    while (clock100ms_flag == 0); // �ȴ�TM1638�ϵ�
    TM1638_Init();
    LCD_Init();
    
    InitAct();
    
    // ��ʾ��ҳ
    DisplayUiAct(UI_ACT_MENU);
    
    
    while (1) {
        if(g_ui8INTStatus == 1) {  // 1s��ʱ��������ʼ����Ƶ��
            uint32_t measure_freq;
            
            g_ui8INTStatus = 0;
            
            measure_freq = g_ui32TCurCount >= g_ui32TPreCount ?
                      (g_ui32TCurCount - g_ui32TPreCount):(g_ui32TCurCount - g_ui32TPreCount + 0xFFFF);
            
            // ����
            temp = (measure_freq - 300.0) * (40.0 - 0.0) / (4000 - 300) * 10;
            if (current_act == UI_ACT_VOLTAGE_TEMP) {
                ShowTemperature(temp);
            }
        }
        
        if (clock100ms_flag == 1) {  // ���0.1�붨ʱ�Ƿ�
            clock100ms_flag	= 0;
        }

        if (clock1s_flag == 1) {  // ���1�붨ʱ�Ƿ�
            clock1s_flag = 0;
            if (current_act == UI_ACT_MUSIC) {
                if (player_record) {
                    // ����¼��ʱ��
                    ++player_record_sec;
                    player_current_sec = player_record_sec;
                    ShowMusicPlayer();
                } else if (player_mode == 1 && player_play) {
                    // ���²���ʱ��
                    if (player_current_sec < player_record_sec) {
                        ++player_current_sec;
                    }
                    ShowMusicPlayer();
                }
            }
        }

        if (clock2s_flag == 1) {  // ���2�붨ʱ�Ƿ�
            clock2s_flag = 0;
            if (current_act == UI_ACT_ERROR) {
                ShowFrequency(freq);
                ClearKeyFlags();
            }
        }
        
        switch (current_act) {
            case UI_ACT_MENU: {
                if (key_state[KEY_INC].flag) { // ��һ�����
                    key_state[KEY_INC].flag = 0;
                    select = (select - 1 + 3) % 3;
                }
                if (key_state[KEY_DEC].flag) { // ��һ�����
                    key_state[KEY_DEC].flag = 0;
                    select = (select + 1) % 3;
                }
                if (key_state[KEY_ENTER].flag) {
                    key_state[KEY_ENTER].flag = 0;
                    SetSelect(select, 0);
                    switch (select) {
                        case UI_ELE_MTEMP:
                            TemperatureModeInit(); // ��ʼ��������źͼ�ʱ��
                            current_act = UI_ACT_VOLTAGE_TEMP;
                            SetSelect(select = UI_ELE_VFREQ, 1);
                            ShowTemperature(temp);
                            break;
                        case UI_ELE_MMUSI:
                            MusicModeInit(); // ��ʼ��������źͼ�ʱ��
                            player_play = player_record = 0;
                            current_act = UI_ACT_MUSIC;
                            SetSelect(select = UI_ELE_MMODE, 1);
                            ShowMusicPlayer();
                            break;
                        case UI_ELE_MFREQ:
                            current_act = UI_ACT_FREQUENCY;
                            last_act = UI_ACT_MENU; // ����FREQ����last_act���ڷ���
                            SetSelect(freq_digit_map[select = 0], 1);
                            ShowFrequency(freq);
                            break;
                    }
                    ResetClocks();
                    ClearKeyFlags();
                    continue;
                }
                
                if (old_select != select) {
                    SetSelect(old_select, 0);
                    SetSelect(select, 1);
                    old_select = select;
                    DisplayUiAct(UI_ACT_MENU);
                }
                
                break;
            }
            case UI_ACT_MUSIC: {
                if (key_state[KEY_UP].flag) {
                    key_state[KEY_UP].flag = 0;
                    do {
                        select = (select - 1 + 4) % 4;
                    } while (act[UI_ACT_MUSIC].hide[select]);
                }
                if (key_state[KEY_DOWN].flag) {
                    key_state[KEY_DOWN].flag = 0;
                    do {
                        select = (select + 1) % 4;
                    } while (act[UI_ACT_MUSIC].hide[select]);
                }
                
                if (key_state[KEY_ENTER].flag) {
                    key_state[KEY_ENTER].flag = 0;
                    switch (select) {
                        case UI_ELE_MMODE:
                            player_mode = !player_mode;
                            if (player_mode == 1) {
                                // ����ģʽ����ʹ��PWM
                                PWMInit();
                            } else {
                                // �㲥ģʽ������PF3���
                                MusicModeInit();
                            }
                            ShowMusicPlayer();
                            break;
                        case UI_ELE_PLAYPAUSE:
                            player_play = !player_play;
                            if (player_mode == 1) {
                                if (player_play) {
                                    // ���ű���¼��
                                    TimerDisable(TIMER1_BASE, TIMER_A);
                                    TimerLoadSet(TIMER1_BASE, TIMER_A, 0);
                                    TimerEnable(TIMER1_BASE, TIMER_A);
                                    current_note = 0;
                                    player_current_sec = 0;
                                } else {
                                    BuzzerStop();
                                }
                            }
                            ShowMusicPlayer();
                            break;
                        case UI_ELE_RECORD:
                            if (player_record) {
                                // ¼�ƽ���
                                player_current_sec = 0;
                            } else {
                                // ��ʼ¼��
                                player_current_sec = player_record_sec = 0;
                                music[0].len = 0;
                            }
                            player_record = !player_record;
                            ShowMusicPlayer();
                            break;
                        case UI_ELE_RETURN:
                            SetSelect(select, 0);
                            select = UI_ELE_MMUSI;
                            current_act = UI_ACT_MENU;
                            SetSelect(select, 1);
                            DisplayUiAct(UI_ACT_MENU);
                            ClearKeyFlags();
                            break;
                    }
                    continue;
                }
                
                if (old_select != select) {
                    SetSelect(old_select, 0);
                    SetSelect(select, 1);
                    old_select = select;
                    DisplayUiAct(UI_ACT_MUSIC);
                }
                
                break;
            }
            case UI_ACT_VOLTAGE_TEMP: {
                if (key_state[KEY_UP].flag || key_state[KEY_DOWN].flag) {
                    key_state[KEY_UP].flag = key_state[KEY_DOWN].flag = 0;
                    if (select == UI_ELE_VFREQ) {
                        select = UI_ELE_VRET;
                    } else {
                        select = UI_ELE_VFREQ;
                    }
                }
                
                if (key_state[KEY_ENTER].flag) {
                    key_state[KEY_ENTER].flag = 0;
                    SetSelect(select, 0);
                    if (select == UI_ELE_VFREQ) {
                        // �л����沢��¼ԭ�������ڷ���
                        current_act = UI_ACT_FREQUENCY;
                        last_act = UI_ACT_VOLTAGE_TEMP;
                        select = 0;
                        SetSelect(freq_digit_map[select], 1);
                        ShowFrequency(freq);
                        ClearKeyFlags();
                    } else {
                        // ���ص��˵�
                        current_act = UI_ACT_MENU;
                        select = UI_ELE_MTEMP;
                        SetSelect(select, 1);
                        DisplayUiAct(UI_ACT_MENU);
                        ClearKeyFlags();
                    }
                    continue;
                }
                
                if (old_select != select) {
                    SetSelect(old_select, 0);
                    SetSelect(select, 1);
                    old_select = select;
                    ShowTemperature(temp);
                }
                
                break;
            }
            case UI_ACT_FREQUENCY: {
                uint32_t old_freq = freq;
                
                if (key_state[KEY_UP].flag) {
                    key_state[KEY_UP].flag = 0;
                    select = (select - 1 + 5) % 5;
                }
                if (key_state[KEY_DOWN].flag) {
                    key_state[KEY_DOWN].flag = 0;
                    select = (select + 1) % 5;
                }
                if (key_state[KEY_INC].flag) {
                    key_state[KEY_INC].flag = 0;
                    if (select < 4) {
                        freq_digit[select] = (freq_digit[select] + 1) % 10;
                    }
                }
                if (key_state[KEY_DEC].flag) {
                    key_state[KEY_DEC].flag = 0;
                    if (select < 4) {
                        freq_digit[select] = (freq_digit[select] - 1 + 10) % 10;
                    }
                }
                
                freq = freq_digit[0] * 1000 + freq_digit[1] * 100 + freq_digit[2] * 10 + freq_digit[3];

                if (key_state[KEY_ENTER].flag) {
                    key_state[KEY_ENTER].flag = 0;
                    if (select == 4) {
                        if (freq >= 880 && freq <= 1080) {
                            SetSelect(freq_digit_map[select], 0);
                            FMSetFreq(freq);
                            
                            // �л�����
                            current_act = last_act;
                            if (last_act == UI_ACT_MUSIC) {
                                SetSelect(select = UI_ELE_MMODE, 1);
                                ShowMusicPlayer();
                            } else if (last_act == UI_ACT_VOLTAGE_TEMP) {
                                SetSelect(select = UI_ELE_VFREQ, 1);
                                ShowTemperature(temp);
                            } else {
                                SetSelect(select = UI_ELE_MFREQ, 1);
                                DisplayUiAct(UI_ACT_MENU);
                            }
                            ClearKeyFlags();
                        } else {
                            DisplayUiAct(UI_ACT_ERROR);
                            ResetClocks(); // ����ҳ����Ҫ���������ʱ
                        }
                        continue;
                    }
                }
                
                if (old_select != select) {
                    if (old_select != 0xff) SetSelect(freq_digit_map[old_select], 0);
                    SetSelect(freq_digit_map[select], 1);
                    old_select = select;
                    ShowFrequency(freq);
                } else if (old_freq != freq) {
                    old_freq = freq;
                    ShowFrequency(freq);
                }
                break;
            }
            default: { // UI_ACT_ERROR;
                break;
            }
        }
    }
    
}

//*****************************************************************************
// 
// ����ԭ�ͣ�DevicesInit(void)
// �������ܣ�MCU��ʼ��
// ������������
// ��������ֵ����
//
//*****************************************************************************
void DevicesInit(void) {
    // ʹ���ⲿ25MHz��ʱ��Դ������PLL��Ȼ���ƵΪ20MHz
    g_ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |SYSCTL_OSC_MAIN | 
                                       SYSCTL_USE_PLL |SYSCTL_CFG_VCO_480), 
                                       20000000);
    
    // ʹ�ܸ������㵥Ԫ
    FPULazyStackingEnable(); 
    FPUEnable(); 
    
    GPIOInit();             // GPIO��ʼ��
    UARTInit();             // UART��ʼ�� 
    I2C0Init();             // I2C0��ʼ��
    PWMInit();              // PWM��ʼ��
    ADCInit();              // ADC��ʼ��
    SysTickInit();          // ����SysTick�ж�
    IntMasterEnable();		// ���ж�����
    
    IntPrioritySet(INT_TIMER1A, 0x00);		// ����INT_TIMER1A������ȼ�   
    IntPrioritySet(INT_TIMER0A, 0x01);		// ����INT_TIMER0A������ȼ�
    IntPrioritySet(FAULT_SYSTICK, 0xe0);	// ����SYSTICK���ȼ�����INT_TIMER0A�����ȼ�
}

//*******************************************************************************************************
//
// ����ԭ�ͣ�void TemperatureModeInit(void)
// �������ܣ�PL4��Ϊ�������뵽Timer0����ʼ��Timer0��Timer1
// ������������
// ��������ֵ����
//
//*******************************************************************************************************
void TemperatureModeInit(void) {
    // �ر�ԭ��TIMER
    if (SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER0)) {
        TimerDisable(TIMER0_BASE, TIMER_A);
    }
    if (SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER1)) {
        TimerDisable(TIMER1_BASE, TIMER_A);
    }
    
    // ʹ��PL4
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOL);   // ʹ��GPIOL
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOL));
    GPIOPinConfigure(GPIO_PL4_T0CCP0);             // �������Ÿ���   
    GPIOPinTypeTimer(GPIO_PORTL_BASE, GPIO_PIN_4); // ����ӳ��
    GPIOPadConfigSet(GPIO_PORTL_BASE, GPIO_PIN_4, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU); // ������������
    
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);  // ʹ��TIMER0
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER0));
    TimerConfigure(TIMER0_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_CAP_COUNT_UP); // �볤��ʱ����������
    TimerControlEvent(TIMER0_BASE, TIMER_A, TIMER_EVENT_POS_EDGE); // ��ʼ������Ϊ��׽������
    g_ui32TPreCount = g_ui32TCurCount = TimerValueGet(TIMER0_BASE, TIMER_A); // �������
    
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);  // TIMER1 ʹ��
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER1));
    TimerConfigure(TIMER1_BASE, TIMER_CFG_PERIODIC); // ����Ϊ 32 λ���ڶ�ʱ��
    TimerLoadSet(TIMER1_BASE, TIMER_A, g_ui32SysClock);  // TIMER1Aװ�ؼ���ֵ1s
    
    TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);    // Timer1A ��ʱ�ж�ʹ��
    IntEnable(INT_TIMER1A);                             // ���� TIMER1A �ж�Դ
    TimerEnable(TIMER1_BASE, TIMER_A);                  // TIMER1 ��ʼ��ʱ
    TimerEnable(TIMER0_BASE, TIMER_A);                  // TIMER0 ��ʼ����
}

//*******************************************************************************************************
//
// ����ԭ�ͣ�void MusicModeInit(void)
// �������ܣ�PL4��Ϊ���룬ʹ��Timer1��Ϊ��ʱ����������
// ������������
// ��������ֵ����
//
//*******************************************************************************************************
void MusicModeInit(void) {
    // �ر�ԭ��TIMER
    if (SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER0)) {
        TimerDisable(TIMER0_BASE, TIMER_A);
    }
    if (SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER1)) {
        TimerDisable(TIMER1_BASE, TIMER_A);
    }
    
    // ʹ��PL4����ΪGPIO����
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOL);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOL));
    GPIOPinTypeGPIOInput(GPIO_PORTL_BASE, GPIO_PIN_4);
    GPIOPadConfigSet(GPIO_PORTL_BASE, GPIO_PIN_4, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU); // ��Ϊ������
    
    // ʹ��PF3����ΪGPIO���
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF));
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_3);
    
    // ����PL4�����ж�
    /*GPIOIntDisable(GPIO_PORTL_BASE, GPIO_PIN_4);
	GPIOIntClear(GPIO_PORTL_BASE, GPIO_PIN_4);
	GPIOIntRegister(GPIO_PORTL_BASE, GPIOL_Handler);
	GPIOIntTypeSet(GPIO_PORTL_BASE, GPIO_PIN_4, GPIO_BOTH_EDGES);
	GPIOIntEnable(GPIO_PORTL_BASE, GPIO_PIN_4);*/
    
    // ��ʼ��Timer1��Ϊ����ʱ��
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER1));
    TimerConfigure(TIMER1_BASE, TIMER_CFG_PERIODIC); // 32λ���ڼ�ʱ��
    TimerLoadSet(TIMER1_BASE, TIMER_A, g_ui32SysClock / SAMPLING_FREQ);
    
    TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT); // �����ж�
    IntEnable(INT_TIMER1A); // ���� TIMER1A �ж�Դ
    TimerEnable(TIMER1_BASE, TIMER_A); // ����TIMER1A
}

//*******************************************************************************************************
// 
// ����ԭ�ͣ�void TIMER1A_Handler(void)
// �������ܣ�Timer1A�жϷ�����򣬼�¼���񷽲�������ʱ��ʱ����TIMER0���ļ���ֵ
// ������������
// ��������ֵ����
//
//*******************************************************************************************************
void TIMER1A_Handler(void) {
    TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);     // ����жϱ�־
    if (current_act == UI_ACT_VOLTAGE_TEMP) {
        // �¶���ʾ
        g_ui32TPreCount = g_ui32TCurCount;                  // ������һ��TIMER0���ؼ���ֵ
        g_ui32TCurCount = TimerValueGet(TIMER0_BASE, TIMER_A);  // ��ȡTIMER0���ؼ���ֵ

        g_ui8INTStatus = 1; // 1s�������
    } else if (player_mode == 0) { // FM����ҳ
        static uint8_t last = 0;
        static uint8_t last_note = 0; // ��һ��note
        uint8_t value = GPIOPinRead(GPIO_PORTL_BASE, GPIO_PIN_4) & GPIO_PIN_4;
        
        if (player_record) {
            ++counter;
            ++note_counter;
        }
        
        if (value != last) { // ��������
            last = value;
            if (player_play) { // ���ڲ���
                GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, value ? GPIO_PIN_3 : 0);
            }
            
            if (value && player_record) { // ������������¼��
                uint8_t note = DetectNote(SAMPLING_FREQ / counter);
                if (note != last_note) { // ���仯
                    int32_t len = (double)note_counter / SAMPLING_FREQ * 1000.0 / NOTE_LEN_BASE + 0.5;
                    if (len > 0) { // �˳�����
                        music[0].note[music[0].len].note = last_note;
                        music[0].note[music[0].len].len = len;
                        ++music[0].len;
                        note_counter -= len * NOTE_LEN_BASE * SAMPLING_FREQ / 1000; // �������
                        last_note = note;
                    } else {
                        if (note_counter > 0) {
                            note_counter = 0; // ��ֹ��������
                        }
                        last_note = note;
                    }
                }
                counter = 0;
            }
        }
    } else { // ���ز���ҳ
        if (player_play) {
            BuzzerStop();
            
            if (current_note < music[0].len) {
                TimerLoadSet(TIMER1_BASE, TIMER_A, g_ui32SysClock / 1000.0 * music[0].note[current_note].len * NOTE_LEN_BASE);
                if (note_frequency[music[0].note[current_note].note] > 0) {
                    BuzzerStart(note_frequency[music[0].note[current_note].note]);
                }
                ++current_note;
            } else {
                // ���Ž���
                player_play = 0;
                ShowMusicPlayer();
            }
        }
    }
}

//*******************************************************************************************************
// 
// ����ԭ�ͣ�void GPIOL_Handler(void)
// �������ܣ�GPIOL�жϷ�����򣬴���PL4�����źţ�δʹ��
// ������������
// ��������ֵ����
//
//*******************************************************************************************************
void GPIOL_Handler(void) {
    GPIOIntClear(GPIO_PORTL_BASE, GPIOIntStatus(GPIO_PORTL_BASE, true));
    
    if (player_play) {
        // ���ڲ���ʱ�����������
        if (GPIOPinRead(GPIO_PORTL_BASE, GPIO_PIN_4) & GPIO_PIN_4) { // ������
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_PIN_3);
        } else { // �½���
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0);
        }
    }
}

//*****************************************************************************
// 
// ����ԭ�ͣ�void FMSetFreq(uint32_t freq)
// �������ܣ�����FM�ز�Ƶ�ʣ�ͨ��UART6����
// ������������
// ��������ֵ����
//
//*****************************************************************************
void FMSetFreq(uint32_t freq) {
    uint8_t cmd[13] = {'A', 'T', '+', 'F', 'R', 'E', 'Q', '='};
    cmd[8] = freq / 1000 % 10 + '0';
    cmd[9] = freq / 100 % 10 + '0';
    cmd[10] = freq / 10 % 10 + '0';
    cmd[11] = freq % 10 + '0';
    cmd[12] = '\0';
    
    UARTStringPutNOBlocking(UART6_BASE, cmd);
}

//*****************************************************************************
// 
// ����ԭ�ͣ�void ShowTemperature(uint16_t temp)
// �������ܣ���ʾ�¶�
// ����������temp - �¶�
// ��������ֵ����
//
//*****************************************************************************
void ShowTemperature(int16_t temp) {
    unsigned char str_temp[10];
    ConvertTemperatureToString(str_temp, temp);
    act[UI_ACT_VOLTAGE_TEMP].str[UI_ELE_TEMP] = str_temp;
    DisplayUiAct(UI_ACT_VOLTAGE_TEMP);
}

//*****************************************************************************
// 
// ����ԭ�ͣ�void ShowFrequency(uint32_t freq)
// �������ܣ���ʾ�ز�Ƶ��
// ����������freq - �ز�Ƶ��
// ��������ֵ����
//
//*****************************************************************************
void ShowFrequency(uint32_t freq) {
    unsigned char buffer[8];
    
    buffer[1] = buffer[3] = buffer[5] = buffer[7] = '\0';
    buffer[0] = freq / 1000 % 10 + '0';
    buffer[2] = freq / 100 % 10 + '0';
    buffer[4] = freq / 10 % 10 + '0';
    buffer[6] = freq % 10 + '0';
    
    act[UI_ACT_FREQUENCY].str[UI_ELE_DIGIT1] = buffer;
    act[UI_ACT_FREQUENCY].str[UI_ELE_DIGIT2] = buffer + 2;
    act[UI_ACT_FREQUENCY].str[UI_ELE_DIGIT3] = buffer + 4;
    act[UI_ACT_FREQUENCY].str[UI_ELE_DIGIT4] = buffer + 6;

    DisplayUiAct(UI_ACT_FREQUENCY);
}

//*****************************************************************************
// 
// ����ԭ�ͣ�void ShowMusicPlayer(void)
// �������ܣ���ʾ������ҳ��
// ������������
// ��������ֵ����
//
//*****************************************************************************
void ShowMusicPlayer(void) {
    static uint8_t buffer[12];
    
    // ��ʾ����ʱ��/��¼��ʱ����00:00/99:99
    buffer[0] = player_current_sec / 60 / 10 % 10 + '0';
    buffer[1] = player_current_sec / 60 % 10 + '0';
    buffer[2] = ':';
    buffer[3] = player_current_sec % 60 / 10 + '0';
    buffer[4] = player_current_sec % 60 % 10 + '0';
    buffer[5] = '/';
    buffer[6] = player_record_sec / 60 / 10 % 10 + '0';
    buffer[7] = player_record_sec / 60 % 10 + '0';
    buffer[8] = ':';
    buffer[9] = player_record_sec % 60 / 10 + '0';
    buffer[10] = player_record_sec % 60 % 10 + '0';
    buffer[11] = '\0';
    
    act[UI_ACT_MUSIC].str[UI_ELE_RTIME] = buffer;
    
    if (player_mode == 0) {
        act[UI_ACT_MUSIC].str[UI_ELE_MMODE] = (unsigned char *)"�㲥";
        act[UI_ACT_MUSIC].hide[UI_ELE_RECORD] = 0;
        
        if (player_play == 1) {
            act[UI_ACT_MUSIC].str[UI_ELE_PLAYPAUSE] = (unsigned char *)"����";
        } else {
            act[UI_ACT_MUSIC].str[UI_ELE_PLAYPAUSE] = (unsigned char *)"����";
        }
    } else {
        act[UI_ACT_MUSIC].str[UI_ELE_MMODE] = (unsigned char *)"����";
        act[UI_ACT_MUSIC].hide[UI_ELE_RECORD] = 1;
    
        if (player_play == 1) {
            act[UI_ACT_MUSIC].str[UI_ELE_PLAYPAUSE] = (unsigned char *)"��ͣ";
        } else {
            act[UI_ACT_MUSIC].str[UI_ELE_PLAYPAUSE] = (unsigned char *)"����";
        }
    }
    
    if (player_record == 1) {
        act[UI_ACT_MUSIC].str[UI_ELE_RECORD] = (unsigned char *)"����";
    } else {
        act[UI_ACT_MUSIC].str[UI_ELE_RECORD] = (unsigned char *)"¼��";
    }
    
    DisplayUiAct(UI_ACT_MUSIC);
}

//*****************************************************************************
// 
// ����ԭ�ͣ�void SetSelect(int ele, int new_select)
// �������ܣ������������
// ����������ele - ���ID
//          new_select - �Ƿ����
// ��������ֵ����
//
//*****************************************************************************
void SetSelect(int ele, int new_select) {
    act[current_act].inverse[ele] = new_select;
}
