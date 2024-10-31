#pragma once
#include "qtmaterial_global.h"

class QTMATERIAL_EXPORT ThemeHelper {
public:
    enum ThemeEnum {
        LIGHT = 0,
        DARK,
    };

    static QString themeEnumToString(ThemeEnum);
    static ThemeHelper::ThemeEnum stringToThemeEnum(const QString &);

    static void setTheme(ThemeEnum);
    static QString currentTheme();

private:
    static void setStyleToApp(const QString &);

private:
    static QString m_strCurrentTheme;
};