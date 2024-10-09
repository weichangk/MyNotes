/*
 * @Author: weick
 * @Date: 2023-12-05 22:48:07
 * @Last Modified by:   weick
 * @Last Modified time: 2023-12-05 22:48:07
 */

#pragma once
#include "acore_global.h"
#include <QTranslator>

enum ACORE_EXPORT ALangLocale {
    lang_locale_en,
    lang_locale_zh,
};

struct ACORE_EXPORT ALangConstInfo {
    ALangLocale ll;
    QString dirname;
    QString simplename1;
    QString simplename2;
    QString fullname;
};

struct ACORE_EXPORT AOneLang {
    ALangLocale m_lang;
    QList<QTranslator *> m_lstTranslators;
    AOneLang(ALangLocale lang);
    ~AOneLang();
    void init(ALangLocale lang);
};

class ACORE_EXPORT ALangMgr : public QObject {
    Q_OBJECT
public:
    static ALangMgr *getInstance();
    ~ALangMgr();
    static void release();
    void setSysLangLocale();
    void setLangLocale(ALangLocale lang);

protected:
    ALangMgr(QObject *parent = nullptr);
    void init();
    void loadCurrentLang();
    void removeCurrentLang();

private:
    int getQtLocale(ALangLocale lang);

private:
    QList<AOneLang *> m_lstLangs;
    ALangLocale m_curLang;
};
