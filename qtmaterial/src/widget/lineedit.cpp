#include "widget/lineedit.h"
#include "core/font.h"

#include <QKeyEvent>

namespace widget {
LineEdit::LineEdit(QWidget *parent) :
    QLineEdit(parent) {
}

LineEdit::~LineEdit() {
}

void LineEdit::keyPressEvent(QKeyEvent *event) {
    QLineEdit::keyPressEvent(event);
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        emit sigEditingConfirm(text());
    }
}

void LineEdit::focusOutEvent(QFocusEvent *event) {
    QLineEdit::focusOutEvent(event);
    emit sigFocusOut(text());
    emit sigEditingConfirm(text());
}

SearchLineEdit::SearchLineEdit(QWidget *parent) :
    QLineEdit(parent) {
    setObjectName("SearchLineEdit");
    setAttribute(Qt::WA_StyledBackground, true);
    setContextMenuPolicy(Qt::NoContextMenu);
    setFocusPolicy(Qt::StrongFocus);
#ifdef Q_OS_MAC
    setAttribute(Qt::WA_MacShowFocusRect, false);
#endif
    setFixedHeight(36);

	m_pLayout = new QHBoxLayout(this);
	m_pLayout->setContentsMargins(m_nLayoutLeftRightMargin, 0, m_nLayoutLeftRightMargin, 0);

	m_pSearchVectorBtn = new widget::VectorButton(this);
    m_pSearchVectorBtn->setObjectName("SearchLineEdit_SearchVectorBtn");
    m_pSearchVectorBtn->setFixedSize(m_nBtnSize, m_nBtnSize);
    m_pSearchVectorBtn->setCursor(Qt::ArrowCursor);
	m_pSearchVectorBtn->setFocusPolicy(Qt::NoFocus);
    m_pSearchVectorBtn->setFont(core::Font::currentIconFont());
    m_pSearchVectorBtn->setText(QChar(0xe601));

	m_pClearVectorBtn = new widget::VectorButton(this);
    m_pClearVectorBtn->setObjectName("SearchLineEdit_ClearVectorBtn");
    m_pClearVectorBtn->setFixedSize(m_nBtnSize, m_nBtnSize);
    m_pClearVectorBtn->setCursor(Qt::ArrowCursor);
	m_pClearVectorBtn->setFocusPolicy(Qt::NoFocus);
    m_pClearVectorBtn->setFont(core::Font::currentIconFont());
    m_pClearVectorBtn->setText(QChar(0xe60c));

    m_pLayout->setSpacing(0);
    m_pLayout->addWidget(m_pSearchVectorBtn, 0, Qt::AlignVCenter);
    m_pLayout->addStretch();
    m_pLayout->addWidget(m_pClearVectorBtn, 0, Qt::AlignVCenter);

    setClearBtnVisible(false);
    
    connect(this, &QLineEdit::textChanged, this, &SearchLineEdit::slotTextChanged);
    connect(m_pClearVectorBtn, &QPushButton::clicked, this, &SearchLineEdit::slotTextClear);
}

void SearchLineEdit::setObjectName(const QString &name) {
    QLineEdit::setObjectName(name);
    if(m_pSearchVectorBtn) {
        m_pSearchVectorBtn->setObjectName(name + "_VectorBtn");
        m_pSearchVectorBtn->setStyle(m_pSearchVectorBtn->style());
    }
    if (m_pClearVectorBtn) {
        m_pClearVectorBtn->setObjectName(name + "_ClearVectorBtn");
        m_pClearVectorBtn->setStyle(m_pClearVectorBtn->style());
    }
}

void SearchLineEdit::setSearchBtnVisible(bool b) {
    m_pSearchVectorBtn->setVisible(b);
    updateTextMargin();
}

bool SearchLineEdit::searchBtnVisible() const {
    return m_pSearchVectorBtn->isVisible();
}

void SearchLineEdit::setClearBtnVisible(bool b) {
    m_pClearVectorBtn->setVisible(b);
    updateTextMargin();
}

bool SearchLineEdit::clearBtnVisible() const {
    return m_pClearVectorBtn->isVisible();
}

void SearchLineEdit::setButtonSize(int n) {
    m_nBtnSize = n;
    m_pSearchVectorBtn->setFixedSize(m_nBtnSize, m_nBtnSize);
    m_pClearVectorBtn->setFixedSize(m_nBtnSize, m_nBtnSize);
}

int SearchLineEdit::buttonSize() const {
    return m_nBtnSize;
}

void SearchLineEdit::setLayoutLeftRightMargin(int n) {
    m_nLayoutLeftRightMargin = n;
    m_pLayout->setContentsMargins(m_nLayoutLeftRightMargin, 0, m_nLayoutLeftRightMargin, 0);
}

int SearchLineEdit::layoutLeftRightMargin() const {
    return m_nLayoutLeftRightMargin;
}

void SearchLineEdit::setTextLeftRightMargin(int n) {
    m_nTextLeftRightMargin = n;
    updateTextMargin();
}

int SearchLineEdit::textLeftRightMargin() const {
    return m_nTextLeftRightMargin;
}

void SearchLineEdit::setSearchVectorBtnFont(const QFont &font) {
    m_pSearchVectorBtn->setFont(font);
}

void SearchLineEdit::setSearchVectorBtnText(const QString &text) {
    m_pSearchVectorBtn->setText(text);
}

void SearchLineEdit::setClearVectorBtnFont(const QFont &font) {
    m_pClearVectorBtn->setFont(font);
}

void SearchLineEdit::setClearVectorBtnText(const QString &text) {
    m_pClearVectorBtn->setText(text);
}

void SearchLineEdit::showEvent(QShowEvent *event) {
    QLineEdit::showEvent(event);
    updateTextMargin();
}

void SearchLineEdit::resizeEvent(QResizeEvent *event) {
    QLineEdit::resizeEvent(event);
}

void SearchLineEdit::updateTextMargin() {
    int leftMargin = 0;
    if(m_pSearchVectorBtn->isVisible()) {
        leftMargin += (m_nBtnSize + m_nTextLeftRightMargin);
    }
    int rightMargin = m_nTextLeftRightMargin;
    if(m_pClearVectorBtn->isVisible()) {
        rightMargin += (m_nBtnSize + m_nTextLeftRightMargin);
    }
    setTextMargins(leftMargin, 0, rightMargin, 0);
}

void SearchLineEdit::slotTextChanged(const QString &text) {
    setClearBtnVisible(!text.isEmpty());
}

void SearchLineEdit::slotTextClear() {
    setText("");
    setFocus();
}
} // namespace widget