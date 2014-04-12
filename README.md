*For English  scroll down*

###Программа для управления компьютером при помощи глаз и одного из устройств: трекера движения глаз Tobii REX, трекера The Eye Tribe или любого устройства, способного перемещать курсор мыши по экрану (например, аэромыши).

Подробнее можно прочитать здесь:  

http://habrahabr.ru/post/208108/ - про работу с трекерами движения глаз

http://habrahabr.ru/post/213715/ - про работу с аэромышью без рук

###Установка:
Просто разархивируйте bkb32b-Russian.zip в любой каталог на вашем компьютере.
Для запуска программы необходимо, чтобы на компьютере был установлен Microsoft Visual C++ 2010 Redistributable Package (x86). Его можно взять вот здесь:
http://www.microsoft.com/en-us/download/details.aspx?id=5555
Если он у вас не установлен, то при запуске программы вы увидите сообщение о том, что не найден файл msvcrt100.dll

###использование с Tobii Rex
Для работы программы с Tobii Rex необходимы файлы, которые идут в комплекте с Gaze SDK 2.0:
 TobiiGazeConfig32.dll
 TobiiGazeCore32.dll
Скопируйте их в рабочий каталог программы.
Эти файлы поставлялись на флешке вместе с самим устройством Tobii REX. Там же были необходимые драйверы устройства.
Перед началом работы зайдите в кортрольную панель Windows, запустите "Tobii Eye Tracking" и  откалибруйте устройство.

В ближайшее время я планирую переписать программу для поддержки последнийх версий  драйверов/программного обеспечения от Tobii.

###Использование с The Eye Tribe
Должна быть запущена программа Eye Tribe Server. Также нужно откалибровать устройство при помощи программы Eye Tribe UI перед запуском программы bkb32b.exe

###Звук нажатия клавиш
При нажатии клавиш слышится звук. Если он вас не устраивает, положите в рабочий каталог программы WAV-файл со звуком, который вам нравится, и назовите его "click.wav".

###Общие принципы работы

После запуска программы и выбора устройства для работы, с правой стороны экрана появляется панель инструментов. Если вы используете трекер движения глаз, то также появляется прозрачное окно с курсором, который следует за вашим взглядом. При использовании аэромыши управляйте обычным курсором. Для выбора режима задержите взгляд на одном из пунктов меню панели инструментов.

Посмотреть, как пользоваться программой можно на этих видео:

http://youtu.be/O68C4d2SNC8

http://youtu.be/rqcN9IZ39_4


###Известные проблемы:
- нет простого способа выйти из программы, надо закрывать окна в панели задач
- не работает в полноэкранных приложениях
- не работает в Metro-интерфейсе Windows 8/8.1, нужно пользоваться старорежимным рабочим столом
- некоторые программы (созданные до эпохи unicode) неправильно воспринимают символы на русской клавиатуре
- drag-and-drop не всегда корректно работает, в частности, не перетаскивает иконки на рабочем столе
- сбивается положение окон после logout/switch user
- нельзя настроить время нажатия клавиш клавиатуры и вообще все задержки
- нельзя щелкнуть мышью, удерживая нажатой клавиши (Ctrl, например)
ну и ещё куча по мелочи...

###Перевод на другие языки:
Вы можете легко перевести интерфейс программы на другой язык, если возьмёте у же переведённую версию (Английскую) и отредактирете файлы messages.bkb и keyboard.bkb (это обычные текстовые файлы с unicode-кодировкой).
Но формат и содержимое файлов может измениться в будущих версиях!

###Компилирование исходников

Нужно использовать Microsoft Visual Studio 2010 (предпочтительно с 1 сервис-паком).
Это объясняется тем, что так были скомпилированы библиотеки Tobii Gaze SDK 2.0, которые используются в программе.
В свойствах проекта установите поддержку unicode
Включаемые каталоги должны иметь в своем составе каталог "include" из Tobii Gaze SDK 2.0.
Его (пока ещё) можно скачать с сайта Tobii:
http://www.tobii.com/en/eye-experience/support/old-tobii-gaze-sdk/gaze-sdk-c-api/

