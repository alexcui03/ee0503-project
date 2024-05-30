//*****************************************************************************
//
// Copyright: 2024, 上海交通大学工程实践与科技创新II-47组
// File name: ui.c
// Description: LCD界面显示和按键交互相关函数
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
#include "ui.h"
#include "JLX12864G.h"            // LCD显示屏

//*****************************************************************************
//
// 变量声明
//
//*****************************************************************************
struct act_t act[UI_ACT_COUNT];
struct key_state_t key_state[10];
uint8_t any_key_flag = 0;
uint8_t current_act = 0;
uint8_t last_act = 0;

//*****************************************************************************
//
// 函数原型：void InitAct(void)
// 函数功能：初始化各界面元素
// 函数参数：无
// 函数返回值：无
//
//*****************************************************************************
void InitAct(void) {
    // 4, Menu
    act[UI_ACT_MENU].num = 3;
    
    act[UI_ACT_MENU].str[UI_ELE_MTEMP] = (unsigned char *)"温度显示";
    act[UI_ACT_MENU].x[UI_ELE_MTEMP] = 2;
    act[UI_ACT_MENU].y[UI_ELE_MTEMP] = 5;
    act[UI_ACT_MENU].inverse[UI_ELE_MTEMP] = 0;
    act[UI_ACT_MENU].str[UI_ELE_MMUSI] = (unsigned char *)"音乐播放";
    act[UI_ACT_MENU].x[UI_ELE_MMUSI] = 4;
    act[UI_ACT_MENU].y[UI_ELE_MMUSI] = 5;
    act[UI_ACT_MENU].inverse[UI_ELE_MMUSI] = 0;
    act[UI_ACT_MENU].str[UI_ELE_MFREQ] = (unsigned char *)"载频设置";
    act[UI_ACT_MENU].x[UI_ELE_MFREQ] = 6;
    act[UI_ACT_MENU].y[UI_ELE_MFREQ] = 5;
    act[UI_ACT_MENU].inverse[UI_ELE_MFREQ] = 0;
    
    // 5, Player
#ifdef CODE_TRANSMITTER
    act[UI_ACT_MUSIC].num = 3;
    act[UI_ACT_MUSIC].str[UI_ELE_RETURN] = (unsigned char *)"返回";
    act[UI_ACT_MUSIC].x[UI_ELE_RETURN] = 7;
    act[UI_ACT_MUSIC].y[UI_ELE_RETURN] = 13;
    act[UI_ACT_MUSIC].inverse[UI_ELE_RETURN] = 0;
    act[UI_ACT_MUSIC].x[UI_ELE_MUTE] = 3;
    act[UI_ACT_MUSIC].y[UI_ELE_MUTE] = 4;
    act[UI_ACT_MUSIC].inverse[UI_ELE_MUTE] = 0;
    act[UI_ACT_MUSIC].str[UI_ELE_PLAYPAUSE] = (unsigned char *)"1234";
    act[UI_ACT_MUSIC].x[UI_ELE_PLAYPAUSE] = 3;
    act[UI_ACT_MUSIC].y[UI_ELE_PLAYPAUSE] = 10;
    act[UI_ACT_MUSIC].inverse[UI_ELE_PLAYPAUSE] = 0;
#else
    act[UI_ACT_MUSIC].num = 6;
    
    act[UI_ACT_MUSIC].str[UI_ELE_RETURN] = (unsigned char *)"返回";
    act[UI_ACT_MUSIC].x[UI_ELE_RETURN] = 7;
    act[UI_ACT_MUSIC].y[UI_ELE_RETURN] = 13;
    act[UI_ACT_MUSIC].inverse[UI_ELE_RETURN] = 0;
    act[UI_ACT_MUSIC].x[UI_ELE_MMODE] = 1;
    act[UI_ACT_MUSIC].y[UI_ELE_MMODE] = 6;
    act[UI_ACT_MUSIC].inverse[UI_ELE_MMODE] = 0;
    act[UI_ACT_MUSIC].x[UI_ELE_PLAYPAUSE] = 3;
    act[UI_ACT_MUSIC].y[UI_ELE_PLAYPAUSE] = 4;
    act[UI_ACT_MUSIC].inverse[UI_ELE_PLAYPAUSE] = 0;
    act[UI_ACT_MUSIC].x[UI_ELE_RECORD] = 3;
    act[UI_ACT_MUSIC].y[UI_ELE_RECORD] = 10;
    act[UI_ACT_MUSIC].inverse[UI_ELE_RECORD] = 0;
    act[UI_ACT_MUSIC].str[4] = (unsigned char *)"模式:";
    act[UI_ACT_MUSIC].x[4] = 1;
    act[UI_ACT_MUSIC].y[4] = 1;
    act[UI_ACT_MUSIC].inverse[4] = 0;
    act[UI_ACT_MUSIC].x[UI_ELE_RTIME] = 5;
    act[UI_ACT_MUSIC].y[UI_ELE_RTIME] = 3;
    act[UI_ACT_MUSIC].inverse[UI_ELE_RTIME] = 0;
#endif
    
    // 0, Voltage and Temperature
#ifdef CODE_TRANSMITTER
    act[UI_ACT_VOLTAGE_TEMP].num = 8;
    act[UI_ACT_VOLTAGE_TEMP].str[0] = (unsigned char *)"电压 :";
    act[UI_ACT_VOLTAGE_TEMP].x[0] = 1;
    act[UI_ACT_VOLTAGE_TEMP].y[0] = 1;
    act[UI_ACT_VOLTAGE_TEMP].inverse[0] = 0;
    act[UI_ACT_VOLTAGE_TEMP].x[UI_ELE_VOLTAGE] = 1;
    act[UI_ACT_VOLTAGE_TEMP].y[UI_ELE_VOLTAGE] = 8;
    act[UI_ACT_VOLTAGE_TEMP].inverse[UI_ELE_VOLTAGE] = 0;
    act[UI_ACT_VOLTAGE_TEMP].str[2] = (unsigned char *)"温度1:";
    act[UI_ACT_VOLTAGE_TEMP].x[2] = 3;
    act[UI_ACT_VOLTAGE_TEMP].y[2] = 1;
    act[UI_ACT_VOLTAGE_TEMP].inverse[2] = 0;
    act[UI_ACT_VOLTAGE_TEMP].x[UI_ELE_TEMP1] = 3;
    act[UI_ACT_VOLTAGE_TEMP].y[UI_ELE_TEMP1] = 7;
    act[UI_ACT_VOLTAGE_TEMP].inverse[UI_ELE_TEMP1] = 0;
    act[UI_ACT_VOLTAGE_TEMP].str[4] = (unsigned char *)"温度2:";
    act[UI_ACT_VOLTAGE_TEMP].x[4] = 5;
    act[UI_ACT_VOLTAGE_TEMP].y[4] = 1;
    act[UI_ACT_VOLTAGE_TEMP].inverse[4] = 0;
    act[UI_ACT_VOLTAGE_TEMP].x[UI_ELE_TEMP2] = 5;
    act[UI_ACT_VOLTAGE_TEMP].y[UI_ELE_TEMP2] = 7;
    act[UI_ACT_VOLTAGE_TEMP].inverse[UI_ELE_TEMP2] = 0;
    act[UI_ACT_VOLTAGE_TEMP].str[UI_ELE_VFREQ] = (unsigned char *)"载频设置";
    act[UI_ACT_VOLTAGE_TEMP].x[UI_ELE_VFREQ] = 7;
    act[UI_ACT_VOLTAGE_TEMP].y[UI_ELE_VFREQ] = 1;
    act[UI_ACT_VOLTAGE_TEMP].inverse[UI_ELE_VFREQ] = 0;
    act[UI_ACT_VOLTAGE_TEMP].str[UI_ELE_VRET] = (unsigned char *)"返回";
    act[UI_ACT_VOLTAGE_TEMP].x[UI_ELE_VRET] = 7;
    act[UI_ACT_VOLTAGE_TEMP].y[UI_ELE_VRET] = 13;
    act[UI_ACT_VOLTAGE_TEMP].inverse[UI_ELE_VRET] = 0;
#else
    act[UI_ACT_VOLTAGE_TEMP].num = 4;
    act[UI_ACT_VOLTAGE_TEMP].str[0] = (unsigned char *)"温度:";
    act[UI_ACT_VOLTAGE_TEMP].x[0] = 3;
    act[UI_ACT_VOLTAGE_TEMP].y[0] = 1;
    act[UI_ACT_VOLTAGE_TEMP].inverse[0] = 0;
    act[UI_ACT_VOLTAGE_TEMP].x[UI_ELE_TEMP] = 3;
    act[UI_ACT_VOLTAGE_TEMP].y[UI_ELE_TEMP] = 6;
    act[UI_ACT_VOLTAGE_TEMP].inverse[UI_ELE_TEMP] = 0;
    act[UI_ACT_VOLTAGE_TEMP].str[UI_ELE_VFREQ] = (unsigned char *)"载频设置";
    act[UI_ACT_VOLTAGE_TEMP].x[UI_ELE_VFREQ] = 7;
    act[UI_ACT_VOLTAGE_TEMP].y[UI_ELE_VFREQ] = 1;
    act[UI_ACT_VOLTAGE_TEMP].inverse[UI_ELE_VFREQ] = 0;
    act[UI_ACT_VOLTAGE_TEMP].str[UI_ELE_VRET] = (unsigned char *)"返回";
    act[UI_ACT_VOLTAGE_TEMP].x[UI_ELE_VRET] = 7;
    act[UI_ACT_VOLTAGE_TEMP].y[UI_ELE_VRET] = 13;
    act[UI_ACT_VOLTAGE_TEMP].inverse[UI_ELE_VRET] = 0;
#endif
    
    // 1, Frequency
    act[UI_ACT_FREQUENCY].num = 8;
    
    act[UI_ACT_FREQUENCY].str[0] = (unsigned char *)"载频:";
    act[UI_ACT_FREQUENCY].x[0] = 1;
    act[UI_ACT_FREQUENCY].y[0] = 1;
    act[UI_ACT_FREQUENCY].inverse[0] = 0;
    act[UI_ACT_FREQUENCY].x[UI_ELE_DIGIT1] = 1;
    act[UI_ACT_FREQUENCY].y[UI_ELE_DIGIT1] = 6;
    act[UI_ACT_FREQUENCY].inverse[UI_ELE_DIGIT1] = 0;
    act[UI_ACT_FREQUENCY].x[UI_ELE_DIGIT2] = 1;
    act[UI_ACT_FREQUENCY].y[UI_ELE_DIGIT2] = 7;
    act[UI_ACT_FREQUENCY].inverse[UI_ELE_DIGIT2] = 0;
    act[UI_ACT_FREQUENCY].x[UI_ELE_DIGIT3] = 1;
    act[UI_ACT_FREQUENCY].y[UI_ELE_DIGIT3] = 8;
    act[UI_ACT_FREQUENCY].inverse[UI_ELE_DIGIT3] = 0;
    act[UI_ACT_FREQUENCY].str[4] = (unsigned char *)".";
    act[UI_ACT_FREQUENCY].x[4] = 1;
    act[UI_ACT_FREQUENCY].y[4] = 9;
    act[UI_ACT_FREQUENCY].inverse[4] = 0;
    act[UI_ACT_FREQUENCY].x[UI_ELE_DIGIT4] = 1;
    act[UI_ACT_FREQUENCY].y[UI_ELE_DIGIT4] = 10;
    act[UI_ACT_FREQUENCY].inverse[UI_ELE_DIGIT4] = 0;
    act[UI_ACT_FREQUENCY].str[6] = (unsigned char *)"MHz";
    act[UI_ACT_FREQUENCY].x[6] = 1;
    act[UI_ACT_FREQUENCY].y[6] = 11;
    act[UI_ACT_FREQUENCY].inverse[6] = 0;
    act[UI_ACT_FREQUENCY].str[UI_ELE_CONFIRM] = (unsigned char *)"确定";
    act[UI_ACT_FREQUENCY].x[UI_ELE_CONFIRM] = 7;
    act[UI_ACT_FREQUENCY].y[UI_ELE_CONFIRM] = 13;
    act[UI_ACT_FREQUENCY].inverse[UI_ELE_CONFIRM] = 0;
    
    // 2, Error
    act[UI_ACT_ERROR].num = 1;
    
    act[UI_ACT_ERROR].str[0] = (unsigned char *)"载频超范围!";
    act[UI_ACT_ERROR].x[0] = 3;
    act[UI_ACT_ERROR].y[0] = 1;
    act[UI_ACT_ERROR].inverse[0] = 0;
}

