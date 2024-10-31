#include "helper/themehelper.h"

#include <QApplication>
#include <QDirIterator>

QString ThemeHelper::themeEnumToString(ThemeEnum e) {
    switch (e) {
    case LIGHT: return "light";
    case DARK: return "dark";
    default: return "light";
    }
}

ThemeHelper::ThemeEnum ThemeHelper::stringToThemeEnum(const QString& s) {
    if (s == "dark") return DARK;
    return LIGHT;
}

void ThemeHelper::setTheme(ThemeEnum theme) {
    m_strCurrentTheme = themeEnumToString(theme);
    setStyleToApp(":/qss/" + m_strCurrentTheme);
}

QString ThemeHelper::currentTheme() {
    return m_strCurrentTheme;
}

void ThemeHelper::setStyleToApp(const QString &qssFolder) {
    QString allStyle;
    QDirIterator it(qssFolder, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        it.next();
        if (it.fileInfo().isFile()  && it.fileInfo().suffix() == "qss") {
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

QString ThemeHelper::m_strCurrentTheme = ThemeHelper::themeEnumToString(LIGHT);