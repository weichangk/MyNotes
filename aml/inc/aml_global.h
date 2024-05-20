/*
 * @Author: weick 
 * @Date: 2024-05-20 07:55:39 
 * @Last Modified by:   weick 
 * @Last Modified time: 2024-05-20 07:55:39 
 */


#pragma once
#include <QtCore/qglobal.h>

#ifdef WIN32

#ifdef AML_LIB
#define AML_EXPORT __declspec(dllexport)
#else
#define AML_EXPORT __declspec(dllimport)
#endif

#else

#define AML_EXPORT

#endif
