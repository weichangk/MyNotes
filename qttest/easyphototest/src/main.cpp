#include <QApplication>
#include <QMessageBox>

#include "setupwindow.h"
#include "rembgtest.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    // 设置应用程序信息
    app.setApplicationName("EasyPhoto");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("EasyPhoto");
    
    // 创建主窗口（但不显示）
    RembgTestWindow *mainWindow = new RembgTestWindow();
    
    // 创建并显示设置窗口
    SetupWindow *setupWindow = new SetupWindow();
    setupWindow->setWindowModality(Qt::ApplicationModal);
    setupWindow->show();
    
    // 连接设置完成信号
    QObject::connect(setupWindow, &SetupWindow::setupCompleted, 
                     [mainWindow, setupWindow](bool success) {
        if (success) {
            // 环境配置成功，显示主窗口
            mainWindow->show();
        } else {
            // 配置失败或跳过，询问用户
            QMessageBox::StandardButton reply = QMessageBox::warning(
                nullptr,
                "环境未就绪",
                "Python 环境未正确配置，部分功能可能无法使用。\n\n"
                "是否继续启动应用程序？",
                QMessageBox::Yes | QMessageBox::No
            );
            
            if (reply == QMessageBox::Yes) {
                mainWindow->show();
            } else {
                QApplication::quit();
                return;
            }
        }
        
        // 删除设置窗口
        setupWindow->deleteLater();
    });
    
    // 确保主窗口在应用退出时被删除
    QObject::connect(&app, &QApplication::aboutToQuit, [mainWindow]() {
        mainWindow->deleteLater();
    });
    
    return app.exec();
}
