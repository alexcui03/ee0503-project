//*****************************************************************************
//
// Copyright: 2024, 上海交通大学工程实践与科技创新II-47组
// File name: music.h
// Description: 播放音乐相关内容
// Author:	上海交通大学工程实践与科技创新II-47组
// Version: 1.0.0.20240520
// Date：2024-05-20
// History：
//
//*****************************************************************************

#ifndef __MUSIC_H__
#define __MUSIC_H__

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
// 音符对应的频率表下标
#define NOTE_C4 1
#define NOTE_D4 3
#define NOTE_E4 5
#define NOTE_F4 6
#define NOTE_G4 8
#define NOTE_A4 10
#define NOTE_B4 12
#define NOTE_C5 13
#define NOTE_D5 15
#define NOTE_E5 17
#define NOTE_F5 18
#define NOTE_G5 20
#define NOTE_A5 22
#define NOTE_B5 24
#define NOTE_C6 25
#define NOTE_D6 27
#define NOTE_E6 29
#define NOTE_F6 30
#define NOTE_G6 32
#define NOTE_A6 34
#define NOTE_B6 36

// 音符时值倍数
#define NOTE_LEN_16 1
#define NOTE_LEN_8  2
#define NOTE_LEN_8D 3
#define NOTE_LEN_4  4
#define NOTE_LEN_4D 6
#define NOTE_LEN_2  8
#define NOTE_LEN_2D 12
#define NOTE_LEN_1  16

// 十六分音符时长，100bpm，4/4时为150ms
#define NOTE_LEN_BASE 150

//*****************************************************************************
//
// 类型声明
//
//*****************************************************************************
struct note_t {     // 音符类型
    uint8_t note;   // 音符下标，用于对应表中频率
    uint8_t len;    // 时长倍数，16分音符的整数倍
};

struct fnote_t {
    uint32_t freq;
    uint32_t count;
};

struct music_t {    // 乐曲类型
    uint16_t len;   // 音符数量
#ifdef CODE_TRANSMITTER
    struct note_t note[100];
#else
    struct note_t note[500];
#endif
};

//*****************************************************************************
//
// 函数原型声明
//
//*****************************************************************************
#ifdef CODE_TRANSMITTER
void InitMusic(void);   // 初始化乐谱数据（发射端）
#endif
uint8_t DetectNote(uint32_t freq); // 从频率查询音符

//*****************************************************************************
//
// 变量声明
//
//*****************************************************************************
// 音符数据，C大调C4-B6
extern const uint16_t note_frequency[37];
extern struct music_t music[1];
    
#endif
