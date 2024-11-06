#pragma once
#include "qtmaterial_global.h"
#include <QFont>

namespace core {
class QTMATERIAL_EXPORT Font {
public:
    enum FontEnum {
        ROBOTO_BLACK = 0,
        ROBOTO_BLACKITALIC,
        ROBOTO_BOLD,
        ROBOTO_BOLDITALIC,
        ROBOTO_ITALIC,
        ROBOTO_LIGHT,
        ROBOTO_LIGHTITALIC,
        ROBOTO_MEDIUM,
        ROBOTO_MEDIUMITALIC,
        ROBOTO_REGULAR,
        ROBOTO_THIN,
        ROBOTO_THINITALIC,
        ROBOTOCONDENSED_BOLD,
        ROBOTOCONDENSED_BOLDITALIC,
        ROBOTOCONDENSED_ITALIC,
        ROBOTOCONDENSED_LIGHT,
        ROBOTOCONDENSED_LIGHTITALIC,
        ROBOTOCONDENSED_REGULAR,
    };

    enum IconFontEnum {
        IconFont = 0,
    };

    static void setFont(FontEnum);
    static QString currentFont();

    static void setIconFont(IconFontEnum);
    static QFont currentIconFont();

private:
    static QString fontEnumToFontPath(FontEnum);
    static QString iconFontEnumToFontPath(IconFontEnum);

private:
    static QFont m_currentIconFont;
};
} // namespace core