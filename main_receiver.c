//*****************************************************************************
//
// Copyright: 2024, 上海交通大学工程实践与科技创新II-47组
// File name: main_receiver.c
// Description: 综合实验任务接收端
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
#include "ui.h"
#include "common.h"
#include "music.h"

//*****************************************************************************
//
// 宏定义
//
//*****************************************************************************
#define SAMPLING_FREQ       48000

//*****************************************************************************
//
// 函数原型声明
//
//*****************************************************************************
void DevicesInit(void);
void GPIOL_Handler(void);       // PL4边沿触发，未使用
void TemperatureModeInit(void); // 初始化GPIO为温度显示功能
void MusicModeInit(void);       // 初始化GPIO为音乐播放功能
void FMSetFreq(uint32_t freq);
void ShowFrequency(uint32_t freq);
void ShowTemperature(int16_t temp);
void ShowMusicPlayer(void);
void SetSelect(int ele, int new_select);

//*****************************************************************************
//
// 变量定义
//
//*****************************************************************************
// 1s计数结束标志
volatile uint8_t g_ui8INTStatus = 0;

// 保存上一次TIMER0边沿计数值
volatile uint32_t g_ui32TPreCount = 0;

// 保存本次TIMER0边沿计数值
volatile uint32_t g_ui32TCurCount = 0;

// 载波频率各位和确定对应的组件ID
const uint8_t freq_digit_map[5] = {
    UI_ELE_DIGIT1, UI_ELE_DIGIT2, UI_ELE_DIGIT3, UI_ELE_DIGIT4, UI_ELE_CONFIRM
};

// 播放器页状态
uint8_t player_play = 0, player_record = 0, player_mode = 0;
uint16_t player_current_sec = 0, player_record_sec = 0;

// 录制相关变量
int32_t counter = 0; // 上升沿间隔计数
int32_t note_counter = 0; // 音符间隔计数

// 播放相关变量
uint32_t current_note = 0;

uint8_t freq_digit[4] = { 0, 8, 9, 5 }; // 载波频率各位数字
uint32_t freq = 0;                      // 载波频率数值
int16_t temp = 0;                       // 温度
uint8_t select = 0, old_select = 0xff;  // 载波频率页选择的位或确认按钮

