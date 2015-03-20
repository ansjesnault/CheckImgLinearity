#pragma once

#ifdef WIN32
#define THIS_FILE (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#define THIS_FUNCTION __FUNCTION__
#else
#define THIS_FILE (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define THIS_FUNCTION __PRETTY_FUNCTION__
#endif

#define _STR(x) #x
#define STR(x) _STR(x)

#ifndef FILE_LINE_STR
#define FILE_LINE_STR std::string(std::string(THIS_FILE) + std::string("(") + STR(__LINE__) + std::string(")"))
#endif

#ifndef FILE_LINE_FUNC_STR
#define FILE_LINE_FUNC_STR std::string( FILE_LINE_STR + std::string(" in ") + std::string(THIS_FUNCTION))
#endif