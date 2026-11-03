 #include "setup.h"    
 #include "header.h"
 #include <ElegantOTA.h>
 #include <Arduino.h>
 #include <ESPAsyncWebServer.h>
 //#include "esp32-hal-ledc.h"
 #include <ArduinoJson.h> 

 HardwareSerial MBSerial(1);
 AsyncWebServer server(80); 

 #define BUILD_VERSION PROJECT_NAME "_" BUILD_DATE                    // версия прошивки
 #define BUZER        GPIO_NUM_13                                     // пищалка
 #define faza_A       GPIO_NUM_12                                     // шим нагрева ТЭН(плата версии 3) 
 #define faza_B       GPIO_NUM_14                                     // шим нагрева ТЭН(плата версии 3) 
 #define faza_C       GPIO_NUM_18   
 #define PWM_HEATER   GPIO_NUM_22                                     // шим вентилятора ЭБУ 
 FILE *f;
//--------------------------------------------------------------------*-------------------------------------------------
void MBSerial_read()                                                  // чтение из Serial
 {
  if(MBSerial.available()>0){
    uint8_t mb = 0;
    //unsigned char mb1[50];                                            // команда для порта
    //unsigned char mb2[50];                                            // команда для порта    

    delay (60);                                                       // чтобы успел прийти весь запрос 
    mb = MBSerial.available();
    Serial.println("Пришло в MBSerial - " + String(mb));

    unsigned char mb1[mb];                                            // команда для порта
    unsigned char mb2[mb];                                            // команда для порта 

    for(int n=0; n < mb; n++){                                        // читаем в массив любое колличество (MBSerial.flush() не работает) 
        mb1[n] = (char)MBSerial.read();
    }; 
    
    /*Serial.println("-----------------");
    for(int n=0; n < mb; n++){Serial.println(String(mb1[n]));}; 
    Serial.println("-----------------");*/

    if((mb == 18) ||(mb == 9)){
        unsigned short crc = myCRC(mb1,6);                            // проверка контрольной суммы приходящего пакета 
        //Serial.println("crc - " + String(crc));
        if((mb1[6] == uint8_t(crc&0xFF)) and (mb1[7] == crc>>8)){     //Serial.println("Контрольная сумма совпала CRC 9");
            //for(int n=0; n < 9; n++){Serial.println(String(mb1[n]));};  

            if(mb1[3] == 2){speed_mesh = ((uint16_t(mb1[4]))<<8) + uint8_t(mb1[5]);}// частота преобразователя
            else if(mb1[3] == 3){
                if     (mb1[5] == 1) bit->mesh_on_off = true;         // 1-пуск, 2-стоп, 3-реверс
                else if(mb2[5] == 2) bit->mesh_on_off = false;   
            };
          if(mb == 18){
            for(int n=0; n < 9; n++){mb2[n] = mb1[n+9];};             //Serial.println(String(mb2[n])); 

            crc = myCRC(mb2,6);                                       // проверка контрольной суммы приходящего пакета 
            if((mb2[6] == uint8_t(crc&0xFF)) and (mb2[7] == crc>>8)){ // контрольная сумма//Serial.println("Контрольная сумма совпала CRC 18");
                if     (mb2[5] == 1){bit->mesh_on_off = true;}        // 1-пуск, 2-стоп, 3-реверс
                else if(mb2[5] == 2){bit->mesh_on_off = false;};            
            };
          };
        };
    }
    else if(mb == 23){                                                // если пакет не равен 9 или 18 байтам
        unsigned short crc = myCRC(mb1,21);                           // проверка контрольной суммы приходящего пакета     
        if((mb1[21] == uint8_t(crc&0xFF)) and (mb1[22] == crc>>8)){
            //Serial.println("Контрольная сумма совпала CRC 23");
            speed_mesh = ((uint16_t(mb1[3]))<<8) + uint8_t(mb1[4]);   // частота преобразователя   
            //Serial.println("speed_mesh = " + String(speed_mesh));        

            if(mb1[6] == 1)bit->mesh_on_off = true;                   // 1-пуск, 2-стоп, 3-реверс
            else if(mb1[6] == 2) bit->mesh_on_off = false;
            //Serial.println("mesh_on_off = " + String(bit->mesh_on_off));  
            // u_load1= mb1[15];                                      // напряжение выпрямителя преобразователя         
            // u_load2= mb1[16]; 
            loadF1 = mb1[17];
            loadF2 = mb1[18];
            temper = mb1[20];  
        };
    }
    else if(mb == 7){                                                 // read адрес платы реле, у нас 01
      unsigned short crc = myCRC(mb1,5);                              // проверка контрольной суммы приходящего пакета 
      if((mb1[5] == uint8_t(crc&0xFF)) and (mb1[6] == crc>>8)){  
          num_dev_relay = mb1[4];                                     // 5 byte в ответе является адресом платы расширения
          Serial.println("Адрес платы реле = " + String(num_dev_relay));
      };
    }
    else if(mb == 8){                                                 // ответ от команды (03 05 00 03 FF 00) включить реле №3  
      unsigned short crc = myCRC(mb1,6);                              // проверка контрольной суммы приходящего пакета
      if((mb1[6] == uint8_t(crc&0xFF)) and (mb1[7] == crc>>8)){       
   

        //3–4-й байт адрес реле. Адреса реле 1–8 соответственно: 0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007.
        //5–6-й байт переданного кадра представляет собой данные: 0xFF00 означает включение реле, 0x0000 — выключение реле.
        Serial.println("управление реле");       
      };
    }
    else if(mb == 6){                                                 // запрос состояния реле,The 4th byte of the Return frame,the Bit0--Bit7 of 0x01 representing relay 1-relay 8, 0 is turn off .1 is turn on.     
      unsigned short crc = myCRC(mb1,4);                              // проверка контрольной суммы приходящего пакета
      if((mb1[4] == uint8_t(crc&0xFF)) and (mb1[5] == crc>>8)){      
          if(mb1[3] & 1 << 0){bit->relay1 = true;} else bit->relay1 = false; // реле №1 платы расширения
          if(mb1[3] & 1 << 1){bit->relay2 = true;} else bit->relay2 = false; // реле №2 платы расширения
          if(mb1[3] & 1 << 2){bit->relay3 = true;} else bit->relay3 = false; // реле №3 платы расширения
          if(mb1[3] & 1 << 3){bit->relay4 = true;} else bit->relay4 = false; // реле №4 платы расширения   
          Serial.println("Состояние реле");          
      };     
    };
  } 
 }
