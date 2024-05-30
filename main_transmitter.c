//*****************************************************************************
//
// Copyright: 2024, �Ϻ���ͨ��ѧ����ʵ����Ƽ�����II-47��
// File name: main_transmitter.c
// Description: �ۺ�ʵ���������
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
#include "ui.h"                   // UI��غ���
#include "common.h"               // �������ͷ�������ͨ�ú���
#include "music.h"                // ������ض���

//*****************************************************************************
//
// �궨��
//
//*****************************************************************************


//*****************************************************************************
//
// ����ԭ������
//
//*****************************************************************************
void DevicesInit(void);
void Timer1Init(void);
uint32_t VoltageStabilizer(uint32_t voltage);
void FMSetFreq(uint32_t freq);
void ShowVoltageAndTemperature(uint32_t voltage, int16_t temp1, int16_t temp2);
void ShowFrequency(uint32_t freq);
void ShowMusicPlayer(void);
void SetSelect(int ele, int new_select);

//*****************************************************************************
//
// ��������
//
//*****************************************************************************
// �ز�Ƶ�ʸ�λ��ȷ����Ӧ�����ID
const uint8_t freq_digit_map[5] = {
    UI_ELE_DIGIT1, UI_ELE_DIGIT2, UI_ELE_DIGIT3, UI_ELE_DIGIT4, UI_ELE_CONFIRM
};

// ������״̬
uint8_t player_play = 0, player_mute = 0;

uint8_t current_note = 0; // ��ǰ����

