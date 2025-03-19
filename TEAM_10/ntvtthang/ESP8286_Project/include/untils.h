#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>
#include <stdarg.h> // Thư viện hỗ trợ định dạng chuỗi

// Hàm kiểm tra thời gian trôi qua
inline bool IsReady(unsigned long &lastTime, unsigned long interval) {
    unsigned long currentTime = millis();
    if (currentTime - lastTime >= interval) {
        lastTime = currentTime;
        return true;
    }
    return false;
}

// Hàm định dạng chuỗi tương tự sprintf
inline String StringFormat(const char *format, ...) {
    char buffer[64];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    return String(buffer);
}

#endif // UTILS_H
