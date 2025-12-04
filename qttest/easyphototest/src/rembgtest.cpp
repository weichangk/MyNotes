#include "rembgtest.h"
#include "rembgprocessor.h"

RembgTestWindow::RembgTestWindow(QWidget *parent) : QWidget(parent) {
    setWindowTitle("背景移除工具 - Powered by rembg");
    setMinimumSize(900, 700);
    setupUI();
}

void RembgTestWindow::showEvent(QShowEvent *event) {
    QWidget::showEvent(event);
    // 窗口首次显示时检查安装
    checkInstallation();
    setupRembg();
}

void RembgTestWindow::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // 标题
    QLabel *title = new QLabel("🎨 AI智能背景移除");
    title->setStyleSheet("font-size: 20px; font-weight: bold; color: #2c3e50; padding: 10px;");
    title->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(title);

    // 设置区域
    QGroupBox *settingsGroup = new QGroupBox("设置");
    QHBoxLayout *settingsLayout = new QHBoxLayout(settingsGroup);

    settingsLayout->addWidget(new QLabel("模型:"));
    m_modelCombo = new QComboBox();
    m_modelCombo->addItem("U2Net (通用)", RembgProcessor::U2Net);
    m_modelCombo->addItem("U2Net-P (轻量)", RembgProcessor::U2NetP);
    m_modelCombo->addItem("U2Net Human (人像)", RembgProcessor::U2NetHumanSeg);
    m_modelCombo->addItem("U2Net Cloth (衣服)", RembgProcessor::U2NetCloth);
    m_modelCombo->addItem("Silueta (移动端)", RembgProcessor::SiluetaMobile);
    m_modelCombo->addItem("BiRefNet (高精度)", RembgProcessor::BiRefNetGeneral);
    m_modelCombo->addItem("BiRefNet Lite (轻量)", RembgProcessor::BiRefNetGeneralLite);
    m_modelCombo->addItem("BiRefNet Portrait (肖像)", RembgProcessor::BiRefNetPortrait);
    m_modelCombo->addItem("BiRefNet DIS (高精度分割)", RembgProcessor::BiRefNetDIS);
    m_modelCombo->addItem("BiRefNet HRSOD (高分辨率)", RembgProcessor::BiRefNetHRSOD);
    m_modelCombo->addItem("BiRefNet COD (伪装目标)", RembgProcessor::BiRefNetCOD);
    m_modelCombo->addItem("BiRefNet DIS-5K (5K)", RembgProcessor::BiRefNetDIS5K);
    m_modelCombo->addItem("ISNet (通用)", RembgProcessor::IsNetGeneral);
    m_modelCombo->addItem("ISNet Anime (动漫)", RembgProcessor::IsNetAnime);
    m_modelCombo->addItem("SAM (通用分割)", RembgProcessor::Sam);
    settingsLayout->addWidget(m_modelCombo);

    m_alphaCheckBox = new QCheckBox("精细抠图 (Alpha Matting)");
    m_alphaCheckBox->setToolTip("启用后处理质量更好，但速度较慢");
    settingsLayout->addWidget(m_alphaCheckBox);

    settingsLayout->addStretch();
    mainLayout->addWidget(settingsGroup);

    // 按钮区域
    QHBoxLayout *buttonLayout = new QHBoxLayout();

    m_loadBtn = new QPushButton("📁 加载图片");
    m_loadBtn->setMinimumHeight(40);
    m_loadBtn->setStyleSheet("QPushButton { background-color: #3498db; color: white; font-size: 14px; border-radius: 5px; } QPushButton:hover { background-color: #2980b9; }");
    connect(m_loadBtn, &QPushButton::clicked, this, &RembgTestWindow::onLoadImage);
    buttonLayout->addWidget(m_loadBtn);

    m_processBtn = new QPushButton("🚀 移除背景");
    m_processBtn->setMinimumHeight(40);
    m_processBtn->setEnabled(false);
    m_processBtn->setStyleSheet("QPushButton { background-color: #2ecc71; color: white; font-size: 14px; border-radius: 5px; } QPushButton:hover { background-color: #27ae60; } QPushButton:disabled { background-color: #95a5a6; }");
    connect(m_processBtn, &QPushButton::clicked, this, &RembgTestWindow::onProcessImage);
    buttonLayout->addWidget(m_processBtn);

    m_saveBtn = new QPushButton("💾 保存结果");
    m_saveBtn->setMinimumHeight(40);
    m_saveBtn->setEnabled(false);
    m_saveBtn->setStyleSheet("QPushButton { background-color: #e74c3c; color: white; font-size: 14px; border-radius: 5px; } QPushButton:hover { background-color: #c0392b; } QPushButton:disabled { background-color: #95a5a6; }");
    connect(m_saveBtn, &QPushButton::clicked, this, &RembgTestWindow::onSaveResult);
    buttonLayout->addWidget(m_saveBtn);

    mainLayout->addLayout(buttonLayout);

    // 进度条
    m_progressBar = new QProgressBar();
    m_progressBar->setVisible(false);
    mainLayout->addWidget(m_progressBar);

    // 图片显示区域
    QHBoxLayout *imageLayout = new QHBoxLayout();

    // 原图
    QVBoxLayout *originalLayout = new QVBoxLayout();
    originalLayout->addWidget(new QLabel("原图"));
    m_originalLabel = new QLabel();
    m_originalLabel->setMinimumSize(400, 400);
    m_originalLabel->setAlignment(Qt::AlignCenter);
    m_originalLabel->setStyleSheet("border: 2px dashed #bdc3c7; border-radius: 10px; background-color: #ecf0f1;");
    m_originalLabel->setText("📷 请加载图片");
    originalLayout->addWidget(m_originalLabel);
    imageLayout->addLayout(originalLayout);

    // 结果图
    QVBoxLayout *resultLayout = new QVBoxLayout();
    resultLayout->addWidget(new QLabel("处理结果"));
    m_resultLabel = new QLabel();
    m_resultLabel->setMinimumSize(400, 400);
    m_resultLabel->setAlignment(Qt::AlignCenter);
    m_resultLabel->setStyleSheet("border: 2px dashed #bdc3c7; border-radius: 10px; background-color: #ecf0f1;");
    m_resultLabel->setText("✨ 结果将显示在这里");
    resultLayout->addWidget(m_resultLabel);
    imageLayout->addLayout(resultLayout);

    mainLayout->addLayout(imageLayout);

    // 状态栏
    m_statusLabel = new QLabel("就绪");
    m_statusLabel->setStyleSheet("color: #7f8c8d; padding: 5px;");
    mainLayout->addWidget(m_statusLabel);
}

