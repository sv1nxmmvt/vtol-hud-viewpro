#pragma once

#include <QObject>
#include <QTimer>
#include <QAtomicInteger>
#include <QString>

#include <SDL2/SDL.h>

namespace gimbal {

class ControlStream;

/**
 * @brief Обработчик джойстика для управления подвесом
 * @details Использует SDL2 для чтения состояния джойстика и управления
 *          сигналами ControlStream для движения подвеса и зума.
 *
 * Управление:
 * - Ось 4 (Y): Pitch (вверх/вниз)
 * - Ось 5 (X): Yaw (влево/вправо)
 * - Кнопка 1: Zoom in
 * - Кнопка 3: Zoom out
 *
 * Мертвая зона: 10% от полного диапазона оси
 *
 * @note Джойстик отправляет команды только когда оси/кнопки активны.
 *       В покое не обнуляет значения, чтобы не блокировать клавиатуру.
 * @note Поддерживается горячее подключение/отключение джойстика.
 */
class JoystickHandler : public QObject {
    Q_OBJECT

public:
    explicit JoystickHandler(QObject* parent = nullptr);
    ~JoystickHandler() override;

    /**
     * @brief Запустить опрос джойстиков
     * @details Периодически проверяет подключение джойстиков
     */
    void start();

    /**
     * @brief Остановить опрос джойстиков
     */
    void stop();

    /**
     * @brief Проверить, запущен ли опрос
     */
    bool isRunning() const;

    /**
     * @brief Получить количество доступных джойстиков
     */
    static int getJoystickCount();

    /**
     * @brief Получить имя джойстика по индексу
     */
    static QString getJoystickName(int index);

    /**
     * @brief Установить мертвую зону для осей
     * @param deadzone Значение от 0.0 до 1.0 (по умолчанию 0.1 = 10%)
     */
    void setDeadzone(float deadzone);

    /**
     * @brief Получить текущую мертвую зону
     */
    float getDeadzone() const;

    /**
     * @brief Проверить, подключен ли джойстик
     */
    bool isJoystickConnected() const;

    /**
     * @brief Применить текущее положение оси видео (при подключении)
     */
    void applyCurrentVideoState();

private slots:
    /**
     * @brief Периодический опрос джойстиков
     */
    void pollJoysticks();

private:
    /**
     * @brief Инициализировать джойстик по индексу
     */
    bool openJoystick(int index);

    /**
     * @brief Закрыть текущий джойстик
     */
    void closeJoystick();

    /**
     * @brief Обработать состояние джойстика
     */
    void handleJoystickState();

    /**
     * @brief Обработать ось 6 (видео источник и PIP)
     */
    void handleVideoAxis();

    /**
     * @brief Нормализовать значение оси с учетом мертвой зоны
     * @param value Сырое значение оси (-32768..32767)
     * @return Нормализованное значение (-1.0..1.0 или 0 если в мертвой зоне)
     */
    float normalizeAxis(Sint16 value);

    SDL_Joystick* m_joystick = nullptr;
    QTimer* m_pollTimer = nullptr;
    float m_deadzone = 0.1f;  // 10% мертвая зона
    bool m_running = false;

    // Предыдущее состояние кнопок зума
    bool m_zoomInPressed = false;
    bool m_zoomOutPressed = false;

    // Предыдущее состояние кнопок позиции
    bool m_nadirButtonPressed = false;    // Кнопка 23 (SDL) / 24 (виртуальный)
    bool m_frontButtonPressed = false;    // Кнопка 33 (SDL) / 34 (виртуальный)

    // Флаг активности джойстика (оси вне мертвой зоны или кнопки нажаты)
    bool m_joystickActive = false;

    // Предыдущее состояние оси видео (для отслеживания переключений)
    enum class VideoState {
        VisiblePipOff,      // < -0.75
        ThermalPipOff,      // -0.75 .. -0.25
        Neutral,            // -0.25 .. +0.25
        VisiblePipOn,       // +0.25 .. +0.75
        ThermalPipOn        // > +0.75
    };
    VideoState m_lastVideoState = VideoState::Neutral;

    // Индексы осей и кнопок
    static constexpr int PITCH_AXIS = 4;
    static constexpr int YAW_AXIS = 5;
    static constexpr int VIDEO_AXIS = 7;  // Ось для управления источником видео и PIP
    static constexpr int ZOOM_IN_BUTTON = 1;
    static constexpr int ZOOM_OUT_BUTTON = 3;
    static constexpr int NADIR_BUTTON = 23;   // Кнопка 24: позиция надир (home)
    static constexpr int FRONT_BUTTON = 33;   // Кнопка 34: позиция фронт (0, 0)

    // Скорости управления
    static constexpr int MOVE_SPEED = 2000;
    static constexpr int ZOOM_SPEED = 4;

    // Мертвая зона для дискретных переключений (ось 6)
    // Центры позиций: -0.75 (низ), 0 (центр), +0.75 (верх)
    // Зона нечувствительности вокруг каждой позиции: ±0.25
    static constexpr float VIDEO_DEADZONE = 0.25f;
};

} // namespace gimbal