//--------------------------------------------------------------------*-------------------------------------------------
String processor(const String& var){
    if (var == "T0")  {return String(T[0]);}
    if (var == "T1")  {return String(T[1]);}
    if (var == "T2")  {return String(T[2]);}
    if (var == "T3")  {return String(T[3]);}            
  return String();
}
//--------------------------------------------------------------------*-----
void setupPWM() {                                                     // конфигурация ШИМ и каналов
  pinMode(faza_A, OUTPUT);
  pinMode(faza_B, OUTPUT);
  pinMode(faza_C, OUTPUT);

  ledcSetup(0,1,8);                                                  // (управление ШИМ ТЭНа) номер канала (0), частоту ШИМ (1 Гц) и разрешение (8 бит).
  ledcSetup(1,1,8);                                                  // (управление ШИМ ТЭНа) номер канала (0), частоту ШИМ (1 Гц) и разрешение (8 бит).
  ledcSetup(2,1,8);                                                  // (управление ШИМ ТЭНа) номер канала (0), частоту ШИМ (1 Гц) и разрешение (8 бит).      
  ledcSetup(3,1000,8);                                               // BUZER номер канала (4), частоту ШИМ (1 КГц) и разрешение (8 бит).   
  ledcSetup(8,20000,8);                                              // (вентилятор ЭБУ) номер канала (8), частоту ШИМ (20 КГц) и разрешение (8 бит).  

  ledcAttachPin(BUZER,  3);                                           // (пищалка)привязать наш канал ШИМ (4) к какому-либо выводу GPIO 13    
  ledcAttachPin(PWM_HEATER, 8);                                       // (вентилятор ЭБУ)привязать наш канал ШИМ (8) к какому-либо выводу GPIO 22      
 // конфигурация входов АЦП для токов фаз
  adc1_config_width(ADC_WIDTH_BIT_12);                                // АЦП 12 бит
  adc1_config_channel_atten(ADC1_CHANNEL_3,ADC_ATTEN_DB_12);          // (давление)GPIO39, вход 1,1 вольт делитель 1/3.6 (120ом-20мА=2,4в)   
  adc1_config_channel_atten(ADC1_CHANNEL_6,ADC_ATTEN_DB_12);          // GPIO34, вход 1,1 вольт делитель 1/3.6 (120ом-20мА=2,4в)
  adc1_config_channel_atten(ADC1_CHANNEL_0,ADC_ATTEN_DB_12);          // GPIO36, вход 1,1 вольт делитель 1/3.6 (120ом-20мА=2,4в)  
  adc1_config_channel_atten(ADC1_CHANNEL_7,ADC_ATTEN_DB_12);          // GPIO35, вход 1,1 вольт делитель 1/3.6 (120ом-20мА=2,4в) 

   // установка режима ввода-вывода
  gpio_pad_select_gpio(LEVEL_SS);                                     // уровень приёмной ёмкости
  gpio_pad_select_gpio(valve_golov);                                  // клапан голов при дистилляции
  gpio_pad_select_gpio(valve_telo);                                   // клапан тела      
  gpio_pad_select_gpio(LEVEL_HOOP);                                   // уровень в рубашке
  gpio_pad_select_gpio(valve_hvost);                                  // клапан хвостов      
  gpio_pad_select_gpio(AVTOMAT);                                      // автомат безопасности  
  gpio_pad_select_gpio(valve_holy);                                   // клапан охлаждения    
  gpio_pad_select_gpio(valve_gas);                                    // клапан блокировки газа    

   // конфигурация выходов на вход или выход
  gpio_set_direction(LEVEL_SS, GPIO_MODE_INPUT);                      // датчик уровня приёмной ёмкости (вход)   
  gpio_set_direction(LEVEL_HOOP, GPIO_MODE_INPUT);                    // уровень в рубашке (вход)
  gpio_set_direction(valve_golov, GPIO_MODE_INPUT_OUTPUT);            // клапан голов (активный вход-выход для определения состояния входа)
  gpio_set_direction(valve_telo, GPIO_MODE_INPUT_OUTPUT);             // клапан тела  (активный вход-выход для определения состояния входа)      
  gpio_set_direction(valve_hvost, GPIO_MODE_INPUT_OUTPUT);            // клапан хвостов(активный вход-выход для определения состояния входа) 
  gpio_set_direction(AVTOMAT, GPIO_MODE_OUTPUT);                      // (активный выход) автомат безопасности     
  gpio_set_direction(valve_holy, GPIO_MODE_INPUT_OUTPUT);             // (активный вход-выход) клапан охлаждения     
  gpio_set_direction(valve_gas, GPIO_MODE_INPUT_OUTPUT);              // (активный вход-выход) клапан блокировки газа           

  gpio_set_pull_mode(LEVEL_SS, GPIO_PULLUP_ONLY);                     // уровень в приёмной ёмкости СС(подтяжка к VCC)
  gpio_set_pull_mode(LEVEL_HOOP, GPIO_PULLUP_ONLY);                   // подтяжка к VCC
  gpio_set_level(valve_holy, 1);                                      // клапан охлаждения, сразу выключаем(реле работают инверсно)  
}
//--------------------------------------------------------------------*-------------------------------------------------
void web_cod()
{
 server.on("/data", [](AsyncWebServerRequest *request){               // запрос идёт только в начале для передачи адреса и порта
  IPAddress remoteIP = request->client()->remoteIP(); 
  ip_indy = remoteIP.toString();
  if(ip_indy != "") ip_adr_on = true;
  else  ip_adr_on = true;

  if(request->hasParam("port")) {                                     // 2. Ищем параметр "port" в URL и сохраняем его
         port_indy = request->getParam("port")->value().toInt();
  }
  //Serial.println("port_indy = " + String(port_indy));
  String json = "";
  for(uint8_t n = 0; n < BUFSIZE; n++){json += String(Tdata[n]) + ",";};
  json += "";
  request->send(200, "application/json", json);
 });
//-------------------------------------------------------------------------
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
  request->send(SPIFFS, "/index.html", String(), false);
 });
