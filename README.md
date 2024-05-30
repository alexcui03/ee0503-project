# SJTU EE0503 Course Project

Course Project of SJTU EE0503 (Engineering Practice and Technological Innovation II) By Group 47 (2023-2024-2).

## Overview

**All code files are encoded in GB2312.**

Source file in the following list is provided by 上海交通大学电子工程系实验教学中心.
- ADC.c, ADC.h
- JLX12864G.c, JLX12864G.h
- LM75BD.c, LB75BD.h
- tm1638.c, tm1638.h

Other codes in the following list is written by EE0503(2023-2024-2) Group 47.
- main_receiver.c: main for receiver
- main_transmitter.c: main for transmitter
- common.c, common.h: common functions shared by receiver and transmitter
- music.c, music.h: marcos and utilities related to music
- ui.c, ui.h: definitions and functions for user interface in LCD

The system is depending on TI TM4C1294NCPDT's launchpad and its corresponding test board provided by SJTU. It has transmitter and receiver part. Transmitter is designed to send temperature or music through FM (88MHz~108MHz). Receiver is designed to receive these signals and show temperature or play music. Receiver also enables user to record the music receive from FM signal then play it afterwards.

## Usage

- Open proj.uvprojx with Keil uVision 5.
- Set debug options of each target to use Stellaris ICDI debugger.
- Compile target Receiver and Transmitter, then download code to flash memory with Keil.
- Connect pins and power, then press reset button on board to run.

## Pin Connection

|Component|MCU Pin|Target Pin|Comment|
|---|---|---|---|
|LCD|PB4|CS||
||PB5|SCK||
||PK0|RESET||
||PK1|RS||
||PK2|ROM_IN||
||PE4|SDA||
||PC4|ROM_CS||
||PC5|ROM_SCK||
||PC6|ROM_OUT||
||PE5|/||
|FM TX|PP0|MCU_RX|Transmitter|
||PP1|MCU_TX||
||PG1|AUDIO_R||
|FM RX|PP0|MCU_RX|Receiver|
||PP1|MCU_TX||
||RX_P4|AUDIO_R|RX Board|
|RX Board|PL4|RX_P5||
|TX Board|PB2|TX_P4|SCL|
||PB3|TX_P5|SDA|
||PE1|TX_P3||

## Encoding/Decoding Description

### Temperature

Transmitter gets temperature $t$ (celsius degree) from LM75BD chip and encoded it to a frequency $f$ (Hz) between 300~4000Hz, with following formula:

$$f = \frac{t\cdot(4000-300)}{40-0}+300$$

Receiver gets a frequency signal $f$ and decode it into $t$.

### Music

Music is encoded as an array of note that records the index to frequency and its time relative to semiquaver. For example, a note `note_t { note = NOTE_A4(10); len = NOTE_LEN_4(4) }` means its frequency is 10th element of frequency array which is 440Hz and has a 4 times of semiquaver length.

We agree that a music should be 100bpm and in C major, that is a semiquaver costs 150ms.

We play the music with given note array with buzzer by transmitter, and send the same signal with FM. Then receiver can play it.

When recording a music by receiver, it fits the frequency it got to a nearest frequency defined in note frequency array, and record the time interval. Then the receiver stores the frequency index and time relative to semiquaver to a note list.
