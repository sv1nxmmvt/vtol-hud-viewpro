#include "joystick_handler.h"
#include "control_stream.h"

#include <QDebug>
#include <cmath>

namespace gimbal {

JoystickHandler::JoystickHandler(QObject* parent)
    : QObject(parent)
{
    // Инициализируем SDL (только подсистема джойстика)
    if (SDL_InitSubSystem(SDL_INIT_JOYSTICK) < 0) {
        qWarning() << "[JoystickHandler] Failed to init SDL joystick:" << SDL_GetError();
    }

    // Создаём таймер для опроса джойстика
    m_pollTimer = new QTimer(this);
    connect(m_pollTimer, &QTimer::timeout, this, &JoystickHandler::pollJoystick);
}

JoystickHandler::~JoystickHandler() {
    shutdown();
    SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
}

bool JoystickHandler::init(int joystickIndex) {
    if (m_joystick) {
        qWarning() << "[JoystickHandler] Joystick already initialized";
        return false;
    }

    int joystickCount = SDL_NumJoysticks();
    if (joystickCount == 0) {
        qWarning() << "[JoystickHandler] No joysticks connected";
        return false;
    }

    if (joystickIndex < 0 || joystickIndex >= joystickCount) {
        qWarning() << "[JoystickHandler] Invalid joystick index:" << joystickIndex
                   << ", available:" << joystickCount;
        return false;
    }

    m_joystick = SDL_JoystickOpen(joystickIndex);
    if (!m_joystick) {
        qWarning() << "[JoystickHandler] Failed to open joystick:" << SDL_GetError();
        return false;
    }

    qDebug() << "[JoystickHandler] Opened joystick:" << SDL_JoystickName(m_joystick);
    qDebug() << "[JoystickHandler] Axes:" << SDL_JoystickNumAxes(m_joystick)
             << ", Buttons:" << SDL_JoystickNumButtons(m_joystick);

    // Запускаем опрос джойстика (50 Гц для отзывчивости)
    m_pollTimer->start(20);  // 20 мс = 50 Гц

    return true;
}

void JoystickHandler::shutdown() {
    if (m_pollTimer) {
        m_pollTimer->stop();
    }

    if (m_joystick) {
        SDL_JoystickClose(m_joystick);
        m_joystick = nullptr;
        qDebug() << "[JoystickHandler] Joystick closed";
    }

    // Сбрасываем управляющие сигналы
    ControlStream::pitchSpeed = 0;
    ControlStream::yawSpeed = 0;
    ControlStream::zoomSpeed = 0;
}

bool JoystickHandler::isInitialized() const {
    return m_joystick != nullptr;
}

int JoystickHandler::getJoystickCount() {
    return SDL_NumJoysticks();
}

QString JoystickHandler::getJoystickName(int index) {
    if (index < 0 || index >= SDL_NumJoysticks()) {
        return QString();
    }
    return QString(SDL_JoystickNameForIndex(index));
}

void JoystickHandler::setDeadzone(float deadzone) {
    m_deadzone = qBound(0.0f, deadzone, 1.0f);
    qDebug() << "[JoystickHandler] Deadzone set to:" << m_deadzone;
}

float JoystickHandler::getDeadzone() const {
    return m_deadzone;
}

float JoystickHandler::normalizeAxis(Sint16 value) {
    // Нормализуем значение оси к диапазону -1.0..1.0
    float normalized = static_cast<float>(value) / 32767.0f;

    // Применяем мертвую зону
    if (qAbs(normalized) < m_deadzone) {
        return 0.0f;
    }

    // Пересчитываем значение вне мертвой зоны к полному диапазону
    // чтобы получить более плавное управление
    float scaled = (normalized - std::copysign(m_deadzone, normalized)) / (1.0f - m_deadzone);
    return qBound(-1.0f, scaled, 1.0f);
}

void JoystickHandler::pollJoystick() {
    if (!m_joystick) {
        return;
    }

    // Обновляем состояние джойстика
    SDL_JoystickUpdate();

    // Проверяем количество осей
    int axisCount = SDL_JoystickNumAxes(m_joystick);
    if (axisCount <= qMax(PITCH_AXIS, YAW_AXIS)) {
        qWarning() << "[JoystickHandler] Not enough axes:" << axisCount;
        return;
    }

    // Читаем значения осей
    Sint16 pitchAxisValue = SDL_JoystickGetAxis(m_joystick, PITCH_AXIS);
    Sint16 yawAxisValue = SDL_JoystickGetAxis(m_joystick, YAW_AXIS);

    // Нормализуем с учетом мертвой зоны
    float pitchNormalized = normalizeAxis(pitchAxisValue);
    float yawNormalized = normalizeAxis(yawAxisValue);

    // Преобразуем в скорости управления
    // Ось 4 (pitch): вверх = отрицательное значение, вниз = положительное
    // В gimbal: pitch > 0 = вверх, pitch < 0 = вниз
    int pitchSpeed = static_cast<int>(pitchNormalized * MOVE_SPEED);

    // Ось 5 (yaw): влево = отрицательное, вправо = положительное
    // В gimbal: yaw > 0 = вправо, yaw < 0 = влево
    int yawSpeed = static_cast<int>(yawNormalized * MOVE_SPEED);

    // Отправляем команды на движение
    ControlStream::pitchSpeed = pitchSpeed;
    ControlStream::yawSpeed = yawSpeed;

    // Логируем только при изменении
    static int logCounter = 0;
    if (++logCounter % 50 == 0) {
        qDebug() << "[JoystickHandler] Axis:" << "pitch=" << pitchSpeed
                 << ", yaw=" << yawSpeed;
    }

    // Обработка кнопок зума
    int buttonCount = SDL_JoystickNumButtons(m_joystick);
    if (buttonCount <= qMax(ZOOM_IN_BUTTON, ZOOM_OUT_BUTTON)) {
        return;
    }

    bool zoomInNow = SDL_JoystickGetButton(m_joystick, ZOOM_IN_BUTTON);
    bool zoomOutNow = SDL_JoystickGetButton(m_joystick, ZOOM_OUT_BUTTON);

    // Zoom in (кнопка 1)
    if (zoomInNow && !m_zoomInPressed) {
        qDebug() << "[JoystickHandler] Zoom IN button pressed";
        ControlStream::zoomSpeed = ZOOM_SPEED;
    } else if (!zoomInNow && m_zoomInPressed) {
        qDebug() << "[JoystickHandler] Zoom IN button released";
        if (!zoomOutNow) {
            ControlStream::zoomSpeed = 0;
        }
    }
    m_zoomInPressed = zoomInNow;

    // Zoom out (кнопка 3)
    if (zoomOutNow && !m_zoomOutPressed) {
        qDebug() << "[JoystickHandler] Zoom OUT button pressed";
        ControlStream::zoomSpeed = -ZOOM_SPEED;
    } else if (!zoomOutNow && m_zoomOutPressed) {
        qDebug() << "[JoystickHandler] Zoom OUT button released";
        if (!zoomInNow) {
            ControlStream::zoomSpeed = 0;
        }
    }
    m_zoomOutPressed = zoomOutNow;

    // Приоритет zoom in над zoom out если обе нажаты
    if (zoomInNow && zoomOutNow) {
        ControlStream::zoomSpeed = ZOOM_SPEED;
    }
}

} // namespace gimbal