//------------------  сортировка датчиков из программы  -------------------------------------------------------
  server.on("/sort_DS", [](AsyncWebServerRequest *request){           //HTTP_POST, 
  if (request->params() == 0) return;  
  const AsyncWebParameter *p = request->getParam((size_t)0);
  String com = p->value();
  uint8_t web[4] = {0, 0, 0, 0};
  Serial.println("com = " + com);
  Serial.println("com-length = " + String(com.length()));  

  const char* ptr = p->value().c_str();                               // Получаем указатель на начало строки
  for (int i = 0; i < 4; i++) {
    web[i] = (unsigned char)strtoul(ptr, (char**)&ptr, 10);           // Читаем число
    if (*ptr == ',') ptr++;                                           // Пропускаем запятую
  }
	/*int i = 0;
	while (int ID = com.indexOf(',')) {                                 // ищем символ
    if(ID == -1) break;
    //Serial.println("ID- "+String(ID) + "  " +  com.substring(0, ID));
	    web[i] = com.substring(0, ID).toInt();                          // SubString(1, ID - 1).ToInt();
			i++;
      com.remove(0, ID + 1);
	}*/
  Serial.println("web -  " + String(web[1]) + "" + String(web[1]));
  sua.sort_ds18b20(web[1], web[2]); sua.beep(2500,100);    // сортировка датчиков

  JsonDocument root;
  String json;
 
  if(web[3] == 6)                                                     // если запрос из программы
  {
     //json = String(web[1]) + "," + String(dt) + ",";
     json = String(web[1]) + "," + "11" + ",";     
  }
  else serializeJson(root,json);

 request->send(200, "application/json", json);                        // после нажатия кнопок возвращает пакет данных
 }); 
