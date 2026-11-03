#ifndef HEADER_H
#define HEADER_H
#include <Preferences.h>   
#include <my_lib.h>
#include "strukt.h"

extern step_prog *pvk_step;                                             //Выделение памяти под массив структур, ради 8 структур
extern bits *bit;

#define BUFSIZE  130                                                    // буфер передачи по порту

#define LEVEL_HOOP  GPIO_NUM_2                                          // датчик уровня рубашки
#define valve_telo  GPIO_NUM_4                                          // клапан тела
#define valve_gas   GPIO_NUM_5                                          // клапан блокировки газа
#define valve_holy  GPIO_NUM_19                                         // клапан охлаждения
#define LEVEL_SS    GPIO_NUM_23                                         // датчик уровня приёмной ёмкости
#define valve_golov GPIO_NUM_27                                         // клапан голов    
#define valve_hvost GPIO_NUM_32                                         // клапан хвостов   
#define AVTOMAT     GPIO_NUM_33                                         // автомат безопасности   

extern uint8_t sensorCount;
extern bool conversionPending;
extern unsigned long lastReadingTime;




extern MY_LIB sua;
extern char ssid_ust[32];                                               // читаем из памяти уставок
extern char password_ust[32];                                           // читаем из памяти уставок  
extern bool sensor_hoop;                                                // иапользовать датчик по уровню в рубашке
extern bool sensor_ss;                                                  // использовать датчик приёмной ёмкости
extern bool ss_level;                                                   // приёмная ёмкость полна
extern bool hoop_level;                                                 // уровень рубашки
extern bool time_pause_on;                                              // время паузы истекло(флаг)
extern bool ip_adr_on;                                                  // получен IP адрес

extern uint8_t power_r;                                                 // мощность тэна в единицах 
extern uint8_t t_telo_On;                                               // дистиляция время открытия клапана сброса тела
extern uint8_t t_telo_Off;                                              // дистиляция время закрытия клапана сброса тела
extern uint8_t t_dis_potOn;                                             // дистиляция время открытия клапана сброса голов 
extern uint8_t t_dis_potOff;                                            // дистиляция время закрытия клапана сброса голов   
extern uint8_t power_real;                                              // текущаа мощность тэна в единицах от 1 до 256
extern uint8_t KP_power;                                                // КП ПИД ТЭН
extern uint8_t KI_power;                                                // КИ ПИД ТЭН 
extern uint8_t n_step_m;                                                // изменился ли шаг
extern uint8_t ds_kol;                                                  // колличество датчиков при работе системы
extern uint8_t id_forms;                                                // номер открытой формы
extern uint8_t Ki_cool, Kp_cool, Kd_cool;                               // коэффициенты интэгральный, пропорциональный      
extern int8_t I_power;                                                  // интегралка воздействие
extern int16_t D_power;                                                 // дифференциалка воздействие
extern uint8_t zadanie_coll;                                            // задание Т. охлаждения блока управления, выход регулятора
extern uint8_t t_avto;                                                  // задание температуры в автомате 
extern uint8_t step;                                                    // номер шага программы
extern uint8_t u_as_ebu;                                                // аварийная уставка ЭБУ
extern uint8_t u_ts_ebu;                                                // сигнализация уставка ЭБУ
extern uint8_t u_as_hol;                                                // аварийная уставка Т.холодильника
extern uint8_t u_ts_hol;                                                // уставка сигнализации Т.холодильника  
extern uint8_t f_conv_r;                                                // ручное задание частотнику
extern uint8_t max_scale;                                               // максимальная шкала преобразователя тока
extern uint8_t meh_on;                                                  // время включения мешалки
extern uint8_t meh_off;                                                 // время выключения мешалки
extern uint8_t as;                                                      // 
extern uint8_t ts;                                                      // 
extern uint8_t k_mesh;                                                  // соответствие оборотов - токам
extern uint8_t mode_pvk;                                                // 0-затирание,1-потстил,2-дистилляция
extern uint8_t filtr_pres;                                              // коэффициент фильтра по давлению
extern uint8_t audio_as;                                                // номер файла музыки
extern uint8_t Tdata[]; 
extern uint8_t tenA;                                                    // Состояние ТЭНа 1
extern uint8_t tenB;                                                    // Состояние ТЭНа 2
extern uint8_t tenC;                                                    // Состояние ТЭНа 3
extern uint8_t wifi_db;                                                 // уровень wifi

extern uint16_t reverse_time;       
extern uint16_t time_pauses;                                            // таймер времени текущей паузы
extern uint16_t speed_mesh;                                             // скорость вентилятора в Гц(ответ преобразователя)
extern uint32_t data_bit;                                               // бинарник
extern uint32_t data_bit2;                                              // дополнительный бинарник
extern uint16_t ghart;                                                  // счётчик циклов записи графмков
extern uint8_t speed;                                                   // ЭБУ скорость вращения вентилятора 
extern uint8_t uta[4];                                                  // уставки температур в автомате
extern uint8_t num_dev_relay;                                           // адрес релейной платы расширения
extern uint8_t ds18b20_er_summ;                                         // все ошибки
extern uint8_t ds18b20_er;                                              // для оперативного контроля неисправности даласа

extern unsigned int zikl;                                               // время главного цикла 

extern float e_tenA;                                                    // ток ТЭНа Фазы А
extern float e_tenB;                                                    // ток ТЭНа Фазы В
extern float e_tenC;                                                    // ток ТЭНа Фазы С
extern float T[];                                                       // температуры T[0]-т.куб, T[1]-т.ЭБУ, T[2]-т.холодильника, T[3]-т.дефлегматора(связь с атмосферой)
extern float pres_m;                                                    // для фильтра по давлению
extern float U[];                                                       // U[0]-т.окончания процесса прямоток, U[1]-т.окончания процесса дистиляция, U[2]-(rezerv), U[3]-т.отбора тела дистиляция, U[4]-т.отбора тела прямоток, U[5]-т.отбора хвостов дистиляция
extern float I;                                                         // интэгралка ПИД ТЭН  
extern float gist_hol;                                                  // гистерезис охлаждения
extern float gist_nagrev;                                               // гистерезис нагрева
extern float subgist;                                                   // дополнительный гистерезис охлаждения

//******************************************************************************************************
extern String COM_read_BT(uint8_t w, uint16_t params, uint8_t web1, uint8_t web0);
extern void states();                                                   // функция состояния
extern void Serial_read_Nextion();
extern void TFT_refresh(uint8_t getDD);
extern void send_port();
extern void imitaziya();                                                // режим имитации
extern void WriteRegister(unsigned char DevID,uint16_t ADRESS,uint16_t val);
extern void MBSerial_read();                                            // чтение порта RS485
extern void ReadRegisters(unsigned char DevID,uint16_t START_ADRESS,uint16_t len); // запрос на выдачу регистров
extern void mix_oper(uint16_t zadanie);                                 // управление мешалкой, вкл-выкл и обороты
extern void ten_upravlenie(uint8_t ten_power);                          // управление ТЭНами
extern void relay(unsigned char DevID, uint16_t n, uint16_t on_off, unsigned char funz);// управление реле платы расширения
extern void chart_metka(uint8_t val);                                   // метка на графике
extern void add_vector();
#endif