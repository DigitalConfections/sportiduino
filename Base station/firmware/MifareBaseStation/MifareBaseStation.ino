#include <deprecated.h>
#include <MFRC522.h>
#include <MFRC522Extended.h>
#include <require_cpp11.h>

#include <Wire.h>
#include <SPI.h>
#include <EEPROM.h>

#include <ds3231.h>
#include <Adafruit_SleepyDog.h>

// Версия прошивки
#define FIRMWARE_VERSION 105

// Для компиляции проекта необходимо установить бибилиотеки
// - AdafruitSpeepyDog by Adafruit (https://github.com/adafruit/Adafruit_SleepyDog)
// - MFRC522 by GithubCommunity (https://github.com/miguelbalboa/rfid)
// - DS3231FS by Petre Rodan (https://github.com/Jorropo/ds3231)
// Для этого откройте Скетч->Подключить Библиотеку->Управление Бибилиотеками.
// В поиске введите соответствующие библиотеки и нажмите для каждой INSTALL

//-------------------------------------------------------------------
// Настройка

// По умолчанию с завода МК настроен на работу от встроенного RC генератора с делителем на 8
// В итоге системная частота = 1 МГц
// Ардуино по умолчанию настроен на частоту 16 МГц, поэтому необходимо отредактировать
// текстовый файл с настройками платы Ардиуно и установить системную частоту 1 МГц

// Уберите комментарий со строки ниже, если на плате впаян пьезоизлучатель без генератора
//#define PIEZO_SPEAKER

// Коэффициент усиления антенны модуля RC522. Max = (7<<4) (48 dB), Mid = (3<<4) (23 dB), Min = (0<<4) (18 dB)
#define RC522_ANTENNA_GAIN (7<<4)

// Максимально-допустимый адерс страницы на карточке = Кол-во блоков на карте - кол-во трейл-блоков + 2
// Количество отметок = Максимальный адрес - 8
// MIFARE Classic 1K (MF1S503x) = 50 страниц (Всего 64 блока - 16 трейл-блоков + 2 = 50); 42 отметки = 50 - 8
// MIFARE Classic 4K (MF1S703x) = 98 страниц (Всего 128 основных блоков - 32 трейл-блока + 2 = 98); 90 отметок = 98 - 8. Расширенные блоки не поддерживаются!
// MIFARE Classic Mini (MF1 IC S20) = 17 страниц (Всего 20 блоков - 5 трейл-блоков + 2 = 17); 9 отметок = 17 - 8
// Карты MIFARE Ultralight (MF0ICU1) и MIFARE Ultralight C (MF0ICU2) не поддерживаются!
#define CARD_PAGE_MAX    50

// Срок годности карточки участника (в секундах)
// 31 день = 2678400
#define CARD_EXPIRE_TIME 2678400L

// Максимально-допустимый номер карточки
#define MAX_CARD_NUM              4000

// Настройки станции
// Bit1,Bit0 - Время перехода в режим ожидания при бездействия
// (0,0) - сон через <период1>;
// (0,1) - сон через <период2>;
// (1,0) - всегда проверять чипы через 1 секундe;
// (1,1) - всегда проверять чипы через 0.25с
// Bit2 - Проверять отметки стартовой и финишной станции на чипах участников (0 - нет, 1 - да)
// Bit3 - Проверять время на чипах участников (0 - нет, 1 - да)
// Bit4 - Сбрасывать настройки при переходе в сон (0 - нет, 1 -да)
// Bit7 - Настройки валидны (0 - да, 1 - нет)

#define SETTINGS_INVALID                0x80
#define SETTINGS_WAIT_PERIOD1           0x0
#define SETTINGS_WAIT_PERIOD2           0x1
#define SETTINGS_ALWAYS_WAIT            0x2
#define SETTINGS_ALWAYS_ACTIVE          0x3
#define SETTINGS_CHECK_START_FINISH     0x4
#define SETTINGS_CHECK_CARD_TIME        0x8
#define SETTINGS_CLEAR_ON_SLEEP         0x10

// Настройки по умолчанию (побитовое или из макросов SETTINGS, например, SETTINGS_WAIT_PERIOD1 | SETTINGS_CHECK_START_FINISH)
#define DEFAULT_SETTINGS  SETTINGS_WAIT_PERIOD1

// Зарезервированный номер стартовой станции
#define START_STATION_NUM         240
// Зарезервированный номер финишной станции
#define FINISH_STATION_NUM        245
// Зарезервированный номер станции проверки
#define CHECK_STATION_NUM         248
// Зарезервированный номер станции очистки
#define CLEAR_STATION_NUM         249

// Номер станции по умолчанию после сборки
#define DEFAULT_STATION_NUM       CHECK_STATION_NUM

