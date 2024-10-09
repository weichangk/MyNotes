/*
 * @Author: weick
 * @Date: 2023-12-05 22:49:17
 * @Last Modified by:   weick
 * @Last Modified time: 2023-12-05 22:49:17
 */

#include "inc/alangmgr.h"
#include <QApplication>
#include <QDir>

static const size_t g_nInfos = 2;
static const ALangConstInfo g_LangInfos[g_nInfos] = {
    {lang_locale_en, "en", "ENG", "en-us", "English"},
    {lang_locale_zh, "zh", "CHS", "zh-cn", "Chinese"}};

AOneLang::AOneLang(ALangLocale lang) {
    init(lang);
}

void AOneLang::init(ALangLocale lang) {
    QString langDir("");
    for (auto langInfo : g_LangInfos) {
        if (lang == langInfo.ll) {
            m_lang = lang;
            langDir = langInfo.dirname;
            break;
        }
    }

    if (langDir.isEmpty()) {
        m_lang = lang_locale_en;
        langDir = "en";
    }

    langDir = qApp->applicationDirPath() + "/" + "lang/" + langDir + "/";

    QDir dir(langDir);
    QStringList filelist = dir.entryList(QStringList() << "*.qm", QDir::Files);

    for (auto file : filelist) {
        QTranslator *pTrans = new QTranslator;
        pTrans->load(langDir + file);
        m_lstTranslators.push_back(pTrans);
    }
}

AOneLang::~AOneLang() {
    for (auto translator : m_lstTranslators) {
        delete translator;
    }
}

static ALangMgr *m_instance = nullptr;

ALangMgr *ALangMgr::getInstance() {
    if (m_instance == nullptr) {
        m_instance = new ALangMgr;
    }
    return m_instance;
}

ALangMgr::ALangMgr(QObject *parent) :
    QObject(parent) {
    QTranslator translator;
    QLocale locale = QLocale::system();
    switch (locale.language()) {
    case QLocale::English:
        m_curLang = lang_locale_en;
        break;
    case QLocale::Chinese:
        m_curLang = lang_locale_zh;
        break;
    default:
        m_curLang = lang_locale_en;
        break;
    }
    init();
    loadCurrentLang();
}

ALangMgr::~ALangMgr() {
    for (auto lang : m_lstLangs) {
        delete lang;
    }
}

void ALangMgr::release() {
    if (m_instance) {
        delete m_instance;
        m_instance = nullptr;
    }
}

void ALangMgr::setSysLangLocale() {
    QLocale::setDefault((QLocale::Language)getQtLocale(m_curLang));
}

void ALangMgr::setLangLocale(ALangLocale lang) {
    removeCurrentLang();
    m_curLang = lang;
    loadCurrentLang();
    QLocale::setDefault((QLocale::Language)getQtLocale(m_curLang));
}

void ALangMgr::init() {
    for (auto langInfo : g_LangInfos) {
        AOneLang *lang = new AOneLang(langInfo.ll);
        m_lstLangs.push_back(lang);
    }
}

void ALangMgr::loadCurrentLang() {
    for (auto lang : m_lstLangs) {
        if (m_curLang == lang->m_lang) {
            for (auto translator : lang->m_lstTranslators) {
                qApp->installTranslator(translator);
            }
        }
    }
}

void ALangMgr::removeCurrentLang() {
    for (auto lang : m_lstLangs) {
        if (m_curLang == lang->m_lang) {
            for (auto translator : lang->m_lstTranslators)
                qApp->removeTranslator(translator);
        }
    }
}

int ALangMgr::getQtLocale(ALangLocale lang) {
    switch (lang) {
    case lang_locale_en:
        return QLocale::English;
    case lang_locale_zh:
        return QLocale::Chinese;
    }
    return QLocale::English;
}
