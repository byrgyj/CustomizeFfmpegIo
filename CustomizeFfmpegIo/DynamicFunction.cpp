#include "StdAfx.h"
#include "DynamicFunction.h"
#include "windows.h"

#define GET_PROC(dll, funcName) \
    function_##funcName = (dynamic_##funcName##_t)GetProcAddress(dll, #funcName); \
    if (function_##funcName == NULL) { \
        return false; \
    }

DynamicFunction::DynamicFunction(){
}


DynamicFunction::~DynamicFunction(){
}

bool DynamicFunction::init() {
    av_register_all();
    HINSTANCE dll = LoadLibraryA("avformat-57.dll");
    if (dll == NULL) {
        return false;
    }

    GET_PROC(dll, avformat_open_input);
    GET_PROC(dll, avformat_find_stream_info);

    return true;
}

int DynamicFunction::wrapper_avformat_open_input(AVFormatContext **ps, const char *filename, AVInputFormat *fmt, AVDictionary **options) {
    return function_avformat_open_input(ps, filename, fmt, options);
}
int DynamicFunction::wrapper_avformat_find_stream_info(AVFormatContext *ic, AVDictionary **options) {
    return function_avformat_find_stream_info(ic, options);
}