Никакие библиотеки из Tobii Gaze SDK при компиляции не требуются, а вот стандартные библиотеки  
Windows нужны: Ws2_32.lib, winmm.lib,Msimg32.lib

Проект должен динамически подключать библиотеку MSVCRT100.dll, статическая линковка будет  конфликтовать с библиотеками Tobii Gaze SDK!

##English

bkb is a program to control keyboard/mouse with eyes It supports TobiiREX eye tracker, The Eye Trybe eye tracker, and any device that can move a mouse cursor (e.g. an airmouse).

###Installation:
Just unzip the bkb32b-English.zip to any folder. Make sure that this folder remains the working  directory of the program. Otherwise the program won't load messages.bkb and keyboard.bkb files,  and you'll get Russian interface instead of English one.

To run the program you also need Microsoft Visual C++ 2010 Redistributable Package (x86). It can be downloaded here:

http://www.microsoft.com/en-us/download/details.aspx?id=5555

If it is not installed, you'll get the error message complaining that the file "msvcrt100.dll"  cannot be found.

###Using the program with the Tobii REX eye tracker
You need the following files from the Tobii Gaze SDK 2.0 to be copied to the working directory of  the program:

 TobiiGazeConfig32.dll

 TobiiGazeCore32.dll

These files were shipping along with the drivers and the device itself.
Before starting the program, visit Windows Control Panel, run the "Tobii Eye Tracking" program, and calibrate the device.

Adaptation of the program to the new Tobii SDK is coming soon.

###Using the program with The Eye Tribe tracker

The "Eye Tribe Server" program must be running. Also you need to calibrate the device with the "Eye Tribe UI" before running  the bkb32b.exe

###Keyboard click sounds
There is a click sound when you press the keyboard buttons. If you don't like the sound, place a WAV-file with the desired sound into the working directory of the program and name it "click.wav".

###Basic work principles
After program started and a supported device is selected, you may see the toolbar on the right side. If you ise an eye tracker, the transparent window with the cursor will be shown, it will follow your eyes movements. When using an [air]mouse, the regular cursor is used. To select a tool fixate your eyes on the tool button.

Take a look at these videos to understand the modes of operation:

http://youtu.be/O68C4d2SNC8

**IMPORTANT**: choose Swahili language to watch English subtitles. Sorry, I don't know other ways to switch off the subtitles by default.

http://youtu.be/rqcN9IZ39_4

###Known issues:
- no easy way to exit the program. One have to close windows in the task bar
- doesn't work with fullscreen applacations so far
- doesn't work with the Metro-style interface of Windows 8/8.1, you have to use good old desktop
- drag-and-drop doesn't work in some cases, for example you cannot move desktop icons
- windows moved and doesn't work properly after logout/switch user
- you cannot define timings (keyboard press, fixations, etc.)
- impossible to click with a mouse holding the keyboard button pressed (e.g. Ctrl + click)
- and many more small things....

###Translate to other languages:
One can easily translate the User Interface and modify a keyboard. Just edit the "messages.bkb" and "keyboard.bkb" files. These are text unicode files. But (!) the file format and contents can be changed in future!!!

###Compiling the source codes
As for now, you have to use Microsoft Visual Studio 2010 (service pack 1 preferred). This is due to the fact that the libraries used (from the Tobii Gaze SDK 2.0) are compiled the same way.

In the project properties enable unicode support

Include directories must contain the "include" one from the Tobii Gaze SDK 2.0.

It (still) can be downloaded from the Tobii site:

http://www.tobii.com/en/eye-experience/support/old-tobii-gaze-sdk/gaze-sdk-c-api/

No Tobii Gaze SDK libraries needed during the compilation/build. 

You need just standard Windows libraries: Ws2_32.lib, winmm.lib,Msimg32.lib

The project must be linked dynamically to the MSVCRT100.dll, if you link statically, this will be in conflict with the Tobii Gaze SDK libraries used!
