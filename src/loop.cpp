#include "header.h"
#include "loop.h" 
//#include "strukt.h"
//#include <stdint.h>     // для видения uint8_t
#include <HTTPClient.h>

enum SystemMode { MODE_IDLE = 0, MODE_HEATING = 1, MODE_COOLING = 2 };
//--------------------------------------------------------------------*-------------------------------------------------  
void states()                                                         // функция состояния
 {
  if(bit->state){                                                     // режим "В РАБОТЕ"
  }
  else{                                                               // режим "В СТУПОРЕ"
    gpio_set_level(valve_gas, 0);                                     // клапан блокировки газа(выключаем газ при останове апарата)  
    bit->bool_imitazia = false;                                            // при ступоре процесса выключаем имитацию
   };
   //Serial.println("state = " + String(bit->state));
 } 
//--------------------------------------------------------------------*-------------------------------------------------------------------
void chart_metka(uint8_t val){                                        // метка на графике
  if(val != 200){Tdata[99] = val;n_event = 0;};                       // 200 посылаем из цикла loop

  if(n_event > 3){                                                    // для гарантии при плохой связи
      Tdata[99] = 0;                                                  // обнуляем значение событий графиков
      n_event = 0;
      }
  else n_event++;
  //Serial.println("Tdata[99] = " + String(Tdata[99]));
  } 