//*****************************************************************************
//
// ������
//
//*****************************************************************************
int main(void) {
    uint8_t freq_digit[4] = { 0, 8, 9, 5 }; // �ز�Ƶ�ʸ�λ����
    uint32_t freq = 1017;                   // �ز�Ƶ����ֵ
    int16_t temp1 = 0, temp2 = 0;           // �¶���ֵ
    uint32_t voltage = 0;                   // ��ѹֵ
    uint8_t select = 0, old_select = 0xff;  // ѡ������

    DevicesInit(); // MCU��ʼ��
    while (clock100ms_flag == 0); // �ȴ�1638�ϵ�
    TM1638_Init();
    LCD_Init();
    
    InitAct();
    
    InitMusic();

    // �״βɼ�����
    temp1 = GetTemputerature(LM75BD_ADR1);
    temp2 = GetTemputerature(LM75BD_ADR2);
    voltage = VoltageStabilizer(ADC_Sample() * 330 / 4095);
    
    // ��ʾ��ҳ
    DisplayUiAct(UI_ACT_MENU);

    while (1) {
        // 100ms��ʱ
        if (clock100ms_flag == 1) {
            clock100ms_flag	= 0;
        }

        // 1s��ʱ
        if (clock1s_flag == 1) {
            clock1s_flag = 0;
            if (current_act == UI_ACT_VOLTAGE_TEMP) {
                // ���ݲɼ�
                temp1 = GetTemputerature(LM75BD_ADR1);
                temp2 = GetTemputerature(LM75BD_ADR2);
                voltage = VoltageStabilizer(ADC_Sample() * 330 / 4095);
                
                ShowVoltageAndTemperature(voltage, temp1, temp2);
                
                // ���벢�����PWM
                PWMStart(temp1 / 10.0 / (40.0  - 0.0) * (4000 - 300) + 300);
            }
        }

        // 2s��ʱ
        if (clock2s_flag == 1) {
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
                            current_act = UI_ACT_VOLTAGE_TEMP;
                            SetSelect(select = UI_ELE_VFREQ, 1);
                            ShowVoltageAndTemperature(voltage, temp1, temp2);
                            break;
                        case UI_ELE_MMUSI:
                            current_act = UI_ACT_MUSIC;
                            player_play = 0;
                            player_mute = 1;
                            SetSelect(select = UI_ELE_MUTE, 1);
                            ShowMusicPlayer();
                            // ���벥�Ź���
                            current_note = 0;
                            PWMStop();
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
                    select = (select - 1 + 3) % 3;
                }
                if (key_state[KEY_DOWN].flag) {
                    key_state[KEY_DOWN].flag = 0;
                    select = (select + 1) % 3;
                }
                if (key_state[KEY_ENTER].flag) {
                    key_state[KEY_ENTER].flag = 0;
                    
                    if (select == UI_ELE_PLAYPAUSE) {
                        player_play = !player_play;
                        if (player_play) {
                            // ��ʼ����
                            TimerEnable(TIMER1_BASE, TIMER_A);
                        } else {
                            // ֹͣ����
                            TimerDisable(TIMER1_BASE, TIMER_A);
                            BuzzerStop();
                            PWMStop();
                        }
                        ShowMusicPlayer();
                    } else if (select == UI_ELE_MUTE) {
                        player_mute = !player_mute;
                        if (player_mute) {
                            BuzzerStop();
                        } else {
                            if (player_play) {
                                // ȡ��������Ҫ���ŵ�ǰ�����������ڲ���
                                uint32_t freq;
                                if (current_note == 0) {
                                    freq = note_frequency[music[0].note[current_note].note];
                                } else {
                                    freq = note_frequency[music[0].note[music[0].len - 1].note];
                                }
                                BuzzerStart(freq);
                            }
                        }
                        ShowMusicPlayer();
                    } else {
                        // ���ز˵�
                        if (player_play) {
                            // ֹͣ����
                            TimerDisable(TIMER1_BASE, TIMER_A);
                            BuzzerStop();
                            PWMStop();
                        }
                        SetSelect(select, 0);
                        current_act = UI_ACT_MENU;
                        SetSelect(select = UI_ELE_MMUSI, 1);
                        DisplayUiAct(UI_ACT_MENU);
                        ClearKeyFlags();
                    }
                    continue;
                }
                
                if (old_select != select) {
                    SetSelect(old_select, 0);
                    SetSelect(select, 1);
                    old_select = select;
                    ShowMusicPlayer();
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
                        SetSelect(freq_digit_map[select = 0], 1);
                        ShowFrequency(freq);
                        ClearKeyFlags();
                    } else {
                        // ���ص��˵�
                        current_act = UI_ACT_MENU;
                        SetSelect(select = UI_ELE_MTEMP, 1);
                        DisplayUiAct(UI_ACT_MENU);
                        ClearKeyFlags();
                        PWMStop(); // ֹͣ����
                    }
                    continue;
                }
                
                if (old_select != select) {
                    SetSelect(old_select, 0);
                    SetSelect(select, 1);
                    old_select = select;
                    ShowVoltageAndTemperature(voltage, temp1, temp2);
                }
                
                break;
            }
            case UI_ACT_FREQUENCY: {
                uint32_t old_freq = freq; // ԭ����freq����������ˢ��ҳ��
                
                if (key_state[KEY_UP].flag) { // ��һ�����
                    key_state[KEY_UP].flag = 0;
                    select = (select - 1 + 5) % 5;
                }
                if (key_state[KEY_DOWN].flag) { // ��һ�����
                    key_state[KEY_DOWN].flag = 0;
                    select = (select + 1) % 5;
                }
                if (key_state[KEY_INC].flag) { // ����ֵ
                    key_state[KEY_INC].flag = 0;
                    if (select < 4) {
                        freq_digit[select] = (freq_digit[select] + 1) % 10;
                    }
                }
                if (key_state[KEY_DEC].flag) { // ��Сֵ
                    key_state[KEY_DEC].flag = 0;
                    if (select < 4) {
                        freq_digit[select] = (freq_digit[select] - 1 + 10) % 10;
                    }
                }
                
                freq = freq_digit[0] * 1000 + freq_digit[1] * 100 + freq_digit[2] * 10 + freq_digit[3];

                if (key_state[KEY_ENTER].flag) {
                    key_state[KEY_ENTER].flag = 0;
                    if (select == 4) {
                        SetSelect(freq_digit_map[select], 0);
                        if (freq >= 880 && freq <= 1080) {
                            FMSetFreq(freq);
                            
                            // �л�����
                            ClearKeyFlags();
                            ShowVoltageAndTemperature(voltage, temp1, temp2);
                            
                            // �л�����
                            current_act = last_act;
                            if (last_act == UI_ACT_MUSIC) {
                                SetSelect(select = UI_ELE_MUTE, 1);
                                ShowMusicPlayer();
                            } else if (last_act == UI_ACT_VOLTAGE_TEMP) {
                                SetSelect(select = UI_ELE_VFREQ, 1);
                                ShowVoltageAndTemperature(voltage, temp1, temp2);
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
                    ShowFrequency(freq);
                }
                break;
            }
            default: {
                break;
            }
        }
    }
    
}

//*****************************************************************************
// 
// ����ԭ�ͣ�void DevicesInit(void)
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
    Timer1Init();           // Timer1��ʼ��
    UARTInit();             // UART��ʼ�� 
    I2C0Init();             // I2C0��ʼ��
    PWMInit();              // PWM��ʼ��
    ADCInit();              // ADC��ʼ��
    SysTickInit();          // ����SysTick�ж�
    IntMasterEnable();		// ���ж�����
}

