//*****************************************************************************
//
// Copyright: 2024, 上海交通大学工程实践与科技创新II-47组
// File name: ui.h
// Description: LCD界面显示和按键交互相关函数
// Author:	上海交通大学工程实践与科技创新II-47组
// Version: 1.0.0.20240506
// Date：2024-05-06
// History：
//
//*****************************************************************************

#ifndef __UI_H__
#define __UI_H__

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
// 界面相关定义
#define UI_ACT_COUNT        5       // ACT数量
#define UI_ELEMENT_COUNT    20      // 每个ACT的组件数量

#define UI_ACT_MENU         3       // ACT 3：主菜单
#define UI_ELE_MTEMP        0       // 温度显示功能
#define UI_ELE_MMUSI        1       // 音乐播放功能
#define UI_ELE_MFREQ        2       // 设置载波频率

#define UI_ACT_MUSIC        4       // ACT 4：播放界面
#define UI_ELE_RETURN       0       // 返回按钮
#define UI_ELE_MUTE         1       // 静音/输出到蜂鸣器（发射机）
#define UI_ELE_MMODE        1       // 接收模式/播放录制（接收机）
#define UI_ELE_PLAYPAUSE    2       // 播放/暂停按钮（接收机）
#define UI_ELE_RECORD       3       // 录制/保存按钮（接收机）
#define UI_ELE_RTIME        5       // 录制时间（接收机）

#define UI_ACT_VOLTAGE_TEMP 0       // ACT 0：电压温度显示页
#define UI_ELE_VOLTAGE      1       // 电压（发射机）
#define UI_ELE_TEMP         1       // 温度（接收机）
#define UI_ELE_TEMP1        3       // 温度1（发射机）
#define UI_ELE_TEMP2        5       // 温度2（发射机）
#ifdef CODE_TRANSMITTER
#define UI_ELE_VFREQ        6       // 载波频率
#define UI_ELE_VRET         7       // 返回
#else
#define UI_ELE_VFREQ        2       // 载波频率
#define UI_ELE_VRET         3       // 返回
#endif

#define UI_ACT_FREQUENCY    1       // ACT 1：载波频率页
#define UI_ELE_DIGIT1       1       // 数字百位
#define UI_ELE_DIGIT2       2       // 数字十位
#define UI_ELE_DIGIT3       3       // 数字个位
#define UI_ELE_DIGIT4       5       // 数字十分位
#define UI_ELE_CONFIRM      7       // 确认按键

#define UI_ACT_ERROR        2       // ACT 2：错误页

// 按键相关定义
#define KEYTMR_OF           10      // 长按间隔
#define KEY_ENTER           5
#define KEY_UP              4
#define KEY_DOWN            6
#define KEY_INC             2
#define KEY_DEC             8

// 其他定义
#define CHAR_CELSIUS        41446   // 字符℃的GB2312编码，使用时拆到两个字节防止WARNING

//*****************************************************************************
//
// 类型声明
//
//*****************************************************************************
struct act_t {                          // ACT界面定义
    uint8_t num;                        // 当前ACT的组件数量
    uint8_t *str[UI_ELEMENT_COUNT];     // 各组件显示的字符串
    uint8_t x[UI_ELEMENT_COUNT];        // 起始显示行
    uint8_t y[UI_ELEMENT_COUNT];        // 起始显示列
    uint8_t inverse[UI_ELEMENT_COUNT];  // 是否反显
    uint8_t hide[UI_ELEMENT_COUNT];     // 是否隐藏
};

struct key_state_t {
    uint8_t state;
    uint8_t prestate;
    uint8_t timer;
    uint8_t flag;                       // 按键按下标志
};

//*****************************************************************************
//
// 函数原型声明
//
//*****************************************************************************
void InitAct(void);                     // 初始化各ACT内容
void DisplayUiAct(int index);           // 显示第index个ACT

void InitKeyStates(void);               // 初始化按键状态
void DetectKey(uint8_t key_code);       // 侦测按键
void ClearKeyFlags(void);               // 清除按键状态

//*****************************************************************************
//
// 变量声明
//
//*****************************************************************************
extern struct act_t act[UI_ACT_COUNT];
extern struct key_state_t key_state[10];
extern uint8_t any_key_flag;
extern uint8_t current_act;             // 当前ACT编号
extern uint8_t last_act;                // 先前的ACT，用于返回

#ifdef __cplusplus
}
#endif

#endif
