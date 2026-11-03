#ifndef SETUP_H
#define SETUP_H

#include <Preferences.h>  
#include "SPIFFS.h"

Preferences prefs;                                                      // Создаём объект prefs
//----------------------------------------------------------------------*-------------------------------------------------
typedef union tmpp_64                                                   // для имитации температур
 {
  int64_t tmpp_64;
  unsigned char arrr[8];                                                // 8
 }UN_FL64; UN_FL64 bitt8;
//----------------------------------------------------------------------*-------------------------------------------------

String qwerty;
String ip_indy;                                                         // адрес компьютера с приложением

bool ip_adr_on;                                                         // получен IP адрес

uint16_t port_indy;                                                     // порт типа 8080
uint8_t sensorCount = 0;
bool conversionPending = false;
unsigned long lastReadingTime = 0; 

uint64_t chipid     ;
uint8_t wr_data[8];
char ssid_my[32];                                                       // = "AndroidAP1ACA";
char password_my[32];  
char ssid_ust[32];                                                      // читаем из памяти уставок
char password_ust[32];                                                  // читаем из памяти уставок

uint8_t t_telo_On = 0;                                                  // дистиляция время открытия клапана сброса тела
uint8_t t_telo_Off = 0;                                                 // дистиляция время закрытия клапана сброса тела
uint8_t t_dis_potOn = 0;                                                // дистиляция время открытия клапана сброса голов 
uint8_t t_dis_potOff = 0;                                               // дистиляция время закрытия клапана сброса голов   
uint8_t KP_power = 0;                                                   // КП ПИД ТЭН
uint8_t KI_power = 0;                                                   // КИ ПИД ТЭН 
int8_t I_power = 0;                                                     // интегралка воздействие
int16_t D_power = 0;                                                    // дифференциалка воздействие
uint8_t zadanie_coll = 0;                                               // задание Т. охлаждения блока управления, выход регулятора
uint8_t id_forms = 0;                                                   // номер открытой формы
uint8_t ds_kol = 0;                                                     // колличество датчиков при работе системы
uint8_t Ki_cool = 0, Kp_cool = 0, Kd_cool = 0;                          // коэффициенты интэгральный, пропорциональный 
uint8_t step = 0;                                                       // номер шага программы
uint8_t u_as_ebu = 0;                                                   // аварийная уставка ЭБУ
uint8_t u_ts_ebu = 0;                                                   // сигнализация уставка ЭБУ
uint8_t u_as_hol = 0;                                                   // аварийная уставка Т.холодильника
uint8_t u_ts_hol = 0;                                                   // уставка сигнализации Т.холодильника  
uint8_t f_conv_r = 0;                                                   // ручное задание частотнику
uint8_t max_scale = 0;                                                  // максимальная шкала преобразователя тока
uint8_t loadF1 = 0;                                                     // ток частотника
uint8_t loadF2 = 0;                                                     // ток частотника
uint8_t temper = 0;                                                     // температура радиатора частотника
uint8_t meh_on = 0;                                                     // время включения мешалки
uint8_t meh_off = 0;                                                    // время выключения мешалки
uint8_t as = 0;                                                         // 
uint8_t ts = 0;                                                         // 
uint8_t k_mesh = 0;                                                     // соответствие оборотов - токам
uint8_t hour_val = 0;                                                   // часовой пояс
uint8_t mode_pvk = 0;                                                   // 0-затирание,1-потстил,2-дистилляция
uint8_t filtr_pres = 0;                                                 // коэффициент фильтра по давлению
//uint8_t ds18b20_er = 0;                                                 // в каком датчике ds18b20 ошибка
uint8_t power_r = 0;                                                    // мощность тэна в % (ручной режим) 
uint8_t num_dev_relay = 0;                                              // адрес релейной платы расширения
uint8_t ds18b20_er_summ = 0;                                            // все ошибки
uint8_t ds18b20_er = 0;                                                 // для оперативного контроля неисправности даласа

uint16_t reverse_time = 0;                                              // оставшееся время паузы
uint16_t time_pauses = 0;                                               // таймер времени текущей паузы
uint16_t speed_mesh = 0;                                                // скорость вентилятора в Гц(ответ преобразователя)
//uint16_t ds18b20_er_summ = 0;                                           // все ошибки
unsigned int zikl = 0;                                                  // время главного цикла

float T[4];                                                             // температуры T[0]-т.куб, T[1]-т.ЭБУ, T[2]-т.холодильника, T[3]-т.дефлегматора(связь с атмосферой)
float U[]={97.0,98.0,86.0,87.0,92.0,93.0};                              // U[0]-т.окончания процесса прямоток, U[1]-т.окончания процесса дистиляция, U[2]-(rezerv), U[3]-т.отбора тела дистиляция, U[4]-т.отбора тела прямоток, U[5]-т.отбора хвостов дистиляция
float I;                                                              // интэгралка ПИД ТЭН  

//**********************************************************************************************
void write_mem_struct(uint8_t n);                                       // запись структуры в память
void read_mem_struct(uint8_t n);                                        // запись структуры в память
void read_Ust();
void ust_default();                                                     // установка уставок по умолчанию
void Serial_read_Nextion();
void TFT_refresh(uint8_t getDD);
void setup();
void send_port();
void imitaziya();                                                       // режим имитации
void MBSerial_read();                                                   // чтение порта RS485
uint16_t myCRC(byte message[], int length);
void WriteRegister(unsigned char DevID,uint16_t ADRESS,uint16_t val);   // запись в регистр (управление частотником)
void ReadRegisters(unsigned char DevID,uint16_t START_ADRESS,uint16_t len); // запрос на выдачу регистров
void mix_oper(uint16_t zadanie);                                        // управление мешалкой, вкл-выкл и обороты
String web_button(u_char web0, u_char web1, u_char web2);
#endif
