// Выдвет сообщение о (системных и несистемных) ошибках
#ifndef __BKB_REPERR
#define __BKB_REPERR

#include "WIDEFILE.h"

// Заголовочные файлы из Tobii Gaze SDK - избавились от них
// #include "tobiigaze_error_codes.h"
// #include "tobiigaze.h"

void BKBReportError(TCHAR *SourceFile, TCHAR *FuncName, int LineNumber);
void BKBReportError(TCHAR *Error); // Для НЕсистемных ошибок (перегружена)
void BKBReportError(int tbg_error_code, TCHAR *SourceFile, TCHAR *FuncName, int LineNumber); // Для ошибок Tobii Gaze SDK (перегружена)


#endif