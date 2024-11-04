#include "helper/fonthelper.h"

#include <QApplication>
#include <QFontDatabase>

void FontHelper::setFont(FontEnum e) {
    QString fontPath = FontHelper::fontEnumToFontPath(e);
    int fontId = QFontDatabase::addApplicationFont(fontPath);
    QStringList fontFamilies = QFontDatabase::applicationFontFamilies(fontId);
    if (!fontFamilies.isEmpty()) {
        QFont font(fontFamilies.at(0));
        QApplication::setFont(font);
    }
}

QString FontHelper::currentFont() {
    return QApplication::font().family();
}

void FontHelper::setIconFont(IconFontEnum e) {
    QString fontPath = FontHelper::iconFontEnumToFontPath(e);
    int fontId = QFontDatabase::addApplicationFont(fontPath);
    QStringList fontFamilies = QFontDatabase::applicationFontFamilies(fontId);
    if (!fontFamilies.isEmpty()) {
        m_currentIconFont =  QFont(fontFamilies.at(0));
    }
}

QFont FontHelper::currentIconFont() {
    return m_currentIconFont;
}

QFont FontHelper::defaultIconFont() {
    QFont font = QFont();
    QString fontPath = FontHelper::iconFontEnumToFontPath(IconFont);
    int fontId = QFontDatabase::addApplicationFont(fontPath);
    QStringList fontFamilies = QFontDatabase::applicationFontFamilies(fontId);
    if (!fontFamilies.isEmpty()) {
        font =  QFont(fontFamilies.at(0));
    }
    return font;
}

QString FontHelper::fontEnumToFontPath(FontEnum e) {
    switch (e) {
        case ROBOTO_BLACK: return ":/fonts/Roboto-Black.ttf";
        case ROBOTO_BLACKITALIC: return ":/fonts/Roboto-BlackItalic.ttf";
        case ROBOTO_BOLD: return ":/fonts/Roboto-Bold.ttf";
        case ROBOTO_BOLDITALIC: return ":/fonts/Roboto-BoldItalic.ttf";
        case ROBOTO_ITALIC: return ":/fonts/Roboto-Italic.ttf";
        case ROBOTO_LIGHT: return ":/fonts/Roboto-Light.ttf";
        case ROBOTO_LIGHTITALIC: return ":/fonts/Roboto-LightItalic.ttf";
        case ROBOTO_MEDIUM: return ":/fonts/Roboto-Medium.ttf";
        case ROBOTO_MEDIUMITALIC: return ":/fonts/Roboto-MediumItalic.ttf";
        case ROBOTO_REGULAR: return ":/fonts/Roboto-Regular.ttf";
        case ROBOTO_THIN: return ":/fonts/Roboto-Thin.ttf";
        case ROBOTO_THINITALIC: return ":/fonts/Roboto-ThinItalic.ttf";
        case ROBOTOCONDENSED_BOLD: return ":/fonts/RobotoCondensed-Bold.ttf";
        case ROBOTOCONDENSED_BOLDITALIC: return ":/fonts/RobotoCondensed-BoldItalic.ttf";
        case ROBOTOCONDENSED_ITALIC: return ":/fonts/RobotoCondensed-Italic.ttf";
        case ROBOTOCONDENSED_LIGHT: return ":/fonts/RobotoCondensed-Light.ttf";
        case ROBOTOCONDENSED_LIGHTITALIC: return ":/fonts/RobotoCondensed-LightItalic.ttf";
        case ROBOTOCONDENSED_REGULAR: return ":/fonts/RobotoCondensed-Regular.ttf";
        default: return ":/fonts/Roboto-Regular.ttf";
    }
}

QString FontHelper::iconFontEnumToFontPath(IconFontEnum e) {
    switch (e) {
        case IconFont: return ":/fonts/iconfont.ttf";
        default: return ":/fonts/iconfont.ttf";
    }
}

QFont FontHelper::m_currentIconFont = FontHelper::defaultIconFont();