// Период1 работы в режиме ожидания (в миллисекундах)
// 6 часов = 2160000
#define WAIT_PERIOD1              21600000L
// Период2 работы в режиме ожидания (в миллисекундах)
// 48 часов = 172800000
#define WAIT_PERIOD2              172800000L

// Период проверки чипов в активном режиме (в миллисекундах)
#define MODE_ACTIVE_CARD_CHECK_PERIOD     250
// Период проверки чипов в режиме ожидания (в миллисекундах)
#define MODE_WAIT_CARD_CHECK_PERIOD       1000
// Период проверки чипов в режиме сна (в миллисекундах)
#define MODE_SLEEP_CARD_CHECK_PERIOD      25000

// Cигнал при загрузке станции
#define BEEP_SYSTEM_STARTUP     beep(1000,1)

// Сигнал-ошибка при чтении еепром памяти
#define BEEP_EEPROM_ERROR       beep(50,2)
// Сигнал-ошибка не верно идут часы
#define BEEP_TIME_ERROR         beep(50,3)
// Сигнал-ошибка не подходит пароль мастер чипа
#define BEEP_PASS_ERROR         beep(50,4)
// Сигнал нужно заменить батареи
#define BEEP_LOW_BATTERY        beep(50,5)
#define BEEP_BATTERY_OK         beep(500,1)
// Сигнал чип не прошёл проверку

#define BEEP_CARD_CHECK_ERROR   beep(200,3)
#define BEEP_CARD_CHECK_OK      beep(500,1)
// Сигнал записи отметки на карточку участника
#define BEEP_CARD_MARK_WRITTEN  beep(500,1)
// Сигнал повторного прикладывания карточки участника
#define BEEP_CARD_MARK_OK       beep(250,2)
// Сигнал ошибки записи на карточку участника
#define BEEP_CARD_MARK_ERROR
// Сигнал успешной очистки карточку участника
#define BEEP_CARD_CLEAR_OK      beep(200,1)
// Сигнал успешной проверки карточки участника
#define BEEP_CARD_CLEAR_ERROR

// Сигнал прочитан мастер чип паролей
#define BEEP_MASTER_CARD_PASS_OK            beep(500,2)
#define BEEP_MASTER_CARD_PASS_ERROR
// Сигнал прочитан мастер-чип времени
#define BEEP_MASTER_CARD_TIME_OK            beep(500,3)
#define BEEP_MASTER_CARD_TIME_ERROR
// Сигнал прочитан мастер-чип сна
#define BEEP_MASTER_CARD_SLEEP_OK           beep(500,4)
#define BEEP_MASTER_CARD_SLEEP_ERROR
// Сигнал прочитан мастер-чип номера станции
#define BEEP_MASTER_CARD_STATION_OK         beep(500,5)
#define BEEP_MASTER_CARD_STATION_ERROR      beep(50,6)
// Сигнал прочитан мастер-чип дампа
#define BEEP_MASTER_CARD_DUMP_OK            beep(500,6)
#define BEEP_MASTER_CARD_DUMP_ERROR

//--------------------------------------------------------------------
// Пины

#define LED           4
#define BUZ           3
#define RC522_RST     9
#define RC522_SDA     10
#define RC522_IRQ     6
#define DS3231_IRQ    A3
#define DS3231_32K    5

#define UNKNOWN_PIN 0xFF

//--------------------------------------------------------------------
// Константы

#define SET_TIME_MASTER_CARD        250
#define SET_NUMBER_MASTER_CARD      251
#define SLEEP_MASTER_CARD           252
#define READ_DUMP_MASTER_CARD       253
#define SET_PASS_MASTER_CARD        254

// Адрес страницы на карторчке с информацией о ней
#define CARD_PAGE_INFO              4
// Адрес страницы на карточке со временем инициализации чипа
#define CARD_PAGE_INIT_TIME         5
// Адрес страницы на карточке с информацией о последней отметке (не используется!)
#define CARD_PAGE_LAST_RECORD_INFO  6
// Адрес начала отметок на чипе
#define CARD_PAGE_START             8

#define EEPROM_STATION_NUM_ADDR     800
#define EEPROM_PASS_ADDR            850
#define EEPROM_SETTINGS_ADDR        859

//--------------------------------------------------------------------
// Переменные

// Текущие дата и время
struct ts t;
// Время работы (в миллисекундах)
uint32_t workTimer;
// Пароль для проверки мастер-чипов
uint8_t pass[3];
// Ключ шифрования карточек
MFRC522::MIFARE_Key key;
// RC522
MFRC522 mfrc522(RC522_SDA, RC522_RST);
// Номер станции
uint8_t stationNum;
// Настройки станции
uint8_t settings;
// режим работы станции
uint8_t mode;

