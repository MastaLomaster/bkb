*For English 'readme' scroll down*

### Программа для управления компьютером при помощи глаз и одного из трекеров движения глаз: Tobii REX, Tobii EyeX, Tobii Eye Tracker 4C (только 32-битная версия программы: bkb32*.exe), The Eye Tribe, Gazepoint GP3. Также поддерживается управление поворотом головы вместе с любым устройством, способным перемещать курсор мыши по экрану (например, с аэромышью).

Подробнее можно прочитать здесь:  

http://habrahabr.ru/post/208108/ - про работу с трекерами движения глаз

http://habrahabr.ru/post/213715/ - про работу с аэромышью без рук

### Установка:
Просто разархивируйте bkb32c-Russian.zip в любой каталог на вашем компьютере.
Для запуска программы необходимо, чтобы на компьютере был установлен Microsoft Visual C++ Redistributable for Visual Studio 2012 Update 4. Его можно взять вот здесь:
http://www.microsoft.com/ru-ru/download/details.aspx?id=30679
Если он у вас не установлен, то при запуске программы вы увидите сообщение о том, что не найден файл msvcrt110.dll


### Использование с Tobii Rex или Tobii EyeX или Tobii Eye Tracker 4C
Для работы программы с **Tobii Rex** или **Tobii EyeX** или **Tobii Eye Tracker 4C** необходим файл "TobiiGazeCore32.dll" версии не ниже 4.1.3.938, который нужно скопировать в рабочий каталог программы.

