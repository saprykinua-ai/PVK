#include "header.h"
#include "com_port_BT.h"
#include <driver/adc.h>    

//MY_LIB sua;


step_prog *pvk_step = new step_prog[8];                               //Выделение памяти под массив структур, ради 8 структур
bits *bit = new bits;
//  int  a = (int)sizeof(bit);
//  Serial.println("sizeof = " + String(bit));
//--------------------------------------------------------------------*-----------------------------------------------------------
void send_port()
  { 
  test = !test;                                                       // nextionu о посылке данных

  data_bit = 0;                                                       // обнуление обязательно для операций
  
  data_bit  |=bit->mesh_on_off                        <<0;            // 1 мешалка включена-выключена
  data_bit  |=bit->mixer_timer                        <<1;            // 2 ручное-таймер мешалки 
  data_bit  |=bit->oper_pause                         <<2;            // 4 стоим на паузе оператора  
  data_bit  |=bit->state                              <<3;            // 8 ПВК включен-выключен            
  data_bit  |=bit->auto_prog                          <<4;            // 16 программа пауз в автомате             
  data_bit  |=bit->power_auto_t                       <<5;            // 32 ПИД-ручное регулятора по температуре
  if(as)data_bit  |=1                                 <<6;            // 64 активность аварии
  if(ts)data_bit  |=1                                 <<7;            // 128 активность сигнализации 
  data_bit  |=bit->no_ten                             <<8;            // 256 запрет работы всех ТЭН (работа на газу)
  if(power_real > 1)  data_bit  |=1                   <<9;            // 512 ТЭН А работает 
  if(power_real > 85) data_bit  |=1                   <<10;           // 1024 ТЭН Б работает     
  if(power_real > 170)data_bit  |=1                   <<11;           // 2048 ТЭН С работает           
  data_bit  |=sensor_ss                               <<12;           // 4096 использовать датчик приёмной ёмкости        
  data_bit  |=bit->valve_avt                          <<13;           // 8192 клапан охлаждения в автомате
  if(WiFi.status() == WL_CONNECTED) data_bit  |=1     <<14;           // 16384 подключено к WIFI  
  data_bit  |=test                                    <<15;           // 32768 синхронизация некстиона 
  data_bit  |=bit->sleep_n                            <<16;           // 65536 спать дисплею
  data_bit  |=bit->as_ts_on                           <<17;           // 131072 защиты выведены
  data_bit  |=bit->mixer_avto                         <<18;           // 262144 мешалка в автомате
  data_bit  |=bit->shot_golov                         <<19;           // 524288 прямоток залповый сброс голов(шим)
  data_bit  |=bit->shot_distil                        <<20;           // 1048576 дистиляция залповый сброс тела(шим) 
  data_bit  |=gpio_get_level(valve_golov)             <<21;           // 2097152 клапан голов включен     
  data_bit  |=gpio_get_level(valve_telo)              <<22;           // 4194304 клапан тела включен      
  data_bit  |=bit->mix_on                             <<23;           // 8388608 прокрутка мешалки в автомате
  data_bit  |=bit->r_golova                           <<24;           // 16777216 принудительное включение клапана голов
  data_bit  |=bit->r_telo                             <<25;           // 33554432 принудительное включение клапана тела 
  data_bit  |=!gpio_get_level(valve_holy)             <<26;           // 67108864 клапан охлаждения  
  data_bit  |=!gpio_get_level(valve_gas)              <<27;           // 134217728 отсечной клапан газа   
  data_bit  |= bit->r_hvost                           <<28;           // 268435456 принудительное включение клапана хвостов 
  data_bit  |=gpio_get_level(valve_hvost)             <<29;           // 536870912 клапан хвостов включен  
  data_bit  |=time_pause_on                           <<30;           // 1073741824 время паузы истекло(флаг)  
  data_bit  |= sensor_hoop                            <<31;           // 2147483648 использование датчика в рубашке
  //-------------------------------------------------------------------------------------------------------------------
  data_bit2 = 0;                                                      // обнуление обязательно для операций
  data_bit2 |= bit->relay1                            <<0;            // 1 реле платы расширения
  data_bit2 |= bit->relay2                            <<1;            // 2 реле платы расширения 
  data_bit2 |= bit->relay3                            <<2;            // 4 реле платы расширения 
  data_bit2 |= bit->relay4                            <<3;            // 8 реле платы расширения 
  data_bit2 |= bit->auto_relay                        <<4;            // 16 автомат платы реле расширения 
  data_bit2 |= ip_adr_on                              <<5;            // 32 

  Tdata[1] = data_bit;                                                // сборка uint32_t
  Tdata[2] = data_bit >> 8; 
  Tdata[3] = data_bit >> 16; 
  Tdata[4] = data_bit >> 24;

  int16_t t;
  t = T[0]*16; Tdata[5] = t; Tdata[6] = t >> 8;                       // температуры T[0]-т.куб
  t = T[1]*16; Tdata[7] = t; Tdata[8] = t >> 8;                       // T[1]-т.ЭБУ
  t = T[2]*16; Tdata[9] = t; Tdata[10] = t >> 8;                      // T[2]-т.холодильника
  t = T[3]*16; Tdata[11] = t; Tdata[12] = t >> 8;                     // T[3]-т.колонны

  Tdata[13] = data_bit2;                                              // сборка бинарника №2 16 бит
  Tdata[14] = data_bit2 >> 8;   
  Tdata[15] = speed;                                                  // ЭБУ скорость вращения вентилятора в %
  Tdata[16] = max_scale;                                              // максимальная шкала преобразователя тока
  Tdata[17] = uint8_t(power_real/2.55);                               // мощность тэна теперь в %  
  Tdata[18] = KP_power;                                               // КП ПИД ТЭН
  Tdata[19] = byte(e_tenC*10);                                        // ток тэна фазы Б
  Tdata[20] = byte(e_tenA*10);                                        // ток рабочего тэна  e_tenA
  Tdata[21] = byte(e_tenB*10);                                        // ток тэна фазы А
  Tdata[22] = P_power;                                                // воздействие пропорционалка 
  Tdata[23] = KI_power;                                               // КИ ПИД ТЭН
  Tdata[24] = zadanie_coll;                                           // ЭБУ задание Т. блока      
  Tdata[25] = Kp_cool;                                                // ЭБУ
  Tdata[26] = Ki_cool;                                                // ЭБУ
  Tdata[27] = sua._ds_bit_er;                                         // ошибки DS18B20
  Tdata[28] = I_power;                                                // воздействие интегралка   
  Tdata[29] = uint8_t(abs(D_power));                                  // дифференциалка воздействие  
  if(D_power < 0){Tdata[30] = 1;} else Tdata[30] = 0;                 // дифференциалка воздействие
  Tdata[31] = uint8_t(abs(P_power));                                  // пропорционалка воздействие   
  if(P_power < 0){Tdata[32] = 1;} else Tdata[32] = 0;                 // пропорционалка воздействие
  Tdata[33] = meh_on;                                                 // время включения мешалки
  Tdata[34] = meh_off;                                                // время выключения мешалки
  Tdata[35] = ds_kol;                                                 // количество найденных DS18B20
  Tdata[36] = mode_pvk;                                               // 0-затирание,1-потстил,2-дистилляция   
  Tdata[37] = bit->voice;                                             // включен ли голос
  Tdata[38] = id_forms;                                               // номер открытой формы   
  Tdata[39] = as;                                                     // аварийная сигнализация
  Tdata[40] = ts;                                                     // технологическая сигнализация  
  Tdata[41] = power_r;                                                // задание ручное мощности в %  
  Tdata[42] = t_avto;                                                 // задание температуры в автомате

 // U[0]-т.окончания процесса прямоток, U[1]-т.окончания процесса дистиляция, U[2]-(rezerv), U[3]-т.отбора тела дистиляция, U[4]-т.отбора тела прямоток, U[5]-т.отбора хвостов дистиляция
  t = U[0]*16; Tdata[43] = t; Tdata[44] = t >> 8;     
  t = U[1]*16; Tdata[45] = t; Tdata[46] = t >> 8; 
  t = U[2]*16; Tdata[47] = t; Tdata[48] = t >> 8; 
  t = U[3]*16; Tdata[49] = t; Tdata[50] = t >> 8; 
  t = U[4]*16; Tdata[51] = t; Tdata[52] = t >> 8; 
  t = U[5]*16; Tdata[53] = t; Tdata[54] = t >> 8; 

  Tdata[55] = t_dis_potOn;                                            // дистиляция время открытия клапана сброса голов 
  Tdata[56] = t_dis_potOff;                                           // дистиляция время закрытия клапана сброса голов 
  Tdata[57] = t_telo_On;                                              // дистиляция время открытия клапана сброса тела
  Tdata[58] = t_telo_Off;                                             // дистиляция время закрытия клапана сброса тела   
  Tdata[59] =  u_ts_ebu;                                              // ЭБУ уставка сигнализации 
  Tdata[60] =  u_as_ebu;                                              // ЭБУ аварийная уставка   
  Tdata[61] =  u_ts_hol;                                              // уставка сигнализации Т.холодильника 
  Tdata[62] =  u_as_hol;                                              // аварийная уставка Т.холодильника

  Tdata[63] =  pvk_step[0].t_time;
  Tdata[64] =  pvk_step[1].t_time;
  Tdata[65] =  pvk_step[2].t_time;
  Tdata[66] =  pvk_step[3].t_time;
  Tdata[67] =  pvk_step[4].t_time;
  Tdata[68] =  pvk_step[5].t_time;
  Tdata[69] =  pvk_step[6].t_time;
  Tdata[70] =  pvk_step[7].t_time;      
  Tdata[71] =  pvk_step[0].t_zad;
  Tdata[72] =  pvk_step[1].t_zad;
  Tdata[73] =  pvk_step[2].t_zad;
  Tdata[74] =  pvk_step[3].t_zad;
  Tdata[75] =  pvk_step[4].t_zad;
  Tdata[76] =  pvk_step[5].t_zad;
  Tdata[77] =  pvk_step[6].t_zad;
  Tdata[78] =  pvk_step[7].t_zad;

  uint8_t on_step1=0, on_oper1=0, on_mash1=0;
  for(uint8_t ii = 0; ii < 8; ii ++ ){
    on_step1|= pvk_step[ii].step_aktiv << ii;
    on_oper1|= pvk_step[ii].oper       << ii;
    on_mash1|= pvk_step[ii].meh        << ii;
  };
  Tdata[79] =  on_step1;
  Tdata[80] =  on_oper1;
  Tdata[81] =  on_mash1; 

  if(speed_mesh == 0){Tdata[82] = f_conv_r;}                          // когда частотник стоит, то выдаёт 0 то посылаем ручную настройку
  else Tdata[82] = speed_mesh/10;   

  Tdata[83]    = step + 1;                                            // номер шага
  Tdata[84]    = pvk_step[step].t_zad;                                // температура задания
  Tdata[85]    = pvk_step[step].t_time;                               // время паузы  
  Tdata[86]    = reverse_time;                                        // оставшееся время паузы    
  Tdata[87]    = reverse_time >> 8;                                   // оставшееся время паузы 
  Tdata[88]    = Kd_cool;                                             // ЭБУ
  Tdata[89]    = audio_as;                                            // номер файла музыки                
  Tdata[90]    = hour_val;                                            // часовой пояс
  Tdata[91]    = gist_hol*10;                                         // гистерезис охлаждения
  Tdata[92]    = gist_nagrev*10;                                      // гистерезис нагрева     
  Tdata[93]    = subgist*10;                                          // гистерезис нагрева   
  Tdata[94]    = speed_mesh/10;                                       // чистые обороты для графика
  Tdata[95]    = f_conv_r;                                            // ручное задание частотнику
  Tdata[96]    = data_bit2 >> 16;                                     // сборка бинарника №2 16 бит
  Tdata[97]    = data_bit2 >> 24;  
  Tdata[98]    = num_dev_relay;                                       // адрес релейной платы расширения
  //Tdata[99];                                                        // метки событий
  Tdata[100]   = sua._ds_er_summ;
  Tdata[101]   = sua._ds_bit_er;                                      // в каком бите ошибка даласа
  //Tdata[102]   = mode_N_H;  присваиваем в коде                                          // 0-нет нагрева нет охл. 1-нагрев,2-охл.,3-недействительное значение
  Tdata[103]   = uta[0];                                              // фиксированные уставки температур в автомате  
  Tdata[104]   = uta[1];                                              // фиксированные уставки температур в автомате 
  Tdata[105]   = uta[2];                                              // фиксированные уставки температур в автомате 
  Tdata[106]   = uta[3];                                              // фиксированные уставки температур в автомате  
  Tdata[107]   = static_cast<uint8_t>(zikl/1000);                     // время цикла
  Tdata[108]   = wifi_db;                                             // сигнал WIFI

  // C 110 + 12 бит - версия ESP32
 };  