#define MODE_ACTIVE   0
#define MODE_WAIT     1
#define MODE_SLEEP    2

#define DEFAULT_MODE MODE_WAIT

//--------------------------------------------------------------------
// Прототипы функций

/**
 * Фунцкия программной перезагрузки
 */
void(*resetFunc)(void) = 0;

/**
 * Возвращает текущий режим работы пина
 */
uint8_t getPinMode(uint8_t pin);

/**
 * Функция записи во внутреннюю память микроконтроллера
 * Запись приосходит с мажоритарным резервированием в три подряд ячейки
 */
void eepromWrite (uint16_t adr, uint8_t val);

/**
 * Считывание ячейки из внутренней памяти МК с учетом мажоритарного резервирования
 */
uint8_t eepromRead(uint16_t adr);

/**
 * Переводит устройство в энергосберегающий режим сна на заданное время
 */
void sleep(uint16_t ms);

/**
 * Запись ячейки соответствующей номеру чипа во внутреннюю память. Только факт отметки.
 */
void writeNumEeprom (uint16_t num);

/**
 * Очистка внутренней памяти станции
 * адреса 0 - 750
 */
void cleanEeprom ();

/**
 * Выдача сигнала. Принимает продолжительность сигнала и число сигналов подряд
 * В ходе работы сбрасывает вотчдог, чтобы не произошла перезагрузка
 */
void beep(uint16_t ms, uint8_t n);

/**
 * Функция считывания напряжения питания МК. 
 * Сравнение происходит по внутреннему источнику опроного напряжения в 1.1 В
 */
uint32_t readVcc(uint32_t refConst);

/**
 * Измерение напряжения. Включает диод на 5 секунд. Затем происходит измерение.
 * Если напряжение меньше 3.1 В, то станция выдает три длинные сигнала. Если больше, то один.
 */
void voltage();

/**
 * Читает страницу карточки. Эта функция должна вызываться после инициализации RC522.
 * 
 * @param data Указатель на буфер, куда будут сохранены данные. Длина буфера должна быть не менее 18 байт
 * @param size Указатель на переменную, хранящую размер буфера, после чтения в этой переменной будет количество прочитанных байт
 */
bool cardPageRead(uint8_t pageAdr, byte *data, byte *size);

/**
 * Записывает блок данных на карточку
 * 
 * @param data Буфер данных. Размер буфер должен быть не менее 16 байт
 * @param size Размер буфера
 */
bool cardPageWrite(uint8_t pageAdr, byte *data, byte size);

/**
 * Основная функция работы с чипами
 */
void rfid();

/**
 * Функция обработки мастер-чипа времени.
 * С чипа считыввается новое время и устанавливается 
 * внутреннее время. Станция пикает 5 раз в случае успеха
 */
void processTimeMasterCard(byte *data, byte dataSize);


/**
 * Функция установки нового номера станции
 * станция считывает чип, записывает новый номер
 * во внутреннюю память, пикает 5 раз и перезагружается
 */
void processStationMasterCard(byte *data, byte dataSize);

/**
 * Функция обработки мастер-чипа сна. 
 * Станция стирает данные о пароле и настройках,
 * пикает три раза и входит в сон
 */
void processSleepMasterCard(byte *data, byte dataSize);

/**
 * Функция записи дамп-чипа. Станция считывает все данные по чипам из внутренней памяти
 * и записывает их последовательно на дамп-чип. После чего один раз пикает и выходит.
 */
void processDumpMasterCard(byte *data, byte dataSize);

/**
 * Функция обработки мастер-чипа смены пароля. Станция считывает новый пароль и байт настроек. Записывает его в память.
 * Пикает два раза и перезагружается.
 */
void processPassMasterCard(byte *data, byte dataSize);

/**
 * Функция обработки карточки участника
 */
void processParticipantCard(uint16_t cardNum);

/**
 * Функция поиска последней записанной страницы по алгоритму бинарного поиска.
 * 
 * @note Проще не использовать бинарный поиск и при отметке станции на чипе записывать, например,
 * в страницу 6 номер последней станции и адрес свободной страницы. Но при этом эта страница
 * будет очень часто перезаписываться и чип быстро выйдет из строя. Поэтому реализован
 * бинарный поиск свободной страницы
 * 
 * @param newPage Указатель на переменную, куда будет сохранен номер первой чистой страницы
 * @param lastNum Указатель на переменную, куда будет сохранён номер последней записанной станции
 */
void findNewPage(uint8_t *newPage, uint8_t *lastNum);

/**
 * Функция записи отметки в карточку участника.
 * Записывает номер и поседние 3 байта юникстайм в чип.
 */
