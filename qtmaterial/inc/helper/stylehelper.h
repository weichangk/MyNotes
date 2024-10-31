#pragma once
#include "qtmaterial_global.h"

class QTMATERIAL_EXPORT StyleHelper {
public:
    static void setStyleToApp(const QString &qssFolder);
    static void setStyleToWidget(QWidget *widget, const QString &fileName);
};
