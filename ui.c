//*****************************************************************************
//
// Copyright: 2024, �Ϻ���ͨ��ѧ����ʵ����Ƽ�����II-47��
// File name: ui.c
// Description: LCD������ʾ�Ͱ���������غ���
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
#include "ui.h"
#include "JLX12864G.h"            // LCD��ʾ��

//*****************************************************************************
//
// ��������
//
//*****************************************************************************
struct act_t act[UI_ACT_COUNT];
struct key_state_t key_state[10];
uint8_t any_key_flag = 0;
uint8_t current_act = 0;
uint8_t last_act = 0;

//*****************************************************************************
//
// ����ԭ�ͣ�void InitAct(void)
// �������ܣ���ʼ��������Ԫ��
// ������������
// ��������ֵ����
//
//*****************************************************************************
void InitAct(void) {
    // 4, Menu
    act[UI_ACT_MENU].num = 3;
    
    act[UI_ACT_MENU].str[UI_ELE_MTEMP] = (unsigned char *)"�¶���ʾ";
    act[UI_ACT_MENU].x[UI_ELE_MTEMP] = 2;
    act[UI_ACT_MENU].y[UI_ELE_MTEMP] = 5;
    act[UI_ACT_MENU].inverse[UI_ELE_MTEMP] = 0;
    act[UI_ACT_MENU].str[UI_ELE_MMUSI] = (unsigned char *)"���ֲ���";
    act[UI_ACT_MENU].x[UI_ELE_MMUSI] = 4;
    act[UI_ACT_MENU].y[UI_ELE_MMUSI] = 5;
    act[UI_ACT_MENU].inverse[UI_ELE_MMUSI] = 0;
    act[UI_ACT_MENU].str[UI_ELE_MFREQ] = (unsigned char *)"��Ƶ����";
    act[UI_ACT_MENU].x[UI_ELE_MFREQ] = 6;
    act[UI_ACT_MENU].y[UI_ELE_MFREQ] = 5;
    act[UI_ACT_MENU].inverse[UI_ELE_MFREQ] = 0;
    
    // 5, Player
#ifdef CODE_TRANSMITTER
    act[UI_ACT_MUSIC].num = 3;
    act[UI_ACT_MUSIC].str[UI_ELE_RETURN] = (unsigned char *)"����";
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
    
    act[UI_ACT_MUSIC].str[UI_ELE_RETURN] = (unsigned char *)"����";
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
    act[UI_ACT_MUSIC].str[4] = (unsigned char *)"ģʽ:";
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
    act[UI_ACT_VOLTAGE_TEMP].str[0] = (unsigned char *)"��ѹ :";
    act[UI_ACT_VOLTAGE_TEMP].x[0] = 1;
    act[UI_ACT_VOLTAGE_TEMP].y[0] = 1;
    act[UI_ACT_VOLTAGE_TEMP].inverse[0] = 0;
    act[UI_ACT_VOLTAGE_TEMP].x[UI_ELE_VOLTAGE] = 1;
    act[UI_ACT_VOLTAGE_TEMP].y[UI_ELE_VOLTAGE] = 8;
    act[UI_ACT_VOLTAGE_TEMP].inverse[UI_ELE_VOLTAGE] = 0;
    act[UI_ACT_VOLTAGE_TEMP].str[2] = (unsigned char *)"�¶�1:";
    act[UI_ACT_VOLTAGE_TEMP].x[2] = 3;
    act[UI_ACT_VOLTAGE_TEMP].y[2] = 1;
    act[UI_ACT_VOLTAGE_TEMP].inverse[2] = 0;
    act[UI_ACT_VOLTAGE_TEMP].x[UI_ELE_TEMP1] = 3;
    act[UI_ACT_VOLTAGE_TEMP].y[UI_ELE_TEMP1] = 7;
    act[UI_ACT_VOLTAGE_TEMP].inverse[UI_ELE_TEMP1] = 0;
    act[UI_ACT_VOLTAGE_TEMP].str[4] = (unsigned char *)"�¶�2:";
    act[UI_ACT_VOLTAGE_TEMP].x[4] = 5;
    act[UI_ACT_VOLTAGE_TEMP].y[4] = 1;
    act[UI_ACT_VOLTAGE_TEMP].inverse[4] = 0;
    act[UI_ACT_VOLTAGE_TEMP].x[UI_ELE_TEMP2] = 5;
    act[UI_ACT_VOLTAGE_TEMP].y[UI_ELE_TEMP2] = 7;
    act[UI_ACT_VOLTAGE_TEMP].inverse[UI_ELE_TEMP2] = 0;
    act[UI_ACT_VOLTAGE_TEMP].str[UI_ELE_VFREQ] = (unsigned char *)"��Ƶ����";
    act[UI_ACT_VOLTAGE_TEMP].x[UI_ELE_VFREQ] = 7;
    act[UI_ACT_VOLTAGE_TEMP].y[UI_ELE_VFREQ] = 1;
    act[UI_ACT_VOLTAGE_TEMP].inverse[UI_ELE_VFREQ] = 0;
    act[UI_ACT_VOLTAGE_TEMP].str[UI_ELE_VRET] = (unsigned char *)"����";
    act[UI_ACT_VOLTAGE_TEMP].x[UI_ELE_VRET] = 7;
    act[UI_ACT_VOLTAGE_TEMP].y[UI_ELE_VRET] = 13;
    act[UI_ACT_VOLTAGE_TEMP].inverse[UI_ELE_VRET] = 0;
#else
    act[UI_ACT_VOLTAGE_TEMP].num = 4;
    act[UI_ACT_VOLTAGE_TEMP].str[0] = (unsigned char *)"�¶�:";
    act[UI_ACT_VOLTAGE_TEMP].x[0] = 3;
    act[UI_ACT_VOLTAGE_TEMP].y[0] = 1;
    act[UI_ACT_VOLTAGE_TEMP].inverse[0] = 0;
    act[UI_ACT_VOLTAGE_TEMP].x[UI_ELE_TEMP] = 3;
    act[UI_ACT_VOLTAGE_TEMP].y[UI_ELE_TEMP] = 6;
    act[UI_ACT_VOLTAGE_TEMP].inverse[UI_ELE_TEMP] = 0;
    act[UI_ACT_VOLTAGE_TEMP].str[UI_ELE_VFREQ] = (unsigned char *)"��Ƶ����";
    act[UI_ACT_VOLTAGE_TEMP].x[UI_ELE_VFREQ] = 7;
    act[UI_ACT_VOLTAGE_TEMP].y[UI_ELE_VFREQ] = 1;
    act[UI_ACT_VOLTAGE_TEMP].inverse[UI_ELE_VFREQ] = 0;
    act[UI_ACT_VOLTAGE_TEMP].str[UI_ELE_VRET] = (unsigned char *)"����";
    act[UI_ACT_VOLTAGE_TEMP].x[UI_ELE_VRET] = 7;
    act[UI_ACT_VOLTAGE_TEMP].y[UI_ELE_VRET] = 13;
    act[UI_ACT_VOLTAGE_TEMP].inverse[UI_ELE_VRET] = 0;
#endif
    
    // 1, Frequency
    act[UI_ACT_FREQUENCY].num = 8;
    
    act[UI_ACT_FREQUENCY].str[0] = (unsigned char *)"��Ƶ:";
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
    act[UI_ACT_FREQUENCY].str[UI_ELE_CONFIRM] = (unsigned char *)"ȷ��";
    act[UI_ACT_FREQUENCY].x[UI_ELE_CONFIRM] = 7;
    act[UI_ACT_FREQUENCY].y[UI_ELE_CONFIRM] = 13;
    act[UI_ACT_FREQUENCY].inverse[UI_ELE_CONFIRM] = 0;
    
    // 2, Error
    act[UI_ACT_ERROR].num = 1;
    
    act[UI_ACT_ERROR].str[0] = (unsigned char *)"��Ƶ����Χ!";
    act[UI_ACT_ERROR].x[0] = 3;
    act[UI_ACT_ERROR].y[0] = 1;
    act[UI_ACT_ERROR].inverse[0] = 0;
}