bool writeMarkToParticipantCard(uint8_t newPage);

/**
 * Функция очистки карточки участника
 */
void clearParticipantCard();

/**
 * Функция проверки карточки участника
 */
void checkParticipantCard();

/**
 * Проверяет время инициализации чипа
 */
bool doesCardExpire();

//--------------------------------------------------------------------
// Реализация

/**
 * После запуска или перезагрузки станция считывает показание часов
 * Если они сбиты, то издает три коротких звуковых сигнала
 * Запоминает промежуточное время temph, чтобы вовремя перейти в режим ожиданиния при бездействии
 * 
 * Далее присходит счтиывание из EEPROM памяти настроек станций:
 * - номера станции
 * - паролей мастер-чипа
 * - байта настроек работы станции
 * 
 * Затем станция выжидает 5 секунд и после длинного сигнала выходит в цикл loop
 */
void setup()
{
  // Блокируем и сбрасываем вотчдог
  Watchdog.disable();
  Watchdog.reset();

  // Настраиваем пины
  pinMode(LED,OUTPUT);
  pinMode(BUZ,OUTPUT);
  pinMode(RC522_RST,OUTPUT);
  pinMode(RC522_SDA,OUTPUT);
  pinMode(RC522_IRQ,INPUT_PULLUP);
  pinMode(DS3231_IRQ,INPUT_PULLUP);
  pinMode(DS3231_32K,INPUT_PULLUP);

  digitalWrite(LED,LOW);
  digitalWrite(BUZ,LOW);
  digitalWrite(RC522_RST,LOW);
  
  // Настраиваем неиспользуемые пины
  // Согласно документации на них лучше включить встроенные pull-up резисторы
  for(byte pin = 0; pin < A5; pin++)
  {
    if(getPinMode(pin) == INPUT)
      pinMode(pin, INPUT_PULLUP);
  }
  
  // Настраиваем RTC
  // Сбрасываем все прерывания
  DS3231_clear_a1f();
  DS3231_clear_a2f();
  // Внимание: DS3231_Init выключает выход 32 кГц!
  DS3231_init(DS3231_INTCN);// | DS3231_A1IE);
  // Читаем текущее время
  DS3231_get(&t);
  
  if(t.year < 2017)
  {
    // Часы не настроены
    BEEP_TIME_ERROR;
  }

  // Читаем настройки из EEPROM
  stationNum = eepromRead(EEPROM_STATION_NUM_ADDR);

  for (uint8_t i = 0; i < 3; i++)
    pass[i] = eepromRead(EEPROM_PASS_ADDR + i*3);
    
  settings = eepromRead(EEPROM_SETTINGS_ADDR);

  // После сборки станции номер не установлен, применяем по умолчанию
  if(stationNum == 0 || stationNum == 255)
    stationNum = DEFAULT_STATION_NUM;

  // Применяем настройки по умолчанию после сборки станции
  if(settings & SETTINGS_INVALID)
  {
    settings = DEFAULT_SETTINGS;
    pass[0] = pass[1] = pass[2] = 0;

    // Сохраняем настройки и пароль по умолчаню в EEPROM
    eepromWrite(EEPROM_SETTINGS_ADDR, settings);
    for (uint8_t i = 0; i < 3; i++)
      eepromWrite(EEPROM_PASS_ADDR + i*3, pass[i]);
  }

  // Устанавливаем режим работы по умолчанию
  mode = DEFAULT_MODE;

  // Перенастраиваем режим в соответствии с настройками
  if(settings & SETTINGS_ALWAYS_WAIT)
    mode = MODE_WAIT;
  else if(settings & SETTINGS_ALWAYS_ACTIVE)
    mode = MODE_ACTIVE;
 
  // Инициализруем ключ для карточек MIFARE
  for (byte i = 0; i < 6; i++)
    key.keyByte[i] = 0xFF;
  
  // Проверяем батарейки
  voltage();
  // Сигнализируем о переходе в основной цикл
  BEEP_SYSTEM_STARTUP;
  // Сбрасываем программный таймер
  workTimer = 0;
  // Включаем вотчдог для защиты от зависаний
  Watchdog.enable(8000);
}

