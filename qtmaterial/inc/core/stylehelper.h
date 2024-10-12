#pragma once
#include "inc/qtmaterial_global.h"

class QTMATERIAL_EXPORT StyleHelper {
public:
    static void setStyleToApp(const QString &qssFolder);
    static void setStyleToWidget(QWidget *widget, const QString &fileName);
};