В настоящий момент компания Tobii закрыла возможность скачивания Gaze SDK, содержащего этот файл. Но вы можете найти его, если установите "Tobii EyeX 2.1.1" ( http://files.update.tech.tobii.com/engine/Tobii.EyeX_2.1.1.293.exe ), как 
указано в документе https://help.tobii.com/hc/en-us/articles/213546185-Attention-all-REX-users.

~~Gaze SDK 4.0 можно скачать вот здесь: http://developer.tobii.com/downloads/ (требуется регистрация)
Ищите файл с именем TobiiGazeSdk-CApi-4.0.X.XXX-Win32, где X-цифры текущей версии.~~

Перед началом работы откалибруйте устройство.

### Использование с The Eye Tribe
Должна быть запущена программа Eye Tribe Server. Также нужно откалибровать устройство при помощи программы Eye Tribe UI перед запуском программы bkb32d.exe

### Использование с Gazepoint GP3
Должна быть запущена программа Gazepoint Control.

### Звук нажатия клавиш
При нажатии клавиш слышится звук. Если он вас не устраивает, положите в рабочий каталог программы WAV-файл со звуком, который вам нравится, и назовите его "click.wav".

### Общие принципы работы

После запуска программы и выбора устройства для работы, с правой стороны экрана появляется панель инструментов. Если вы используете трекер движения глаз, то также появляется прозрачное окно с курсором, который следует за вашим взглядом. При использовании аэромыши управляйте обычным курсором. Для выбора режима задержите взгляд на одном из пунктов меню панели инструментов.

Посмотреть, как пользоваться программой можно на этих видео:

http://youtu.be/O68C4d2SNC8

http://youtu.be/rqcN9IZ39_4


### Известные проблемы:
- нет простого способа выйти из программы, надо закрывать окна в панели задач
- не работает в полноэкранных приложениях
- не работает в Metro-интерфейсе Windows 8/8.1, нужно пользоваться старорежимным рабочим столом
- некоторые программы (созданные до эпохи unicode) неправильно воспринимают символы на русской клавиатуре
- drag-and-drop не всегда корректно работает, в частности, не перетаскивает иконки на рабочем столе на некоторых машинах
- сбивается положение окон после logout/switch user
ну и ещё куча по мелочи...

### Перевод на другие языки:
Вы можете легко перевести интерфейс программы на другой язык, если возьмёте уже переведённую версию (Английскую) и отредактируете файлы messages.bkb и keyboard.bkb (это обычные текстовые файлы с unicode-кодировкой).
Но формат и содержимое файлов может измениться в будущих версиях!

### Компилирование исходников

Нужно использовать Microsoft Visual Studio 2012 (предпочтительно с последним обновлением).
Это объясняется тем, что так были скомпилированы библиотеки Tobii Gaze SDK 4.0, которые используются в программе.
В свойствах проекта установите поддержку unicode

~~Включаемые каталоги должны иметь в своем составе каталог "include" из Tobii Gaze SDK 4.0.
Gaze SDK 4.0 можно скачать вот здесь: http://developer.tobii.com/downloads/ (требуется регистрация)
Ищите файл с именем TobiiGazeSdk-CApi-4.0.X.XXX-Win32, где X-цифры текущей версии.~~

Никакие библиотеки из Tobii Gaze SDK при компиляции не требуются, а вот стандартные библиотеки  
Windows нужны: Ws2_32.lib, winmm.lib,Msimg32.lib

Проект должен динамически подключать библиотеку MSVCRT110.dll, статическая линковка будет  конфликтовать с библиотеками Tobii Gaze SDK!

## in English

bkb is a program to control keyboard/mouse with eyes. It supports the following eye trackers: TobiiREX, Tobii EyeX, Tobii Eye Tracker 4C (32-bit version of the program only: bkb32*.exe), The Eye Trybe, Gazepoint GP3. It also supports controlling keyboard/mouse by turning the head along with any device that can move a mouse cursor (e.g. an airmouse).

### Installation:
Just unzip the bkb32c-English.zip to any folder. Make sure that this folder remains the working  directory of the program. Otherwise the program won't load messages.bkb and keyboard.bkb files,  and you'll get Russian interface instead of English one.

To run the program you also need Microsoft Visual C++ Redistributable for Visual Studio 2012 Update 4. It can be downloaded here:

http://www.microsoft.com/en-us/download/details.aspx?id=30679

If it is not installed, you'll get the error message complaining that the file "msvcrt110.dll"  cannot be found.

### Using the program with Tobii REX or Tobii EyeX or Tobii Eye Tracker 4C

You will need "TobiiGazeCore32.dll" version 4.1.3.938 or newer to be copied to the working directory of the program. 

At the moment Tobii has discontinued the ability to download Gaze SDK, containing the file. But you can get it if you install "Tobii EyeX 2.1.1" ( http://files.update.tech.tobii.com/engine/Tobii.EyeX_2.1.1.293.exe ), as 
stated in the document: https://help.tobii.com/hc/en-us/articles/213546185-Attention-all-REX-users.

~~Alternatively, Gaze SDK 4.0 can be downloaded from: http://developer.tobii.com/downloads/ (registration required).
Look for the "TobiiGazeSdk-CApi-4.0.X.XXX-Win32" file, where X-current release numbers.~~

Calibrate the device before starting the program.

### Using the program with The Eye Tribe tracker

The "Eye Tribe Server" program must be running. Also you need to calibrate the device with the "Eye Tribe UI" before running  the bkb32d.exe

### Using the program with Gazepoint GP3 tracker

The "Gazepoint Control" program must be running.

### Keyboard click sounds
There is a click sound when you press the keyboard buttons. If you don't like the sound, place a WAV-file with the desired sound into the working directory of the program and name it "click.wav".

### Basic work principles
After program started and a supported device is selected, you may see the toolbar on the right side. If you use an eye tracker, the transparent window with the cursor will be shown, it will follow your eyes movements. When using an [air]mouse, the regular cursor is used. To select a tool fixate your eyes on the tool button.

Take a look at these videos to understand the modes of operation:

http://youtu.be/O68C4d2SNC8

(English subtitles)

http://youtu.be/rqcN9IZ39_4

### Known issues:
- no easy way to exit the program. One have to close windows in the task bar
- doesn't work with fullscreen applacations so far
- doesn't work with the Metro-style interface of Windows 8/8.1, you have to use good old desktop
- drag-and-drop doesn't work in some cases, for example you cannot move desktop icons on some PCs
- windows moved and doesn't work properly after logout/switch user
- and many more small things....

### Translate to other languages:
One can easily translate the User Interface and modify a keyboard. Just edit the "messages.bkb" and "keyboard.bkb" files. These are text unicode files. But (!) the file format and contents can be changed in future!!!

### Compiling the source codes
As for now, you have to use Microsoft Visual Studio 2012 (latest update preferred). This is due to the fact that the libraries used (from the Tobii Gaze SDK 4.0) are compiled the same way.

In the project properties enable unicode support

~~Include directories must contain the "include" one from the Tobii Gaze SDK 4.0.
Gaze SDK 4.0 can be downloaded from: http://developer.tobii.com/downloads/ (registration required).
Look for the "TobiiGazeSdk-CApi-4.0.X.XXX-Win32" file, where X-current release numbers.~~

No Tobii Gaze SDK libraries needed during the compilation/build. 

You need just standard Windows libraries: Ws2_32.lib, winmm.lib,Msimg32.lib

The project must be linked dynamically to the MSVCRT110.dll, if you link statically, this will be in conflict with the Tobii Gaze SDK libraries used!