void loop()
{
  Watchdog.reset();
  
  rfid();

  switch(mode)
  {
    case MODE_ACTIVE:
      sleep(MODE_ACTIVE_CARD_CHECK_PERIOD);
      if(settings & SETTINGS_ALWAYS_ACTIVE)
      {
         workTimer = 0;
      }
      else if(workTimer >= WAIT_PERIOD1)
      {
        workTimer = 0;
        mode = MODE_WAIT;
      }
      break;
    case MODE_WAIT:
      sleep(MODE_WAIT_CARD_CHECK_PERIOD);
      if(settings & SETTINGS_ALWAYS_WAIT)
      {
        workTimer = 0;
      }
      else if(workTimer >= WAIT_PERIOD1 && (settings & SETTINGS_WAIT_PERIOD1))
      {
        workTimer = 0;
        mode = MODE_SLEEP;
      }
      else if(workTimer >= WAIT_PERIOD2 && (settings & SETTINGS_WAIT_PERIOD2))
      {
        workTimer = 0;
        mode = MODE_SLEEP;
      }
      break;
    case MODE_SLEEP:
      sleep(MODE_SLEEP_CARD_CHECK_PERIOD);
      break;
  }
}

uint8_t getPinMode(uint8_t pin)
{
  uint8_t bit = digitalPinToBitMask(pin);
  uint8_t port = digitalPinToPort(pin);

  // I don't see an option for mega to return this, but whatever...
  if (NOT_A_PIN == port) return UNKNOWN_PIN;

  // Is there a bit we can check?
  if (0 == bit) return UNKNOWN_PIN;

  // Is there only a single bit set?
  if (bit & bit - 1) return UNKNOWN_PIN;

  volatile uint8_t *reg, *out;
  reg = portModeRegister(port);
  out = portOutputRegister(port);

  if (*reg & bit)
    return OUTPUT;
  else if (*out & bit)
    return INPUT_PULLUP;
  else
    return INPUT;
}

void sleep(uint16_t ms)
{
  uint16_t period;
  // Выключаем модуль RC522
  digitalWrite(RC522_RST,LOW);
  // Выключаем светодиод
  digitalWrite(LED,LOW);
  // Выключаем буззер
  digitalWrite(BUZ,LOW);
  // Сбрасываем вотчдог и засыпаем
  Watchdog.reset();
  period = Watchdog.sleep(ms);
  workTimer += period;
  // Используем рекурсию, если время проведённое во сне меньше заданного
  if(ms > period)
    sleep(ms - period);
}

void eepromWrite (uint16_t adr, uint8_t val)
{
  for(uint16_t i = 0; i < 3; i++)
    EEPROM.write(adr + i, val);
}

uint8_t eepromRead(uint16_t adr)
{
  uint8_t val1 = EEPROM.read(adr);
  uint8_t val2 = EEPROM.read(adr + 1);
  uint8_t val3 = EEPROM.read(adr + 2);
  
  if(val1 == val2 || val1 == val3)
  {
    return val1;
  }
  else if(val2 == val3)
  {
    return val2;
  }
  else
  {
    BEEP_EEPROM_ERROR;
    return 0;
  }
}

void writeNumEeprom (uint16_t num)
{
  if(num > MAX_CARD_NUM)
  {
    BEEP_EEPROM_ERROR;
    return;
  }
    
  uint16_t byteAdr = num/8;
  uint16_t bitAdr = num%8;
  uint8_t eepromByte = EEPROM.read(byteAdr);
  bitSet(eepromByte, bitAdr);
  EEPROM.write(byteAdr, eepromByte);
}

void cleanEeprom ()
{
  for (uint16_t a = 0; a < 750; a++)
  {
    if(a%100 == 0)
      Watchdog.reset();
      
    EEPROM.write(a,0);
    
    delay(5);
  }
}

void beep(uint16_t ms, uint8_t n)
{
  for (uint8_t i = 0; i < n; i++)
  {
    Watchdog.reset();
    
    digitalWrite(LED, HIGH);
    #ifdef PIEZO_SPEAKER
      tone(BUZ, 4000, ms);
    #else
      digitalWrite (BUZ, HIGH);
    #endif
    
    delay(ms);
    Watchdog.reset();
    
    digitalWrite(LED, LOW);
    digitalWrite(BUZ, LOW);
    
    if (i < n - 1)
    {
      delay(ms);
      Watchdog.reset();
    }
  }
}

uint32_t readVcc(uint32_t refConst)
{
  // Включаем АЦП
  ADCSRA |=  bit(ADEN); 
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  // Ждём стабилиазции опорного источника
  delay(5);
  // Начинаем измерение
  ADCSRA |= _BV(ADSC);
  while(bit_is_set(ADCSRA, ADSC));

  // Читаем обязательно сначала младший байт, затем старший
  uint8_t low  = ADCL;
  uint8_t high = ADCH;

  uint32_t result = (high << 8) | low;

  result = refConst / result; // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000
  // Выключаем АЦП
  ADCSRA ^= bit(ADEN);
  
  return result;
}

