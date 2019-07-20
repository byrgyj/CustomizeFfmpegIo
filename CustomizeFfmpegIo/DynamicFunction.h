#pragma once

extern "C"{
#include <libavformat/avformat.h>
};

#define FUNCTION_DEFINE_PARAMTER_4(returnType, funcName, Param1, Param2, Param3, Param4)   \
    typedef returnType##(*dynamic_##funcName##_t)(Param1, Param2, Param3, Param4)

#define FUNCTION_DEFINE_PARAMTER_3(returnType, funcName, Param1, Param2, Param3)   \
    typedef returnType##(*dynamic_##funcName##_t)(Param1, Param2, Param3)

#define FUNCTION_DEFINE_PARAMTER_2(returnType, funcName, Param1, Param2)   \
    typedef returnType##(*dynamic_##funcName##_t)(Param1, Param2)

#define FUNCTION_DEFINE_PARAMTER_1(returnType, funcName, Param1)   \
    typedef returnType##(*dynamic_##funcName##_t)(Param1)

#define CLASS_MEMBER(funcName) \
    dynamic_##funcName##_t  function_##funcName


FUNCTION_DEFINE_PARAMTER_4(int, avformat_open_input, AVFormatContext **ps, const char *filename, AVInputFormat *fmt, AVDictionary **options);
FUNCTION_DEFINE_PARAMTER_2(int, avformat_find_stream_info, AVFormatContext *ic, AVDictionary **options);
class DynamicFunction
{
public:
    DynamicFunction();
    ~DynamicFunction();

    bool init();
    int wrapper_avformat_open_input(AVFormatContext **ps, const char *filename, AVInputFormat *fmt, AVDictionary **options);
    int wrapper_avformat_find_stream_info(AVFormatContext *ic, AVDictionary **options);
private:
    CLASS_MEMBER(avformat_open_input);
    CLASS_MEMBER(avformat_find_stream_info);
};