//--------------------------------------------------------------------*-------------------------------------------------   
void handleHeaters(uint8_t power_raw) {
  unsigned long now = millis();

  if (now - ten_power >= WINDOW) {                                    // 1. Обработка начала нового окна
    ten_power = now;
    uint8_t effective_power = (bit->as_off || !bit->state) ? 0 : power_raw;           // Защита: если принудительное выключение — сбрасываем мощность в 0
    unsigned long totalOnTime = (unsigned long)effective_power * (WINDOW * 3) / 255;  // Расчет общего времени работы (используем 1UL для точности unsigned long)

    for (int i = 0; i < 3; i++) {
      unsigned long currentHeaterOnTime = 0;

      if (totalOnTime >= WINDOW) {
        currentHeaterOnTime = WINDOW;
        totalOnTime -= WINDOW;
      } else {
        currentHeaterOnTime = totalOnTime;
        totalOnTime = 0;
      }
      heaters[i].offTime = now + currentHeaterOnTime;                 // Устанавливаем offTime ВСЕГДА. Если мощность 0 — offTime станет равен now.
      digitalWrite(heaters[i].pin, (currentHeaterOnTime > 0) ? HIGH : LOW); // Включаем, если есть время работы, иначе выключаем сразу
    }
  }
  for (int i = 0; i < 3; i++) {                                       // 2. Блок выключения (работает постоянно)
    if (now >= heaters[i].offTime) {                                  // Если время работы вышло — гасим ТЭН
      digitalWrite(heaters[i].pin, LOW);
    }
  }
}
//--------------------------------------------------------------------*------------------------------------------------- 
void sendArray() {                                                    // отправка пакета данных на сервер
    if(ip_indy == NULL) return;
    HTTPClient http;
    http.setTimeout(1000);                                            // ← Таймаут 2 секунды (вместо 5-30)    
    http.begin("http://" + ip_indy + ":" + String(port_indy));        //  sua._ip
    http.addHeader("Content-Type", "application/octet-stream");
    //int httpCode = http.POST(Tdata, 130);                           // Отправляем массив напрямую из памяти
    http.POST(Tdata, 130);  
    //http.end();
}
//--------------------------------------------------------------------*------------------------------------------------- 
void loop()                                                           // главный цикл.
 {
  if(Serial2.available())Serial_read_Nextion();                       // чтение порта Nextion   
  MBSerial_read();                                                    // чтение порта RS485
  // **********************   цикл для ТЭН   ***************************************************************   
  if(millis() - current_millis2 >= 115){     
    current_millis2 = millis(); 

    uint8_t activePhase = 0;                                          // Опрашиваем только ту фазу, которая сейчас регулируется (ШИМ/ПД-режим)
    if      (tenC == 1) activePhase = 3;
    else if (tenB == 1) activePhase = 2;
    else if (tenA == 1) activePhase = 1;
 
    if (activePhase > 0) {
        e_ten = sua.ten_fast(activePhase, max_scale, 0);
        summ += e_ten;
        count++;
    }
    handleHeaters(power_real);                                          // управление ТЭНами(цикл для включения ТЭН)   
  } 
  // **********************   цикл для таймеров клапанов   ***************************************************************
  if(millis() - millis_valve >= 1000)                                 // 
   { 
    millis_valve = millis();    
    klapan();                                                         // управление клапанами отбора
   };
  //**********************   основной цикл программы 4 секунды   *****************************************************
  if(millis() - current_millis > zikl){
    current_millis = millis();  

    Serial.println("Цикл!");

  /*  on_cikl++;
    if(on_cikl > 1){                                                  // первый цикл пропускаем для ТЭН
      on_cikl = 3; 
      if      (power_real <85){                      ten_n = 1;}      // ток ТЭНа A  
      else if(power_real >=85  && power_real <170){  ten_n = 2;}      // ток ТЭНа B 
      else if(power_real >=170 )                  {  ten_n = 3;};     // ток ТЭНа С 
    };
    if(num_dev_relay>0 && !relay_flag){relay(1,0,8,1); relay_flag = true;  }                             // запрос состояния реле FF 01 00 00 00 08 28 12
                               
    if(ten_n != 1)e_tenA = sua.ten(1, 1);                             // ток фазы A
    if(ten_n != 2)e_tenB = sua.ten(2, 1);                             // ток фазы Б    
    if(ten_n != 3)e_tenC = sua.ten(3, 1);                             // ток фазы C  
 */
    ds_kol = sua.read_val_ds();                                        // читаем значения температур с датчиков    
    ds18b20_er_summ = sua._ds_er_summ;    
    for(uint8_t ds = 0; ds < ds_kol; ds++){T[ds] = sua._T[ds];};      // читаем температуры из класса
    ds18b20_er = sua._ds_bit_er;                                      // в каком бите ошибка даласа
    bit->as_ds1820 = sua._as_dalas;                                   // авария по даласам

    speed = sua.PIDcool(T[1]-zadanie_coll, T[1], Ki_cool, Kp_cool, Kd_cool);// T[1]-ntvgthfnehf ,kjrf ЭБУ скорость вращения вентилятора 

    if(bit->bool_imitazia){zikl = 1000; imitaziya();}                 // работа в режиме имитации 
    else zikl = 4000;      

    if(bit->state){
      switch(mode_pvk){                                               // переключение режимов работы  
        case 0: break;        
        case 1: if(T[0] >= U[0]){bit->state = false; sua.muzon();};   // прямоток окончание процесса по температуре    
                bit->auto_prog = false;                               // потстил // автомат по паузам запрещён
                break;                                     
        case 2: if(T[0] >= U[1]){bit->state = false; sua.muzon();};   // дистилляция окончание процесса по температуре
                bit->auto_prog = false;                               // дистилляция // автомат по паузам запрещён
                break;        
      };
    };
    mode_N_H = mode_nagrev_ohlagdenie(T[0], t_avto);                  // определение нагрев-охлаждение
    Tdata[102]   = mode_N_H;                                          // 0-нет нагрева нет охл. 1-нагрев,2-охл.,3-недействительное значение
    ten_upravlenie(auto_pauses());                                    // управление ТЭНами 
       
    klapan_hol(mode_N_H);                                             // управление клапаном подачи воды в рубашку   
    mixer(mode_N_H);                                                  // управление мешалкой    
    ss_level = level_ss();                                            // функция уровня приёмной ёмкости
    hoop_level = level_hoop();                                        // уровень в рубашке
    chart_metka(200); 
    // **************************   засыпание дисплея   *********************************
    if(bit->sleep_n){                                                 // разрешено засыпать
      if(id_forms == 0 && id_forms != 5){sleep_d++;}                  // запыпать только с главной страницы
      else sleep_d = 0;
      if(bit->id_f) sleep_d = 0;                                      // по признаку смены формы обнуляем время засыпания
      if(sleep_d > 75){                                               // засыпаем через 5 минут
            Serial2.print("page e");
            Serial2.print((String)char(255)+char(255)+char(255));
            sleep_d = 0;
      };
    }
    else sleep_d = 0;
    states();                                                         // функция состояния
  
    ts = TS();                                                        // обработка технологической сигнализации    
    as = AS();                                                        // обработка аварийной сигнализации

    //Serial.println("TS - " + String(ts) + "    AS - " + String(as));

    if(!bit->voice){                                                  // если не включен голос то пищим
        if(as){sua.beep(2800,100); delay(50); sua.beep(2700,100);delay(50);sua.beep(2600,100);}
        else if(ts){sua.beep(2700,100);};
    };
   // if(!bit->send_port_off){
      send_port();
    //}                             // шлём данные CKADe если нет пропуска   
    //else bit->send_port_off = false;
    send_port();    
    sendArray();                                                      // отправляем на сервер WIN данные собранные в send_port()

   TFT_refresh(id_forms);                                            // работа с дисплеем
    // ************************** работа с графиками   ***************
   /* if(id_forms != 0){    
      std::rotate(VectorKUB.begin(), VectorKUB.begin() + 489,VectorKUB.end()); // графики предистория
      std::rotate(VectorEBU.begin(), VectorEBU.begin() + 489,VectorEBU.end()); // сдвигаем массив в право на 1 элемент
      std::rotate(VectorZT.begin(),  VectorZT.begin()  + 489,VectorZT.end());    
      //rotate(VectorPres.begin(),VectorPres.begin()+ 489,VectorPres.end());     


      //VectorKUB[0]  = uint8_t(T[0]);                                  // записываем текущие значения в первый элемент
     // VectorEBU[0]  = uint8_t(T[1]);                                  // 
     // VectorZT[0]   = uint8_t(t_avto);      
      //VectorPres[0] = uint8_t(pres);

      VectorKUB[0]  = constrain(uint8_t(T[0] + 0.5f), 0, 105); // с округлением
      VectorEBU[0]  = constrain(uint8_t(T[1] + 0.5f), 0, 105);  // Запись новых значений в первую позицию
      VectorZT[0]   = constrain(t_avto, 0, 105);


         
    //VectorKUB[0] = constrain(uint8_t(T[0]), 0, 255);
    //VectorEBU[0] = constrain(uint8_t(T[1]), 0, 255);
    //VectorZT[0] = constrain(uint8_t(t_avto), 0, 255);




      ghart++;
      if(ghart > 490)ghart = 490;                                     // ширина поля графиков 491 ед.
    };*/
       add_vector();                                                     // добавить значение в график   

    sua.WIFI_connected(ssid_ust, password_ust);
    static uint8_t n_db = 0;
    n_db++;
    if(n_db>= 4){ 
      wifi_db = sua.printSignalStrength();                            // уровень WIFI
      n_db = 0;
  }
  //*********** автоматическое выключение расцепителя  *********************
  /*if(bit->auto_off && stat_auto)                                      // включено автовыключение и процесс авто-закончился// флаг автоматического завершения процесса
  {
     tim_off++;
     if(tim_off > 225)                                                // 15 минут до отключения
     {
        gpio_set_level(AVTOMAT, 1);                                   // выключаем автомат безопасности
        delay(500);
        gpio_set_level(AVTOMAT, 0);                                   // возвращаем автомат безопасности
     }                                       
  }
  else tim_off = 0;*/
  };                                                                 // milliss(4000)
 };  