void voltage()
{
  const uint32_t refConst = 1125300L; //voltage constanta
  uint32_t value = 0;

  Watchdog.reset();

  digitalWrite(LED, HIGH);
  delay(5000);

  for (uint8_t i = 0; i < 10; i++)
    value += readVcc(refConst);

  value /= 10;

  digitalWrite(LED, LOW);
  delay(500);
  
  Watchdog.reset();

  if (value < 3100)
    BEEP_LOW_BATTERY;
  else
    BEEP_BATTERY_OK;
}

bool cardPageRead(uint8_t pageAdr, byte *data, byte *size)
{
  if(pageAdr < 3)
    return false;
    
  MFRC522::StatusCode status;
  // Адрес читаемого блока
  byte blockAddr = pageAdr-3 + ((pageAdr-3)/3);
  // Адрес блока ключей
  byte trailerBlock = blockAddr + (3-blockAddr%4);

  // Авторизация по ключу А
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
  
  if(status != MFRC522::STATUS_OK)
    return false;
  
  status = mfrc522.MIFARE_Read(blockAddr, data, size);
  
  if(status != MFRC522::STATUS_OK)
    return false;
 
  return true; 
}

bool cardPageWrite(uint8_t pageAdr, byte *data, byte size)
{
  MFRC522::StatusCode status;
  
  byte blockAddr = pageAdr-3 + ((pageAdr-3)/3);
  byte trailerBlock = blockAddr + (3-blockAddr%4);

  // Авторизация по ключу А
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
  
  if(status != MFRC522::STATUS_OK)
    return false;

  // Записываем данные в блок
  status = mfrc522.MIFARE_Write(blockAddr, data, size);
  
  if(status != MFRC522::STATUS_OK)
    return false;
}

void rfid()
{
  byte pageData[18];
  byte dataSize;
  byte masterCardData[16];
  bool result;

  // Включаем SPI и RC522. Ищем карту вблизи. Если не находим выходим из функции чтения чипов
  SPI.begin();
  mfrc522.PCD_Init();
  mfrc522.PCD_AntennaOff();
  mfrc522.PCD_SetAntennaGain(RC522_ANTENNA_GAIN);
  mfrc522.PCD_AntennaOn();
  
  delay(5);
  
  if(mfrc522.PICC_IsNewCardPresent())
  {
    if(mfrc522.PICC_ReadCardSerial())
    {
      // Переходим в активный режим
      mode = MODE_ACTIVE;
      //Читаем блок информации
      dataSize = sizeof(pageData);
      if(cardPageRead(CARD_PAGE_INFO, pageData, &dataSize))
      {
        // Проверяем тип чипа
        if(pageData[2] == 0xFF)
        {
          // Мастер-чип
          
          // Копируем информацию о мастер-чипе
          memcpy(masterCardData, pageData, 4);
          // Читаем данные с мастер-чипа
          result = true;
          
          dataSize = sizeof(pageData);
          result &= cardPageRead(CARD_PAGE_INFO + 1, pageData, &dataSize);
          if(result)
            memcpy(masterCardData + 4, pageData, 4);
            
          dataSize = sizeof(pageData);
          result &= cardPageRead(CARD_PAGE_INFO + 2, pageData, &dataSize);
          if(result)
            memcpy(masterCardData + 8, pageData, 4);
          
          dataSize = sizeof(pageData);
          result &= cardPageRead(CARD_PAGE_INFO + 3, pageData, &dataSize);
          if(result)
            memcpy(masterCardData + 12, pageData, 4);

          if(result)
          {
            // Обрабатываем мастер-чип

            // Проверяем пароль
            if( (pass[0] == masterCardData[4]) &&
                (pass[1] == masterCardData[5]) &&
                (pass[2] == masterCardData[6]) )
            {
              switch(masterCardData[1])
              {
                case SET_TIME_MASTER_CARD:
                  processTimeMasterCard(masterCardData, sizeof(masterCardData));
                  break;
                case SET_NUMBER_MASTER_CARD:
                  processStationMasterCard(masterCardData, sizeof(masterCardData));
                  break;
                case SLEEP_MASTER_CARD:
                  processSleepMasterCard(masterCardData, sizeof(masterCardData));
                  break;
                case READ_DUMP_MASTER_CARD:
                  processDumpMasterCard(masterCardData, sizeof(masterCardData));
                  break;
                case SET_PASS_MASTER_CARD:
                  processPassMasterCard(masterCardData, sizeof(masterCardData));
                  break;
              }
              // Засыпаем на 3 секунды, чтобы спокойно убрать мастер-чип
              Watchdog.sleep(3000);
            }
            else
              BEEP_PASS_ERROR;
          }
        } // Конец обработки мастер-чипа
        else
        {
          // Обработка чипа участника
          switch(stationNum)
          {
            case CLEAR_STATION_NUM:
              clearParticipantCard();
              break;
            case CHECK_STATION_NUM:
              checkParticipantCard();
              break;
            default:
              processParticipantCard((((uint16_t)pageData[0])<<8) + pageData[1]);
              break;
          }
        }
      } 
    }
  }
  // Завершаем работу с карточкой
  mfrc522.PICC_HaltA();
  // Завершаем работу с SPI
  SPI.end();
  // Переводим RC522 в хард-ресет
  digitalWrite(RC522_RST,LOW);
}

