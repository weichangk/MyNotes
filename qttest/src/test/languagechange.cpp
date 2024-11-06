#include "languagechange.h"
#include <QHBoxLayout>
#include "core/lang.h"

LanguageChangeTest::LanguageChangeTest(QWidget *parent) :
    QWidget(parent) {
    createUi();
    sigConnect();
    slotLanguageChanged();
}

LanguageChangeTest::~LanguageChangeTest() {
}

void LanguageChangeTest::createUi() {
    lang_ = new filter::Lang(this);
    
    setWindowTitle("Language Change Test");
    setFixedSize(800, 600);

    auto layout = new QHBoxLayout(this);

    lang_combox_ = new QComboBox(this);
    lang_combox_->addItem(tr("Chinens"));
    lang_combox_->addItem(tr("English"));
    layout->addWidget(lang_combox_);

    layout->addSpacing(64);

    label_ = new QLabel(this);
    layout->addWidget(label_);

    layout->addStretch();
}

void LanguageChangeTest::sigConnect() {
    connect(lang_, &filter::Lang::sigLanguageChange,
            this, &LanguageChangeTest::slotLanguageChanged);

    connect(lang_combox_, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &LanguageChangeTest::slotLangComboxIndexChanged);
}

void LanguageChangeTest::slotLanguageChanged() {
    label_->setText(tr("Language Change Test"));
}

void LanguageChangeTest::slotLangComboxIndexChanged(int index) {
    switch (index) {
    case 0:
        core::Lang::getInstance()->setLangLocale(core::LangLocale::lang_locale_zh);
        break;
    case 1:
        core::Lang::getInstance()->setLangLocale(core::LangLocale::lang_locale_en);
        break;
    default:
        core::Lang::getInstance()->setLangLocale(core::LangLocale::lang_locale_zh);
        break;
    }
}