//--------------------------------------------------------------------*-------------------------------------------------  
uint8_t mode_nagrev_ohlagdenie(float current_t, uint8_t zad_t)
 {
  static uint8_t last_mode = MODE_IDLE;                               // Храним предыдущее состояние
  float target_t = static_cast<float>(zad_t);
    
  if (current_t < (target_t - gist_nagrev)) {last_mode = MODE_HEATING;}// Логика НАГРЕВА
  else if(current_t > (target_t + gist_hol)) {last_mode = MODE_COOLING;}// Логика ОХЛАЖДЕНИЯ
  else if((last_mode == MODE_HEATING && current_t >= target_t) ||     // Условие ОСТАНОВКИ (когда достигли цели с учетом гистерезиса)
           (last_mode == MODE_COOLING && current_t <= target_t)) {    // Для нагрева: выключаем, если стала >= target   // Для охлаждения: выключаем, если стала <= target
        last_mode = MODE_IDLE;
  }
    return last_mode;
 }    
//--------------------------------------------------------------------*------------------------------------------------- 
bool level_ss(){                                                      // функция уровня приёмной ёмкости 
  if(!sensor_ss){ss_level = false; return false;};                    // не используем датчик приёмной ёмкости
  bool level = false;
  if(gpio_get_level(LEVEL_SS)){                                       // датчик уровеня СС
    if(level_cc <5)level_cc++;
    if(level_cc > 3){
      level = true;                                                // приёмная ёмкость полна
    }
  }
  else{
    level_cc = 0;
    level = false;
  }
  return level;
 }
