// Внешняя клавиатура для программы bkb
// Работает только на Arduino Leonardo!!!
// Используйте, если какая-нибудь программа отказывается работать с программно-эмулируемой клавиатурой
// External keyboard emulator
// Works with Arduino Leonardo only!!!
// Use it when software keyboard emulation does not work with some applications

#include <Keyboard.h>

void setup() {
  // put your setup code here, to run once:
    Serial.begin(9600);
    while (!Serial); // wait for Leonardo enumeration, others continue immediately
    Keyboard.begin();
}

unsigned char c,byte2,byte3,CRC;
int mstep=0;

void loop() {
   // 1. Принимаем команду от компьютера
  while(Serial.available())
  {
    c=Serial.read();
    // Ловим заголовок пакета
    if(0xFF==c)
    {
      mstep=1;
      CRC=0xFF;
    }
    else
    {
      mstep++;
      switch(mstep)
      {
        case 2: // Здесь будут модификаторы
          byte2=c;
          CRC+=c;
          break;

        case 3: // Собственно символ
          byte3=c;
          CRC+=c;
          // Для отладки!! Без проверки CRC!!!
          //Keyboard.write(byte3);
          break;

        case 4: // Здесь отловим CRC, проверим его, и вышлем символ
          if(CRC==c) 
          {
            if(byte2&1) // Нажать Shift
            {
              Keyboard.press(KEY_LEFT_SHIFT);
            }
            
            if(byte2&2) // Нажать Ctrl
            {
              Keyboard.press(KEY_LEFT_CTRL);
            }

            if(byte2&4) // Нажать Alt
            {
              Keyboard.press(KEY_LEFT_ALT);
            }
            
            Keyboard.press(byte3);
            delay(80);

            Keyboard.releaseAll();
          }
          break;

        default:
          mstep=0;
          
          
      } // switch
    }

    // Пока просто пишем что угодно
    //Keyboard.write('A');
    
  }

}
