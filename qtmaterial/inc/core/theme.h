#pragma once
#include "qtmaterial_global.h"

namespace core {
class QTMATERIAL_EXPORT Theme {
public:
    enum ThemeEnum {
        LIGHT = 0,
        DARK,
    };

    static QString themeEnumToString(ThemeEnum);
    static Theme::ThemeEnum stringToThemeEnum(const QString &);

    static void setTheme(ThemeEnum);
    static QString currentTheme();

private:
    static void setStyleToApp(const QString &);

private:
    static QString m_strCurrentTheme;
};
} // namespace core