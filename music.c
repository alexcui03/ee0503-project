//*****************************************************************************
//
// Copyright: 2024, 上海交通大学工程实践与科技创新II-47组
// File name: music.c
// Description: 播放音乐相关内容
// Author:	上海交通大学工程实践与科技创新II-47组
// Version: 1.0.0.20240520
// Date：2024-05-20
// History：
//
//*****************************************************************************

//*****************************************************************************
//
// 头文件
//
//*****************************************************************************
#include "music.h"

//*****************************************************************************
//
// 宏定义
//
//*****************************************************************************
// 辅助宏，用于定义音符
#define M0_NOTE(n, l) music[0].note[i].note = (n); music[0].note[i].len = (l); ++i

//*****************************************************************************
//
// 变量定义
//
//*****************************************************************************
// 音符数据，C大调C4-B6
const uint16_t note_frequency[37] = {
    0, // 0 位表示休止符
    262, 277, 294, 311, 330, 349, 370, 392, 415, 440, 466, 494,
    523, 554, 587, 622, 659, 698, 740, 784, 831, 880, 932, 988,
    1046, 1109, 1175, 1245, 1318, 1397, 1480, 1568, 1661, 1760, 1865, 1976
};

struct music_t music[1];

#ifdef CODE_TRANSMITTER
//*****************************************************************************
// 
// 函数原型：void InitMusic(void)
// 函数功能：初始化乐谱数据，仅用于发射端
// 函数参数：无
// 函数返回值：无
//
//*****************************************************************************
void InitMusic(void) {
    static uint8_t i = 0;
    
    M0_NOTE(NOTE_A4, NOTE_LEN_8);
    M0_NOTE(NOTE_C5, NOTE_LEN_8);
    M0_NOTE(NOTE_D5, NOTE_LEN_8);
    M0_NOTE(NOTE_E5, NOTE_LEN_4D);
    M0_NOTE(NOTE_G5, NOTE_LEN_8);
    M0_NOTE(NOTE_E5, NOTE_LEN_8);
    M0_NOTE(NOTE_C5, NOTE_LEN_8);
    M0_NOTE(NOTE_D5, NOTE_LEN_4);
    M0_NOTE(NOTE_A4, NOTE_LEN_2);
    M0_NOTE(0, NOTE_LEN_8);
    
    M0_NOTE(NOTE_C5, NOTE_LEN_8);
    M0_NOTE(NOTE_D5, NOTE_LEN_8);
    M0_NOTE(NOTE_E5, NOTE_LEN_8);
    M0_NOTE(NOTE_G5, NOTE_LEN_4D);
    M0_NOTE(NOTE_G5, NOTE_LEN_8);
    M0_NOTE(NOTE_A5, NOTE_LEN_4);
    M0_NOTE(NOTE_D5, NOTE_LEN_4);
    M0_NOTE(NOTE_E5, NOTE_LEN_2D);
    
    M0_NOTE(NOTE_E5, NOTE_LEN_8);
    M0_NOTE(NOTE_G5, NOTE_LEN_8);
    M0_NOTE(NOTE_A5, NOTE_LEN_2);
    M0_NOTE(NOTE_G5, NOTE_LEN_4D);
    M0_NOTE(NOTE_A5, NOTE_LEN_8);
    M0_NOTE(NOTE_C6, NOTE_LEN_8);
    M0_NOTE(NOTE_B5, NOTE_LEN_16);
    M0_NOTE(NOTE_A5, NOTE_LEN_16);
    M0_NOTE(NOTE_G5, NOTE_LEN_8);
    M0_NOTE(NOTE_A5, NOTE_LEN_8);
    M0_NOTE(NOTE_E5, NOTE_LEN_4);
    
    M0_NOTE(NOTE_C5, NOTE_LEN_8);
    M0_NOTE(NOTE_D5, NOTE_LEN_8);
    M0_NOTE(NOTE_E5, NOTE_LEN_4D);
    M0_NOTE(NOTE_G5, NOTE_LEN_8);
    M0_NOTE(NOTE_C5, NOTE_LEN_4);
    M0_NOTE(NOTE_A4, NOTE_LEN_4);
    M0_NOTE(NOTE_C5, NOTE_LEN_8);
    M0_NOTE(NOTE_D5, NOTE_LEN_8);
    M0_NOTE(NOTE_E5, NOTE_LEN_8);
    M0_NOTE(NOTE_A5, NOTE_LEN_8);
    M0_NOTE(NOTE_G5, NOTE_LEN_2D);
    M0_NOTE(0, NOTE_LEN_4);
    
    M0_NOTE(NOTE_E5, NOTE_LEN_2);
    M0_NOTE(NOTE_A5, NOTE_LEN_4D);
    M0_NOTE(NOTE_A5, NOTE_LEN_8);
    M0_NOTE(NOTE_G5, NOTE_LEN_8);
    M0_NOTE(NOTE_F5, NOTE_LEN_8);
    M0_NOTE(NOTE_E5, NOTE_LEN_4);
    M0_NOTE(NOTE_D5, NOTE_LEN_2);
    M0_NOTE(NOTE_E5, NOTE_LEN_4D);
    M0_NOTE(NOTE_G5, NOTE_LEN_8);
    M0_NOTE(NOTE_A4, NOTE_LEN_8);
    M0_NOTE(NOTE_C5, NOTE_LEN_8);
    M0_NOTE(NOTE_D5, NOTE_LEN_4);
    M0_NOTE(0, NOTE_LEN_4);
    
    M0_NOTE(NOTE_C5, NOTE_LEN_8);
    M0_NOTE(NOTE_D5, NOTE_LEN_8);
    M0_NOTE(NOTE_E5, NOTE_LEN_4);
    M0_NOTE(NOTE_G5, NOTE_LEN_8D);
    M0_NOTE(NOTE_G5, NOTE_LEN_16);
    M0_NOTE(NOTE_A5, NOTE_LEN_4);
    M0_NOTE(NOTE_C6, NOTE_LEN_4);
    M0_NOTE(NOTE_B5, NOTE_LEN_8);
    M0_NOTE(NOTE_A5, NOTE_LEN_8);
    M0_NOTE(NOTE_G5, NOTE_LEN_8);
    M0_NOTE(NOTE_E5, NOTE_LEN_8);
    M0_NOTE(NOTE_A5, NOTE_LEN_1);
    M0_NOTE(0, NOTE_LEN_2);
    
    music[0].len = i;
}
#endif

//*****************************************************************************
// 
// 函数原型：uint8_t DetectNote(uint32_t freq)
// 函数功能：查询频率最接近的音符
// 函数参数：freq - 频率
// 函数返回值：音符下标
//
//*****************************************************************************
uint8_t DetectNote(uint32_t freq) {
    uint8_t i;
    int32_t delta, last_delta = 48000;
    
    for (i = 0; i < 37; ++i) {
        delta = freq - note_frequency[i];
        if (delta < 0) delta = -delta;
        if (delta > last_delta) break;
        last_delta = delta;
    }
    
    if (i == 0 || last_delta > 100) return 0;
    
    return i - 1;
}