//--------------------------------------------------------------------*------------------------------------------------- 
bool level_hoop(){                                                    // функция уровня рубашки 
  bool hoop = false;                                                  // уровень рубашки

  if(!sensor_hoop){return false;};                                    // не используем датчик уровня рубашки  

  if(gpio_get_level(LEVEL_HOOP)){hoop = true;}                        // датчик уровеня рубашки
  else{hoop = false;}
    
  return hoop;
 } 
//--------------------------------------------------------------------*------------------------------------------------- 
void klapan_hol(uint8_t mode)                                         // mode=mode_N_H  управление клапаном охлаждения     
 {
  if(!bit->valve_avt) return;                                         // если клапан не в автомате то ничего не делаем 

  bool shouldBeOpen = (bit->state) &&                                 // Клапан открыт ТОЛЬКО если: в работе И режим затирания И нет нагрева И режим охлаждения
                        (mode_pvk == 0) && 
                        (power_real == 0) && 
                        (mode == 2);

  if (shouldBeOpen) {                                                 // Пытаемся открыть, если закрыт (уровень 1)
      if (gpio_get_level(valve_holy)) {
          gpio_set_level(valve_holy, 0);
          chart_metka(15);
      }
  } 
  else{
      if (!gpio_get_level(valve_holy)) {                              // В любой другой ситуации (останов, другой режим, нагрев) — закрываем, если открыт (уровень 0)
          gpio_set_level(valve_holy, 1);
          chart_metka(10);
      }
  }
 }
//--------------------------------------------------------------------*-------------------------------------------------
void stepper(uint8_t n_step, bool step_aktiv, bool oper, bool meh, uint8_t t_zad, uint8_t t_time)     // , uint8_t t_time номер шага, шаг включен, вызов оператора, мешалка, задание температуры шага, время паузы 
 {
    static bool is_initialized = false;                               // 1. Инициализация при первом запуске
    if (!is_initialized && n_step == 0) {
        time_pauses = 0;
        is_initialized = true;
    }

    if (step >= MAX_STEPS) {_shutdown_procedure(); return;}           // 2. Проверка на необходимость выхода(шагов > 8)
    
    if (!step_aktiv) {_move_to_next_step(); return;}                  // 3. Проверка активности текущего шага. Пропускаем неактивные.
        
    if (previous_step != step) {                                      // 4. Определение режима (нагрев/охлаждение) при смене шага
        mode_N_H = mode_nagrev_ohlagdenie(T[0], t_zad);               // видимо для быстрого реагирования
        previous_step = step;
    } 
        
    if (_is_target_temperature_reached(t_zad)) {                      // 5. Проверка, достигнута ли целевая температура
        temperature_stable_counter++;
        if (temperature_stable_counter > 1) {                         // Ждём два замера для стабильности
            _start_pause_countdown();
        }
    } 
    else {temperature_stable_counter = 0;}
        
    if (step_on) {                                                    // 6. Обработка активного отсчёта паузы
        time_pauses += STEP_CALL_PERIOD_SEC;                          // прибавляем по 4 секунды
        reverse_time = (t_time * 60) - time_pauses;
        if (time_pauses >= (t_time * 60) || time_pause_on) {_finish_current_step(oper);}
    }
    else reverse_time = 0;                                            // если не обрабатываем время паузы, то оставшееся время обнуляем
 }
