#include <QApplication>
#include <QDebug>

#include "ui/main_window.h"
#include "gimbal/application_manager.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    qDebug() << "=== Viewpro Gimbal Control ===";
    qDebug() << "Starting application...";

    // Создаем и инициализируем ApplicationManager
    // Это глобальный менеджер, который управляет всеми сервисами
    gimbal::ApplicationManager appManager;
    
    if (!appManager.initialize()) {
        qCritical() << "Failed to initialize ApplicationManager";
        return -1;
    }

    // Создаем главное окно и передаем ему менеджер
    MainWindow window(&appManager);
    window.show();

    qDebug() << "Application started successfully";

    // Запускаем цикл событий
    int result = app.exec();

    // Корректная очистка при выходе
    qDebug() << "Shutting down application...";
    appManager.shutdown();
    qDebug() << "Application shutdown complete";

    return result;
}