//------------------  для телефона (браузера) управление с кнопок  -------------------------------------------------------
 server.on("/button", [](AsyncWebServerRequest *request){             //HTTP_POST, 
 if (request->params() == 0) return;  
  const AsyncWebParameter *p = request->getParam((size_t)0);
  String com = p->value();
  unsigned char web[3] = {0, 0, 0};

  const char* ptr = p->value().c_str();                               // Получаем указатель на начало строки
  for (int i = 0; i < 3; i++) {
    web[i] = (unsigned char)strtoul(ptr, (char**)&ptr, 10);           // Читаем число
    if (*ptr == ',') ptr++;                                           // Пропускаем запятую
  }
  String json = COM_read_BT(7, 0, web[1], web[0]); 
  request->send(200, "application/json", json);                              // после нажатия кнопок возвращает пакет данных
 }); 
 //------------------  паузы  -------------------------------------------------------
  server.on("/pauses", [](AsyncWebServerRequest *request){            //HTTP_POST, 
  if (request->params() == 0) return;   
  const AsyncWebParameter *p = request->getParam((size_t)0);
  String com = p->value();
  unsigned char web[3] = {0, 0, 0};
  const char* ptr = p->value().c_str(); // Получаем указатель на начало строки

  for (int i = 0; i < 3; i++) {
    web[i] = (unsigned char)strtoul(ptr, (char**)&ptr, 10); // Читаем число
    if (*ptr == ',') ptr++;                                           // Пропускаем запятую
  }
  String json = COM_read_BT(7, 0, web[1], web[0]); 
  request->send(200, "application/json", json);                       // (Правильный MIME-тип) после нажатия кнопок возвращает состояние кнопок
 });  
 //------------------  для  значения EDIT  -------------------------------------------------------
 server.on("/edit", HTTP_POST, [](AsyncWebServerRequest *request){
  if (request->params() == 0) return;  
  const AsyncWebParameter *p = request->getParam((size_t)0);
  String com = p->value();

  Serial.println("com -  " + String(com));
  unsigned char web[5] = {0, 0, 0, 0, 0};                             // Обнуляем массив для безопасности
  char buf[32];
  com.toCharArray(buf, sizeof(buf)); 
  char* token = strtok(buf, ",");                                     // Заменили p на token
  int i = 0;
  while (token != NULL && i < 5) {
      web[i++] = (unsigned char)atoi(token); 
      token = strtok(NULL, ",");
  }
  sua.bit4.arr[0] = web[2]; sua.bit4.arr[1] = web[3];                 // вычисляем присланное значение для эдит
  String json = COM_read_BT(7, sua.bit4.bit4_16, web[1], web[0]);   
  //JsonDocument root; 

  //String dt;
 // switch(web[1])
 // {
    //  case 2:   dt = String(bit->auto_rect);                                                break; // регулятор мощности (автомат - ручное)    
    //case 1:   root["TMR"] = String(meh_on);         break; 
    //case 2:   root["TMP"] = String(meh_off);        break; 
    //case 3:   root["KPW"] = String(KP_power);    break; 
    //case 4:   root["KPW"] = String(KP_power);    break; 
   // default: root["edit"] = "55";
 // }

  //if(web[4] == 6){ json = x_st;}                                      // если запрос из программы
  //else serializeJson(root,json);                                      // ответ web браузеру
  request->send(200, "application/json", json);                       // возвращает пакет данных, раньше было так //request->send(200, "text/json", json); 
 }); 
 //------------------  для  значения EDIT PAUSES  -------------------------------------------------------
  server.on("/e_pauses", HTTP_POST, [](AsyncWebServerRequest *request){
  if (request->params() == 0) return;  
  const  AsyncWebParameter *p = request->getParam((size_t)0);
  String com = p->value();
  unsigned char web[5] = {0, 0, 0, 0, 0};
    char buf[32];
  com.toCharArray(buf, sizeof(buf)); 
  char* token = strtok(buf, ",");                                     // Заменили p на token
  int i = 0;
  while (token != NULL && i < 5) {
      web[i++] = (unsigned char)atoi(token); 
      token = strtok(NULL, ",");
  }

  /*
	int i = 0;
	while(i < 5){
    int ID = com.indexOf(',');                                        // ищем символ
    if(ID == -1) break;
	    web[i] = com.substring(0, ID).toInt();                          // SubString(1, ID - 1).ToInt();
			i++;
      com.remove(0, ID + 1);
	}*/
  sua.bit4.arr[0] = web[2]; sua.bit4.arr[1] = web[3];                 // вычисляем присланное значение для эдит
  String json = COM_read_BT(7, sua.bit4.bit4_16, web[1], web[0]);  
 
 // if(web[4] == 6)                                                     // если запрос из программы
 // {
     // json = String(web[1]) + "," + String(x) + ",";
     // временно закоментировал    json = String(x);      
 // }
 // else serializeJson(root,json);                                      // ответ web браузеру

  request->send(200, "application/json", json);                       // (Правильный MIME-тип) после нажатия кнопок возвращает состояние кнопок
 }); 
}
//--------------------------------------------------------------------*-------------------------------------------------
void setup()
{
  bit->r_golova = 0;                                                  // открыть клапан голов в ручном
  bit->r_telo = 0;                                                    // открыть клапан тела в ручном
  bit->r_hvost = 0;                                                   // открыть клапан хвостов в ручном
  bit->as_ds1820 = 0;                                                 // защита по неисправности даласов
  bit->mikserOnR = 0;                                                 // включить мешалку в ручном режиме
  bit->sequent = 0;                                                   // команда оператора продолжить шаг
  bit->oper_pause = 0;                                                // стоим на паузе оператора                           
  bit->mesh_on_off = 0;                                               // вкл-выкл мешалка(ответ частотника)
  bit->mix_on = 0;                                                    // прокрутка мешалки в автомате
  bit->lisense = 0;
  bit->as_off = 0;                                                    // действие защит(сброс нагрузки)
  bit->send_port_off = 0;                                             // для пропуска посылки в порт
  bit->bool_imitazia = 0;
  bit->id_f = 0;                                                      // признак смены формы             

  Serial.begin(115200);                                               // отладочный последовательный порт 
  Serial2.begin(921600);                                              // Nextion последовательный порт
  MBSerial.begin(38400, SERIAL_8N1, 25, 26);   
  Serial.println();  Serial.println("Прошивка ПВК.");

  const char* vers = BUILD_VERSION;
  size_t len = strlen(vers);
  memcpy(&Tdata[110], vers, len);

  id_forms = 0;

  Serial.println("Инициализация SPIFFS...");                          // Инициализация файловой системы
  if (!SPIFFS.begin(true)) {
    Serial.println("ОШИБКА: Не удалось смонтировать SPIFFS!");
    }
        
  prefs.begin("Settings", false);                                     // выделяем область памяти для настроек
                                                      
  read_Ust();  
  setupPWM();
  
  WiFi.mode(WIFI_STA);                                                // режим вайфай

  ElegantOTA.begin(&server);                                          // Start ElegantOTA
  server.serveStatic("/", SPIFFS, "/");  
  server.begin();
  Serial.println("HTTP server started");

  //Serial.println("Ревизия чипа: " + ESP.getChipRevision());
  bit->lisense = sua.licensing();                                     // прочитали номер чипа

  //chipid = ESP.getEfuseMac(); //The chip ID is essentially its MAC address(length: 6 bytes).
  chipid = sua._chipid;                                               //The chip ID is essentially its MAC address(length: 6 bytes).  
  Serial.printf("ESP32 Chip ID = %04X", (uint16_t)(chipid >> 32));    //print High 2 bytes
  Serial.printf("%08X\n", (uint32_t)chipid);                          //print Low 4bytes.
  Serial.println();  
  bit->as_ts_on = true;    
  relay(0,0,1,3);                                                     /*00 03 00 00 00 01 85 DB  */      // запрос адреса платы

  zikl = 4000;
   
  web_cod();                                                          // работа с WEB
  sua.begin_ds();
  sua.read_addr_DS18B20();
}
//--------------------------------------------------------------------*------------------------------------------------- 
void WriteRegister(unsigned char DevID,uint16_t ADRESS,uint16_t val)
 {
    wr_data[0]=(char)DevID;                                          // адрес устройства
    wr_data[1]=(char)0x06;                                           // функция запись регистра
    wr_data[2]= ADRESS>>8;                                           // адрес        addr>>8;
    wr_data[3]= ADRESS&0xFF;                                         // регистра     addr&0xFF;
    wr_data[4]= val>>8;                                              // байт 1
    wr_data[5]= val&0xFF;                                            // байт 0

    unsigned short crc = myCRC(wr_data,6);

    wr_data[6]= crc&0xFF;                                            // crc
    wr_data[7]= crc>>8;                                              // crc

    MBSerial.write(wr_data,8);                                       // массив данных в порт

    delay(50);    
  }  
