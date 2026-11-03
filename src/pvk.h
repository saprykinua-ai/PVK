
#ifndef PBK_H
#define PBK_H

#include <stdint.h>                                                     // для видения uint8_t

uint8_t as_time1{};                                                     // задержка отключения по защитам
uint8_t as_time2{};                                                     // задержка отключения по защитам TEN
uint8_t audio_as{};                                                     // номер файла музыки
uint8_t as_timeTS{};                                                    // задержка ТЭНам по сигнализации

float e_tenA{};                                                         // для фильтра тэна
float e_tenB{};                                                         // для фильтра тэна
float e_tenC{};                                                         // для фильтра тэна

void ten_upravlenie(uint8_t ten_power);                                 // управление ТЭНами
extern void read_Ust(); 
//----------------------------------------------------------------------*-------------------------------------------------
#endif