#pragma once
#ifndef UTILS_H  // Đảm bảo chỉ khai báo một lần
#define UTILS_H
 #include <Arduino.h>
 #include <stdarg.h>

 
 bool IsReady(unsigned long &ulTimer, uint32_t millisecond);
 String StringFormat(const char* fmt, ...);
 #endif