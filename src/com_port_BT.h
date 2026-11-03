#ifndef COM_PORT_BT_H
#define COM_PORT_BT_H

//----------------------------------------------------------------------*----------------------------------------------------------------
bool name = false;
bool water_valve = false;                                               // использование клапана подачи воды 
bool t_cooler = false;                                                  // использование датчика  охладителя
bool test = false;                                                      // для запуска таймеров некстиона
bool sensor_ss = false;                                                 // использовать датчик приёмной ёмкости
bool nos = false;                                                       // организация пропуска массива в порт
//extern bool mode;                                                       // режим нагрев-охлаждение true- охлаждение, false-нагрев
uint8_t Tdata[BUFSIZE];
uint8_t uta[] = {35,45,55,65};                                          // уставки температур в автомате
extern int16_t P_power;                                                 // пропорционалка воздействие
uint32_t data_bit = 0;                                                  // бинарник
uint32_t data_bit2 = 0;                                                 // дополнительный бинарник №2

void ust_default();                                                     // установка уставок по умолчанию
String COM_read_BT(uint8_t w, uint16_t params, uint8_t web1, uint8_t web0);
extern void mixer(uint8_t mode);                                        // управление мешалкой
void relay(unsigned char DevID, uint16_t n, uint16_t on_off, unsigned char funz);             // управление реле платы расширения

//----------------------------------------------------------------------*-------------------------------------------------
#endif