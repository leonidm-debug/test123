#include <Arduino.h>
#include <SoftwareSerial.h>
#include <limits.h>


//commend was made for test of commits in Gihub
//
// второй коммит
// третий коммит
// четвертий комміт
// Fifth commit
// Sixth commit
// Seventh commit

int numberOfRecord = 0;
#define maxOfRecords 10
bool limitPerDayReached = false; 
bool awaitingDate = false;
bool awaitingType = false;
bool awaitingNumerOfDays = false;

int numberOfDays = 0;

String actualVarForDate;
String currDateVar;

boolean flagDateWasEnteredBefore = false;
String dateIsEnteredVar = "";


struct Date {
  uint8_t day;
  uint8_t month;
  uint8_t year;
  String recordType;
  int measurmentValue;
};


Date myDate[maxOfRecords];


struct Stats {
  long sum = 0;
  int count = 0;
  int min = INT_MAX;
  int max = INT_MIN;
};


bool isInteger(const String &s) {
  int len = s.length();
  if (len == 0) return false;
  int start = (s[0] == '+' || s[0] == '-') ? 1 : 0;
  if (start == 1 && len == 1) return false;
  for (int i = start; i < len; i++) {
    if (!isDigit(s.charAt(i))) return false;
  }
  return true;
}



bool isFloatNum(const String &s) {
  int len = s.length();
  if (len == 0) return false;
  int start = (s[0] == '+' || s[0] == '-') ? 1 : 0;
  bool dotFound = false;
  int digitCount = 0;
  for (int i = start; i < len; i++) {
    char c = s.charAt(i);
    if (c == '.') {
      if (dotFound) return false;
      dotFound = true;
    } else if (isDigit(c)) {
      digitCount++;
    } else {
      return false;
    }
  }
  return dotFound && (digitCount >= 1);  // должна быть точка и хотя бы одна цифра
}



// Функция проверки: является ли введенное значение датой или одной из комманд
String checkSerialInput(String &input) {
  input.trim();

  // 1. Проверка на команду
  const String cmds[] = {"Date","List","Temperature","Humidity","Pressure","Reset", "GetStats"};
  for (const String &cmd : cmds) {
    if (input.equalsIgnoreCase(cmd)) {
      return "command";
    }
  }

  // 2. Проверка формата даты dd.mm.yy
  bool isDate = input.length() == 8
    && isDigit(input.charAt(0)) && isDigit(input.charAt(1))
    && input.charAt(2) == '.'
    && isDigit(input.charAt(3)) && isDigit(input.charAt(4))
    && input.charAt(5) == '.'
    && isDigit(input.charAt(6)) && isDigit(input.charAt(7));

  if (isDate) {
    int d = input.substring(0,2).toInt();
    int m = input.substring(3,5).toInt();
    int y = input.substring(6,8).toInt();
    if (d >= 1 && d <= 31 && m >= 1 && m <= 12 && y >= 0 && y <= 99) {
      return "date";
    }
  }


  if (isInteger(input)) return "int";
  if (isFloatNum(input)) return "float";


  return "";
}



// Функция сравнения двух дат
bool isSameDate(const Date &a, const Date &b) {
  return a.day == b.day && a.month == b.month && a.year == b.year;
}



// Функция проверки: есть ли дата, встречающаяся необходимое количество раз
bool ChecksAffordableDuplicatesFn(const String &input, Date fnDate[], int maxMatches) {

  String s = input;
  s.trim();

  Date userDate;
  int day   = s.substring(0, 2).toInt();
  int month = s.substring(3, 5).toInt();
  int year  = s.substring(6, 8).toInt();

  userDate.day   = day;
  userDate.month = month;
  userDate.year  = year; 
 
    int count = 1; // начальное значение — текущая запись
    for (int j = 0; j <= maxOfRecords; j++) {
      if (isSameDate(userDate, fnDate[j])) {


        if (count >= maxMatches) {          
          //return true;
          Serial.println(""); 
          Serial.println("Максимальна кількість вимірювань для цієї дати вже введена!");   
          Serial.print(count);
          awaitingDate = false;
          return true;
          break;                
        }

          count++;          

      }
  }
  return false;
}



