// Загружает строки из messages.bkb и выдаёт из по запросу (индексу)
#ifndef __BKB_INTERNL
#define __BKB_INTERNL

#define BKB_MAX_MESSAGES 100

class Internat
{
public:
	static void LoadMessages();
	static void Unload(); // Освобождаем память
	static TCHAR *Message(int i, TCHAR *default_string);
protected:
	static TCHAR *messages[BKB_MAX_MESSAGES]; // Здесь лежат все загруженные строки
};

#endif