//*****************************************************************************
//
// 主程序
//
//*****************************************************************************
int main(void) {
    DevicesInit();  //  MCU器件初始化
    while (clock100ms_flag == 0); // 等待TM1638上电
    TM1638_Init();
    LCD_Init();
    
    InitAct();
    
    // 显示首页
    DisplayUiAct(UI_ACT_MENU);
    
    
    while (1) {
        if(g_ui8INTStatus == 1) {  // 1s定时结束，开始计算频率
            uint32_t measure_freq;
            
            g_ui8INTStatus = 0;
            
            measure_freq = g_ui32TCurCount >= g_ui32TPreCount ?
                      (g_ui32TCurCount - g_ui32TPreCount):(g_ui32TCurCount - g_ui32TPreCount + 0xFFFF);
            
            // 解码
            temp = (measure_freq - 300.0) * (40.0 - 0.0) / (4000 - 300) * 10;
            if (current_act == UI_ACT_VOLTAGE_TEMP) {
                ShowTemperature(temp);
            }
        }
        
        if (clock100ms_flag == 1) {  // 检查0.1秒定时是否到
            clock100ms_flag	= 0;
        }

        if (clock1s_flag == 1) {  // 检查1秒定时是否到
            clock1s_flag = 0;
            if (current_act == UI_ACT_MUSIC) {
                if (player_record) {
                    // 更新录制时间
                    ++player_record_sec;
                    player_current_sec = player_record_sec;
                    ShowMusicPlayer();
                } else if (player_mode == 1 && player_play) {
                    // 更新播放时间
                    if (player_current_sec < player_record_sec) {
                        ++player_current_sec;
                    }
                    ShowMusicPlayer();
                }
            }
        }

        if (clock2s_flag == 1) {  // 检查2秒定时是否到
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
                            TemperatureModeInit(); // 初始化相关引脚和计时器
                            current_act = UI_ACT_VOLTAGE_TEMP;
                            SetSelect(select = UI_ELE_VFREQ, 1);
                            ShowTemperature(temp);
                            break;
                        case UI_ELE_MMUSI:
                            MusicModeInit(); // 初始化相关引脚和计时器
                            player_play = player_record = 0;
                            current_act = UI_ACT_MUSIC;
                            SetSelect(select = UI_ELE_MMODE, 1);
                            ShowMusicPlayer();
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
                                // 本地模式，先使能PWM
                                PWMInit();
                            } else {
                                // 广播模式，设置PF3输出
                                MusicModeInit();
                            }
                            ShowMusicPlayer();
                            break;
                        case UI_ELE_PLAYPAUSE:
                            player_play = !player_play;
                            if (player_mode == 1) {
                                if (player_play) {
                                    // 播放本地录音
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
                                // 录制结束
                                player_current_sec = 0;
                            } else {
                                // 开始录制
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
                        // 切换界面并记录原界面用于返回
                        current_act = UI_ACT_FREQUENCY;
                        last_act = UI_ACT_VOLTAGE_TEMP;
                        select = 0;
                        SetSelect(freq_digit_map[select], 1);
                        ShowFrequency(freq);
                        ClearKeyFlags();
                    } else {
                        // 返回到菜单
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
                            
                            // 切换界面
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
// 函数原型：DevicesInit(void)
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
    UARTInit();             // UART初始化 
    I2C0Init();             // I2C0初始化
    PWMInit();              // PWM初始化
    ADCInit();              // ADC初始化
    SysTickInit();          // 设置SysTick中断
    IntMasterEnable();		// 总中断允许
    
    IntPrioritySet(INT_TIMER1A, 0x00);		// 设置INT_TIMER1A最高优先级   
    IntPrioritySet(INT_TIMER0A, 0x01);		// 设置INT_TIMER0A最高优先级
    IntPrioritySet(FAULT_SYSTICK, 0xe0);	// 设置SYSTICK优先级低于INT_TIMER0A的优先级
}

//*******************************************************************************************************
//
// 函数原型：void TemperatureModeInit(void)
// 函数功能：PL4作为边沿输入到Timer0，初始化Timer0和Timer1
// 函数参数：无
// 函数返回值：无
//
//*******************************************************************************************************
void TemperatureModeInit(void) {
    // 关闭原有TIMER
    if (SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER0)) {
        TimerDisable(TIMER0_BASE, TIMER_A);
    }
    if (SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER1)) {
        TimerDisable(TIMER1_BASE, TIMER_A);
    }
    
    // 使能PL4
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOL);   // 使能GPIOL
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOL));
    GPIOPinConfigure(GPIO_PL4_T0CCP0);             // 配置引脚复用   
    GPIOPinTypeTimer(GPIO_PORTL_BASE, GPIO_PIN_4); // 引脚映射
    GPIOPadConfigSet(GPIO_PORTL_BASE, GPIO_PIN_4, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU); // 将引脚弱上拉
    
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);  // 使能TIMER0
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER0));
    TimerConfigure(TIMER0_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_CAP_COUNT_UP); // 半长定时器，增计数
    TimerControlEvent(TIMER0_BASE, TIMER_A, TIMER_EVENT_POS_EDGE); // 初始化配置为捕捉上升沿
    g_ui32TPreCount = g_ui32TCurCount = TimerValueGet(TIMER0_BASE, TIMER_A); // 清零计数
    
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);  // TIMER1 使能
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER1));
    TimerConfigure(TIMER1_BASE, TIMER_CFG_PERIODIC); // 设置为 32 位周期定时器
    TimerLoadSet(TIMER1_BASE, TIMER_A, g_ui32SysClock);  // TIMER1A装载计数值1s
    
    TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);    // Timer1A 超时中断使能
    IntEnable(INT_TIMER1A);                             // 开启 TIMER1A 中断源
    TimerEnable(TIMER1_BASE, TIMER_A);                  // TIMER1 开始计时
    TimerEnable(TIMER0_BASE, TIMER_A);                  // TIMER0 开始计数
}

//*******************************************************************************************************
//
// 函数原型：void MusicModeInit(void)
// 函数功能：PL4作为输入，使用Timer1作为定时器连续采样
// 函数参数：无
// 函数返回值：无
//
//*******************************************************************************************************
void MusicModeInit(void) {
    // 关闭原有TIMER
    if (SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER0)) {
        TimerDisable(TIMER0_BASE, TIMER_A);
    }
    if (SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER1)) {
        TimerDisable(TIMER1_BASE, TIMER_A);
    }
    
    // 使能PL4并作为GPIO输入
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOL);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOL));
    GPIOPinTypeGPIOInput(GPIO_PORTL_BASE, GPIO_PIN_4);
    GPIOPadConfigSet(GPIO_PORTL_BASE, GPIO_PIN_4, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU); // 设为弱上拉
    
    // 使能PF3并作为GPIO输出
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF));
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_3);
    
    // 设置PL4边沿中断
    /*GPIOIntDisable(GPIO_PORTL_BASE, GPIO_PIN_4);
	GPIOIntClear(GPIO_PORTL_BASE, GPIO_PIN_4);
	GPIOIntRegister(GPIO_PORTL_BASE, GPIOL_Handler);
	GPIOIntTypeSet(GPIO_PORTL_BASE, GPIO_PIN_4, GPIO_BOTH_EDGES);
	GPIOIntEnable(GPIO_PORTL_BASE, GPIO_PIN_4);*/
    
    // 初始化Timer1作为采样时钟
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER1));
    TimerConfigure(TIMER1_BASE, TIMER_CFG_PERIODIC); // 32位周期计时器
    TimerLoadSet(TIMER1_BASE, TIMER_A, g_ui32SysClock / SAMPLING_FREQ);
    
    TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT); // 设置中断
    IntEnable(INT_TIMER1A); // 开启 TIMER1A 中断源
    TimerEnable(TIMER1_BASE, TIMER_A); // 开启TIMER1A
}

