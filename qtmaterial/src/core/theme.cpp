#include "core/theme.h"

#include <QApplication>
#include <QDirIterator>

namespace core {
QString Theme::themeEnumToString(ThemeEnum e) {
    switch (e) {
    case LIGHT: return "light";
    case DARK: return "dark";
    default: return "light";
    }
}

Theme::ThemeEnum Theme::stringToThemeEnum(const QString &s) {
    if (s == "dark") return DARK;
    return LIGHT;
}

void Theme::setTheme(ThemeEnum theme) {
    m_strCurrentTheme = themeEnumToString(theme);
    setStyleToApp(":/qss/" + m_strCurrentTheme);
}

QString Theme::currentTheme() {
    return m_strCurrentTheme;
}

void Theme::setStyleToApp(const QString &qssFolder) {
    QString allStyle;
    QDirIterator it(qssFolder, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        it.next();
        if (it.fileInfo().isFile() && it.fileInfo().suffix() == "qss") {
            QString path = it.filePath();
            QFile file(path);
            file.open(QIODevice::ReadOnly);
            QString style = file.readAll();
            if (!style.isEmpty()) {
                allStyle += style;
            }
            file.close();
        }
    }
    qApp->setStyleSheet(allStyle);
}

QString Theme::m_strCurrentTheme = Theme::themeEnumToString(LIGHT);
} // namespace core