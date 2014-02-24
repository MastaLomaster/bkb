// Выдвет сообщение о (системных и несистемных) ошибках
#ifndef __BKB_REPERR
#define __BKB_REPERR

// Заголовочные файлы из Tobii Gaze SDK
#include "tobiigaze_error_codes.h"
#include "tobiigaze.h"

void BKBReportError(char *SourceFile, char *FuncName, int LineNumber);
void BKBReportError(char *Error); // Для НЕсистемных ошибок (перегружена)
void BKBReportError(tobiigaze_error_code tbg_error_code, char *SourceFile, char *FuncName, int LineNumber); // Для ошибок Tobii Gaze SDK (перегружена)


#endif