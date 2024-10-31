#pragma once
#include "qtmaterial_global.h"

class QTMATERIAL_EXPORT ThemeHelper {
public:
    enum ThemeEnum {
        LIGHT = 0,
        DARK,
    };

    static  QString themeEnumToString(ThemeEnum e) {
        switch (e) {
        case LIGHT: return "light";
        case DARK: return "dark";
        default: return "light";
        }
    }

    static ThemeEnum stringToThemeEnum(const QString& s) {
        if (s == "dark") return DARK;
        return LIGHT;
    }

    static void setTheme(ThemeEnum theme) {
        m_strCurrentTheme = themeEnumToString(theme);
    }

    static QString currentTheme() {
        return m_strCurrentTheme;
    }

private:
    static QString m_strCurrentTheme;
};