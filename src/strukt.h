#ifndef STRUCT
#define STRUCT

struct step_prog
{
  uint8_t n_step{};                                                   // номер шага
  bool step_aktiv = false;                                            // шаг активен то есть не пропускаем
  bool oper = false;                                                  // нужен ли вызов оператора
  bool meh = false;                                                   // разрешение работы мешалки
  uint8_t t_zad{};                                                    // задание температуры шага
  uint8_t t_time{};                                                   // время шага
  //uint16_t time_meh_off;                                            // время простоя мешалки  
};

 struct bits
 {
  unsigned r_golova :1;                                               // открыть клапан голов в ручном
  unsigned r_telo   :1;                                               // открыть клапан тела в ручном
  unsigned r_hvost  :1;                                               // открыть клапан хвостов в ручном
  unsigned voice:1;                                                   // включен ли голос
  unsigned no_ten:1;                                                  // работа на газу без ТЭН
  unsigned as_ds1820:1;                                               // защита по неисправности даласов
  unsigned mikserOnR:1;                                               // включить мешалку в ручном режиме
  unsigned mixer_timer:1;                                             // мешалка в режиме таймера
  unsigned mixer_avto:1;                                              // мешалка в автомате
  unsigned sequent:1;                                                 // команда оператора продолжить шаг
  unsigned oper_pause:1;                                              // стоим на паузе оператора                               // признак смены формы
  unsigned mesh_on_off:1;                                             // вкл-выкл мешалка(ответ частотника)
  unsigned valve_avt:1;                                               // клапан охлаждения в автомате 
  unsigned mix_on:1;                                                  // прокрутка мешалки в автомате
  unsigned lisense:1;
  unsigned as_off:1;                                                  // действие защит(сброс нагрузки)
  unsigned shot_distil:1;                                             // дистиляция залповый сброс тела
  unsigned shot_golov:1;                                              // дистиляция залповый сброс голов
  unsigned send_port_off:1;                                           // для пропуска посылки в порт
  unsigned sleep_n:1;                                                 // спать дисплею
  unsigned bool_imitazia:1;
  unsigned as_ts_on:1;
  unsigned auto_prog:1;                                               // программа по паузам в автомате
  unsigned power_auto_t:1;                                            // режим работы автомата стабилизации мощности по температуре
  unsigned state:1;
  unsigned id_f:1;                                                    // признак смены формы

  unsigned relay1:1;                                                  // реле платы расширения
  unsigned relay2:1;                                                  // реле платы расширения
  unsigned relay3:1;                                                  // реле платы расширения
  unsigned relay4:1;                                                  // реле платы расширения
  unsigned auto_relay:1;                                              // автомат-ручное платы расширения  

         /*   unsigned a30:1;
            unsigned a31:1; */
 };
#endif