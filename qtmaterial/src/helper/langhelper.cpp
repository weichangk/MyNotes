#include "helper/langhelper.h"
#include <QApplication>
#include <QDir>

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

static LangHelper *m_instance = nullptr;

LangHelper *LangHelper::getInstance() {
    if (m_instance == nullptr) {
        m_instance = new LangHelper;
    }
    return m_instance;
}

LangHelper::LangHelper(QObject *parent) :
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

LangHelper::~LangHelper() {
    for (auto lang : m_lstLangs) {
        delete lang;
    }
}

void LangHelper::release() {
    if (m_instance) {
        delete m_instance;
        m_instance = nullptr;
    }
}

void LangHelper::setSysLangLocale() {
    QLocale::setDefault((QLocale::Language)getQtLocale(m_curLang));
}

void LangHelper::setLangLocale(LangLocale lang) {
    removeCurrentLang();
    m_curLang = lang;
    loadCurrentLang();
    QLocale::setDefault((QLocale::Language)getQtLocale(m_curLang));
}

void LangHelper::init() {
    for (auto langInfo : g_LangInfos) {
        OneLang *lang = new OneLang(langInfo.ll);
        m_lstLangs.push_back(lang);
    }
}

void LangHelper::loadCurrentLang() {
    for (auto lang : m_lstLangs) {
        if (m_curLang == lang->m_lang) {
            for (auto translator : lang->m_lstTranslators) {
                qApp->installTranslator(translator);
            }
        }
    }
}

void LangHelper::removeCurrentLang() {
    for (auto lang : m_lstLangs) {
        if (m_curLang == lang->m_lang) {
            for (auto translator : lang->m_lstTranslators)
                qApp->removeTranslator(translator);
        }
    }
}

int LangHelper::getQtLocale(LangLocale lang) {
    switch (lang) {
    case lang_locale_en:
        return QLocale::English;
    case lang_locale_zh:
        return QLocale::Chinese;
    }
    return QLocale::English;
}
