//-------------------------------------------------------------------*-------------------------------------------------
#include "pvk.h"
#include "header.h"
#include "strukt.h"
//--------------------------------------------------------------------*-------------------------------------------------
void imitaziya()
 {
  static float Tmem[] ={static_cast<float>(t_avto)};                  // температуры T[0]-т.куб, T[1]-т.ЭБУ, T[2]-т.холодильника, T[3]-т.дефлегматора(связь с атмосферой)
  static bool regim = false;                                          // true нагрев, false охлаждение

  if(Tmem[0] < (t_avto - gist_nagrev - 0.5f) || regim)   {Tmem[0] += 0.1f; regim = true;}     //  нагрев
  if(Tmem[0] > (t_avto + gist_hol + 0.5f )   || !regim)  {Tmem[0] -= 0.1f; regim = false;}         // охлаждение
  T[0] = Tmem[0];
   
  Serial.println("T[0] - " + String(T[0]));
  //if(Tmem[0] <= 86.0) {T[0] = Tmem[0] + 0.5;}                         // бак
  //else if(Tmem[0] > 86.0){T[0] = Tmem[0] + 0.1;};                                             //
  //if(Tmem[0] > 86.7)T[0] = 86.1;
  //Tmem[0] = T[0];                                                     // T[0]-т.бака

  if      ((power_real > 1)&& (power_real <= 85)) {e_tenA = 6.5;e_tenB = 0;e_tenC = 0;}
  else if ((power_real > 85)&&(power_real <= 170)){e_tenA = 6.5;e_tenB = 6.4;e_tenC = 0;}                                                      // ток рабочего ТЭНа 
  else if (power_real > 170){e_tenA = 6.5;e_tenB = 6.4;e_tenC = 6.3;};     

 // T[2] = 71;                                                        // температура дефлегматора
 }
//--------------------------------------------------------------------*-------------------------------------------------------------------
uint8_t AS()                                                          //
 {
  /*    AS|=1<<0;                                                     // нулевой бит делаем единицей
  0 - ток ТЭНа A аварийный
  1 - ток ТЭНа B аварийный 
  2 - ток ТЭНа С аварийный
  3 - Т.холодильника аварийная 
  4 - температура ЭБУ аварийная
  5 - неисправность даласов
  6 - уровень в приёмной ёмкости
  6 - уровень в рубашке */  

  uint8_t AS = 0;                                                     // слово состояние аварийной сигнализации
  static uint8_t as_time1 = 0;                                        // задержка отключения по защитам
  static uint8_t as_time2 = 0;            

  if((tenA == 0) && (e_tenA > 1)) { AS|=1<<1; audio_as = 4;};         // 2 ток ТЭНа B аварийный
  if((tenB == 0) && (e_tenB > 1)) { AS|=1<<1; audio_as = 5;};         // 2 ток ТЭНа B аварийный 
  if((tenC == 0) && (e_tenC > 1)) { AS|=1<<2; audio_as = 6;};         // 4 ток ТЭНа С аварийный 
  if(T[2] > u_as_hol)             { AS|=1<<3; audio_as = 7;};         // 8 Т.холодильника аварийная
  if(T[1] > u_as_ebu)             { AS|=1<<4; audio_as = 3;};         // 16 температура ЭБУ аварийная 
  if(bit->as_ds1820)              { AS|=1<<5; /*audio_as = 15;*/};    // 32 неисправность даласов
  if(ss_level)                    { AS|=1<<6; audio_as = 2;};         // 64 уровень приёмной ёмкости
  if(hoop_level)                  { AS|=1<<7; audio_as = 1;};         // 128 уровень в рубашке(В парогенераторе звук)

  if(!bit->voice) audio_as = 0;                                       // если нет голоса то пищим

  if(bit->as_off || !bit->no_ten){gpio_set_level(valve_gas, 0);}      // при сбросе нагрузки  и работе на ТЭНах - отключаем газ// импульсом отключаем отсечной клапан газа(клапан в инверсии)
  else if(T[0] >= t_avto){gpio_set_level(valve_gas, 0);}              // (не зависимо от активности защит)Т.бака достигла уставки нагрева в автомате// импульсом отключаем отсечной клапан газа(клапан в инверсии)
  else{if(bit->state && bit->no_ten){gpio_set_level(valve_gas, 1);};};// в работе аппарат и работа на газу

  if(bit->as_ts_on){                                                  // активны защиты и сигнализация
    if((AS&1<<3)||(AS&1<<4)||(AS&1<<5)||(AS&1<<6)||(AS&1<<7)){        // при срабатывании AS кроме ТЭН
        as_time1++;                                                   // считаем задержку на сброс нагрузки(от ошибочного срабатывания)     
        if(as_time1 > 1){bit->as_off = true;};                        // сброс нагрузки
    }
    else{ as_time1 = 0; bit->as_off = false;};                        // при восстановлении параметров

    if((AS&1<<0)||(AS&1<<1)||(AS&1<<2)){                              // при срабатывании AS ТЭН(срабатывание через зброс нагрузки)
        as_time2++;                                                   // считаем задержку на отключение            
        if(as_time2 > 2){          
            if      (AS&1<<0){audio_as = 4;}  
            else if (AS&1<<1){audio_as = 5;}
            else if (AS&1<<2){audio_as = 6;}            
        }
        if(as_time2 >= 4){gpio_set_level(AVTOMAT, 1); as_time2 = 4;}; // выключаем автомат безопасности         
    }
    else{as_time2 = 0;    gpio_set_level(AVTOMAT, 0);};               // при восстановлении параметров    
  }
  else{                                                               // если защиты не активны
    bit->as_off = false;                                              // то нет сброса нагрузки
    gpio_set_level(AVTOMAT, 0);                                       // сброс автомата если защиты не активны    
  };
  //Serial.println("AS - " + String(as));
 return AS;
 }
