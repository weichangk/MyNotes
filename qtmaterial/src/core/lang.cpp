#include "core/lang.h"
#include <QApplication>
#include <QDir>

namespace core {
static const size_t g_nInfos = 2;
static const LangConstInfo g_LangInfos[g_nInfos] = {
    {lang_locale_en, "en", "ENG", "en-us", "English"},
    {lang_locale_zh, "zh", "CHS", "zh-cn", "Chinese"}};

OneLang::OneLang(LangLocale lang) {
    init(lang);
}

void OneLang::init(LangLocale lang) {
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

OneLang::~OneLang() {
    for (auto translator : m_lstTranslators) {
        delete translator;
    }
}

static Lang *m_instance = nullptr;

Lang *Lang::getInstance() {
    if (m_instance == nullptr) {
        m_instance = new Lang;
    }
    return m_instance;
}

Lang::Lang(QObject *parent) :
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

Lang::~Lang() {
    for (auto lang : m_lstLangs) {
        delete lang;
    }
}

void Lang::release() {
    if (m_instance) {
        delete m_instance;
        m_instance = nullptr;
    }
}

void Lang::setSysLangLocale() {
    QLocale::setDefault((QLocale::Language)getQtLocale(m_curLang));
}

void Lang::setLangLocale(LangLocale lang) {
    removeCurrentLang();
    m_curLang = lang;
    loadCurrentLang();
    QLocale::setDefault((QLocale::Language)getQtLocale(m_curLang));
}

void Lang::init() {
    for (auto langInfo : g_LangInfos) {
        OneLang *lang = new OneLang(langInfo.ll);
        m_lstLangs.push_back(lang);
    }
}

void Lang::loadCurrentLang() {
    for (auto lang : m_lstLangs) {
        if (m_curLang == lang->m_lang) {
            for (auto translator : lang->m_lstTranslators) {
                qApp->installTranslator(translator);
            }
        }
    }
}

void Lang::removeCurrentLang() {
    for (auto lang : m_lstLangs) {
        if (m_curLang == lang->m_lang) {
            for (auto translator : lang->m_lstTranslators)
                qApp->removeTranslator(translator);
        }
    }
}

int Lang::getQtLocale(LangLocale lang) {
    switch (lang) {
    case lang_locale_en:
        return QLocale::English;
    case lang_locale_zh:
        return QLocale::Chinese;
    }
    return QLocale::English;
}
} // namespace core