#pragma once
#include "qtmaterial_global.h"
#include <QTranslator>

namespace core {
enum QTMATERIAL_EXPORT LangLocale {
    lang_locale_en,
    lang_locale_zh,
};

struct QTMATERIAL_EXPORT LangConstInfo {
    LangLocale ll;
    QString dirname;
    QString simplename1;
    QString simplename2;
    QString fullname;
};

struct QTMATERIAL_EXPORT OneLang {
    LangLocale m_lang;
    QList<QTranslator *> m_lstTranslators;
    OneLang(LangLocale lang);
    ~OneLang();
    void init(LangLocale lang);
};

class QTMATERIAL_EXPORT Lang : public QObject {
    Q_OBJECT
public:
    static Lang *getInstance();
    ~Lang();
    static void release();
    void setSysLangLocale();
    void setLangLocale(LangLocale lang);

protected:
    Lang(QObject *parent = nullptr);
    void init();
    void loadCurrentLang();
    void removeCurrentLang();

private:
    int getQtLocale(LangLocale lang);

private:
    QList<OneLang *> m_lstLangs;
    LangLocale m_curLang;
};
} // namespace core