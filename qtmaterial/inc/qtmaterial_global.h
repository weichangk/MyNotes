#pragma once
#include <QtWidgets/qwidget.h>
#include <QtCore/qglobal.h>

#ifdef WIN32

#ifdef QTMATERIAL_LIB
#define QTMATERIAL_EXPORT __declspec(dllexport)
#else
#define QTMATERIAL_EXPORT __declspec(dllimport)
#endif

#else

#define QTMATERIAL_EXPORT

#endif