//--------------------------------------------------------------------*-------------------------------------------------
void _finish_current_step(bool is_operator_required) {
    time_pause_on = true;                                             // Флаг - пауза исчерпана
    time_pauses = 0;                                                  // Сбрасываем счётчик времени паузы

    if (is_operator_required) {                                       // ОБРАБОТКА ВЫЗОВА ОПЕРАТОРА
        if (bit->sequent) {                                           // Оператор дал команду "Продолжить"
            time_pause_on = false;                                    // Сбрасываем флаг окончания паузы
            bit->sequent = false;
            bit->oper_pause = false;                                  // Снимаем оповещение оператора
            step_on = false;
            step++;                                                   // Переходим на следующий шаг
            prefs.putUInt("step", step);                              // Сохраняем новый шаг
        } else {                                                      // Ждём команды от оператора
            reverse_time = 0;
            bit->oper_pause = true;                                   // Оповещаем о паузе
            time_muzon++;                                             // Периодическое оповещение (звуком)
            if (time_muzon > 30) {                                    // ~2 минуты при вызове раз в 4 сек (30 * 4 = 120 сек)
                if (!bit->voice) {sua.muzon();};                      // Пищим, если голос запрещён
                time_muzon = 0;
            }
        }
    } else {                                                          // АВТОМАТИЧЕСКОЕ ЗАВЕРШЕНИЕ ШАГА (ОПЕРАТОР НЕ ТРЕБУЕТСЯ)
        step_on = false;
        time_pause_on = false;                                        // Сбрасываем флаг
        bit->oper_pause = false;
        step++;                                                       // Переходим на следующий шаг
        prefs.putUInt("step", step);                                  // Сохраняем новый шаг
        auto_pauses();                                                // Запускаем обработку для нового шага
    }
  }
//------------------------------------------------------------------*------------------------------------------------- 
void _start_pause_countdown(void) {
    step_on = true;                                                   // Запускаем отсчёт времени паузы
    temperature_stable_counter = 0;                                   // Сбрасываем счётчик стабильности
    send_port();                                                      // Отправляем данные на порт (как в оригинале)
    bit->send_port_off = true;                                        // Ставим флаг пропуска визуализации (как в оригинале)
}
//--------------------------------------------------------------------*------------------------------------------------- 
bool _is_target_temperature_reached(uint8_t target_temp) {            // Проверяем, достигнута ли целевая температура в текущем режиме
  if(!mode_N_H){return true;}
  else return false;
}
//--------------------------------------------------------------------*-------------------------------------------------  
void _move_to_next_step() {                                           // Вынесенная процедура перехода к следующему шагу
    step++;
    time_pauses = 0;
    // Здесь можно добавить цикл while для пропуска нескольких неактивных шагов подряд,
    // но с обязательной проверкой step < MAX_STEPS!
    prefs.putUInt("step", step);
}
//--------------------------------------------------------------------*------------------------------------------------- 
void _shutdown_procedure() {                                          // Вынесенная процедура завершения программы
    bit->state = false;
    prefs.putBool("state", bit->state);
    states();
    step = 0;
    prefs.putUInt("step", step);
    bit->auto_prog = false;
    prefs.putBool("auto_prog", bit->auto_prog);
    if (!bit->auto_prog) {
        t_avto = prefs.getUInt("t_avto", 65);
    };
    bit->mixer_timer = false;
    sua.muzon();
}
//--------------------------------------------------------------------*------------------------------------------------- 
 void read_mem_struct(uint8_t n)                                      // запись структуры в память 
 {
    char s = char(n);
    pvk_step[n].step_aktiv =  prefs.getBool("step_aktiv"+s, 1);      // шаг включен
    pvk_step[n].oper =        prefs.getBool("oper"+s,       0);      // вызов оператора
    pvk_step[n].meh =         prefs.getBool("meh"+s,        1);      // мешалка
    pvk_step[n].t_zad =       prefs.getUInt("t_zad"+s,     65);      // задание температуры шага
    pvk_step[n].t_time =      prefs.getUInt("t_time"+s,    10);      // время паузы 
 }