//*****************************************************************************
//
// ����ԭ�ͣ�void DisplayUiAct(int index)
// �������ܣ������Ļ����ʾ�±�Ϊindex��ACT�����޸�current_act����
// ����������index - ACT���±�
// ��������ֵ����
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
// ����ԭ�ͣ�void InitKeyStates(void)
// �������ܣ���ʼ������״̬
// ������������
// ��������ֵ����
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
// ����ԭ�ͣ�void DetectKey(uint8_t key_code)
// �������ܣ����ݵ�ǰ���°���������״̬
// ����������key_code - ��ǰ���µİ������
// ��������ֵ����
//
//*****************************************************************************
void DetectKey(uint8_t key_code) {
    int i;
    
    for (i = 1; i < 10; ++i) {
        key_state[i].prestate = key_state[i].state;
        
        /*
        // �޳������
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
        
        // ���������
        if (key_code == i) {
            if (key_state[i].prestate == 0) {
                any_key_flag = 1; // ����ʱ������any_key_flag
                key_state[i].state = 1;
                key_state[i].flag = 1;
                key_state[i].timer = KEYTMR_OF * 2; // �״γ�����Ҫ������ʱ����ֹ��
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
// ����ԭ�ͣ�void ClearKeyFlags(void)
// �������ܣ��������״̬
// ������������
// ��������ֵ����
//
//*****************************************************************************
void ClearKeyFlags(void) {
    int i;
    
    any_key_flag = 0;
    for (i = 1; i < 10; ++i) {
        key_state[i].flag = 0;
    }
}
