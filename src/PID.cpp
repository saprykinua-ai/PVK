#include "header.h"
#include "PID.h"

//--------------------------------------------------------------------*-------------------------------------------------
uint8_t PID(uint8_t t_zadanie, uint8_t _mode)                         // регулятор мощности ПВК
 {
  float t_zadanie_f = t_zadanie;

  if(!bit->state || bit->no_ten || bit->as_off){return 0;};           // в ступоре, работа на газу, сброс нагрузки мощность ТЭН в 0, запрет работе ТЭН
  if(!bit->power_auto_t){return (power_r*2.55 +1);}                   // в ручном режиме ручное задание(+1 занижает преобразование)
  if(!gpio_get_level(valve_holy)) return 0;                           // при включенном клапане охлаждения (инверсия клапана)
 
  float P;
  float delta;
  uint8_t pid_val;
    
  //if(!mode_regim && (t_zadanie_f - T[0]) > 2){mode_regim = true;};    // если не нагрев и задание больше температуры то НАГРЕВ     (тригер)
  //if( mode_regim && (T[0] >= t_zadanie_f))   {mode_regim = false;};   // если нагрев и температура больше задания    то ОХЛАЖДЕНИЕ (тригер)
  //if(mode_regim) t_zadanie_f -= gist_nagrev;                          // при нагреве уменьшаем задание на гистерезис

  if(_mode == 1) t_zadanie_f -= gist_nagrev;                          // при нагреве уменьшаем задание на гистерезис  

  delta = t_zadanie_f - T[0];                                         // рассогласование по температуре

  P = KP_power * delta;                                               // пропорциональная составляющая
  I = I + KI_power * delta/5;                                         // интегральная составляющая    

  if(T[0] >= t_zadanie_f) {I = 0;}                                    // сброс интегралки
  else if(I > 150){ I = 150;}                                         // ограничение интегралки  
  else if(I < 0)  { I = 0;};                                          // интегралка всегда больше 0 

  if((P + I) > 255){pid_val = 255;}
  else if((P + I) < 0){pid_val = 0;}                                  // выход ШИМ ТЭН
  else pid_val = uint8_t(P + I);

  P_power = int16_t(P);                                               // для скады
  I_power = int8_t(I);                                                // для скады      

  return pid_val;
}
//--------------------------------------------------------------------*-------------------------------------------------