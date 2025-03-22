#include <utils.h>

bool IsReady(unsigned long &ulTimer, uint32_t millisecond) {
    if (millis() - ulTimer < millisecond)
        return false;
    ulTimer = millis();
    return true;
}

String StringFormat(const char* fmt, ...) {
    va_list vaArgs;
    va_start(vaArgs, fmt);
    va_list vaArgsCopy;
    va_copy(vaArgsCopy, vaArgs);
    const int iLen = vsnprintf(NULL, 0, fmt, vaArgsCopy);
    va_end(vaArgsCopy);
    
    char* buff = (char*)malloc(iLen + 1);
    vsnprintf(buff, iLen + 1, fmt, vaArgs);
    va_end(vaArgs);
    
    String s = buff;
    free(buff);
    return s;
}