//--------------------------------------------------------------------*------------------------------------------------- 
 void write_mem_struct(uint8_t n)                                     // запись структуры в память 
 {
    char s = char(n);
    prefs.putBool("step_aktiv"+s, pvk_step[step].step_aktiv);        // шаг включен
    prefs.putBool("oper"+s,       pvk_step[step].oper);              // вызов оператора
    prefs.putBool("meh"+s,        pvk_step[step].meh);               // мешалка
    prefs.putUInt("t_zad"+s,      pvk_step[step].t_zad);             // задание температуры шага
    prefs.putUInt("t_time"+s,     pvk_step[step].t_time);            // время паузы 
 }
//--------------------------------------------------------------------*-------------------------------------------------  
uint8_t auto_pauses()                                                 // считываем структуру пауз и получаем задание ПИДу
  {
    uint8_t pwr_;
    if(bit->auto_prog){                                               // автоматическая работа по паузам
                //  Serial.println("step - " + String(step));
      stepper(pvk_step[step].n_step,                                  // функция с текущими параметрами шага - номер шага
              pvk_step[step].step_aktiv,                              // шаг включен
              pvk_step[step].oper,                                    // вызов оператора
              pvk_step[step].meh,                                     // мешалка
              pvk_step[step].t_zad,                                   // задание температуры шага
              pvk_step[step].t_time);                                 // время паузы 

      t_avto = pvk_step[step].t_zad;
      pwr_ = PID(t_avto, mode_N_H);                                             // задание мощности ТЭНам
    }
    else{
      bit->oper_pause = false;                                        // снимаем признак вызова оператора
      pwr_ = PID(t_avto, mode_N_H);   
      time_pauses = 0;                                                // сброс счёта паузы   
    };
 return  pwr_;  
 }
