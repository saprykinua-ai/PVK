#include "header.h"
#include "com_port_Nextion.h"
#include "strukt.h"
#include <vector>
 #define VEKTOR 490

std::vector<uint8_t> VectorKUB (VEKTOR,  0);                                 // объявляем вектор размером в 600 элементов и инициализируем их нулями
std::vector<uint8_t> VectorEBU (VEKTOR,  0);                                 // объявляем вектор
std::vector<uint8_t> VectorZT  (VEKTOR,  0);                                 // объявляем вектор
std::vector<uint8_t> VectorPres(VEKTOR,  0);                                 // объявляем вектор
//----------------------------------------------------------------------*-----------------------------------------------------------  
void fff(){Serial2.print((String)char(255)+char(255)+char(255));}
//----------------------------------------------------------------------*-----------------------------------------------------------
/*void history(uint16_t n){                                               // история для графиков
  for(int i = n-3; i > 3; i--){                                         // -1 чтобы в первом байте не выводить ошибку(отсекаем первое и последнее значение)
    Serial2.print("add 25,0,"  + String(VectorKUB[i]));  fff(); 
    Serial2.print("add 25,1,"  + String(VectorEBU[i]));  fff(); 
    Serial2.print("add 25,2,"  + String(VectorZT[i]));   fff();      
    Serial2.print("add 25,3,"  + String(VectorPres[i])); fff();     
   };
}*/
//----------------------------------------------------------------------*-----------------------------------------------------------
void add_vector(){

 std::rotate(VectorKUB.begin(),  VectorKUB.begin() + 1, VectorKUB.end()); // графики предисторияVectorKUB.begin()   + 250
 std::rotate(VectorEBU.begin(),  VectorEBU.begin() + 1, VectorEBU.end()); // сдвигаем массив в право на 1 элемент
 std::rotate(VectorZT.begin(),  VectorZT.begin() + 1, VectorZT.end());    
 //std::rotate(VectorPres.begin(), VectorPres.begin() + 1,VectorPres.end()); //  VectorPres.begin()  + 250

 VectorKUB.back()  = static_cast<uint8_t>(T[0]);                        // Запись в конец (индекс [250] для размера 251)
 VectorEBU.back()  = static_cast<uint8_t>(T[1]);                        // Теперь записываем новое значение в начало
 VectorZT.back()        =                 t_avto;                        // static_cast<uint8_t>(T[0]);
 //VectorPres.back() = static_cast<uint8_t>(pres);                        // VectorPres[0]VectorPres.back()= uint8_t(pres);
}
/*void sendChartDataBatched() {                                           // Альтернативный вариант: отправка пачкой для увеличения скорости
 
    String command = "";                                                // Формируем одну большую команду

    const size_t startIdx = BUFFER_OFFSET;
    const size_t endIdx = min(chartPoints, HISTORY_SIZE) - BUFFER_OFFSET;
    
      for(size_t i = startIdx; i < endIdx; i++) {
            if(i < VectorKUB.size()) {command += "add 25,0," + String(VectorKUB[i]) + "\xFF\xFF\xFF";}
            if(i < VectorEBU.size()) {command += "add 25,1," + String(VectorEBU[i]) + "\xFF\xFF\xFF";}
            if(i < VectorZT.size()) {command += "add 25,2," + String(VectorZT[i]) + "\xFF\xFF\xFF";}
      }
        
        if(command.length() > 500 || i == endIdx - 1) {                 // Отправляем пачкой по 50 команд
            Serial2.print(command);
            command = "";
            delay(10); // Даем время Nextion на обработку
        }      
    
    Serial2.print("ref 25\xFF\xFF\xFF");    // обновление дисплея
}*/