// Функция: парсит строку "dd.mm.yy", заполняет d и выводит через Serial
void parseDate(const String &input, Date &d, int &recordCount, String &typeOfRecord) {
  String s = input;
  s.trim();

  int day   = s.substring(0, 2).toInt();
  int month = s.substring(3, 5).toInt();
  int year  = s.substring(6, 8).toInt();
     
      d.day   = day;
      d.month = month;
      d.year  = year;
      d.recordType = typeOfRecord;
 

    awaitingDate = true;
}


void dataOutput(const String& input, int times, Date myDate[]) {

//if (input == "List") {
  for (int i = 0; i < times; i++) {   
     Serial.print(myDate[i].day, DEC);
     Serial.print('.');
     Serial.print(myDate[i].month, DEC);
     Serial.print('.');
     Serial.print(myDate[i].year, DEC);
     Serial.print(',');
     if(myDate[i].recordType == "") {
       Serial.print("no type");
       Serial.print(',');        
     } else {
       Serial.print(myDate[i].recordType);
       Serial.print(',');  
     }

     Serial.println(myDate[i].measurmentValue);
  //}
 }

}



bool checkInteger(const String &s) {
  // Проверить, пустая ли строка
  if (s.length() == 0) {
    Serial.println("ERROR: пустая строка");
    return false;
  }

  int dots = 0;
  int start = 0;
  if (s.charAt(0) == '+' || s.charAt(0) == '-') {
    if (s.length() == 1) {
      Serial.println("ERROR: только знак без цифр");
      return false;
    }
    start = 1;
  }
  for (int i = start; i < s.length(); i++) {
    char c = s.charAt(i);
    if (isDigit(c)) {
      continue;
    } else if (c == '.') {
      dots++;
      if (dots > 1) {
        Serial.println("ERROR: більше однієї точки");
        return false;
      }
    } else {
      Serial.print("ERROR: недопустимий символ '");
      Serial.print(c);
      Serial.println("'");
      return false;
    }
  }
  // Если разрешена точка, проверим дробную часть
  if (dots == 1) {
    float x = s.toFloat();
    float nearest = roundf(x);
    if (fabsf(x - nearest) < 1e-5) {
      Serial.print("OK: строка '");
      Serial.print(s);
      Serial.println("' — целое (з урахуванням похибки)");
      return true;
    } else {
      float frac = x - (x >= 0 ? floorf(x) : ceilf(x));
      Serial.print("ERROR: '");
      Serial.print(s);
      Serial.print("' — дробна частина: ");
      Serial.println(frac, 6);
      return false;
    }
  }
  // Если точка не была — точно целое
  Serial.print("OK: строка '");
  Serial.print(s);
  Serial.println("' — ціле число");
  return true;
}



void clearDates() {
  for (int i = 0; i < maxOfRecords; ++i) {
    myDate[i].day = 0;
    myDate[i].month = 0;
    myDate[i].year = 0;
    myDate[i].recordType = "";     // очищает String-поле :contentReference[oaicite:1]{index=1}
    myDate[i].measurmentValue = 0;
  }
}




// Структура для хранения результатов
struct MeasurementStatsSt {
  float average = 0.0;
  int min = 0;
  int max = 0;
  int count = 0;
};



// Функция для получения количества дней в месяце
uint8_t daysInMonth(uint8_t month, uint16_t year) {
  switch (month) {
    case 1: case 3: case 5: case 7: case 8: case 10: case 12: return 31;
    case 4: case 6: case 9: case 11: return 30;
    case 2: // Февраль
      return (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)) ? 29 : 28;
    default: return 0; // Некорректный месяц
  }
}


// Вспомогательная функция для преобразования даты в общее количество дней
long dateToTotalDays(const Date& d) {
  long totalDays = 0;
  uint16_t baseYear = 2000;
  
  for (uint16_t y = baseYear; y < d.year; ++y) {
    totalDays += (y % 4 == 0 && (y % 100 != 0 || y % 400 == 0)) ? 366 : 365;
  }
  
  for (uint8_t m = 1; m < d.month; ++m) {
    totalDays += daysInMonth(m, d.year);
  }
  
  totalDays += d.day;
  
  return totalDays;
}