//*******************************************************************************************************
// 
// 函数原型：void TIMER1A_Handler(void)
// 函数功能：Timer1A中断服务程序，记录捕获方波上升沿时定时器（TIMER0）的计数值
// 函数参数：无
// 函数返回值：无
//
//*******************************************************************************************************
void TIMER1A_Handler(void) {
    TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);     // 清除中断标志
    if (current_act == UI_ACT_VOLTAGE_TEMP) {
        // 温度显示
        g_ui32TPreCount = g_ui32TCurCount;                  // 保存上一次TIMER0边沿计数值
        g_ui32TCurCount = TimerValueGet(TIMER0_BASE, TIMER_A);  // 读取TIMER0边沿计数值

        g_ui8INTStatus = 1; // 1s计数完成
    } else if (player_mode == 0) { // FM播放页
        static uint8_t last = 0;
        static uint8_t last_note = 0; // 上一个note
        uint8_t value = GPIOPinRead(GPIO_PORTL_BASE, GPIO_PIN_4) & GPIO_PIN_4;
        
        if (player_record) {
            ++counter;
            ++note_counter;
        }
        
        if (value != last) { // 发生边沿
            last = value;
            if (player_play) { // 正在播放
                GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, value ? GPIO_PIN_3 : 0);
            }
            
            if (value && player_record) { // 上升沿且正在录制
                uint8_t note = DetectNote(SAMPLING_FREQ / counter);
                if (note != last_note) { // 音变化
                    int32_t len = (double)note_counter / SAMPLING_FREQ * 1000.0 / NOTE_LEN_BASE + 0.5;
                    if (len > 0) { // 滤除杂音
                        music[0].note[music[0].len].note = last_note;
                        music[0].note[music[0].len].len = len;
                        ++music[0].len;
                        note_counter -= len * NOTE_LEN_BASE * SAMPLING_FREQ / 1000; // 误差修正
                        last_note = note;
                    } else {
                        if (note_counter > 0) {
                            note_counter = 0; // 防止杂音积累
                        }
                        last_note = note;
                    }
                }
                counter = 0;
            }
        }
    } else { // 本地播放页
        if (player_play) {
            BuzzerStop();
            
            if (current_note < music[0].len) {
                TimerLoadSet(TIMER1_BASE, TIMER_A, g_ui32SysClock / 1000.0 * music[0].note[current_note].len * NOTE_LEN_BASE);
                if (note_frequency[music[0].note[current_note].note] > 0) {
                    BuzzerStart(note_frequency[music[0].note[current_note].note]);
                }
                ++current_note;
            } else {
                // 播放结束
                player_play = 0;
                ShowMusicPlayer();
            }
        }
    }
}

//*******************************************************************************************************
// 
// 函数原型：void GPIOL_Handler(void)
// 函数功能：GPIOL中断服务程序，处理PL4输入信号，未使用
// 函数参数：无
// 函数返回值：无
//
//*******************************************************************************************************
void GPIOL_Handler(void) {
    GPIOIntClear(GPIO_PORTL_BASE, GPIOIntStatus(GPIO_PORTL_BASE, true));
    
    if (player_play) {
        // 仅在播放时输出到蜂鸣器
        if (GPIOPinRead(GPIO_PORTL_BASE, GPIO_PIN_4) & GPIO_PIN_4) { // 上升沿
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_PIN_3);
        } else { // 下降沿
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0);
        }
    }
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
// 函数原型：void ShowTemperature(uint16_t temp)
// 函数功能：显示温度
// 函数参数：temp - 温度
// 函数返回值：无
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
    static uint8_t buffer[12];
    
    // 显示播放时长/总录制时长，00:00/99:99
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
        act[UI_ACT_MUSIC].str[UI_ELE_MMODE] = (unsigned char *)"广播";
        act[UI_ACT_MUSIC].hide[UI_ELE_RECORD] = 0;
        
        if (player_play == 1) {
            act[UI_ACT_MUSIC].str[UI_ELE_PLAYPAUSE] = (unsigned char *)"静音";
        } else {
            act[UI_ACT_MUSIC].str[UI_ELE_PLAYPAUSE] = (unsigned char *)"扬声";
        }
    } else {
        act[UI_ACT_MUSIC].str[UI_ELE_MMODE] = (unsigned char *)"本地";
        act[UI_ACT_MUSIC].hide[UI_ELE_RECORD] = 1;
    
        if (player_play == 1) {
            act[UI_ACT_MUSIC].str[UI_ELE_PLAYPAUSE] = (unsigned char *)"暂停";
        } else {
            act[UI_ACT_MUSIC].str[UI_ELE_PLAYPAUSE] = (unsigned char *)"播放";
        }
    }
    
    if (player_record == 1) {
        act[UI_ACT_MUSIC].str[UI_ELE_RECORD] = (unsigned char *)"保存";
    } else {
        act[UI_ACT_MUSIC].str[UI_ELE_RECORD] = (unsigned char *)"录制";
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
