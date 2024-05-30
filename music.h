//*****************************************************************************
//
// Copyright: 2024, �Ϻ���ͨ��ѧ����ʵ����Ƽ�����II-47��
// File name: music.h
// Description: ���������������
// Author:	�Ϻ���ͨ��ѧ����ʵ����Ƽ�����II-47��
// Version: 1.0.0.20240520
// Date��2024-05-20
// History��
//
//*****************************************************************************

#ifndef __MUSIC_H__
#define __MUSIC_H__

#ifdef __cplusplus
extern "C" {
#endif

//*****************************************************************************
//
// ͷ�ļ�
//
//*****************************************************************************
#include <stdint.h>

//*****************************************************************************
//
// �궨��
//
//*****************************************************************************
// ������Ӧ��Ƶ�ʱ��±�
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

// ����ʱֵ����
#define NOTE_LEN_16 1
#define NOTE_LEN_8  2
#define NOTE_LEN_8D 3
#define NOTE_LEN_4  4
#define NOTE_LEN_4D 6
#define NOTE_LEN_2  8
#define NOTE_LEN_2D 12
#define NOTE_LEN_1  16

// ʮ��������ʱ����100bpm��4/4ʱΪ150ms
#define NOTE_LEN_BASE 150

//*****************************************************************************
//
// ��������
//
//*****************************************************************************
struct note_t {     // ��������
    uint8_t note;   // �����±꣬���ڶ�Ӧ����Ƶ��
    uint8_t len;    // ʱ��������16��������������
};

struct fnote_t {
    uint32_t freq;
    uint32_t count;
};

struct music_t {    // ��������
    uint16_t len;   // ��������
#ifdef CODE_TRANSMITTER
    struct note_t note[100];
#else
    struct note_t note[500];
#endif
};

//*****************************************************************************
//
// ����ԭ������
//
//*****************************************************************************
#ifdef CODE_TRANSMITTER
void InitMusic(void);   // ��ʼ���������ݣ�����ˣ�
#endif
uint8_t DetectNote(uint32_t freq); // ��Ƶ�ʲ�ѯ����

//*****************************************************************************
//
// ��������
//
//*****************************************************************************
// �������ݣ�C���C4-B6
extern const uint16_t note_frequency[37];
extern struct music_t music[1];
    
#endif
