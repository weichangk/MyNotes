#include "qtmaterial_global.h"
#include "widget/label.h"
#include "widget/style.h"

#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QPixmap>

namespace widget {
class QTMATERIAL_EXPORT IconButton : public QPushButton {
    Q_OBJECT
    Q_PROPERTY(int iconSize READ iconSize WRITE setIconSize)
    Q_PROPERTY(QString fiveIcon READ fivePixmapPath WRITE setFivePixmapPath)
    Q_PROPERTY(QString normalIcon READ normalPixmapPath WRITE setNormalPixmapPath)
    Q_PROPERTY(QString hoverIcon READ hoverPixmapPath WRITE setHoverPixmapPath)
    Q_PROPERTY(QString pressedIcon READ pressedPixmapPath WRITE setPressedPixmapPath)
    Q_PROPERTY(QString checkedIcon READ checkedPixmapPath WRITE setCheckedPixmapPath)
    Q_PROPERTY(QString disabledIcon READ disablePixmapPath WRITE setDisablePixmapPath)

public:
    IconButton(QWidget *parent = nullptr);
    ~IconButton() override {
    }

    void setObjectName(const QString &);

    void setIconSize(int);
    int iconSize() const;

    void setFivePixmapPath(const QString &);
    QString fivePixmapPath() const;

    void setNormalPixmapPath(const QString &);
    QString normalPixmapPath() const;

    void setHoverPixmapPath(const QString &);
    QString hoverPixmapPath() const;

    void setPressedPixmapPath(const QString &);
    QString pressedPixmapPath() const;

    void setCheckedPixmapPath(const QString &);
    QString checkedPixmapPath() const;

    void setDisablePixmapPath(const QString &);
    QString disablePixmapPath() const;

protected:
    void resizeEvent(QResizeEvent *) override;
    void mousePressEvent(QMouseEvent *) override;
    void mouseReleaseEvent(QMouseEvent *) override;
    void enterEvent(QEvent *) override;
    void leaveEvent(QEvent *) override;
    void changeEvent(QEvent *) override;

private:
    void updateWidgetStatus(style::WidgetStatus);
    QPixmap getCurrentPixmap() const;

private:
    QLabel *m_pIcon = nullptr;

    int m_nIconSize = 16;

    QString m_strFivePath = "";
    QString m_strNormalPath = "";
    QString m_strHoverPath = "";
    QString m_strPressedPath = "";
    QString m_strCheckedPath = "";
    QString m_strDisabledPath = "";

    QPixmap m_pixmapFive;
    QPixmap m_pixmapNormal;
    QPixmap m_pixmapHover;
    QPixmap m_pixmapPressed;
    QPixmap m_pixmapChecked;
    QPixmap m_pixmapDisabled;

    style::WidgetStatus m_eState = style::WidgetStatus::Normal;
};

class QTMATERIAL_EXPORT HorIconTextButton : public QPushButton {
    Q_OBJECT
    Q_PROPERTY(int iconSize READ iconSize WRITE setIconSize)
    Q_PROPERTY(int leftRightSpacing READ leftRightSpacing WRITE setLeftRightSpacing)
    Q_PROPERTY(int iconTextSpacing READ iconTextSpacing WRITE setIconTextSpacing)
    Q_PROPERTY(QString fiveIcon READ fivePixmapPath WRITE setFivePixmapPath)
    Q_PROPERTY(QString normalIcon READ normalPixmapPath WRITE setNormalPixmapPath)
    Q_PROPERTY(QString hoverIcon READ hoverPixmapPath WRITE setHoverPixmapPath)
    Q_PROPERTY(QString pressedIcon READ pressedPixmapPath WRITE setPressedPixmapPath)
    Q_PROPERTY(QString checkedIcon READ checkedPixmapPath WRITE setCheckedPixmapPath)
    Q_PROPERTY(QString disabledIcon READ disablePixmapPath WRITE setDisablePixmapPath)

public:
    HorIconTextButton(QWidget *parent = nullptr);
    ~HorIconTextButton() override {
    }

