/*
 * @Author: weick
 * @Date: 2023-12-05 22:58:35
 * @Last Modified by:   weick
 * @Last Modified time: 2023-12-05 22:58:35
 */

#pragma once
#include <QtWidgets/qwidget.h>
#include <QtCore/qglobal.h>

#ifdef WIN32

#ifdef AWIDGET_LIB
#define AWIDGET_EXPORT __declspec(dllexport)
#else
#define AWIDGET_EXPORT __declspec(dllimport)
#endif

#else

#define AWIDGET_EXPORT

#endif
