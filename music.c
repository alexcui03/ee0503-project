//*****************************************************************************
//
// Copyright: 2024, �Ϻ���ͨ��ѧ����ʵ����Ƽ�����II-47��
// File name: music.c
// Description: ���������������
// Author:	�Ϻ���ͨ��ѧ����ʵ����Ƽ�����II-47��
// Version: 1.0.0.20240520
// Date��2024-05-20
// History��
//
//*****************************************************************************

//*****************************************************************************
//
// ͷ�ļ�
//
//*****************************************************************************
#include "music.h"

//*****************************************************************************
//
// �궨��
//
//*****************************************************************************
// �����꣬���ڶ�������
#define M0_NOTE(n, l) music[0].note[i].note = (n); music[0].note[i].len = (l); ++i

//*****************************************************************************
//
// ��������
//
//*****************************************************************************
// �������ݣ�C���C4-B6
const uint16_t note_frequency[37] = {
    0, // 0 λ��ʾ��ֹ��
    262, 277, 294, 311, 330, 349, 370, 392, 415, 440, 466, 494,
    523, 554, 587, 622, 659, 698, 740, 784, 831, 880, 932, 988,
    1046, 1109, 1175, 1245, 1318, 1397, 1480, 1568, 1661, 1760, 1865, 1976
};

struct music_t music[1];

#ifdef CODE_TRANSMITTER
//*****************************************************************************
// 
// ����ԭ�ͣ�void InitMusic(void)
// �������ܣ���ʼ���������ݣ������ڷ����
// ������������
// ��������ֵ����
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
// ����ԭ�ͣ�uint8_t DetectNote(uint32_t freq)
// �������ܣ���ѯƵ����ӽ�������
// ����������freq - Ƶ��
// ��������ֵ�������±�
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