//----------------------------------------------------------------------*-----------------------------------------------------------
void Serial_read_Nextion()                                              // чтение из Nextion удалённого дисплея
 {
   delay (15);                                                          // (нужно обязятельно) чтобы успел прийти весь запрос
   uint8_t com = Serial2.available();                                   // колличество байт в порту
   unsigned char tmp = (char)Serial2.read();                            // видимо первый байт
   //Serial.println("ТМР   " + String(tmp));
   //Serial.println("Колличество   " +String(com));

  if((com % 4 == 0)&&(tmp == 26)){                                      // пакет кратный 4, читаем первые 4 байта(26==1А)
      for (int n=0; n < 3; n++)  c_nex[n] = (char)Serial2.read();       // читаем в массив  
       
      if((c_nex[0] == 255)&&(c_nex[1] == 255)&&(c_nex[2] == 255)){      // ответ дисплея при ошибке команды 1A
            if(id_forms != 7 && id_forms != 6){                         // формы клавиатуры и видео
                  Serial2.print("page " + String(id_forms));  fff();    // синхронизируем формы
                  Serial.println("Форма  -  " + String(id_forms));
            }
      }
   }
   else if((com == 7) || (com == 5)){COM_read_Nextion(com, tmp);}      //   
   else if(com > 10){sua.wifi_password(com, false, tmp);}  

   while (Serial2.available()) Serial2.read();                          // очищаем порт                        // очищаем порт
 }
//----------------------------------------------------------------------*-----------------------------------------------------------
void COM_read_Nextion(uint8_t wbt, unsigned char tmp)                   // обработка данных портов Nextion
 {
  if(!wbt) return;     
  c_nex[0] = tmp;                                                       // первый байт признак действия 
  for (int n=1; n < wbt; n++)  c_nex[n] = (char)Serial2.read();         // читаем в массив

  switch(wbt)      
   {       
      case 7: 
           uint16_t tmp;
            if(c_nex[0]==3){tmp = c_nex[2];}                            // если не сортировка DS то переназначаем номер nextiona в номер скады
            else{ sua.bit4.arr[0] = c_nex[3]; sua.bit4.arr[1] = c_nex[4];// вычисляем присланное значение для эдит
                  tmp = sua.bit4.bit4_16;                                     // вычисляем присланное значение для эдит
            }
            if((c_nex[0]==1)||(c_nex[0]==2))c_nex[1] = comand[c_nex[1]][c_nex[2]];// обработка кнопок и эдитов(вычисляем по таблице)
            COM_read_BT(wbt, tmp, c_nex[1], c_nex[0]);                  // передаём обработку из Nextion
            TFT_refresh(id_forms);
      break;
        //------------------------------     при переключении форм "sendme"     ---------------------------------------                     
      case 5: if((c_nex[0] == 102)&&(c_nex[2] == 255)&&(c_nex[3] == 255)&&(c_nex[4] == 255)){
                   id_forms = c_nex[1];    
                   if(forms_id_m != id_forms){                          // признак смены формы
                        bit->id_f = true; forms_id_m = id_forms;        // номер открытой формы 
                        Serial.println("Форма  " + String(id_forms));
                        if(id_forms == 0){
                              Serial.println("Дописываем график ghart = " + String(ghart));    
                              //history(ghart); 
                              ghart = 0;
                        };  
                   };                                                   // сброс времени засыпания дисплея
                   TFT_refresh(id_forms);                               // обновление страниц
                 }
      break;   
    };                                                                  //   switch(w)
 }
