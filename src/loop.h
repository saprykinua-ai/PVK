#ifndef LOOP_H
#define LOOP_H

MY_LIB sua(4);                                                          // Цифра 4 в этом контексте — это размер динамически выделяемых массивов температур:

#include <WiFi.h>
WiFiClient client;
//--------------------------------------------------------------------------

const unsigned long READ_INTERVAL = 2000; // Интервал между опросами (мс)
extern String ip_indy;                                                  // адрес компьютера с приложением

bool step_on = false;                                                   // шаг состоялся
bool ss_level = false;                                                  // приёмная ёмкость полна
bool hoop_level = false;                                                // уровень рубашки
bool sensor_hoop = false;                                               // использовать датчик уровеня рубашки
bool relay_flag;                                                        // было ли считывание состояния платы
bool time_pause_on;                                                     // время паузы истекло(флаг)
//bool nagrev = false;                                                    // режим нагрева, когда требуется нагрев

uint8_t sleep_d{};     
uint8_t t_n = 0;                                                        // счётчик достижения температуры шага
uint8_t on_cikl{};                                                      // первый цикл пропускаем
uint8_t kl_of{};                                                        // считаем время закрытия клапана
uint8_t kl_on{};                                                        // считаем время открытия клапана
uint8_t time_muzon = 20;                                                // что бы сразу был музон
uint8_t level_cc{};                                                     // счётчик-антидребезг поплавка приёмной ёмкости
uint8_t speed{};                                                        // ЭБУ скорость вращения вентилятора 
uint8_t ten_n{};                                                        // какой из 3 ТЭН опрашивать быстро
uint8_t power_real = 0;                                                 // текущаа мощность тэна в единицах 0-255
uint16_t tim_on = meh_on/4;                                             // что бы не запускался после перегрузки
uint16_t tim_off{};                                                     // 
uint16_t ghart{};                                                       // счётчик циклов записи графиков
uint8_t n_step_m = 25;                                                  // запомним режим нагрев-охлаждение
uint8_t n_step_mm = 0;                                                  // признак начала шагов
uint8_t n_event{};                                                      // счётчик обнуления событий
uint8_t mode_N_H{};                                                     // режим нагрев-охл. 0-нет нагрева и охл. 1-нагрев,2-охл.,3-недействительное значение
uint8_t t_avto{};                                                       // задание температуры в автомате
uint8_t tenA{};                                                         // Состояние ТЭНа 1
uint8_t tenB{};                                                         // Состояние ТЭНа 2
uint8_t tenC{};                                                         // Состояние ТЭНа 3
uint8_t count{};
uint8_t wifi_db{};                                                      // уровень wifi
extern uint16_t port_indy;                                              // порт типа 8080

float e_ten{};
float summ{};

float gist_hol{};                                                       // гистерезис охлаждения
float subgist{};                                                        // дополнительный гистерезис охлаждения

unsigned long millis_ten{};                                             // подсчёт времени цикла рабочего тэна
unsigned long current_millis{};                                         // главный цикл программы
unsigned long current_millis2{};                                        // подсчёт времени цикла тэна
unsigned long millis_valve{};                                           // тайминг клапанов

struct Heater {
    int pin;
    unsigned long offTime;
};
Heater heaters[3] = {{18, 0}, {12, 0}, {14, 0}};
const unsigned long WINDOW = 4000;                                      // Период ШИМ 3 сек(поставил 4 секунды для синхронизации измерения тока ТЭН)
unsigned long ten_power = 0;                                            // Таймер окна

uint8_t auto_pauses();                                                  // работа по паузам
uint8_t AS();                                                           // обработка аварийной сигнализации 
uint8_t TS();                                                           // обработка технологической сигнализации  
void klapan();                                                          // управление клапанами
void mixer(uint8_t data);                                               // управление мешалкой
void klapan_hol(uint8_t mode);                                          // управление клапаном охлаждения  
bool level_ss();                                                        // функция уровня приёмной ёмкости
bool level_hoop();                                                      // функция уровня рубашки
void WriteRegisterRelay(unsigned char DevID, unsigned char funz, uint16_t on_off, uint16_t number);
uint8_t mode_nagrev_ohlagdenie(float current_t, uint8_t zad_T);         // определение нагрев-охлаждение
extern uint8_t PID(uint8_t t_zadanie, uint8_t _mode);                   // главный регулятор
const uint8_t MAX_STEPS = 8;
static void _shutdown_procedure();
static void _move_to_next_step();
static uint8_t previous_step = 0;
uint8_t temperature_stable_counter = 0;
static void _start_pause_countdown(void);
static void _finish_current_step(bool is_operator_required);
const uint8_t STEP_CALL_PERIOD_SEC = 4;
bool _is_target_temperature_reached(uint8_t target_temp);
//----------------------------------------------------------------------*-------------------------------------------------
#endif