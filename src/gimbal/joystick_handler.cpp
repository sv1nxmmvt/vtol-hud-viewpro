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

    // Разрешаем события джойстика без фокуса
    SDL_JoystickEventState(SDL_ENABLE);

    // Создаём таймер для опроса джойстиков (10 Гц для проверки подключения)
    m_pollTimer = new QTimer(this);
    connect(m_pollTimer, &QTimer::timeout, this, &JoystickHandler::pollJoysticks);
}

JoystickHandler::~JoystickHandler() {
    stop();
    closeJoystick();
    SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
}

void JoystickHandler::start() {
    if (m_running) {
        return;
    }

    m_running = true;
    m_pollTimer->start(100);  // 100 мс = 10 Гц для проверки подключения
    qDebug() << "[JoystickHandler] Started polling";

    // Пробуем сразу подключить джойстик
    pollJoysticks();
}

void JoystickHandler::stop() {
    m_running = false;

    if (m_pollTimer) {
        m_pollTimer->stop();
    }

    closeJoystick();
    qDebug() << "[JoystickHandler] Stopped polling";
}

bool JoystickHandler::isRunning() const {
    return m_running;
}

bool JoystickHandler::isJoystickConnected() const {
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

bool JoystickHandler::openJoystick(int index) {
    if (m_joystick) {
        qWarning() << "[JoystickHandler] Joystick already open, closing first";
        closeJoystick();
    }

    int joystickCount = SDL_NumJoysticks();
    if (joystickCount == 0) {
        qWarning() << "[JoystickHandler] No joysticks available";
        return false;
    }

    if (index < 0 || index >= joystickCount) {
        index = 0;  // Берем первый доступный
    }

    m_joystick = SDL_JoystickOpen(index);
    if (!m_joystick) {
        qWarning() << "[JoystickHandler] Failed to open joystick:" << SDL_GetError();
        return false;
    }

    // Проверяем что джойстик действительно подключен
    if (!SDL_JoystickGetAttached(m_joystick)) {
        qWarning() << "[JoystickHandler] Joystick opened but not attached";
        SDL_JoystickClose(m_joystick);
        m_joystick = nullptr;
        return false;
    }

    qDebug() << "[JoystickHandler] Opened joystick:" << SDL_JoystickName(m_joystick);
    qDebug() << "[JoystickHandler] Axes:" << SDL_JoystickNumAxes(m_joystick)
             << ", Buttons:" << SDL_JoystickNumButtons(m_joystick)
             << ", Attached:" << SDL_JoystickGetAttached(m_joystick);

    return true;
}

void JoystickHandler::closeJoystick() {
    if (m_joystick) {
        qDebug() << "[JoystickHandler] Closing joystick:" << SDL_JoystickName(m_joystick);
        SDL_JoystickClose(m_joystick);
        m_joystick = nullptr;
        m_joystickActive = false;
        m_zoomInPressed = false;
        m_zoomOutPressed = false;
        qDebug() << "[JoystickHandler] Joystick closed";
    }
}

float JoystickHandler::normalizeAxis(Sint16 value) {
    // Нормализуем значение оси к диапазону -1.0..1.0
    float normalized = static_cast<float>(value) / 32767.0f;

    // Применяем мертвую зону
    if (qAbs(normalized) < m_deadzone) {
        return 0.0f;
    }

    // Пересчитываем значение вне мертвой зоны к полному диапазону
    float scaled = (normalized - std::copysign(m_deadzone, normalized)) / (1.0f - m_deadzone);
    return qBound(-1.0f, scaled, 1.0f);
}

void JoystickHandler::pollJoysticks() {
    // Обновляем состояние джойстиков SDL
    SDL_JoystickUpdate();

    // Проверяем подключение джойстика
    int joystickCount = SDL_NumJoysticks();

    // Отладочное логирование каждые 10 вызовов
    static int pollCounter = 0;
    if (++pollCounter % 10 == 0) {
        qDebug() << "[JoystickHandler] Poll - joysticks:" << joystickCount
                 << ", m_joystick:" << (m_joystick ? "yes" : "no")
                 << ", attached:" << (m_joystick ? SDL_JoystickGetAttached(m_joystick) : false);
    }

    // Если джойстик был подключен, проверяем его состояние
    if (m_joystick) {
        // Проверяем физическое подключение
        if (!SDL_JoystickGetAttached(m_joystick)) {
            qWarning() << "[JoystickHandler] Joystick physically disconnected";
            closeJoystick();
        }
        // Также проверяем, не изменилось ли количество джойстиков
        // (может означать что устройство переподключилось с другим ID)
        else if (joystickCount == 0) {
            qWarning() << "[JoystickHandler] No joysticks available, closing";
            closeJoystick();
        }
    }

    // Если джойстик не подключен, пробуем подключить
    if (!m_joystick && joystickCount > 0) {
        qDebug() << "[JoystickHandler] Joystick detected, count:" << joystickCount;
        // Небольшая задержка перед подключением для стабильности
        if (openJoystick(0)) {
            qDebug() << "[JoystickHandler] Joystick connected:" << SDL_JoystickName(m_joystick);
            // Сбрасываем состояние после подключения
            m_joystickActive = false;
            m_zoomInPressed = false;
            m_zoomOutPressed = false;
        }
    }

    // Если джойстик подключен, обрабатываем его состояние
    if (m_joystick) {
        handleJoystickState();
    }
}

void JoystickHandler::handleJoystickState() {
    if (!m_joystick) {
        qWarning() << "[JoystickHandler] handleJoystickState: m_joystick is null";
        return;
    }

    // Проверяем количество осей
    int axisCount = SDL_JoystickNumAxes(m_joystick);
    if (axisCount <= qMax(PITCH_AXIS, YAW_AXIS)) {
        qWarning() << "[JoystickHandler] Not enough axes:" << axisCount
                   << ", required:" << qMax(PITCH_AXIS, YAW_AXIS) + 1;
        return;
    }

    // Читаем значения осей
    Sint16 pitchAxisValue = SDL_JoystickGetAxis(m_joystick, PITCH_AXIS);
    Sint16 yawAxisValue = SDL_JoystickGetAxis(m_joystick, YAW_AXIS);

    // Отладочное логирование каждые 100 вызовов
    static int debugCounter = 0;
    if (++debugCounter % 100 == 0) {
        qDebug() << "[JoystickHandler] Debug - Axis values:" << pitchAxisValue << yawAxisValue
                 << ", normalized:" << normalizeAxis(pitchAxisValue) << normalizeAxis(yawAxisValue);
    }

    // Нормализуем с учетом мертвой зоны
    float pitchNormalized = normalizeAxis(pitchAxisValue);
    float yawNormalized = normalizeAxis(yawAxisValue);

    // Проверяем, активен ли джойстик (оси вне мертвой зоны или кнопки нажаты)
    bool pitchActive = (pitchNormalized != 0.0f);
    bool yawActive = (yawNormalized != 0.0f);

    // Проверяем кнопки зума
    int buttonCount = SDL_JoystickNumButtons(m_joystick);
    bool zoomInNow = (buttonCount > ZOOM_IN_BUTTON) && SDL_JoystickGetButton(m_joystick, ZOOM_IN_BUTTON);
    bool zoomOutNow = (buttonCount > ZOOM_OUT_BUTTON) && SDL_JoystickGetButton(m_joystick, ZOOM_OUT_BUTTON);
    bool buttonsActive = zoomInNow || zoomOutNow;

    // Джойстик активен если оси или кнопки активны
    bool wasActive = m_joystickActive;
    m_joystickActive = pitchActive || yawActive || buttonsActive;

    // Логируем изменение состояния активности
    if (m_joystickActive && !wasActive) {
        qDebug() << "[JoystickHandler] Joystick became ACTIVE";
    } else if (!m_joystickActive && wasActive) {
        qDebug() << "[JoystickHandler] Joystick became inactive, resetting controls";
        ControlStream::pitchSpeed = 0;
        ControlStream::yawSpeed = 0;
        ControlStream::zoomSpeed = 0;
        m_zoomInPressed = false;
        m_zoomOutPressed = false;
        return;
    }

    // Если джойстик не активен, не отправляем команды (чтобы не блокировать клавиатуру)
    if (!m_joystickActive) {
        return;
    }

    // Преобразуем в скорости управления
    // Ось 4 (pitch): вверх = отрицательное значение, вниз = положительное
    // В gimbal: pitch > 0 = вверх, pitch < 0 = вниз
    int pitchSpeed = static_cast<int>(-pitchNormalized * MOVE_SPEED);

    // Ось 5 (yaw): влево = отрицательное, вправо = положительное
    // В gimbal: yaw > 0 = вправо, yaw < 0 = влево
    int yawSpeed = static_cast<int>(yawNormalized * MOVE_SPEED);

    // Отправляем команды на движение (только если соответствующие оси активны)
    if (pitchActive || yawActive) {
        ControlStream::pitchSpeed = pitchSpeed;
        ControlStream::yawSpeed = yawSpeed;

        // Логируем только при изменении
        static int logCounter = 0;
        if (++logCounter % 50 == 0) {
            qDebug() << "[JoystickHandler] Axis:" << "pitch=" << pitchSpeed
                     << ", yaw=" << yawSpeed;
        }
    }

    // Обработка кнопок зума
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
