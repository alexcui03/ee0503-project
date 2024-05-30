//*****************************************************************************
//
// Copyright: 2024, �Ϻ���ͨ��ѧ����ʵ����Ƽ�����II-47��
// File name: ui.h
// Description: LCD������ʾ�Ͱ���������غ���
// Author:	�Ϻ���ͨ��ѧ����ʵ����Ƽ�����II-47��
// Version: 1.0.0.20240506
// Date��2024-05-06
// History��
//
//*****************************************************************************

#ifndef __UI_H__
#define __UI_H__

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
// ������ض���
#define UI_ACT_COUNT        5       // ACT����
#define UI_ELEMENT_COUNT    20      // ÿ��ACT���������

#define UI_ACT_MENU         3       // ACT 3�����˵�
#define UI_ELE_MTEMP        0       // �¶���ʾ����
#define UI_ELE_MMUSI        1       // ���ֲ��Ź���
#define UI_ELE_MFREQ        2       // �����ز�Ƶ��

#define UI_ACT_MUSIC        4       // ACT 4�����Ž���
#define UI_ELE_RETURN       0       // ���ذ�ť
#define UI_ELE_MUTE         1       // ����/��������������������
#define UI_ELE_MMODE        1       // ����ģʽ/����¼�ƣ����ջ���
#define UI_ELE_PLAYPAUSE    2       // ����/��ͣ��ť�����ջ���
#define UI_ELE_RECORD       3       // ¼��/���水ť�����ջ���
#define UI_ELE_RTIME        5       // ¼��ʱ�䣨���ջ���

#define UI_ACT_VOLTAGE_TEMP 0       // ACT 0����ѹ�¶���ʾҳ
#define UI_ELE_VOLTAGE      1       // ��ѹ���������
#define UI_ELE_TEMP         1       // �¶ȣ����ջ���
#define UI_ELE_TEMP1        3       // �¶�1���������
#define UI_ELE_TEMP2        5       // �¶�2���������
#ifdef CODE_TRANSMITTER
#define UI_ELE_VFREQ        6       // �ز�Ƶ��
#define UI_ELE_VRET         7       // ����
#else
#define UI_ELE_VFREQ        2       // �ز�Ƶ��
#define UI_ELE_VRET         3       // ����
#endif

#define UI_ACT_FREQUENCY    1       // ACT 1���ز�Ƶ��ҳ
#define UI_ELE_DIGIT1       1       // ���ְ�λ
#define UI_ELE_DIGIT2       2       // ����ʮλ
#define UI_ELE_DIGIT3       3       // ���ָ�λ
#define UI_ELE_DIGIT4       5       // ����ʮ��λ
#define UI_ELE_CONFIRM      7       // ȷ�ϰ���

#define UI_ACT_ERROR        2       // ACT 2������ҳ

// ������ض���
#define KEYTMR_OF           10      // �������
#define KEY_ENTER           5
#define KEY_UP              4
#define KEY_DOWN            6
#define KEY_INC             2
#define KEY_DEC             8

// ��������
#define CHAR_CELSIUS        41446   // �ַ����GB2312���룬ʹ��ʱ�������ֽڷ�ֹWARNING

//*****************************************************************************
//
// ��������
//
//*****************************************************************************
struct act_t {                          // ACT���涨��
    uint8_t num;                        // ��ǰACT���������
    uint8_t *str[UI_ELEMENT_COUNT];     // �������ʾ���ַ���
    uint8_t x[UI_ELEMENT_COUNT];        // ��ʼ��ʾ��
    uint8_t y[UI_ELEMENT_COUNT];        // ��ʼ��ʾ��
    uint8_t inverse[UI_ELEMENT_COUNT];  // �Ƿ���
    uint8_t hide[UI_ELEMENT_COUNT];     // �Ƿ�����
};

struct key_state_t {
    uint8_t state;
    uint8_t prestate;
    uint8_t timer;
    uint8_t flag;                       // �������±�־
};

//*****************************************************************************
//
// ����ԭ������
//
//*****************************************************************************
void InitAct(void);                     // ��ʼ����ACT����
void DisplayUiAct(int index);           // ��ʾ��index��ACT

void InitKeyStates(void);               // ��ʼ������״̬
void DetectKey(uint8_t key_code);       // ��ⰴ��
void ClearKeyFlags(void);               // �������״̬

//*****************************************************************************
//
// ��������
//
//*****************************************************************************
extern struct act_t act[UI_ACT_COUNT];
extern struct key_state_t key_state[10];
extern uint8_t any_key_flag;
extern uint8_t current_act;             // ��ǰACT���
extern uint8_t last_act;                // ��ǰ��ACT�����ڷ���

#ifdef __cplusplus
}
#endif

#endif