//--------------------------------------------------------------------*-------------------------------------------------  
void mix_oper(uint16_t zadanie)                                       // управление мешалкой, вкл-выкл и обороты
{
    if((zadanie == 0) && (speed_mesh > 0)){                           // частотник отвечает одним пакетом 18 байт
        WriteRegister(1,2,0);                                         // для пропуска опроса в главном цикле delay(100);                                        // задание частотнику 0;
        if(bit->mesh_on_off){WriteRegister(1,3,2);}                   // для пропуска опроса в главном цикле   
    }
    else if((speed_mesh != zadanie)){WriteRegister(1,2,zadanie);      // для пропуска опроса в главном циклеdelay(100);      // сменить задание частотнику(даже если оно 0)
        if(!bit->mesh_on_off){WriteRegister(1,3,1); }                 // для пропуска опроса в главном цикле                       // RS485 пуск частотника
    }
    else{ReadRegisters(1,2,9);}                                       // RS485 девять параметров: все доступные   
}
//--------------------------------------------------------------------*------------------------------------------------- 
void relay(unsigned char DevID, uint16_t number, uint16_t on_off, unsigned char funz)// управление реле платы расширения
{
 // Serial.println("Relay № " +     String(on_off));
 //Serial.println("Relay № " +     String(number)); 

  WriteRegisterRelay(DevID, funz, on_off, number);                                // unsigned char DevID,uint16_t ADRESS,uint16_t val

}
//--------------------------------------------------------------------*-------------------------------------------------  
void mixer(uint8_t mode)                                              // управление мешалкой
 {
  uint16_t z_f_c = 0;                                                 // задание частотнику
  float z_f;                                                          // для расчёта задания частотнику до 100%

  z_f = power_real * 1.963;                                           // (power_real * 5 / 2.55)скорость вращения по заданию мощности 0-100%           
  z_f = z_f*k_mesh/100;                                               // коррекция вращения от коэффициента 
  z_f_c = uint16_t(z_f);  

  if(!gpio_get_level(valve_holy)){                                    // при включении клапана включаем мешалку(клапан работает в реверсе)
        mix_oper(f_conv_r * 10);                                      // Serial.println(" по клапану.");
        return;
  }
  if(bit->mix_on && (bit->mixer_avto || bit->mixer_timer)){           // прокрутка мешалки принудительно (если в автомаие или в таймере)
        mix_oper(f_conv_r * 10);                                      // Serial.println("Прокрутка мешалки");
  }
  else if(bit->mixer_avto){                                           // мешалка в режиме автомат
      bit->mikserOnR = false;                                         // выключаем ручной, что бы не запустилась при переходе на ручной  
      if(bit->auto_prog && !pvk_step[step].meh){mix_oper(0);return;}  // на паузах если не активна мешалка                                
      if(tim_on < uint16_t(meh_on/4)){                                // включен по таймеру
            if(f_conv_r*10 < z_f_c){mix_oper(z_f_c);}                 // что бы не было провалов оборотов в автомате
            else mix_oper(f_conv_r * 10);                             // делаем обороты не меньше чем расчётные от мощности
            tim_on++;
      }
      else{                                                           // время стоять по таймеру(вращаемся от мощности)
            if(!bit->valve_avt && gpio_get_level(valve_holy)){        // клапан в ручном и закрыт то мешалка стоит
                mix_oper(0);
                return;
            }                                                   
            if     (z_f_c<=20 && bit->valve_avt    && mode == 2)  {mix_oper(110);}  // клапан в автомате и режим охлаждения (докрутка на минималке оборотов)  
            else if(z_f_c<=20 && bit->power_auto_t && mode == 1)  {mix_oper(110);}  // мощность в автомате и режим нагрева (докрутка на минималке оборотов)
            else if(z_f_c <= 20)                                  {mix_oper(0);}    // меньше 5гц не крутим мешалку
            else if(z_f_c > 20 && z_f_c <= 100)                   {mix_oper(110);}  // минимальная скорость 11гц
            else                                                  {mix_oper(z_f_c);};

            tim_off++;
            if(tim_off > meh_off*15){ tim_on = 0;tim_off = 0;} ;      // одна минута 15 циклов
      };
  }                                                                   // мешалка в режиме автомат
  else if(bit->mixer_timer){                                          // мешалка в режиме таймера
      bit->mikserOnR = false;                                         // выключаем ручной, что бы не запустилась при переходе на ручной
      if(tim_on < uint16_t(meh_on/4)){
          if(!bit->mesh_on_off){mix_oper(f_conv_r * 10);};
          tim_on++;
      }
      else{                                                           //Serial.println("Выключить мешалку по таймеру"); 
          mix_oper(0);                                                // RS485 stop частотника            
          tim_off++;
          if(tim_off > meh_off*15){tim_on = 0;tim_off = 0;}           // одна минута 15 циклов
      }
  }// мешалка в режиме таймера
  else{                                                               // мешалка в ручном режиме
      if(bit->mikserOnR){mix_oper(f_conv_r * 10);}                    //Serial.println("Включить мешалку в ручном");
      else{mix_oper(0); }                                             //Serial.println("Выключить мешалку в ручном");
  };
   //Serial.println("tim_on - " + String(tim_on));
   //Serial.println("tim_off - " + String(tim_off));
}
//--------------------------------------------------------------------*-------------------------------------------------
void valv_oper(bool gol, bool telo, bool hvost)                       // оперативное управление клапанами отбора
{
  gpio_set_level(valve_golov,gol);
  gpio_set_level(valve_telo,telo);
  gpio_set_level(valve_hvost,hvost);
}
//--------------------------------------------------------------------*-------------------------------------------------
void klapan()                                                         // управление клапанами отбора
{
  bool kl1 = false;
  bool kl2 = false;
  bool kl3 = false;

  if(bit->r_golova){kl1 = true;};
  if(bit->r_telo)  {kl2 = true;};
  if(bit->r_hvost) {kl3 = true;};  
  
  if(mode_pvk == 0){valv_oper(kl1,kl2,kl3);return;}                   // режим затирания все клапана закрыты(кроме принудиловки)
  else if(mode_pvk == 1){                                             // потстил (прямоток)
    if(T[0] > 84){                                                    // отбор только при нагретом баке  
        if(T[0] <= U[4]){ valv_oper(true,kl2,kl3);}                   // Т куба ниже порога отбора тела, отбираем головы
        else{             valv_oper(kl1,true,kl3);};                  // T достигла уставки начала отбора тела прямоток     
    }
    else{valv_oper(kl1,kl2,kl3);};                                    // холодный бак, нет отбора
  }
  else if(mode_pvk == 2){                                             // дистиляция(колонна)
    if(T[0] > 84){                                                    // отбор только при нагретом баке
      if(T[0] <= U[3]){                                               // отбираем головы
          if(bit->shot_golov){                                        // разрешен залповый сброс голов
              if(kl_of < t_dis_potOff){                               // время закрытия 
                  kl_of++;                          
                  valv_oper(kl1,kl2,kl3);                             // клапан голов закрыт
              }
              else if(kl_on < t_dis_potOn){                           // время открытия клапана голов
                  valv_oper(true,kl2,kl3);
                  kl_on++;
                  if(kl_on >= t_dis_potOn){kl_of = 0;kl_on = 0;};
              };
          }   
          else{valv_oper(true,kl2,kl3);};                             // отбираем головы, без ШИМ                                                      
      } // отбираем головы
      else if((T[0] > U[3])&&(T[0]<=U[5])){                           // (колонна)отбираем тело дистиляции
          if(bit->shot_distil){                                       // разрешен залповый сброс тела
              if( kl_of < t_telo_Off){                                // время закрытия 
                  kl_of++;        
                  valv_oper(kl1,kl2,kl3);                             // клапан тела закрыт
              }
              else if(kl_on < t_telo_On){                             // время открытия клапана тела
                  valv_oper(kl1,true,kl3);
                  kl_on++;
                  if(kl_on >= t_telo_On){kl_of = 0;kl_on = 0;};
              };
          }
          else  valv_oper(kl1,true,kl3);                              // отбираем тело, без ШИМ
      }
      else if(T[0] > U[5]){ valv_oper(kl1,kl2,true);};                // (колонна)отбираем хвосты дистиляции (пока без шима)  
    }// бак нагрет(колонна)  
    else{valv_oper(kl1,kl2,kl3);};                                    // (колонна) бак холодный, отбора нет 
  };
 // Serial.println("kl_on - " + String(kl_on) + "      kl_of - " + String(kl_of));
 //Serial.println("2     kl1 - " + String(kl1) + "      kl2 - " + String(kl2) + "      kl3 - " + String(kl3)); 

}
//--------------------------------------------------------------------*-------------------------------------------------    
//--------------------------------------------------------------------*-------------------------------------------------  
//void _update_heating_mode(uint8_t target_temp) {
    // Режим (mode) должен быть объявлен как static внутри stepper 
    // или быть частью структуры состояния котла
    
   // if (T[0] >= target_temp) { mode = true;}                          // Режим охлаждения
   // else if (T[0] < target_temp) { mode = false;}                     // Режим нагрева
    
    // Логирование для отладки (можно убрать в релизе)
   /* #ifdef DEBUG
    Serial.print("Режим изменён на: ");
    Serial.println(mode ? "ОХЛАЖДЕНИЕ" : "НАГРЕВ");
    Serial.print("Текущая T[0]=");
    Serial.print(T[0]);
    Serial.print(", Целевая temp=");
    Serial.println(target_temp);
    #endif*/
//}