//--------------------------------------------------------------------*------------------------------------------------- 
uint8_t TS()                                                          //
 {
  /*      TS|=1<<1;                                                   // первый бит ставим в 1
  0 - ток ТЭНа фA не соответствует
  1 - ток ТЭНа фБ не соответствует
  2 - ток ТЭНа фС не соответствует  
  3 - температура ЭБУ велика
  4 - Т.холодильника высока 
  5 - 
  6 - 
  7 - вызов оператора   */

  uint8_t TS = 0;                                                     // слово состояние технологической сигнализации      

  if(bit->as_ts_on){                                                  // активны защиты и сигнализация 
      if(!bit->no_ten){                                               // без ТЭН сигнализация не нужна
          if((power_real > 50)  && (e_tenA < 1)) {TS|=1<<0; audio_as = 9;};  // 1 неисправность ТЭНа audio_as = 9;
          if((power_real > 135) && (e_tenB < 1)) {TS|=1<<1; audio_as = 10;}; // 2 неисправность ТЭНаaudio_as = 10;
          if((power_real > 220) && (e_tenC < 1)) {TS|=1<<2; audio_as = 11;}; // 4 неисправность ТЭНаaudio_as = 11;
      }
      if(T[1] > u_ts_ebu)                        {TS|=1<<3; audio_as = 12;};// температура ЭБУ высока
      if(T[2] > u_ts_hol)                        {TS|=1<<4; audio_as = 14;};// Т.холодильника высока
      if(bit->oper_pause)                        {TS|=1<<7; audio_as = 15;};// вызов оператора      
  };

  if(!bit->voice) audio_as = 0;                                       // если нет голоса то пищим 
  return TS;  
 } 
