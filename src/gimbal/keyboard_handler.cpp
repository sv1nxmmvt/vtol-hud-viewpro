#include "keyboard_handler.h"
#include "control_stream.h"
#include "command_handler.h"

#include <QDebug>

namespace gimbal {

KeyboardHandler::KeyboardHandler(QObject* parent)
    : QObject(parent)
{
}

KeyboardHandler::~KeyboardHandler() = default;

bool KeyboardHandler::isControlKey(int key) {
    switch (key) {
        case Qt::Key_W:
        case Qt::Key_S:
        case Qt::Key_A:
        case Qt::Key_D:
        case Qt::Key_Up:
        case Qt::Key_Down:
        case Qt::Key_Left:
        case Qt::Key_Right:
        case Qt::Key_R:
        case Qt::Key_F:
        // Команды
        case Qt::Key_V:
        case Qt::Key_T:
        case Qt::Key_P:
        case Qt::Key_O:
        case Qt::Key_I:
        case Qt::Key_K:
        case Qt::Key_Space:
        case Qt::Key_L:
        case Qt::Key_G:
            return true;
        default:
            return false;
    }
}

bool KeyboardHandler::handleKeyPress(QKeyEvent* event, ControlStream* controlStream) {
    if (!controlStream) {
        return false;
    }

    int yaw = 0;
    int pitch = 0;

    switch (event->key()) {
        // Управление движением (Pan/Tilt)
        case Qt::Key_W:
        case Qt::Key_Up:
            pitch = MOVE_SPEED;
            break;

        case Qt::Key_S:
        case Qt::Key_Down:
            pitch = -MOVE_SPEED;
            break;

        case Qt::Key_A:
        case Qt::Key_Left:
            yaw = -MOVE_SPEED;
            break;

        case Qt::Key_D:
        case Qt::Key_Right:
            yaw = MOVE_SPEED;
            break;

        // Управление зумом
        case Qt::Key_R:
            qDebug() << "[KeyboardHandler] zoom in";
            ControlStream::zoomSpeed = ZOOM_SPEED;
            return true;

        case Qt::Key_F:
            qDebug() << "[KeyboardHandler] zoom out";
            ControlStream::zoomSpeed = -ZOOM_SPEED;
            return true;

        // Команды через CommandHandler
        case Qt::Key_V:
            qDebug() << "[KeyboardHandler] set video source: VISIBLE";
            CommandHandler::setVideoSourceVisible();
            return true;

        case Qt::Key_T:
            qDebug() << "[KeyboardHandler] set video source: THERMAL";
            CommandHandler::setVideoSourceThermal();
            return true;

        case Qt::Key_P:
            qDebug() << "[KeyboardHandler] enable PIP";
            CommandHandler::enablePip();
            return true;

        case Qt::Key_O:
            qDebug() << "[KeyboardHandler] disable PIP";
            CommandHandler::disablePip();
            return true;

        case Qt::Key_I:
            qDebug() << "[KeyboardHandler] IR digital zoom IN";
            CommandHandler::irDigitalZoomIn();
            return true;

        case Qt::Key_K:
            qDebug() << "[KeyboardHandler] IR digital zoom OUT";
            CommandHandler::irDigitalZoomOut();
            return true;

        case Qt::Key_Space:
            qDebug() << "[KeyboardHandler] laser single";
            CommandHandler::laserSingle();
            return true;

        case Qt::Key_L:
            m_laserRangeFinderEnabled = !m_laserRangeFinderEnabled;
            qDebug() << "[KeyboardHandler] laser range finder:" << (m_laserRangeFinderEnabled ? "ON" : "OFF");
            if (m_laserRangeFinderEnabled) {
                CommandHandler::enableLaserRangeFinder();
            } else {
                CommandHandler::disableLaserRangeFinder();
            }
            return true;

        case Qt::Key_G:
            m_followModeEnabled = !m_followModeEnabled;
            qDebug() << "[KeyboardHandler] follow mode:" << (m_followModeEnabled ? "ON" : "OFF");
            CommandHandler::enableFollowMode(m_followModeEnabled);
            return true;

        default:
            return false;
    }

    qDebug() << "[KeyboardHandler] key pressed:" << event->key()
             << "-> yaw:" << yaw << ", pitch:" << pitch;

    ControlStream::yawSpeed = yaw;
    ControlStream::pitchSpeed = pitch;

    return true;
}

bool KeyboardHandler::handleKeyRelease(QKeyEvent* event, ControlStream* controlStream) {
    if (!controlStream) {
        return false;
    }

    switch (event->key()) {
        // Сброс оси pitch
        case Qt::Key_W:
        case Qt::Key_S:
        case Qt::Key_Up:
        case Qt::Key_Down:
            qDebug() << "[KeyboardHandler] key released:" << event->key()
                     << "-> resetting pitch";
            ControlStream::pitchSpeed = 0;
            return true;

        // Сброс оси yaw
        case Qt::Key_A:
        case Qt::Key_D:
        case Qt::Key_Left:
        case Qt::Key_Right:
            qDebug() << "[KeyboardHandler] key released:" << event->key()
                     << "-> resetting yaw";
            ControlStream::yawSpeed = 0;
            return true;

        // Сброс зума
        case Qt::Key_R:
        case Qt::Key_F:
            qDebug() << "[KeyboardHandler] key released:" << event->key()
                     << "-> resetting zoom";
            ControlStream::zoomSpeed = 0;
            return true;

        default:
            return false;
    }
}

} // namespace gimbal