// Функция для вычисления статистики за N дней от последней даты в массиве
MeasurementStatsSt calculateStats(const Date records[], int recordCount, const String& type, uint8_t daysBack) {
  MeasurementStatsSt stats;
  long sum = 0;
  bool firstRecord = true;

  // Шаг 1: Найти самую позднюю дату в массиве
  long latestTotalDays = 0;
  for (int i = 0; i < recordCount; ++i) {
    long currentTotalDays = dateToTotalDays(records[i]);
    if (currentTotalDays > latestTotalDays) {
      latestTotalDays = currentTotalDays;
    }
  }

  // Определяем пороговое значение дней для фильтрации
  long thresholdTotalDays = latestTotalDays - daysBack;

  // Шаг 2: Вычислить статистику для записей в заданном диапазоне
  for (int i = 0; i < recordCount; ++i) {
    if (records[i].recordType == type) {
      long recordTotalDays = dateToTotalDays(records[i]);

      // Проверяем, находится ли запись в пределах N дней от последней даты
      if (recordTotalDays >= thresholdTotalDays && recordTotalDays <= latestTotalDays) {
        int value = records[i].measurmentValue;
        sum += value;
        stats.count++;

        if (firstRecord) {
          stats.min = value;
          stats.max = value;
          firstRecord = false;
        } else {
          if (value < stats.min) {
            stats.min = value;
          }
          if (value > stats.max) {
            stats.max = value;
          }
        }
      }
    }
  }

  if (stats.count > 0) {
    stats.average = static_cast<float>(sum) / stats.count;
  }

  return stats;
}




// Пользовательская функция для вывода статистики
void printStats(const Date records[], int recordCount, const String& type, uint8_t daysBack) {
  MeasurementStatsSt stats = calculateStats(records, recordCount, type, daysBack);
  
  Serial.print("--- ");
  Serial.print(type);
  Serial.print(" (last ");
  Serial.print(daysBack);
  Serial.println(" days) ---");

  if (stats.count > 0) {
    Serial.print("Average: ");
    Serial.println(stats.average);
    Serial.print("Min: ");
    Serial.println(stats.min);
    Serial.print("Max: ");
    Serial.println(stats.max);
  } else {
    Serial.print("No ");
    Serial.print(type);
    Serial.print(" data found in the last ");
    Serial.print(daysBack);
    Serial.println(" days.");
  }
}



void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial.println("Введіть дату в форматі dd.mm.yy:");
  awaitingDate = true;

   for (int i = 0; i < maxOfRecords; i++) {   
     myDate[i].day = 0;
     myDate[i].month = 0;
     myDate[i].year = 0;
     myDate[i].recordType = "";
     myDate[i].measurmentValue = 0;
 
  }

}



