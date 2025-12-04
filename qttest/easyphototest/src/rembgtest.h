#pragma once

#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>
#include <QProgressBar>
#include <QComboBox>
#include <QCheckBox>
#include <QGroupBox>
#include <QPixmap>
#include <QDateTime>

class QVBoxLayout;
class QHBoxLayout;
class QPushButton;
class QLabel;
class QProgressBar;
class QComboBox;
class QCheckBox;
class RembgProcessor;

class RembgTestWindow : public QWidget
{
    Q_OBJECT

public:
    explicit RembgTestWindow(QWidget *parent = nullptr);

protected:
    void showEvent(QShowEvent *event) override;
    
private:
    void setupUI();
    void setupRembg();
    void checkInstallation();

private slots:
    void onLoadImage();
    void onProcessImage();
    void onSaveResult();

private:
    RembgProcessor *m_processor;
    
    QComboBox *m_modelCombo;
    QCheckBox *m_alphaCheckBox;
    QPushButton *m_loadBtn;
    QPushButton *m_processBtn;
    QPushButton *m_saveBtn;
    QProgressBar *m_progressBar;
    QLabel *m_originalLabel;
    QLabel *m_resultLabel;
    QLabel *m_statusLabel;
    
    QString m_inputPath;
    QString m_outputPath;
    QString m_resultPath;
};