//*****************************************************************************
//
// 函数原型：void DisplayUiAct(int index)
// 函数功能：清除屏幕并显示下标为index的ACT，并修改current_act变量
// 函数参数：index - ACT的下标
// 函数返回值：无
//
//*****************************************************************************
void DisplayUiAct(int index) {
    int j;
    
    clear_screen();
    for (j = 0; j < act[index].num; ++j) {
        if (act[index].hide[j]) continue;
        display_GB2312_string(
            act[index].x[j], (act[index].y[j] - 1) * 8 + 1,
            act[index].str[j], act[index].inverse[j]
        );
    }
    
    current_act = index;
}

//*****************************************************************************
//
// 函数原型：void InitKeyStates(void)
// 函数功能：初始化按键状态
// 函数参数：无
// 函数返回值：无
//
//*****************************************************************************
void InitKeyStates(void) {
    int i;
    
    for (i = 0; i < 10; ++i) {
        key_state[i].state = 0;
        key_state[i].prestate = 0;
        key_state[i].timer = 0;
        key_state[i].flag = 0;
    }
}

//*****************************************************************************
//
// 函数原型：void DetectKey(uint8_t key_code)
// 函数功能：根据当前按下按键处理按键状态
// 函数参数：key_code - 当前按下的按键编号
// 函数返回值：无
//
//*****************************************************************************
void DetectKey(uint8_t key_code) {
    int i;
    
    for (i = 1; i < 10; ++i) {
        key_state[i].prestate = key_state[i].state;
        
        /*
        // 无长按检测
        if (key_code == key_code_map[i]) {
            if (key_state[i].prestate == 0) {
                key_state[i].state = 1;
                key_state[i].flag = 1;
                any_key_flag = 1;
            }
        } else {
            key_state[i].state = 0;
        }
        */
        
        // 含长按检测
        if (key_code == i) {
            if (key_state[i].prestate == 0) {
                any_key_flag = 1; // 长按时不设置any_key_flag
                key_state[i].state = 1;
                key_state[i].flag = 1;
                key_state[i].timer = KEYTMR_OF * 2; // 首次长按需要两倍延时，防止误按
            } else {
                if (--key_state[i].timer == 0) {
                    key_state[i].flag = 1;
                    key_state[i].timer = KEYTMR_OF;
                }
            }
        } else {
            key_state[i].state = 0;
        }
    }
}

//*****************************************************************************
//
// 函数原型：void ClearKeyFlags(void)
// 函数功能：清除按键状态
// 函数参数：无
// 函数返回值：无
//
//*****************************************************************************
void ClearKeyFlags(void) {
    int i;
    
    any_key_flag = 0;
    for (i = 1; i < 10; ++i) {
        key_state[i].flag = 0;
    }
}