void RembgTestWindow::setupRembg() {
    m_processor = new RembgProcessor(this);

    connect(m_processor, &RembgProcessor::started, this, [this]() {
        m_progressBar->setVisible(true);
        m_progressBar->setValue(0);
        m_processBtn->setEnabled(false);
        m_statusLabel->setText("⏳ 正在处理...");
    });

    connect(m_processor, &RembgProcessor::progressChanged, this, [this](int percent) {
        m_progressBar->setValue(percent);
    });

    connect(m_processor, &RembgProcessor::finished, this, [this](const QString &outputPath) {
        m_progressBar->setVisible(false);
        m_processBtn->setEnabled(true);
        m_statusLabel->setText("✅ 处理完成！");

        // 显示结果图片
        m_resultPath = outputPath;
        QPixmap pixmap(outputPath);
        if (!pixmap.isNull()) {
            m_resultLabel->setPixmap(pixmap.scaled(m_resultLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
            m_saveBtn->setEnabled(true);
        }

        QMessageBox::information(this, "成功", "背景移除完成！");
    });

    connect(m_processor, &RembgProcessor::error, this, [this](const QString &error) {
        m_progressBar->setVisible(false);
        m_processBtn->setEnabled(true);
        m_statusLabel->setText("❌ " + error);
        QMessageBox::critical(this, "错误", error);
    });
}

void RembgTestWindow::checkInstallation() {
    QString errorMsg;
    if (!RembgProcessor::checkInstallation(errorMsg)) {
        QMessageBox::warning(this, "警告",
                             "Python环境检测失败：" + errorMsg + "\n\n" + "请确保应用程序目录下有python文件夹，或系统已安装Python和rembg。");
        m_statusLabel->setText("⚠️ Python环境未就绪");
    } else {
        m_statusLabel->setText("✅ 环境检测通过，就绪");
    }
}

void RembgTestWindow::onLoadImage() {
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    "选择图片", "", "图片文件 (*.png *.jpg *.jpeg *.bmp)");

    if (!fileName.isEmpty()) {
        m_inputPath = fileName;
        QPixmap pixmap(fileName);

        if (!pixmap.isNull()) {
            m_originalLabel->setPixmap(pixmap.scaled(m_originalLabel->size(),
                                                     Qt::KeepAspectRatio, Qt::SmoothTransformation));
            m_processBtn->setEnabled(true);
            m_statusLabel->setText("✅ 已加载: " + QFileInfo(fileName).fileName());
        } else {
            QMessageBox::warning(this, "错误", "无法加载图片");
        }
    }
}

void RembgTestWindow::onProcessImage() {
    if (m_inputPath.isEmpty()) {
        return;
    }

    // 设置临时输出路径
    QString tempDir = QApplication::applicationDirPath() + "/temp";
    QDir().mkpath(tempDir);
    m_outputPath = tempDir + "/output_" + QString::number(QDateTime::currentMSecsSinceEpoch()) + ".png";

    // 配置处理器
    auto model = static_cast<RembgProcessor::ModelType>(m_modelCombo->currentData().toInt());
    m_processor->setModel(model);
    m_processor->setAlphaMatting(m_alphaCheckBox->isChecked());

    // 开始处理
    m_processor->processImage(m_inputPath, m_outputPath);
}

void RembgTestWindow::onSaveResult() {
    if (m_resultPath.isEmpty()) {
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(this,
                                                    "保存图片", "", "PNG图片 (*.png)");

    if (!fileName.isEmpty()) {
        if (QFile::copy(m_resultPath, fileName)) {
            QMessageBox::information(this, "成功", "图片已保存！");
        } else {
            QMessageBox::critical(this, "错误", "保存失败");
        }
    }
}