//--------------------------------------------------------------------*-------------------------------------------------  
void WriteRegisterRelay(unsigned char DevID, unsigned char funz, uint16_t on_off, uint16_t number)// WriteRegisterRelay(1,on_off,number);  
{
 //uint8_t b;
 //if(on_off){b = 255;}
 //else b = 0;
 uint8_t wr_data[8];

 // 00 03 00 00 00 01 85 DB     это работает Read device address  The 5th byte of the Return frame, 0xFF is the read device address.
 // FF 05 00 00 FF 00 99 E4     включить первое реле
 // FF 05 00 01 00 00 89 D4     включить второе реле
 // FF 01 00 00 00 08 28 12     read device address 

/*
 wr_data[0]= (char)DevID;                                             // адрес устройства
 wr_data[1]= (char)1;                                                 // функция запись регистра 5-управление реле
 wr_data[2]= (char)on_off >> 8;                                                 // 
 wr_data[3]= (char)on_off&0xFF;                                                 // номер реле
 wr_data[4]= (char)number >> 8;                                                 // FF-включить 0-выключить
 wr_data[5]= (char)8;                                                 // 
*/
 wr_data[0] = (char)DevID;                                            // адрес устройства
 wr_data[1] = funz;                                                   // функция запись регистра 5-управление реле
 wr_data[2] = number >> 8;                                            // 
 wr_data[3] = number&0xFF;                                            // номер реле
 wr_data[4] = on_off >> 8;                                            // FF-включить 0-выключить
 wr_data[5] = on_off&0xFF;                                            // 
//
// 1 0 0 0 1 0 0 90

 unsigned short crc = myCRC(wr_data,6);

 wr_data[6]= crc&0xFF;                                                // crc
 wr_data[7]= crc>>8;                                                  // crc

 MBSerial.write(wr_data,8);                                        // массив данных в порт

 Serial.println(String(wr_data[0])+" "+String(wr_data[1])+" "+String(wr_data[2])+" "+String(wr_data[3])+" "+String(wr_data[4])+" "+String(wr_data[5])+" "+String(wr_data[6])+" "+String(wr_data[7]));

 delay(50);    
}
//--------------------------------------------------------------------*-------------------------------------------------
void ReadRegisters(unsigned char DevID,uint16_t START_ADRESS,uint16_t len) // запрос на выдачу регистров
 {
    wr_data[0]=(char)DevID;                                           // адрес устройства
    wr_data[1]=(char)0x03;                                            // функция чтения регистров
    wr_data[2]= START_ADRESS>>8;                                      // стартовый адрес        addr>>8;
    wr_data[3]= START_ADRESS&0xFF;                                    // регистра     addr&0xFF;
    wr_data[4]= len>>8;                                               // байт 1
    wr_data[5]= len&0xFF;                                             // байт 0

    unsigned short crc = myCRC(wr_data,6);

    wr_data[6]= crc&0xFF;                                             // crc
    wr_data[7]= crc>>8;                                               // crc

    MBSerial.write(wr_data,8);                                        // массив данных в порт
 }   
//--------------------------------------------------------------------*-------------------------------------------------
uint16_t myCRC(byte message[], int length)
 {
    ushort CRCFull = 0xFFFF;
    for (int i = 0; i < length; i++)
     {
        CRCFull = (ushort)(CRCFull ^ message[i]);
        for (int j = 0; j < 8; j++)
         {
            if ((CRCFull & 0x0001) == 0)
            CRCFull = (ushort)(CRCFull >> 1);
            else{
                CRCFull = (ushort)((CRCFull >> 1) ^ 0xA001);
                }
         }
     }
            //CRCHigh = (byte)((CRCFull >> 8) & 0xFF);
            //CRCLow = (byte)(CRCFull & 0xFF);
    return CRCFull;
 }
//--------------------------------------------------------------------*-------------------------------------------------