//--------------------------------------------------------------------*-----------------------------------------------------------  
String COM_read_BT(uint8_t w, uint16_t params, uint8_t nums, uint8_t tip)// обработка данных порта 
 {
   String x = {};  
   //unsigned char c_comand[10];                                         // команда для порта   
   //Serial.println("BT кол-во " + String(w)); 
   //Serial.println(String(w)+ "  " + String(params)+ "  " + String(nums)+ "  " + String(tip));    
   switch(w)      
    {
     //------------------------------   новый формат для НБК   -----------------------------------------     
    case 7:{                                                          // if(scada) send_port(); Serial.println("send_port 1");
        switch(tip){ 
            case 0: break;                                            // отсекаем ложные команды                       
            case 1:{                                                  // обработка команд (эдит по энтер)
              uint16_t u{};                                           // uint16_t для Edit
              switch(nums)                                            // номер эдит из массива
               {                        //  ЭДИТЫ
                 case 1:  meh_on    = sua.read_uint("meh_on", 8,255,    params);  u = meh_on;             break;    // (4 минуты с копейками)период работы миксера в сек.   
                 case 2:  meh_off   = sua.read_uint("meh_off",1,180,    params);  u = meh_off;            break;    // (3 часа) останов мешалки
                 case 3:  t_avto    = sua.read_uint("t_avto", 0, 105,   params);  u = t_avto;             break;    // задание температуры в автомате(105 чтобы не выключались ТЭНы) 
                 case 4:  u_as_ebu  = sua.read_uint("u_as_ebu",50, 75,  params);  u = u_as_ebu;           break;    // ЭБУ аварийная уставка 
                 case 5:  u_ts_ebu  = sua.read_uint("u_ts_ebu",35, 65,  params);  u = u_ts_ebu;           break;    // ЭБУ уставка сигнализации 
                 case 6:  hour_val  = sua.read_uint("hour_val",1, 20,   params);  u = hour_val;           break;    // часовой пояс
                 case 7:  power_r   = sua.read_uint("power_r", 0, 100,  params);                                    // задание ручное мощности %
                          if(!bit->power_auto_t){ten_upravlenie(power_r*2.55+1);};u = power_r;            break;    // если в ручном, то сразу применяем
                 case 8:  KP_power  = sua.read_uint("KP_power",0, 100,  params);  u = KP_power;           break;    // КП ПИД ТЭН
                 case 9:  KI_power  = sua.read_uint("KI_power",0, 100,  params);  u = KI_power;           break;    // КИ ПИД ТЭН 
                 case 10: max_scale = sua.read_uint("max_scale",10,50,  params);  u = max_scale;          break;    // максимальная шкала преобразователя тока
                 case 11: f_conv_r = sua.read_uint("f_conv_r", 0, 50,   params); mixer(10); u = f_conv_r; break;// управление частотником
                 case 12:  u_as_hol  = sua.read_uint("u_as_hol",60,85,  params);  u = u_as_hol;           break;     // аварийная уставка Т.холодильника
                 case 13:  u_ts_hol  = sua.read_uint("u_ts_hol",35,75,  params);  u = u_ts_hol;           break;     // уставка сигнализации Т.холодильника               
                 case 14:  k_mesh    = sua.read_uint("k_mesh",50, 100,  params);  u = k_mesh;             break;     // соответствие оборотов - токам 
                 case 15: gist_hol   = sua.read_float("gist_hol",0, 3,  params);  x = gist_hol;           break;     // гистерезис охлаждения 
                 case 16: gist_nagrev= sua.read_float("gist_nagrev",0,3,params);  x = gist_nagrev;        break;     // гистерезис нагрева            
                 case 17: uta[0]    = sua.read_uint("uta[0]",25, 105,   params);  u = uta[0];             break;     // фиксированные уставки температур в автомате
                 case 18: uta[1]    = sua.read_uint("uta[1]",25, 105,   params);  u = uta[1];             break;     // фиксированные уставки температур в автомате                                                                                
                 case 19: filtr_pres = sua.read_uint("filtr_pres",1,10, params);  u = filtr_pres;         break;      // фильтр по давлению                                                
                 case 20: uta[2]    = sua.read_uint("uta[2]",25, 105,   params);  u = uta[2];             break;     // фиксированные уставки температур в автомате 
                 case 21: uta[3]    = sua.read_uint("uta[3]",25, 105,   params);  u = uta[3];             break;     // фиксированные уставки температур в автомате                  
                 case 22: subgist   = sua.read_float("subgist",0, 1,    params);  x = String(subgist);    break;     // гистерезис дополнительный охлаждения 
                 case 23: zadanie_coll=sua.read_uint("zadanie_coll",15, 60,params);u = zadanie_coll;      break;     // задание температуры охлаждения ЭБУ
                 case 24: Kp_cool   =  sua.read_uint("Kp_cool",1, 25,   params);  u = Kp_cool;            break;     // КП ЭБУ
                 case 25: Ki_cool   =  sua.read_uint("Ki_cool",1, 40,   params);  u = Ki_cool;            break;     // КИ ЭБУ                                                   
                 case 26: U[0]      = sua.read_float("U[0]",90, 105,    params);  x = String(U[0]);       break;     // т.окончания процесса прямоток
                 case 27: U[1]      = sua.read_float("U[1]",90, 105,    params);  x = String(U[1]);       break;     // т.окончания процесса дистиляция    
      /*повтор*/ case 28: t_avto    = sua.read_uint("t_avto", 0, 105,   params);  u = t_avto;             break;     // задание температуры в автомате(105 чтобы не выключались ТЭНы)
                 case 29: U[3]      =  sua.read_float("U[3]",80, 95,    params);  x = String(U[3]);       break;     // т.отбора тела дистиляция
                 case 30: U[4]      =  sua.read_float("U[4]",80, 98,    params);  x = String(U[4]);       break;     // т.отбора хвостов прямоток(тело 2)
                 case 31: U[5]      =  sua.read_float("U[5]",80, 98,    params);  x = String(U[5]);       break;     // т.отбора хвостов дистиляция  
                 case 32: t_telo_On =  sua.read_uint("t_telo_On",1,20,  params);  u = t_telo_On;          break;     // дистиляция время открытия клапана сброса тела
                 case 33: t_telo_Off = sua.read_uint("t_telo_Off",1,40, params);  u = t_telo_Off;         break;     // дистиляция время закрытия клапана сброса тела              // 
                 case 34: t_dis_potOn =sua.read_uint("t_dis_potOn",1,20,params);  u = t_dis_potOn;        break;     // дистиляция время открытия клапана сброса голов 
                 case 35: t_dis_potOff=sua.read_uint("t_dis_potOff",1,40,params); u = t_dis_potOff;       break;    // дистиляция время закрытия клапана сброса голов                                                                    
                 case 36: Kd_cool   =  sua.read_uint("Kd_cool",1, 15,   params);  u = Kd_cool;            break;     // КD ЭБУ                 
               }
               if(u != 0){x = String(u);}                                   // усли не uint то float
              }break;// case 1
            case 2: {                                                                         // обработка команд  от кнопок
              bool b;                                                                         // булевская для кнопок
              switch(nums)                                                                    // номер кнопки из массива
                 {                  //  КНОПКИ
                 case 1:          break;  // выдать данные IP и    
                 case 2:  if(bit->mixer_avto){bit->mixer_avto = false; bit->mixer_timer = false;}             // с автомата на ручное        
                          else if(!bit->mixer_timer){bit->mixer_timer = true; }                               // если в ручном то ставим в таймер
                          else if(bit->mixer_timer){                                                          // из таймера включаем в автомат
                            bit->mixer_timer = false;                           
                            bit->mixer_avto = true;
                          }   
                          bit->mix_on = false;                                                                 // при любых телодвижениях принудительное вращение снимаем  
                          prefs.putBool("mixer_timer",bit->mixer_timer);
                          prefs.putBool("mixer_avto",bit->mixer_avto);  b = bit->mixer_avto;        break; 
                                   
                 case 3:  b = bit->state ^= 1;  prefs.putBool("state", b); states(); 
                          if(!bit->state){bit->auto_prog = false; prefs.putBool("auto_prog",bit->auto_prog);} break;  // остановить - запустить процесс 
                 case 4:  ds_kol = sua.new_sort_ds();   sua._ds_er_summ = 0;                        break;    // новый поиск с замещением датчиков DS18B20 
                 case 5:  b = bit->power_auto_t ^= 1; prefs.putBool("power_auto_t",b);              break;  // автомат-ручное стабилизатора мощности по температуре
                 //case 6:  mpx_k = adc1_get_raw(ADC1_CHANNEL_3);
                           //if(mpx_k < 256){prefs.putUInt("mpx_k", mpx_k);} else mpx_k = 150;          break;  // 
                 //case 7:  power_auto_p = !power_auto_p; prefs.putBool("power_auto_p", power_auto_p);  break;  // автомат-ручное стабилизатора мощности по давлению                
                 case 8:  {uint8_t on_step=0;   n_step_m = 25;
                          for( uint8_t ii = 0; ii < 8; ii ++ ){on_step|= pvk_step[ii].step_aktiv<<ii;};
                          if(bit->state && on_step!=0){b = bit->auto_prog ^= 1;
                              prefs.putBool("auto_prog",b); step = 0;}                                 // программа работы по паузам 
                              reverse_time = 0;
                          if(!bit->auto_prog){t_avto = prefs.getUInt("t_avto", 65);}}                                               break;  // восстанавливаем задание регулятору 
                 case 9:  b = bit->sleep_n ^= 1;     prefs.putBool("sleep_n",bit->sleep_n);break;  // спать дисплею 
                 case 10: mode_pvk++; if(mode_pvk>2){mode_pvk = 0;}   prefs.putUInt("mode_pvk", mode_pvk);  break;  // 0-затирание,1-потстил,2-дистилляция
                 case 11: bit->sequent  = true;                                                                                  break;  // оператор (продолжение программы пауз)                       
                 case 12: if(bit->mixer_avto || bit->mixer_timer){                                                  // принудительная прокрутка мешалки в автомате
                              bit->mix_on ^= 1;
                              if(bit->mix_on){mix_oper(uint16_t(f_conv_r) * 10);}
                              else mix_oper(0);
                          }                                                                                   
                          else if(!bit->mixer_timer && !bit->mixer_avto){                                         // мешалка в ручном                           
                              bit->mikserOnR ^= 1;                                   
                              Serial.println("mikserOnR - " + String(bit->mikserOnR));
                              if(bit->mikserOnR){mix_oper(uint16_t(f_conv_r) * 10);}
                              else mix_oper(0);                           
                          };                                                                        break;  // включить мешалку в ручном режиме
                 case 13: ust_default();                                                                                                        break;  // установка уставок по умолчанию         
                 case 14: b = bit->valve_avt ^= 1; prefs.putBool("valve_avt",bit->valve_avt);  
                          if(!bit->valve_avt){gpio_set_level(valve_holy, 1);}                       break;  // клапан в автомате (видимо инверсный клапан)               
                 case 15: if(bit->state){bit->bool_imitazia ^= 1;}
                          else{if(bit->bool_imitazia) bit->bool_imitazia=false;};                           x = String(); break;  // имитация процесса 
                 case 16: if(!bit->valve_avt){gpio_set_level(valve_holy, !gpio_get_level(valve_holy));};  
                          if(gpio_get_level(valve_holy)){chart_metka(15);} else{chart_metka(10);}   break;  // ручное переключение клапана охлаждения                   
                 case 17: b = bit->shot_golov ^= 1;  prefs.putBool("shot_golov",b);                 break;  // дистиляция залповый сброс голов
                 case 18: if(ds_kol != 0){sua.write_addr_DS18B20();};                               break;  // запись найденных датчиков в память 
                 case 19: b = bit->r_golova ^= 1;                                                   break;  // открыть клапан голов в ручном
                 case 20: b = bit->r_hvost ^= 1;                                                    break;  // открыть клапан хвостов в ручном
                 case 21: b = bit->no_ten ^= 1;  prefs.putBool("no_ten" ,bit->no_ten);  break;// работа на газу без ТЭН
                 case 27: ESP.restart();                                                                                          x = String();  break;  // reset
                 case 28: sua.muzon();                                                                                            x = String();  break;  // музыка                                                                    
                 case 29: digitalWrite(33, HIGH); sleep(1); digitalWrite(33, LOW);                                                x = String();  break;  // тест УЗО (отключение) 
                 case 30: b = bit->as_ts_on ^= 1; prefs.putBool("as_ts_on",bit->as_ts_on);          break;  // использование защит и сигнализации                                  
                 case 31: b = bit->r_telo ^= 1;                                                     break;  // открыть клапан тела в ручном
                 case 32: b = bit->shot_distil ^= 1; prefs.putBool("shot_distil",bit->shot_distil); break;  // дистиляция залповый сброс тела 
                 case 33: b = sensor_ss ^= 1; prefs.putBool("sensor_ss" ,sensor_ss);                break;  // использовать датчик приёмной ёмкости
                 case 34: b = sensor_hoop ^= 1;   prefs.putBool("sensor_hoop" ,sensor_hoop);        break;  // использовать датчик уровня в рубашке   
                 case 35: b = bit->voice ^= 1; prefs.putBool("voice" ,bit->voice);                  break;  // включен ли голос  
                 case 36: b = bit->relay1 ^= 1; relay(1,0,(bit->relay1 ^= 1) ? 0xFF00 : 0,5);       break;  // реле платы расширения
                 case 37: b = bit->relay2 ^= 1; relay(1,0,(bit->relay2 ^= 1) ? 0xFF00 : 0,5);       break;  // реле платы расширения 
                 case 38: b = bit->relay3 ^= 1; relay(1,0,(bit->relay3 ^= 1) ? 0xFF00 : 0,5);       break;  // реле платы расширения 
                 case 39: b = bit->relay4 ^= 1; relay(1,0,(bit->relay4 ^= 1) ? 0xFF00 : 0,5);       break;  // реле платы расширения                  
                 //case 36: b = bit->relay1 ^= 1; if(b){relay(1,0,0xFF00,5);}else relay(1,0,0,5); break;  // реле платы расширения
                 //case 37: b = bit->relay2 ^= 1; if(b){relay(1,1,0xFF00,5);}else relay(1,1,0,5); break;  // реле платы расширения 
                 //case 38: b = bit->relay3 ^= 1; if(b){relay(1,2,0xFF00,5);}else relay(1,2,0,5);break;  // реле платы расширения 
                 //case 39: b = bit->relay4 ^= 1; if(b){relay(1,3,0xFF00,5);}else relay(1,3,0,5);break;  // реле платы расширения                                                                     
                 case 40: b = bit->auto_relay ^= 1;                                                 break;  // автомат-ручное платы расширения
                 case 41: relay(0,0,1,3);        /*00 03 00 00 00 01 85 DB  */                                                    x = String();  break;  // запрос адреса платы
                 case 42: relay(1,0,8,1);        /*FF 01 00 00 00 08 28 12*/                                                      x = String();  break;  // запрос состояния реле                
                 case 43: sua.read_addr_DS18B20();                                                  break;// dhtvtyyj
// void relay(unsigned char DevID, uint16_t number, uint16_t on_off, unsigned char funz)  
                 default: sua.beep(2500,100); sua.beep(2700,100);                                             // оповещение о неправильной команде                    
                 } x = b ? "1" : "0";                                 // конвертируем булевский в строку
                }
                break;
            case 3: if(sua.sort_ds18b20(nums, params)){ sua.beep(2500,100);}
                    else{sua.beep(2500,100); sua.beep(2700,100);};                                        break;  // сортировка датчиков
                                  
            case 4:{    //********     ОБРАБОТКА КОМАНД ШАГОВ ПАУЗ     **************** 
                //Serial.println("Обработка пауз!");
                bool p = false;                                               // ответный сигнал для пауз
                uint16_t v{};
                uint8_t c{};                
                const char *sds;
                String z;
                uint8_t n = nums-1;   

                if(n>=0 && n<=7){                                     // активность шага              
                      z = "step_aktiv" + String(n);
                      sds = z.c_str();
                      p =  pvk_step[n].step_aktiv ^= 1;
                      prefs.putBool(sds, p);
                      c = 1;
                    }
                else if(n>=8 && n<=15){                               // вызов оператора
                      n = n-8;                                    
                      z = "oper" + String(n);
                      sds = z.c_str();
                      p = pvk_step[n].oper ^= 1;
                      prefs.putBool(sds, p);
                      c = 1;
                }   
                else if(n>=16 && n<=23){                              // мешалка
                      n = n-16;          
                      z = "meh" + String(n);
                      sds = z.c_str();
                      p = pvk_step[n].meh ^= 1;
                      prefs.putBool(sds, p);
                      c = 1;   
                }                             
                else if(n>=24 && n<=31){                              // температура паузы
                      n = n-24;    
                      z = "t_zad"+String(n);
                      sds = z.c_str();                    
                      v = sua.read_uint(sds,20,105,params); 
                      pvk_step[n].t_zad = v;
                      c = 2;                      
                }
                else if(n>=32 && n<=39){                              // время паузы
                      n = n-32;                 
                      z = "t_time" + String(n);
                      sds = z.c_str();
                      v = sua.read_uint(sds,0,120,params);
                      pvk_step[n].t_time = v;
                      c = 2;                      
                }
                if     (c == 1){x = String(p);}
                else if(c == 2){x = String(v);}     
            }  break;                                                 // case 4: 
          };
        //  if(c_comand[1] != 28) sua.beep(2500,100);                 // звук нормально принятой команды
        sua.beep(2500,100);                                           // звук нормально принятой команды
        TFT_refresh(id_forms);                                        // работа с дисплеем             
        if(!nos){send_port();} else{ nos = false;};                                                  // после проверки контрольной суммы
        bit->send_port_off = true;                                    // ставим флаг пропуска визуализации        
      } break;                                                        //  case 10:  
    } 
  return x;
 } // switch(w)
//--------------------------------------------------------------------*-----------------------------------------------------------