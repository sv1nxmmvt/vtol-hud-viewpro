#include "command_handler.h"

#include <QDebug>

namespace gimbal {

std::weak_ptr<Gimbal> CommandHandler::s_gimbal;
CommandHandler* CommandHandler::s_instance = nullptr;
ImageType CommandHandler::s_currentImageType = ImageType::Visible1;
bool CommandHandler::s_pipEnabled = false;

CommandHandler::CommandHandler(QObject* parent)
    : QObject(parent)
{
    s_instance = this;
}

CommandHandler::~CommandHandler() {
    s_instance = nullptr;
}

void CommandHandler::init(std::shared_ptr<Gimbal> gimbal) {
    s_gimbal = gimbal;
    if (!s_instance) {
        new CommandHandler();
    }
    qDebug() << "[CommandHandler] Initialized";
}

bool CommandHandler::isInitialized() {
    return !s_gimbal.expired();
}

CommandHandler& CommandHandler::instance() {
    Q_ASSERT_X(s_instance, "CommandHandler::instance()",
               "CommandHandler not initialized. Call init() first.");
    return *s_instance;
}

ImageType CommandHandler::getCurrentImageType() {
    return s_currentImageType;
}

bool CommandHandler::isPipEnabled() {
    return s_pipEnabled;
}

void CommandHandler::setCurrentImageType(ImageType type) {
    s_currentImageType = type;
}

void CommandHandler::setPipEnabled(bool enabled) {
    s_pipEnabled = enabled;
}

// ============================================================================
// Источник видео
// ============================================================================

void CommandHandler::setVideoSourceVisible() {
    auto gimbal = s_gimbal.lock();
    if (!gimbal) {
        qWarning() << "[CommandHandler] Gimbal not initialized";
        return;
    }
    qDebug() << "[CommandHandler] Set video source: VISIBLE (type=Visible1, pip=" << s_pipEnabled << ")";
    gimbal->setImageColor(ImageType::Visible1, s_pipEnabled, IrColor::WhiteHot);
    setCurrentImageType(ImageType::Visible1);
}

void CommandHandler::setVideoSourceThermal() {
    auto gimbal = s_gimbal.lock();
    if (!gimbal) {
        qWarning() << "[CommandHandler] Gimbal not initialized";
        return;
    }
    qDebug() << "[CommandHandler] Set video source: THERMAL (type=Ir1, pip=" << s_pipEnabled << ")";
    gimbal->setImageColor(ImageType::Ir1, s_pipEnabled, IrColor::WhiteHot);
    setCurrentImageType(ImageType::Ir1);
}

// ============================================================================
// Режим "картинка в картинке" (PIP)
// ============================================================================

void CommandHandler::enablePip() {
    auto gimbal = s_gimbal.lock();
    if (!gimbal) {
        qWarning() << "[CommandHandler] Gimbal not initialized";
        return;
    }
    qDebug() << "[CommandHandler] Enable PIP (current type=" << static_cast<int>(s_currentImageType) << ")";
    // Включаем PIP для текущего источника
    gimbal->setImageColor(s_currentImageType, true, IrColor::WhiteHot);
    setPipEnabled(true);
}

void CommandHandler::disablePip() {
    auto gimbal = s_gimbal.lock();
    if (!gimbal) {
        qWarning() << "[CommandHandler] Gimbal not initialized";
        return;
    }
    qDebug() << "[CommandHandler] Disable PIP (current type=" << static_cast<int>(s_currentImageType) << ")";
    // Выключаем PIP для текущего источника
    gimbal->setImageColor(s_currentImageType, false, IrColor::WhiteHot);
    setPipEnabled(false);
}

// ============================================================================
// Зум
// ============================================================================

void CommandHandler::zoomTo(float magnification) {
    auto gimbal = s_gimbal.lock();
    if (!gimbal) {
        qWarning() << "[CommandHandler] Gimbal not initialized";
        return;
    }
    qDebug() << "[CommandHandler] Zoom to:" << magnification;
    gimbal->zoomTo(magnification);
}

void CommandHandler::irDigitalZoomIn() {
    auto gimbal = s_gimbal.lock();
    if (!gimbal) {
        qWarning() << "[CommandHandler] Gimbal not initialized";
        return;
    }
    qDebug() << "[CommandHandler] IR digital zoom IN";
    gimbal->irDigitalZoomIn();
}

void CommandHandler::irDigitalZoomOut() {
    auto gimbal = s_gimbal.lock();
    if (!gimbal) {
        qWarning() << "[CommandHandler] Gimbal not initialized";
        return;
    }
    qDebug() << "[CommandHandler] IR digital zoom OUT";
    gimbal->irDigitalZoomOut();
}

// ============================================================================
// Трекинг целей
// ============================================================================

void CommandHandler::startTrack() {
    auto gimbal = s_gimbal.lock();
    if (!gimbal) {
        qWarning() << "[CommandHandler] Gimbal not initialized";
        return;
    }
    qDebug() << "[CommandHandler] Start tracking";
    gimbal->startTrack();
}

void CommandHandler::stopTrack() {
    auto gimbal = s_gimbal.lock();
    if (!gimbal) {
        qWarning() << "[CommandHandler] Gimbal not initialized";
        return;
    }
    qDebug() << "[CommandHandler] Stop tracking";
    gimbal->stopTrack();
}

void CommandHandler::trackTarget(int x, int y, int videoWidth, int videoHeight) {
    auto gimbal = s_gimbal.lock();
    if (!gimbal) {
        qWarning() << "[CommandHandler] Gimbal not initialized";
        return;
    }
    qDebug() << "[CommandHandler] Track target:" << x << y;
    gimbal->trackTarget(x, y, videoWidth, videoHeight);
}

// ============================================================================
// Лазерный дальномер
// ============================================================================

void CommandHandler::laserSingle() {
    auto gimbal = s_gimbal.lock();
    if (!gimbal) {
        qWarning() << "[CommandHandler] Gimbal not initialized";
        return;
    }
    qDebug() << "[CommandHandler] Laser single measurement";
    gimbal->laserSingle();
}

void CommandHandler::enableLaserRangeFinder() {
    auto gimbal = s_gimbal.lock();
    if (!gimbal) {
        qWarning() << "[CommandHandler] Gimbal not initialized";
        return;
    }
    qDebug() << "[CommandHandler] Enable laser range finder";
    gimbal->switchLaser(true);
}

void CommandHandler::disableLaserRangeFinder() {
    auto gimbal = s_gimbal.lock();
    if (!gimbal) {
        qWarning() << "[CommandHandler] Gimbal not initialized";
        return;
    }
    qDebug() << "[CommandHandler] Disable laser range finder";
    gimbal->switchLaser(false);
}

// ============================================================================
// Режим следования за дроном
// ============================================================================

void CommandHandler::enableFollowMode(bool enable) {
    auto gimbal = s_gimbal.lock();
    if (!gimbal) {
        qWarning() << "[CommandHandler] Gimbal not initialized";
        return;
    }
    qDebug() << "[CommandHandler] Follow mode:" << (enable ? "ON" : "OFF");
    gimbal->enableFollowMode(enable);
    
    // Проверяем статус после включения
    if (enable) {
        bool status = gimbal->isFollowMode();
        qDebug() << "[CommandHandler] Follow mode status:" << (status ? "ENABLED" : "DISABLED");
        if (!status) {
            qWarning() << "[CommandHandler] Follow mode may require GPS data from drone";
        }
    }
}

} // namespace gimbal