//*******************************************************************************************************
//
// ����ԭ�ͣ�void Timer1Init(void)
// �������ܣ�����Timer1Ϊһ���Զ�ʱ������ʱ����Ϊ1s
// ������������
// ��������ֵ����
//
//*******************************************************************************************************
void Timer1Init(void) {
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1); // TIMER1 ʹ��
    TimerConfigure(TIMER1_BASE, TIMER_CFG_PERIODIC); // ����Ϊ 32 λ���ڶ�ʱ��
    TimerLoadSet(TIMER1_BASE, TIMER_A, g_ui32SysClock); // TIMER1A װ�ؼ���ֵ 1s
    TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT); // Timer1A ��ʱ�ж�ʹ��
    IntEnable(INT_TIMER1A); // ���� TIMER1A �ж�Դ
    // TimerEnable(TIMER1_BASE, TIMER_A); // TIMER1 ��ʼ��ʱ
}

//********************************************************************************************
//
// ����ԭ�ͣ�void TIMER1A_Handler(void)
// �������ܣ�Timer1A �жϷ�����򣬲���һ����������PF3 Ϊ���
// ������������
// ��������ֵ����
//
//**************************************************************************
void TIMER1A_Handler(void) {
    TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT); // ����жϱ�־

    BuzzerStop();
    PWMStop();
    
    if (current_note < music[0].len) {
        TimerLoadSet(TIMER1_BASE, TIMER_A, g_ui32SysClock / 1000.0 * (music[0].note[current_note].len * NOTE_LEN_BASE));
        if (note_frequency[music[0].note[current_note].note] > 0) {
            if (player_mute == 0) {
                BuzzerStart(note_frequency[music[0].note[current_note].note]);
            }
            PWMStart(note_frequency[music[0].note[current_note].note]);
        }
        if (++current_note >= music[0].len) current_note = 0; // �Զ��ز�
    }
}

//*****************************************************************************
// 
// ����ԭ�ͣ�uint32_t VoltageStabilizer(uint32_t voltage)
// �������ܣ���ѹ�����ȶ�����ȡ����ƽ��
// ����������voltage - ��ǰ��ѹ
// ��������ֵ������ƽ����ѹ
//
//*****************************************************************************
uint32_t VoltageStabilizer(uint32_t voltage) {
    static uint32_t voltage_history[5]; // ��¼��ʷ����
    static uint8_t vh_used = 0;         // ��¼������ݸ���
    static uint8_t vh_cur = 0;          // �±�λ��
    int8_t i = 0;
    int32_t vh_sum;
    
    voltage_history[vh_cur] = voltage;
    vh_cur = (vh_cur + 1) % 5;
    if (vh_used < 5) ++vh_used;
    for (i = 0; i < vh_used; ++i) {
        vh_sum += voltage_history[i];
    }

    return (uint32_t)((double)vh_sum / vh_used + 0.5);
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
    uint8_t cmd[14] = {'A', 'T', '+', 'F', 'R', 'E', '='};
    cmd[7] = freq / 1000 % 10 + '0';
    cmd[8] = freq / 100 % 10 + '0';
    cmd[9] = freq / 10 % 10 + '0';
    cmd[10] = freq % 10 + '0';
    cmd[11] = '\r';
    cmd[12] = '\n';
    cmd[13] = '\0';
    
    UARTStringPutNOBlocking(UART6_BASE, cmd);
}

//*****************************************************************************
// 
// ����ԭ�ͣ�void ShowVoltageAndTemperature(uint32_t voltage, int16_t temp1, int16_t temp2)
// �������ܣ���ʾ��ѹ�������¶ȴ��������¶�
// ����������voltage - ��ѹ
//          temp1 - �¶�1
//          temp2 - �¶�2
// ��������ֵ����
//
//*****************************************************************************
void ShowVoltageAndTemperature(uint32_t voltage, int16_t temp1, int16_t temp2) {
    unsigned char str_voltage[10], str_temp1[10], str_temp2[10];
    
    ConvertVoltageToString(str_voltage, voltage);
    ConvertTemperatureToString(str_temp1, temp1);
    ConvertTemperatureToString(str_temp2, temp2);
    
    act[UI_ACT_VOLTAGE_TEMP].str[UI_ELE_VOLTAGE] = str_voltage;
    act[UI_ACT_VOLTAGE_TEMP].str[UI_ELE_TEMP1] = str_temp1;
    act[UI_ACT_VOLTAGE_TEMP].str[UI_ELE_TEMP2] = str_temp2;

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
    act[UI_ACT_MUSIC].hide[UI_ELE_PLAYPAUSE] = 0;
    if (player_play == 1) {
        act[UI_ACT_MUSIC].str[UI_ELE_PLAYPAUSE] = (unsigned char *)"��ͣ";
    } else {
        act[UI_ACT_MUSIC].str[UI_ELE_PLAYPAUSE] = (unsigned char *)"����";
    }
    
    if (player_mute == 1) {
        act[UI_ACT_MUSIC].str[UI_ELE_MUTE] = (unsigned char *)"����";
    } else {
        act[UI_ACT_MUSIC].str[UI_ELE_MUTE] = (unsigned char *)"����";
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