void processTimeMasterCard(byte *data, byte dataSize)
{
  if(dataSize < 16)
  {
    BEEP_MASTER_CARD_TIME_ERROR;
    return;
  }
    
  t.mon = data[8];
  t.year = data[9]+2000;
  t.mday = data[10];
  t.hour = data[12];
  t.min = data[13];
  t.sec = data[14];

  DS3231_set(t);
  
  BEEP_MASTER_CARD_TIME_OK;
}

void processStationMasterCard(byte *data, byte dataSize)
{
  if(dataSize < 16)
  {
    BEEP_MASTER_CARD_STATION_ERROR;
    return;
  }

  bool checkOk = false;
  uint8_t newNum = data[8];

  if(newNum > 0)
  {
    checkOk = true;
    
    if(stationNum != newNum)
    {
      stationNum = newNum;
      eepromWrite(EEPROM_STATION_NUM_ADDR, stationNum);
    }  
  }
  
  if(checkOk)
    BEEP_MASTER_CARD_STATION_OK;
  else
    BEEP_MASTER_CARD_STATION_ERROR;
}

void processSleepMasterCard(byte *data, byte dataSize)
{
  if(dataSize < 16)
  {
    BEEP_MASTER_CARD_SLEEP_ERROR;
    return;
  }
    
  mode = MODE_SLEEP;
  
  if(settings & SETTINGS_CLEAR_ON_SLEEP)
  {
    settings = DEFAULT_SETTINGS;
    eepromWrite(EEPROM_SETTINGS_ADDR,settings);
  }

  cleanEeprom();

  BEEP_MASTER_CARD_SLEEP_OK;
}

void processDumpMasterCard(byte *data, byte dataSize)
{
  if(dataSize < 16)
  {
    BEEP_MASTER_CARD_DUMP_ERROR;
    return;
  }
    
  uint8_t dataEeprom[16];
  uint16_t eepromAdr = 0;

  memset(dataEeprom, 0, sizeof(dataEeprom));

  for(uint8_t page = CARD_PAGE_INIT_TIME; page <= CARD_PAGE_MAX; page++)
  {
    Watchdog.reset();
    for(uint8_t m = 0; m < 4; m++)
    {
      dataEeprom[m] = EEPROM.read(eepromAdr);
      eepromAdr++;
    }

    if(!cardPageWrite(page, dataEeprom, sizeof(dataEeprom)))
    {
      BEEP_MASTER_CARD_DUMP_ERROR;
      return;
    }
  }

  BEEP_MASTER_CARD_DUMP_OK;
  return;
}

void processPassMasterCard(byte *data, byte dataSize)
{
  if(dataSize < 16)
  {
    BEEP_MASTER_CARD_PASS_ERROR;
    return;
  }
    
  for(uint8_t i = 0; i < 3; i++)
  {
    pass[i] = data[i + 8];
    eepromWrite(EEPROM_PASS_ADDR + i*3, pass[i]);
  }
  
  settings = data[11];
  eepromWrite(EEPROM_SETTINGS_ADDR, settings);

  BEEP_MASTER_CARD_PASS_OK;
}

void processParticipantCard(uint16_t cardNum)
{
  uint8_t lastNum = 0;
  uint8_t newPage = 0;
  bool checkOk = false;

  if(cardNum)
  {
    //Ищем последнюю пустую страницу в чипе для записи
    findNewPage(&newPage, &lastNum);
  
    if(newPage >= CARD_PAGE_START && newPage <= CARD_PAGE_MAX)
    {
      if(lastNum != stationNum)
      {
        /*
         * если включена функция старта-финиша. То станция старта принимает только пустые чипы
         * все остальные станции принимают только не пустые чипы
         * после станции финиша на чип нельзя записать отметку
         */
        checkOk = true;
        if(settings & SETTINGS_CHECK_START_FINISH)
        {
          if(newPage == CARD_PAGE_START && stationNum != START_STATION_NUM)
            checkOk = false;
          else if(stationNum == START_STATION_NUM && newPage != CARD_PAGE_START)
            checkOk = false;
          else if(lastNum == FINISH_STATION_NUM)
            checkOk = false;
          else if(stationNum == FINISH_STATION_NUM && newPage == CARD_PAGE_START)
            checkOk = false;
        }

        if(settings & SETTINGS_CHECK_CARD_TIME)
        {
          checkOk = !doesCardExpire();
        }

        // Записываем отметку
        if(checkOk)
        {
          if(writeMarkToParticipantCard(newPage))
          {
            // Записывааем номер чипа во внутреннюю память
            writeNumEeprom(cardNum);
              
            BEEP_CARD_MARK_WRITTEN;
          }
        }
      }
      else
      {
        checkOk = true;
        BEEP_CARD_MARK_OK;
      }
    }
  }

  if(!checkOk)
    BEEP_CARD_MARK_ERROR;
}

