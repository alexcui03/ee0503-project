//*****************************************************************************
//
// Copyright: 2024, 上海交通大学工程实践与科技创新II-47组
// File name: main_transmitter.c
// Description: 综合实验任务发射端
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
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"        // 基址宏定义
#include "inc/hw_types.h"         // 数据类型宏定义，寄存器访问函数
#include "inc/hw_timer.h"         // 与定时器有关的宏定义
#include "inc/hw_ints.h"          // 与中断有关的宏定义
#include "driverlib/debug.h"      // 调试用
#include "driverlib/gpio.h"       // 通用IO口宏定义
#include "driverlib/pin_map.h"    // TM4C系列MCU外围设备管脚宏定义
#include "driverlib/pwm.h"        // 与Timer有关的函数原型
#include "driverlib/sysctl.h"     // 系统控制定义
#include "driverlib/systick.h"    // SysTick Driver 原型
#include "driverlib/interrupt.h"  // NVIC Interrupt Controller Driver 原型
#include "driverlib/uart.h"       // 与UART有关的宏定义和函数原型
#include "driverlib/timer.h"      // 与Timer有关的函数原型  
#include "JLX12864G.h"            // LCD显示屏
#include "LM75BD.h"               // 温度传感器
#include "ADC.h"                  // ADC
#include "tm1638.h"               // TM1638
#include "ui.h"                   // UI相关函数
#include "common.h"               // 接收器和发射器的通用函数
#include "music.h"                // 音乐相关定义

//*****************************************************************************
//
// 宏定义
//
//*****************************************************************************


//*****************************************************************************
//
// 函数原型声明
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
// 变量定义
//
//*****************************************************************************
// 载波频率各位和确定对应的组件ID
const uint8_t freq_digit_map[5] = {
    UI_ELE_DIGIT1, UI_ELE_DIGIT2, UI_ELE_DIGIT3, UI_ELE_DIGIT4, UI_ELE_CONFIRM
};

// 播放器状态
uint8_t player_play = 0, player_mute = 0;

uint8_t current_note = 0; // 当前音符