//----------------------------------------------------------------------*-----------------------------------------------------------
void TFT_refresh(uint8_t forms_id)                                      // 
 {
  //Serial.println("Номер формы  " + String(forms_id));
  switch(forms_id)   
   {                        // домашняя страница
     case 0:
            bit_x.x_ar[0] = meh_on;
            bit_x.x_ar[1] = meh_off;
            bit_x.x_ar[2] = t_avto;            
            bit_x.x_ar[3] = uint8_t(power_real/2.55);                   // мощность тена в %
            Serial2.print("h.xA.val=" + String(bit_x.x_32));            fff();

            //Serial.println("power_real NEXTION = " + String(power_real));

            bit_x.x_ar[0] = as;
            bit_x.x_ar[1] = ts;
            if(bit->auto_prog){bit_x.x_ar[2] = pvk_step[step].t_zad;}
            else if(bit->power_auto_t){bit_x.x_ar[2] = t_avto;}
            else  bit_x.x_ar[2] = power_r;
            bit_x.x_ar[3] = mode_pvk;                                       // 0-затирание,1-потстил,2-дистилляция         
            Serial2.print("h.xC.val=" + String(bit_x.x_32));            fff();            

            bit_x.x_ar[0] = uint8_t(e_tenA*10);                         // ток тэна фазы A
            bit_x.x_ar[1] = uint8_t(e_tenB*10);                         // ток тэна фазы Б
            bit_x.x_ar[2] = uint8_t(e_tenC*10);                         // ток тэна фазы C   
            bit_x.x_ar[3] = uint8_t(T[1]);                              // температула ЭБУ 
            Serial2.print("h.xD.val=" + String(bit_x.x_32));            fff();                
            
            //if(bit->auto_prog){
            bit_x.x_ar[0] = step + 1;                                   // номер шага
            bit_x.x_ar[1] = pvk_step[step].t_zad;                       // температура задания
            bit_x.x_ar[2] = pvk_step[step].t_time;                      // время паузы   
            bit_x.x_ar[3] = power_r;                                    // ручное задание мощности в %
            Serial2.print("h.xE.val=" + String(bit_x.x_32));            fff();   

            bit_x.x_ar[0] = uta[0];                                     // уставки температур в автомате
            bit_x.x_ar[1] = uta[1];                                     // уставки температур в автомате
            bit_x.x_ar[2] = uta[2];                                     // уставки температур в автомате
            bit_x.x_ar[3] = uta[3];                                     // уставки температур в автомате
            Serial2.print("h.xF.val=" + String(bit_x.x_32));            fff(); 

            Serial2.print("h.wav0.vid=" + String(audio_as));            fff();// номер файла музыки
            Serial2.print("h.va4.val=" + String(reverse_time));         fff();// 
            //Serial2.print("h.x8.val="  + String(uint16_t(pres*10)));    fff();// давление внутри ПВК 
            Serial2.print("h.x1.val="  + String(uint16_t(T[0]*10)));    fff();// температура внутри ПВК 
            Serial2.print("h.x11.val="  + String(uint16_t(T[2]*10)));   fff();// температура холодильника(дефлегматора) 
            Serial2.print("h.x12.val="  + String(uint16_t(T[3]*10)));   fff();// т. колонны (узел отбора)
  
            if(speed_mesh == 0){Serial2.print("h.x10.val=" + String(f_conv_r));fff();}   // когда частотник стоит, то выдаёт 0 то посылаем ручную настройку
            else {Serial2.print("h.x10.val=" + String(speed_mesh/10));  fff();};   // 

      break;
            //-----------------------         параметры        --------------------------------
     case 1:
            //for(uint8_t ds = 0; ds < ds_kol  ; ds++){     
            for(uint8_t ds = 0; ds < 4  ; ds++){
               Serial2.print("s.x"+String(ds) + ".val=" + String(uint16_t(T[ds]*10))); fff();
            }
            //Serial2.print("s.x7.val="  + String(ds18b20_er_summ));            fff();      // сумма ошибок DS18B20 
            Serial2.print("s.x7.val="  + String(sua._ds_er_summ));            fff();      // сумма ошибок DS18B20            
            Serial2.print("s.x10.val=" + String(hour_val));                   fff();      // часовой пояс                
            Serial2.print("s.t19.txt=\"" + sua._ip + "\"");           fff();      // IP АДРЕС ESP32               
            Serial2.print("s.wf_n.txt=\"" + String(ssid_ust) + "\"");         fff();      // имя точки доступа
            Serial2.print("s.wf_p.txt=\"" + String(password_ust) + "\"");     fff();      // ip адресс для браузера  ipstr  

            if(bit->voice){Serial2.print("s.b4.pic=47");                      fff();}     //
            else Serial2.print("s.b4.pic=48");                                fff();
                         
            bit_x.x_ar[0] = ds_kol;                                     // кол-во DS18B20 
            bit_x.x_ar[1] = max_scale;                                  // максимальная шкала преобразователя тока
            bit_x.x_ar[2] = k_mesh;                                     // % вращения мешалки от нагрузки
            bit_x.x_ar[3] = sua._ds_bit_er;                             // для определения какой далас сбоит
            Serial2.print("s.xA.val=" + String(bit_x.x_32));        fff();     

            bit_x.x_ar[0] = uta[0];                                     // уставки температур в автомате
            bit_x.x_ar[1] = uta[1];                                     // уставки температур в автомате
            bit_x.x_ar[2] = uta[2];                                     // уставки температур в автомате
            bit_x.x_ar[3] = uta[3];                                     // уставки температур в автомате
            Serial2.print("s.xB.val=" + String(bit_x.x_32));        fff();     

            Serial2.print("s.x14.val=" +  String(uint16_t(gist_hol*10)));     fff(); // гистерезис охлаждения      
            Serial2.print("s.x13.val=" +  String(uint16_t(gist_nagrev*10)));  fff(); // гистерезис нагрева            
            Serial2.print("s.x9.val=" +   String(uint16_t(subgist*10)));      fff(); // суб гистерезис охлаждения            
      break;
             //-----------            регуляторы                  ----------------------------
     case 2:
            bit_x.x_ar[0] = power_r;                                    // ручное задание мощности 
            bit_x.x_ar[1] = t_avto;                                     // задание температуры в автомате
            bit_x.x_ar[2] = KP_power;            
            bit_x.x_ar[3] = KI_power;                  
            Serial2.print("r.A.val=" + String(bit_x.x_32));        fff();            

            bit_x.x_ar[0] = uint8_t(power_real/2.55);                   // мощность тэна теперь в %
            bit_x.x_ar[1] = Kd_cool;                                    // ЭБУ
            //bit_x.x_ar[2] = ;            
            //bit_x.x_ar[3] = pres_z;                                   // задание давления в автомате 
            Serial2.print("r.B.val=" + String(bit_x.x_32));        fff();     
     
            bit_x.x_ar[0] = Kp_cool;                                    // ЭБУ
            bit_x.x_ar[1] = Ki_cool;                                    // ЭБУ
            bit_x.x_ar[2] = zadanie_coll;                               // ЭБУ
            bit_x.x_ar[3] = speed;                                      // ЭБУ скорость вращения     
            Serial2.print("r.C.val=" + String(bit_x.x_32));        fff();  

            bit_x.x_ar[0] = t_telo_On;                                  // дистиляция время открытия клапана сброса тела
            bit_x.x_ar[1] = t_telo_Off;                                 // дистиляция время закрытия клапана сброса тела
            bit_x.x_ar[2] = t_dis_potOn;                                // дистиляция время открытия клапана сброса голов 
            bit_x.x_ar[3] = t_dis_potOff;                               // дистиляция время закрытия клапана сброса голов   
            Serial2.print("r.D.val=" + String(bit_x.x_32));        fff();            

            Serial2.print("r.x11.val="+String(uint16_t(T[1]*10)));            fff();      // температура ЭБУ

            Serial2.print("r.x3.val="  +  String(uint16_t(U[0]*10)));         fff();      //             
            Serial2.print("r.x12.val=" +  String(uint16_t(U[1]*10)));         fff();      //
            //Serial2.print("r.x13.val=" +  String(uint16_t(U[2]*10)));         fff();      //
            Serial2.print("r.x14.val=" +  String(uint16_t(U[3]*10)));         fff();      //
            Serial2.print("r.x15.val=" +  String(uint16_t(U[4]*10)));         fff();      //
            Serial2.print("r.x16.val=" +  String(uint16_t(U[5]*10)));         fff();      //                                                            
         break;  
    case 3:  //-----------            АС-ТС                  ---------------------------- 
            Serial2.print("as.vaAS.val=" + String(as));                       fff();      // аварийная защита
            Serial2.print("as.vaTS.val=" + String(ts));                       fff();      // технологическая сигнализация   

            //bit_x.x_ar[0] = u_as_ebu;
            //bit_x.x_ar[1] = u_ts_ebu;
            //bit_x.x_ar[2] = u_ts_hol;            
            //bit_x.x_ar[3] = u_as_hol;
            //Serial2.print("as.xA.txt=\"" + String(bit_x.x_32)+"\"");        fff();

            Serial2.print("as.n0.val=" + String(u_as_ebu));              fff(); 
            Serial2.print("as.n1.val=" + String(u_ts_ebu));              fff();         
            Serial2.print("as.n2.val=" + String(u_ts_hol));              fff();                   
            Serial2.print("as.n3.val=" + String(u_as_hol));              fff();             
            
            Serial2.print("as.x0.val=" + String(uint16_t(T[1]*10)));     fff(); 

         break;       
    case 4: //-----------          паузы        -----------------------------------------------------------------
            uint8_t on_step, on_oper, on_mash;
            on_step = 0; on_oper = 0; on_mash = 0;
            
            for(uint8_t ii = 0; ii < 8; ii ++ ){
               on_step|= pvk_step[ii].step_aktiv << ii;
               on_oper|= pvk_step[ii].oper       << ii;
               on_mash|= pvk_step[ii].meh        << ii;
            };
            bit_x.x_ar[0] = on_step;
            bit_x.x_ar[1] = on_oper;
            bit_x.x_ar[2] = on_mash;            
            //bit_x.x_ar[3] = bit->auto_prog;                           // паузы в автомате
            Serial2.print("p.xA.val=" + String(bit_x.x_32));        fff();            
                        
            //Serial2.print("p.t10.txt=\"STEP " + String(step+1) + "\"");    fff();
            Serial2.print("p.t9.txt=\"" + String(step+1) + "\"");    fff();          
            Serial2.print("p.x1.val="  + String(uint16_t(T[0]*10)));       fff();    // температура внутри ПВК   

            bit_x.x_ar[0] = pvk_step[0].t_time;
            bit_x.x_ar[1] = pvk_step[1].t_time;
            bit_x.x_ar[2] = pvk_step[2].t_time;            
            bit_x.x_ar[3] = pvk_step[3].t_time;
            Serial2.print("p.xC.val=" + String(bit_x.x_32));        fff();              

            bit_x.x_ar[0] = pvk_step[4].t_time;
            bit_x.x_ar[1] = pvk_step[5].t_time;
            bit_x.x_ar[2] = pvk_step[6].t_time;            
            bit_x.x_ar[3] = pvk_step[7].t_time;
            Serial2.print("p.xD.val=" + String(bit_x.x_32));        fff();                    

            bit_x.x_ar[0] = pvk_step[0].t_zad;
            bit_x.x_ar[1] = pvk_step[1].t_zad;
            bit_x.x_ar[2] = pvk_step[2].t_zad;            
            bit_x.x_ar[3] = pvk_step[3].t_zad;
            Serial2.print("p.xE.val=" + String(bit_x.x_32));        fff();               

            bit_x.x_ar[0] = pvk_step[4].t_zad;
            bit_x.x_ar[1] = pvk_step[5].t_zad;
            bit_x.x_ar[2] = pvk_step[6].t_zad;            
            bit_x.x_ar[3] = pvk_step[7].t_zad;
            Serial2.print("p.xF.val=" + String(bit_x.x_32));        fff();              
    break;      
    case 5: //-----------          спим        -----------------------------------------------------------------     
            Serial2.print("e.x0.val="  + String(uint16_t(T[0]*10))); fff();   // температура внутри ПВК 
            Serial2.print("e.va4.val=" + String(reverse_time));      fff();   // оставшееся время паузы            
    break;
    };
   Serial2.print("sys2=" + String(data_bit));                        fff();   // передача бинарника(и синхронизируем дисплей)   
   Serial2.print("sys1=" + String(data_bit2));                        fff();   // передача бинарника(и синхронизируем дисплей)   
   bit->id_f = false;                                                         // сброс признака смены формы
 }
//----------------------------------------------------------------------*-----------------------------------------------------------