void loop() {    
//  Serial.setTimeout(2000); // таймаут 2 секунды
  String input = Serial.readStringUntil('\n');
  String result = checkSerialInput(input);

  if (result == "command") {
      //Serial.println("✅ Получена команда");   

      if (input == "List") {
          Serial.println("Тут введені дати:");
          dataOutput(input, maxOfRecords, myDate);
           awaitingDate = false;
      }else if (input == "Date"){

        if(!awaitingDate){
           Serial.println("Введіть дату:");
           awaitingDate = true;
        }
        //else{
        //   Serial.println("Ліміт записів для цієї дати перевищений");
        //}
        
      }else if(input == "Temperature"){              
          
          if(awaitingDate){
            Serial.println("Спочатку вкажіть дату, для якої вводите дані");
          } else {

            if(!ChecksAffordableDuplicatesFn(currDateVar, myDate, 5)){
              parseDate(currDateVar, myDate[numberOfRecord], numberOfRecord, input);
              Serial.print("Дата:  ");
              Serial.print(myDate[numberOfRecord].day);
              Serial.print(".");
              Serial.print(myDate[numberOfRecord].month);
              Serial.print(".");              
              Serial.print(myDate[numberOfRecord].year);              
              Serial.print(", тип даних, що вводяться:");
              Serial.println(input);
              Serial.println("Введіть значення температури: ");              

              awaitingDate = false;       
              //numberOfRecord = numberOfRecord + 1;

            }  

          }     
      }else if(input == "Humidity"){              
          
          if(awaitingDate){
            Serial.println("Спочатку вкажіть дату, для якої вводите дані");
          } else {

            if(!ChecksAffordableDuplicatesFn(currDateVar, myDate, 5)){
              parseDate(currDateVar, myDate[numberOfRecord], numberOfRecord, input);
              Serial.print("Дата:  ");
              Serial.print(myDate[numberOfRecord].day);
              Serial.print(".");
              Serial.print(myDate[numberOfRecord].month);
              Serial.print(".");              
              Serial.print(myDate[numberOfRecord].year);              
              Serial.print(", тип даних, що вводяться:");
              Serial.println(input);
              Serial.println("Введіть значення вологості: ");      

              awaitingDate = false;
              awaitingType = true;       
              //numberOfRecord = numberOfRecord + 1;
            }  

          }     
      }else if(input == "Reset"){   

        clearDates(); 
        dataOutput(input, maxOfRecords, myDate);
        awaitingDate = false;
        numberOfRecord = 0;

      }else if(input == "GetStats"){

          // Serial.println("GET STATS command was entered!");
          // Вызов функции для вычисления статистики за последние 5 дней от последней даты
            awaitingNumerOfDays = true;
            Serial.println("Enter the number of dates to calculate statistics:");

//      }else if(awaitingNumerOfDays == true){



      }else if(input == "Pressure"){              
          
          if(awaitingDate){
            Serial.println("Спочатку вкажіть дату, для якої вводите дані");
          } else {

            if(!ChecksAffordableDuplicatesFn(currDateVar, myDate, 5)){
              parseDate(currDateVar, myDate[numberOfRecord], numberOfRecord, input);
              Serial.print("Дата:  ");
              Serial.print(myDate[numberOfRecord].day);
              Serial.print(".");
              Serial.print(myDate[numberOfRecord].month);
              Serial.print(".");              
              Serial.print(myDate[numberOfRecord].year);              
              Serial.print(", тип даних, що вводяться:");
              Serial.println(input);
              Serial.println("Введіть значення тиску: ");      

              awaitingDate = false;       
              awaitingType = true;       
            }  

          }     

      }
  }else if (result == "date") {
  
        if(awaitingDate){

          if(!ChecksAffordableDuplicatesFn(input, myDate, 5)){

            Serial.println("📅 Отримано - дату");  
            currDateVar = input;
            awaitingDate = false;

            Serial.println("Введіть тип вимірів які хочете ввести(температура, вологість, тиск):");  

            //parseDate(input, myDate[dateOfRecord], dateOfRecord);
            //currDateVar = input;
            //dateOfRecord = dateOfRecord + 1;
          } 
          /*else if(ChecksAffordableDuplicatesFn(input, myDate, 5)){

          } */

          awaitingDate = false;

        } else if(!awaitingDate){
         Serial.println("Раніше ви не ввели команду Date");    
         currDateVar = "";

         awaitingDate = false;       
         /*    
          if(!ChecksAffordableDuplicatesFn(input, myDate, 5)){
            parseDate(currDateVar, myDate[dateOfRecord], dateOfRecord);          
            dateOfRecord = dateOfRecord + 1;
          }         
        */

    }

  } else if (result == "") {    
      if (input.length() > 0) {
        Serial.println("❌ Некорректний ввод");
      }

  } else if (result == "int") {
      /*
      Serial.print("Ціле число: ");
      Serial.println(input.toInt());
      Serial.println("numberOfRecord = ");
      Serial.println(myDate[numberOfRecord].recordType);
      */


      if(awaitingNumerOfDays == true){
              printStats(myDate, maxOfRecords, "Temperature", input.toInt());
              printStats(myDate, maxOfRecords, "Humidity", input.toInt());
              printStats(myDate, maxOfRecords, "Pressure", input.toInt()); // Будет "No data found"
              awaitingNumerOfDays = false;

      } else if(awaitingNumerOfDays == false){
        if(myDate[numberOfRecord].day!=0 && myDate[numberOfRecord].month!=0){
          if(myDate[numberOfRecord].recordType!=""){
              myDate[numberOfRecord].measurmentValue = input.toInt();
              Serial.println("Введене значення");
              Serial.print(input);
              numberOfRecord = numberOfRecord + 1;
          }
        }
      }


  }
  
  /*else if (result == "float") {
      Serial.print("Десятичне число: ");
      Serial.println(input.toFloat());
              if(!awaitingDate){
                 Serial.println("Ви не ввели дату вимірювання даних");
              } else {
                 Serial.println("Ви не ввели тип даних для введеного вимірювання (температура, вологість, тиск)");
              } 
  }*/
  
}