    void setObjectName(const QString &);

    void setEnabled(bool);
    bool isEnabled() const;

    void setText(const QString &);
    QString text() const;

    void setIconSize(int);
    int iconSize() const;

    void setLeftRightSpacing(int);
    int leftRightSpacing() const;

    void setIconTextSpacing(int);
    int iconTextSpacing() const;

    void setAdjustWidth(bool);
    bool adjustWidth() const;

    void setFivePixmapPath(const QString &);
    QString fivePixmapPath() const;

    void setNormalPixmapPath(const QString &);
    QString normalPixmapPath() const;

    void setHoverPixmapPath(const QString &);
    QString hoverPixmapPath() const;

    void setPressedPixmapPath(const QString &);
    QString pressedPixmapPath() const;

    void setCheckedPixmapPath(const QString &);
    QString checkedPixmapPath() const;

    void setDisablePixmapPath(const QString &);
    QString disablePixmapPath() const;

protected:
    void resizeEvent(QResizeEvent *) override;
    void mousePressEvent(QMouseEvent *) override;
    void mouseReleaseEvent(QMouseEvent *) override;
    void enterEvent(QEvent *) override;
    void leaveEvent(QEvent *) override;
    void changeEvent(QEvent *) override;
    QSize sizeHint() const override;

private:
    void updateWidgetStatus(style::WidgetStatus);
    QPixmap getCurrentPixmap() const;

private:
    QHBoxLayout *m_pLayout = nullptr;
    QLabel *m_pIcon = nullptr;
    QLabel *m_pText = nullptr;

    QString m_strFivePath = "";
    QString m_strNormalPath = "";
    QString m_strHoverPath = "";
    QString m_strPressedPath = "";
    QString m_strCheckedPath = "";
    QString m_strDisabledPath = "";

    QPixmap m_pixmapFive;
    QPixmap m_pixmapNormal;
    QPixmap m_pixmapHover;
    QPixmap m_pixmapPressed;
    QPixmap m_pixmapChecked;
    QPixmap m_pixmapDisabled;

    style::WidgetStatus m_eState = style::WidgetStatus::Normal;

    int m_nIconSize = 16;
    int m_nLeftRightSpacing = 8;
    int m_nIconTextSpacing = 8;

    bool m_bAdjustWidth = true;
};

class QTMATERIAL_EXPORT VectorButton : public QPushButton {
    Q_OBJECT
public:
    VectorButton(QWidget *parent = nullptr);
    ~VectorButton() override {
    }
};

class QTMATERIAL_EXPORT HorIconTextVectorButton : public QPushButton {
    Q_OBJECT
public:
    HorIconTextVectorButton(QWidget *parent = nullptr);
    ~HorIconTextVectorButton() override {
    }

    void setObjectName(const QString &);

    void setIconFont(const QFont &);
    void setIcon(const QString &);
    void setIconSize(int);

    void setText(const QString &);
    QString text() const;

    void setAdjustWidth(bool);
    bool adjustWidth() const;

protected:
    void mousePressEvent(QMouseEvent *) override;
    void mouseReleaseEvent(QMouseEvent *) override;
    void enterEvent(QEvent *) override;
    void leaveEvent(QEvent *) override;
    void changeEvent(QEvent *) override;
    void resizeEvent(QResizeEvent *) override;
    QSize sizeHint() const override;
    
private: 
    void updateWidgetStatus(style::WidgetStatus state);

private Q_SLOTS:
    void slotToggled(bool);

private:
    QHBoxLayout *m_pLayout = nullptr;
    widget::VectorLabel *m_pIcon = nullptr;
    QLabel *m_pText = nullptr;

    style::WidgetStatus m_eState = style::WidgetStatus::Normal;

    int m_nIconSize = 20;
    int m_nLeftRightSpacing = 8;
    int m_nIconTextSpacing = 8;

    bool m_bAdjustWidth = true;
};
} // namespace widget