#include "core/font.h"

#include <QApplication>
#include <QFontDatabase>

namespace core {
void Font::setFont(FontEnum e) {
    QString fontPath = Font::fontEnumToFontPath(e);
    int fontId = QFontDatabase::addApplicationFont(fontPath);
    QStringList fontFamilies = QFontDatabase::applicationFontFamilies(fontId);
    if (!fontFamilies.isEmpty()) {
        QFont font(fontFamilies.at(0));
        QApplication::setFont(font);
    }
}

QString Font::currentFont() {
    return QApplication::font().family();
}

void Font::setIconFont(IconFontEnum e) {
    QString fontPath = Font::iconFontEnumToFontPath(e);
    int fontId = QFontDatabase::addApplicationFont(fontPath);
    QStringList fontFamilies = QFontDatabase::applicationFontFamilies(fontId);
    if (!fontFamilies.isEmpty()) {
        m_currentIconFont = QFont(fontFamilies.at(0));
    }
}

QFont Font::currentIconFont() {
    return m_currentIconFont;
}

QString Font::fontEnumToFontPath(FontEnum e) {
    switch (e) {
    case ROBOTO_BLACK: return ":/font/Roboto-Black.ttf";
    case ROBOTO_BLACKITALIC: return ":/font/Roboto-BlackItalic.ttf";
    case ROBOTO_BOLD: return ":/font/Roboto-Bold.ttf";
    case ROBOTO_BOLDITALIC: return ":/font/Roboto-BoldItalic.ttf";
    case ROBOTO_ITALIC: return ":/font/Roboto-Italic.ttf";
    case ROBOTO_LIGHT: return ":/font/Roboto-Light.ttf";
    case ROBOTO_LIGHTITALIC: return ":/font/Roboto-LightItalic.ttf";
    case ROBOTO_MEDIUM: return ":/font/Roboto-Medium.ttf";
    case ROBOTO_MEDIUMITALIC: return ":/font/Roboto-MediumItalic.ttf";
    case ROBOTO_REGULAR: return ":/font/Roboto-Regular.ttf";
    case ROBOTO_THIN: return ":/font/Roboto-Thin.ttf";
    case ROBOTO_THINITALIC: return ":/font/Roboto-ThinItalic.ttf";
    case ROBOTOCONDENSED_BOLD: return ":/font/RobotoCondensed-Bold.ttf";
    case ROBOTOCONDENSED_BOLDITALIC: return ":/font/RobotoCondensed-BoldItalic.ttf";
    case ROBOTOCONDENSED_ITALIC: return ":/font/RobotoCondensed-Italic.ttf";
    case ROBOTOCONDENSED_LIGHT: return ":/font/RobotoCondensed-Light.ttf";
    case ROBOTOCONDENSED_LIGHTITALIC: return ":/font/RobotoCondensed-LightItalic.ttf";
    case ROBOTOCONDENSED_REGULAR: return ":/font/RobotoCondensed-Regular.ttf";
    default: return ":/font/Roboto-Regular.ttf";
    }
}

QString Font::iconFontEnumToFontPath(IconFontEnum e) {
    switch (e) {
    case IconFont: return ":/font/iconfont.ttf";
    default: return ":/font/iconfont.ttf";
    }
}

QFont Font::m_currentIconFont = QFont();
} // namespace core