void findNewPage(uint8_t *newPage, uint8_t *lastNum)
{
  uint8_t startPage = CARD_PAGE_START;
  uint8_t endPage = CARD_PAGE_MAX;
  uint8_t page;
  byte pageData[18];
  byte dataSize;
  byte num;

  *newPage = 0;
  *lastNum = 0;

  while(startPage < endPage)
  {   
    page = (startPage + endPage)/2;

    dataSize = sizeof(pageData);
    if(!cardPageRead(page, pageData, &dataSize))
      return;

    num = pageData[0];
     
    if(num == 0)
      endPage = page;
    else
      startPage = (startPage != page)? page : page + 1;
  }

  *newPage = page + 1;
  *lastNum = num;
}

bool writeMarkToParticipantCard(uint8_t newPage)
{
  byte pageData[16];
  byte dataSize = sizeof(pageData);
  
  // Читаем текущее время
  DS3231_get(&t);

  pageData[0] = stationNum;
  pageData[1] = (t.unixtime & 0x00FF0000)>>16;
  pageData[2] = (t.unixtime & 0x0000FF00)>>8;
  pageData[3] = (t.unixtime & 0x000000FF);
      
  return cardPageWrite(newPage, pageData, dataSize);
}

/**
 * Функция очистки карточки участника
 */
void clearParticipantCard()
{
  byte pageData[16];
  byte dataSize = sizeof(pageData);
  bool result = true;

  memset(pageData, 0, dataSize);

  for(uint8_t page = CARD_PAGE_INIT_TIME; page <= CARD_PAGE_MAX; page++)
  {
    Watchdog.reset();
    digitalWrite(LED,HIGH);
    result &= cardPageWrite(page, pageData, dataSize);
    digitalWrite(LED,LOW);
  }

  if(result)
  {
    DS3231_get(&t);
    
    pageData[0] = (t.unixtime&0xFF000000)>>24;
    pageData[1] = (t.unixtime&0x00FF0000)>>16;
    pageData[2] = (t.unixtime&0x0000FF00)>>8;
    pageData[3] = (t.unixtime&0x000000FF);

    result &= cardPageWrite(CARD_PAGE_INIT_TIME, pageData, dataSize);
  }

  if(result)
    BEEP_CARD_CLEAR_OK;
  else
    BEEP_CARD_CLEAR_ERROR;
}

void checkParticipantCard()
{
  byte pageData[18];
  byte dataSize = sizeof(pageData);
  uint16_t cardNum = 0;
  uint8_t newPage = 0;
  uint8_t lastNum = 0;
  bool result = false;
  
  if(cardPageRead(CARD_PAGE_INFO, pageData, &dataSize))
  {
    // Проверяем номер чипа 
    cardNum = ((uint16_t)pageData[0])<<8 + pageData[1];
    if(cardNum > 0 && pageData[2] != 0xFF)
    {
      // Проверяем количество отметок на чипе
      findNewPage(&newPage, &lastNum);
      if(newPage == CARD_PAGE_START && lastNum == 0)
      {
        result = true;
        // Проверяем время инициализации чипа
        if(settings & SETTINGS_CHECK_CARD_TIME)
          result = !doesCardExpire();
      }
    }
  }

  if(result)
    BEEP_CARD_CHECK_OK;
  else
    BEEP_CARD_CHECK_ERROR;
}

bool doesCardExpire()
{
  byte pageData[18];
  byte dataSize = sizeof(pageData);
  uint32_t cardTime = 0;
  bool result = true;
  
  if(cardPageRead(CARD_PAGE_INIT_TIME, pageData, &dataSize))
  {
    DS3231_get(&t);

    cardTime = (((uint32_t)pageData[0]) & 0xFF000000)<<24;
    cardTime |= (((uint32_t)pageData[1]) & 0x00FF0000)<<16;
    cardTime |= (((uint32_t)pageData[2]) & 0x0000FF00)<<8;
    cardTime |= (((uint32_t)pageData[3]) & 0x000000FF);

    if(t.unixtime - cardTime >= CARD_EXPIRE_TIME)
      result = true;
    else
      result = false;
  }

  return result;
}