//*****************************************************************************
//
// 主程序
//
//*****************************************************************************
int main(void) {
    uint8_t freq_digit[4] = { 0, 8, 9, 5 }; // 载波频率各位数字
    uint32_t freq = 1017;                   // 载波频率数值
    int16_t temp1 = 0, temp2 = 0;           // 温度数值
    uint32_t voltage = 0;                   // 电压值
    uint8_t select = 0, old_select = 0xff;  // 选择的组件

    DevicesInit(); // MCU初始化
    while (clock100ms_flag == 0); // 等待1638上电
    TM1638_Init();
    LCD_Init();
    
    InitAct();
    
    InitMusic();

    // 首次采集数据
    temp1 = GetTemputerature(LM75BD_ADR1);
    temp2 = GetTemputerature(LM75BD_ADR2);
    voltage = VoltageStabilizer(ADC_Sample() * 330 / 4095);
    
    // 显示首页
    DisplayUiAct(UI_ACT_MENU);

    while (1) {
        // 100ms软定时
        if (clock100ms_flag == 1) {
            clock100ms_flag	= 0;
        }

        // 1s软定时
        if (clock1s_flag == 1) {
            clock1s_flag = 0;
            if (current_act == UI_ACT_VOLTAGE_TEMP) {
                // 数据采集
                temp1 = GetTemputerature(LM75BD_ADR1);
                temp2 = GetTemputerature(LM75BD_ADR2);
                voltage = VoltageStabilizer(ADC_Sample() * 330 / 4095);
                
                ShowVoltageAndTemperature(voltage, temp1, temp2);
                
                // 编码并输出到PWM
                PWMStart(temp1 / 10.0 / (40.0  - 0.0) * (4000 - 300) + 300);
            }
        }

        // 2s软定时
        if (clock2s_flag == 1) {
            clock2s_flag = 0;
            if (current_act == UI_ACT_ERROR) {
                ShowFrequency(freq);
                ClearKeyFlags();
            }
        }
        
        switch (current_act) {
            case UI_ACT_MENU: {
                if (key_state[KEY_INC].flag) { // 上一个组件
                    key_state[KEY_INC].flag = 0;
                    select = (select - 1 + 3) % 3;
                }
                if (key_state[KEY_DEC].flag) { // 下一个组件
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
                            // 进入播放功能
                            current_note = 0;
                            PWMStop();
                            break;
                        case UI_ELE_MFREQ:
                            current_act = UI_ACT_FREQUENCY;
                            last_act = UI_ACT_MENU; // 进入FREQ设置last_act便于返回
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
                            // 开始播放
                            TimerEnable(TIMER1_BASE, TIMER_A);
                        } else {
                            // 停止播放
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
                                // 取消静音需要播放当前音符，若正在播放
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
                        // 返回菜单
                        if (player_play) {
                            // 停止播放
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
                        // 切换界面并记录原界面用于返回
                        current_act = UI_ACT_FREQUENCY;
                        last_act = UI_ACT_VOLTAGE_TEMP;
                        SetSelect(freq_digit_map[select = 0], 1);
                        ShowFrequency(freq);
                        ClearKeyFlags();
                    } else {
                        // 返回到菜单
                        current_act = UI_ACT_MENU;
                        SetSelect(select = UI_ELE_MTEMP, 1);
                        DisplayUiAct(UI_ACT_MENU);
                        ClearKeyFlags();
                        PWMStop(); // 停止发送
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
                uint32_t old_freq = freq; // 原来的freq，若更新则刷新页面
                
                if (key_state[KEY_UP].flag) { // 上一个组件
                    key_state[KEY_UP].flag = 0;
                    select = (select - 1 + 5) % 5;
                }
                if (key_state[KEY_DOWN].flag) { // 下一个组件
                    key_state[KEY_DOWN].flag = 0;
                    select = (select + 1) % 5;
                }
                if (key_state[KEY_INC].flag) { // 增加值
                    key_state[KEY_INC].flag = 0;
                    if (select < 4) {
                        freq_digit[select] = (freq_digit[select] + 1) % 10;
                    }
                }
                if (key_state[KEY_DEC].flag) { // 减小值
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
                            
                            // 切换界面
                            ClearKeyFlags();
                            ShowVoltageAndTemperature(voltage, temp1, temp2);
                            
                            // 切换界面
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
                            ResetClocks(); // 错误页面需要重新软件定时
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
// 函数原型：void DevicesInit(void)
// 函数功能：MCU初始化
// 函数参数：无
// 函数返回值：无
//
//*****************************************************************************
void DevicesInit(void) {
    // 使用外部25MHz主时钟源，经过PLL，然后分频为20MHz
    g_ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |SYSCTL_OSC_MAIN | 
                                       SYSCTL_USE_PLL |SYSCTL_CFG_VCO_480), 
                                       20000000);
    
    // 使能浮点运算单元
    FPULazyStackingEnable();
    FPUEnable();
    
    GPIOInit();             // GPIO初始化
    Timer1Init();           // Timer1初始化
    UARTInit();             // UART初始化 
    I2C0Init();             // I2C0初始化
    PWMInit();              // PWM初始化
    ADCInit();              // ADC初始化
    SysTickInit();          // 设置SysTick中断
    IntMasterEnable();		// 总中断允许
}

//*******************************************************************************************************
//
// 函数原型：void Timer1Init(void)
// 函数功能：设置Timer1为一次性定时器，定时周期为1s
// 函数参数：无
// 函数返回值：无
//
//*******************************************************************************************************
void Timer1Init(void) {
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1); // TIMER1 使能
    TimerConfigure(TIMER1_BASE, TIMER_CFG_PERIODIC); // 设置为 32 位周期定时器
    TimerLoadSet(TIMER1_BASE, TIMER_A, g_ui32SysClock); // TIMER1A 装载计数值 1s
    TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT); // Timer1A 超时中断使能
    IntEnable(INT_TIMER1A); // 开启 TIMER1A 中断源
    // TimerEnable(TIMER1_BASE, TIMER_A); // TIMER1 开始计时
}

//********************************************************************************************
//
// 函数原型：void TIMER1A_Handler(void)
// 函数功能：Timer1A 中断服务程序，产生一个新音符，PF3 为输出
// 函数参数：无
// 函数返回值：无
//
//**************************************************************************
void TIMER1A_Handler(void) {
    TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT); // 清除中断标志

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
        if (++current_note >= music[0].len) current_note = 0; // 自动重播
    }
}

//*****************************************************************************
// 
// 函数原型：uint32_t VoltageStabilizer(uint32_t voltage)
// 函数功能：电压数据稳定器，取滑动平均
// 函数参数：voltage - 当前电压
// 函数返回值：滑动平均电压
//
//*****************************************************************************
uint32_t VoltageStabilizer(uint32_t voltage) {
    static uint32_t voltage_history[5]; // 记录历史数据
    static uint8_t vh_used = 0;         // 已录入的数据个数
    static uint8_t vh_cur = 0;          // 下标位置
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
// 函数原型：void FMSetFreq(uint32_t freq)
// 函数功能：设置FM载波频率，通过UART6发送
// 函数参数：无
// 函数返回值：无
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
// 函数原型：void ShowVoltageAndTemperature(uint32_t voltage, int16_t temp1, int16_t temp2)
// 函数功能：显示电压和两个温度传感器的温度
// 函数参数：voltage - 电压
//          temp1 - 温度1
//          temp2 - 温度2
// 函数返回值：无
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
// 函数原型：void ShowFrequency(uint32_t freq)
// 函数功能：显示载波频率
// 函数参数：freq - 载波频率
// 函数返回值：无
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
// 函数原型：void ShowMusicPlayer(void)
// 函数功能：显示播放器页面
// 函数参数：无
// 函数返回值：无
//
//*****************************************************************************
void ShowMusicPlayer(void) {
    act[UI_ACT_MUSIC].hide[UI_ELE_PLAYPAUSE] = 0;
    if (player_play == 1) {
        act[UI_ACT_MUSIC].str[UI_ELE_PLAYPAUSE] = (unsigned char *)"暂停";
    } else {
        act[UI_ACT_MUSIC].str[UI_ELE_PLAYPAUSE] = (unsigned char *)"播放";
    }
    
    if (player_mute == 1) {
        act[UI_ACT_MUSIC].str[UI_ELE_MUTE] = (unsigned char *)"扬声";
    } else {
        act[UI_ACT_MUSIC].str[UI_ELE_MUTE] = (unsigned char *)"静音";
    }
    
    DisplayUiAct(UI_ACT_MUSIC);
}

//*****************************************************************************
// 
// 函数原型：void SetSelect(int ele, int new_select)
// 函数功能：设置组件高亮
// 函数参数：ele - 组件ID
//          new_select - 是否高亮
// 函数返回值：无
//
//*****************************************************************************
void SetSelect(int ele, int new_select) {
    act[current_act].inverse[ele] = new_select;
}