//--------------------------------------------------------------------*-------------------------------------------------
void ust_default()                                                    // данные установки прописываем при первой загрузке нового пустого контроллера
  {
  Serial.println("Пишем уставки по умолчанию.");

  prefs.putBool("auto_prog", 0);                                      // программа по паузам в автомате
  prefs.putBool("state", 0);                                          // статус (1 в работе)
  prefs.putBool("mixer_timer",0);                                     // ручное-таймер мешалки 
  prefs.putBool("mixer_avto",0);                                      // мешалка в автомате
  prefs.putBool("power_auto_t",0);                                    // автомат-дистанция ТЭН по температуре
  //prefs.putBool("power_auto_p",0);                                    // автомат-дистанция ТЭН по давлению  
  prefs.putBool("sleep_n",1);                                         // засыпать дисплею
  prefs.putBool("t_coolerr", 0);                                      // использование датчика  охладителя
  prefs.putBool("as_ts_on", 1);                                       // использование защит и сигнализации
  prefs.putBool("valve_avt", 0);                                      // клапан в автомате   
  prefs.putBool("golova_on", 0);                                      // отобраны ли головы
  //prefs.putBool("shot_golov", 0);                                   // прямоток залповый сброс голов
  prefs.putBool("shot_distil", 0);                                    // дистиляция залповый сброс голов
  prefs.putBool("shot_golov", 0);                                     // дистиляция залповый сброс тела
  prefs.putBool("sensor_ss", 0);                                      // использовать датчик приёмной ёмкости
  prefs.putBool("no_ten", 0);                                         // работа на газу без ТЭН  
  prefs.putBool("sensor_hoop",1);                                     // использовать датчик уровня в рубашке 
  prefs.putBool("voice",1);                                           // включен ли голос

  prefs.putUInt("power_r", 70);                                       // мощность ручного режима
  prefs.putUInt("zadanie_coll", 35.0);                                // SSR
  prefs.putUInt("KP_power", 5);                                       // КП ПИД ТЭН
  prefs.putUInt("KI_power", 5);                                       // КП ПИД ТЭН  
  //prefs.putUInt("KD_power", 5);                                       // КД ПИД ТЭН 
  prefs.putUInt("meh_on",10);                                         // мешалка включена
  prefs.putUInt("meh_off",10);                                        // мешалка выключена  
  prefs.putUInt("t_avto", 65);                                        // задание температуры в автомате
  prefs.putUInt("u_as_ebu",55);                                       // уставка аварийная Т.ЭБУ
  prefs.putUInt("u_ts_ebu",45);                                       // сигнализация Т.ЭБУ 
  prefs.putUInt("u_as_hol",70);                                       // аварийная уставка Т.холодильника
  prefs.putUInt("u_ts_hol", 60);                                      // уставка сигнализации Т.холодильника   
  prefs.putUInt("f_conv_r", 0);                                       // ручное задание частотнику  
  prefs.putUInt("max_scale", 10);                                     // максимальная шкала преобразователя тока
  prefs.putUInt("k_mesh"  , 100);                                     // максимальные обороты в % 
  prefs.putUInt("hour_val", 3);                                       // часовой пояс   
  prefs.putUInt("mode_pvk",0);                                        // 0-затирание,1-потстил,2-дистилляция  
  prefs.putUInt("Kp_cool", 5);                                        // SSR
  prefs.putUInt("Ki_cool", 6);                                        // SSR   
  prefs.putUInt("Kd_cool", 10);                                        // SSR     
  prefs.putUInt("t_dis_potOn",5);                                     // дистиляция время закрытия клапана сброса голов 
  prefs.putUInt("t_dis_potOff",1);                                    // дистиляция время открытия клапана сброса голов   
  prefs.putUInt("t_telo_On",4);                                       // дистиляция время открытия клапана сброса тела
  prefs.putUInt("t_telo_Off",4);                                      // дистиляция время закрытия клапана сброса тела  
  prefs.putUInt("filtr_pres",1);                                      // коэф. фильтра по давлению     
  prefs.putUInt("uta[0]",35);                                         // фиксированные уставки температур в автомате  
  prefs.putUInt("uta[1]",35);                                         // фиксированные уставки температур в автомате 
  prefs.putUInt("uta[2]",35);                                         // фиксированные уставки температур в автомате 
  prefs.putUInt("uta[3]",35);                                         // фиксированные уставки температур в автомате       

  prefs.putString("ssid","ssid");
  prefs.putString("password","password"); 

  prefs.putUInt("step",0);                                            // номер шага пауз

  prefs.putFloat("gist_hol", 1);                                      // гистерезис охлаждения 
  prefs.putFloat("gist_nagrev", 1);                                   // гистерезис нагрева  
  prefs.putFloat("subgist", 0.5);                                     // дополнительный гистерезис охлаждения  

  prefs.putFloat("U[0]", 99);                                         // 
  prefs.putFloat("U[1]", 99);                                         // 
  prefs.putFloat("U[2]", 90);                                         // (rezerv)т.отбора тела прямоток
  prefs.putFloat("U[3]", 90);                                         // 
  prefs.putFloat("U[4]", 91);                                         // 
  prefs.putFloat("U[5]", 91);                                         //  
  
  read_Ust();                                                         // считываем из памяти уставки
}//
//----------------------------------------------------------------------*-------------------------------------------------
// Вспомогательная функция (добавь её в класс или перед setup)
float getFloatQuietly(const char* key, float defaultValue) {
    if (!prefs.isKey(key)) {
        prefs.putFloat(key, defaultValue); // Создаем ключ, чтобы getFloat не ругался
    }
    return prefs.getFloat(key, defaultValue);
}
//--------------------------------------------------------------------*-------------------------------------------------
void read_Ust()                                                       // считываем из памяти уставки
  {
  Serial.println("Читаем уставки");

  //strcpy(ssid_my, prefs.getString("ssid_my","ssid_my").c_str());              // имя сети WIFI "AndroidAP1ACA";
  //strcpy(password_my, prefs.getString("password_my","password_my").c_str());  // 
  strcpy(ssid_ust, prefs.getString("ssid_ust","ssid_ust").c_str());              // имя сети WIFI "AndroidAP1ACA";
  strcpy(password_ust, prefs.getString("password_ust","password_ust").c_str());  //   

  bit->as_ts_on    = prefs.getBool("as_ts_on", 1);                         // использование защит и сигнализации    
  bit->auto_prog   = prefs.getBool("auto_prog", 0);                        // программа по паузам в автомате
  bit->power_auto_t = prefs.getBool("power_auto_t", 0);                    // режим работы автомат-ручное стабилизатора мощности по температуре
  //power_auto_p = prefs.getBool("power_auto_p", 0);                    // режим работы автомат-ручное стабилизатора мощности по давлению  
  bit->state       = prefs.getBool("state", 0);                            // статус (1 в работе) 
  bit->mixer_timer = prefs.getBool("mixer_timer", 0);                      // ручное-таймер мешалки 
  bit->mixer_avto  = prefs.getBool("mixer_avto",0);  
  bit->sleep_n     = prefs.getBool("sleep_n", 1);                          // засыпать дисплею
  bit->valve_avt   = prefs.getBool("valve_avt", 0);                        // клапан в автомате  
  bit->shot_distil = prefs.getBool("shot_distil", 0);                      // дистиляция залповый сброс голов
  bit->shot_golov  = prefs.getBool("shot_golov", 0);                       // дистиляция залповый сброс тела
  sensor_ss   = prefs.getBool("sensor_ss", 0);                        // использовать датчик приёмной ёмкости
  bit->no_ten      = prefs.getBool("no_ten", 0);                           // работа на газу без ТЭН  
  sensor_hoop = prefs.getBool("sensor_hoop", 1);                      // использовать датчик уровня рубашки    
  bit->voice       = prefs.getBool("voice",1);                             // включен ли голос  

  const char *sds;
  String z;
  for(uint8_t i = 0; i < 8; i ++ ){
      z = "t_zad"+String(i);      sds = z.c_str(); pvk_step[i].t_zad      = prefs.getUInt(sds, 33);
      z = "t_time"+String(i);     sds = z.c_str(); pvk_step[i].t_time     = prefs.getUInt(sds, 60);
      z = "step_aktiv"+String(i); sds = z.c_str(); pvk_step[i].step_aktiv = prefs.getBool(sds, 0);
      z = "oper"+String(i);       sds = z.c_str(); pvk_step[i].oper       = prefs.getBool(sds, 0);
      z = "meh"+String(i);        sds = z.c_str(); pvk_step[i].meh        = prefs.getBool(sds, 0);
   }
  power_r  = prefs.getUInt("power_r", 50);                            // мощность ручного режима
  Kp_cool  = prefs.getUInt("Kp_cool", 5);                             // SSR
  Ki_cool  = prefs.getUInt("Ki_cool", 5);                             // SSR 
  Kd_cool  = prefs.getUInt("Kd_cool", 10);                             // SSR   
  zadanie_coll = prefs.getUInt("zadanie_coll", 35);                   // SSR
  KP_power = prefs.getUInt("KP_power", 5);                            // КП ПИД ТЭН
  KI_power = prefs.getUInt("KI_power", 5);                            // КИ ПИД ТЭН  
  //KD_power = prefs.getUInt("KD_power", 5);                            // КИ ПИД ТЭН  
  meh_on   = prefs.getUInt("meh_on", 10);                             // мешалка включена
  meh_off  = prefs.getUInt("meh_off", 10);                            // мешалка выключена
  t_avto   = prefs.getUInt("t_avto", 65);                             // задание температуры в автомате
  step     = prefs.getUInt("step",0);                                 // номер шага пауз
  u_as_ebu = prefs.getUInt("u_as_ebu",55);                            // уставка аварийная Т.ЭБУ
  u_ts_ebu = prefs.getUInt("u_ts_ebu",45);                            // сигнализация Т.ЭБУ 
  u_as_hol = prefs.getUInt("u_as_hol",70);                            // аварийная уставка Т.холодильника
  u_ts_hol = prefs.getUInt("u_ts_hol", 60);                           // уставка сигнализации Т.холодильника  
  f_conv_r = prefs.getUInt("f_conv_r", 0);                            // ручное задание частотнику
  max_scale= prefs.getUInt("max_scale", 10);                          // максимальная шкала преобразователя тока
  k_mesh   = prefs.getUInt("k_mesh", 100);                            // максимальные обороты в % 
  hour_val = prefs.getUInt("hour_val", 8);                            // часовой пояс   
  mode_pvk     = prefs.getUInt("mode_pvk",0);                         // 0-затирание,1-потстил,2-дистилляция    
  t_dis_potOn  = prefs.getUInt("t_dis_potOn",5);                      // дистиляция время закрытия клапана сброса голов 
  t_dis_potOff = prefs.getUInt("t_dis_potOff",1);                     // дистиляция время открытия клапана сброса голов 
  t_telo_On  = prefs.getUInt("t_telo_On",4);                          // дистиляция время открытия клапана сброса тела
  t_telo_Off = prefs.getUInt("t_telo_Off",4);;                        // дистиляция время закрытия клапана сброса тела
  //mpx =        prefs.getUInt("mpx", 10);                              // шкала датчика давления   
  //mpx_k      = prefs.getUInt("mpx_k",150);                            // начальные миливольты датчика давления  
  filtr_pres = prefs.getUInt("filtr_pres",1);                         // коэф. фильтра по давлению  
  uta[0] = prefs.getUInt("uta[0]",35);                                // фиксированные уставки температур в автомате  
  uta[1] = prefs.getUInt("uta[1]",35);                                // фиксированные уставки температур в автомате 
  uta[2] = prefs.getUInt("uta[2]",35);                                // фиксированные уставки температур в автомате 
  uta[3] = prefs.getUInt("uta[3]",35);                                // фиксированные уставки температур в автомате       

  gist_hol    = prefs.getFloat("gist_hol", 1.0f);                        // гистерезис охлаждения 
  gist_nagrev = prefs.getFloat("gist_nagrev", 1.0f);                     // гистерезис нагрева 
  subgist     = prefs.getFloat("subgist", 0.5f);                       // дополнительный гистерезис охлаждения  

  U[0]     = getFloatQuietly("U[0]", 99.0f);                              // фиксированные уставки температур в автомате  
  U[1]     = getFloatQuietly("U[1]", 99.0f);                              // 
 // U[2]     = getFloatQuietly("U[2]", 90.0f);                              // rezerv
  U[3]     = getFloatQuietly("U[3]", 90.0f);                              // 
  U[4]     = getFloatQuietly("U[4]", 91.0f);                              // 
  U[5]     = getFloatQuietly("U[5]", 91.0f);                              //           

  send_port();                                                        // после чтения уставок обновляем визуализацию
  bit->send_port_off = true;                                               // ставим флаг пропуска визуализации  
} 
 //-------------------------------------------------------------------*-------------------------------------------------
void ten_upravlenie(uint8_t power_)                                   // управление ТЭНами
{
  if(bit->as_off || !bit->state){ power_ = 0;}                        // защита на сброс нагрузки
                                                     
  uint32_t power_real_A = 0;
  uint32_t power_real_B = 0;
  uint32_t power_real_C = 0;    

  if     (power_ < 85)      {power_real_A = power_*3;     power_real_B = 0;                   power_real_C = 0;}
  else if(power_ < 170)     {power_real_A = 255;          power_real_B = (power_ - 85)*3;     power_real_C = 0;}
  else if(power_ < 256)     {power_real_A = 255;          power_real_B = 255;                 power_real_C = (power_ - 170)*3;};

  ledcWrite(0, power_real_A);                                         // управление ШИМ ТЭНа A
  ledcWrite(1, power_real_B);                                         // управление ШИМ ТЭНа B
  ledcWrite(2, power_real_C);                                         // управление ШИМ ТЭНа C       
  power_real = power_;
}
//--------------------------------------------------------------------*-------------------------------------------------
/*float pressure()                                                     // чтение давления в баке
 {
  // MPX5010 напряжение нуля 0.19 вольта по цешке(0.2 datasheet), и 550 единиц val_press(4095 максимум)
  // 1Кпаскаль = 7.501 мм.рт.ст.    (75.01 мм.рт.ст. полная шкала датчика)
  // 7.501*((вход в вольтах - 0.2) * (50 - 0) / (4.5 - 0.2) + 0.2)           исходные формулы
  // return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
  // (pres - 550)*10/(4095-550) + 0.2     MPX5010
  // 50 Kpas = 375 мм.рт.ст 
  // adc1_config_width(ADC_WIDTH_BIT_12);                            // АЦП 12 бит
  // adc1_config_channel_atten(ADC1_CHANNEL_3,ADC_ATTEN_DB_0);       // канал ADC3 GPIO-39, вход 1,1 вольт делитель 1/1 

  // август 2022:  
  // ADC_ATTEN_DB_0   65мв  показывают 1-6 АЦП  1.03 вольт 4095 и больше не поднимается
  // ADC_ATTEN_DB_11  120мв показывают 16 АЦП   3.1  вольт 4095 , 4.5 в = 4095 + 1850 = 5950(пересчитано на полную шкалу MPX)
  // Натурные испытания датчика на ректе 5050: питание 3.3в: минимальное напряжение 114.4мв: 1в=70мм=1110ацп: 2в=146мм=2330ацп: 3в=250мм=3830

  int val_press = adc1_get_raw(ADC1_CHANNEL_3);
  // Serial.println("АЦП - " + String(val_press));
  float pres_ = float(val_press - mpx_k) *mpx/5950;                  // давление в kpaskal  
  pres_ = pres_ * 7.501;                                             // давление в MM
  pres_ = pres_m + (pres_ - pres_m)/filtr_pres;                      // фильтр давления
  pres_m = pres_; 
  if(pres_ < 0) pres_ = 0;                                           // давление отрицательным не бывает(плохо графикам)

  return pres_;
 }*/
