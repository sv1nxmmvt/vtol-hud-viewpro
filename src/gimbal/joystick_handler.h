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
 */
class JoystickHandler : public QObject {
    Q_OBJECT

public:
    explicit JoystickHandler(QObject* parent = nullptr);
    ~JoystickHandler() override;

    /**
     * @brief Инициализировать джойстик
     * @param joystickIndex Индекс джойстика (по умолчанию 0)
     * @return true если успешно
     */
    bool init(int joystickIndex = 0);

    /**
     * @brief Остановить и закрыть джойстик
     */
    void shutdown();

    /**
     * @brief Проверить, инициализирован ли джойстик
     */
    bool isInitialized() const;

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

private slots:
    /**
     * @brief Периодический опрос джойстика
     */
    void pollJoystick();

private:
    /**
     * @brief Нормализовать значение оси с учетом мертвой зоны
     * @param value Сырое значение оси (-32768..32767)
     * @return Нормализованное значение (-1.0..1.0 или 0 если в мертвой зоне)
     */
    float normalizeAxis(Sint16 value);

    SDL_Joystick* m_joystick = nullptr;
    QTimer* m_pollTimer = nullptr;
    float m_deadzone = 0.1f;  // 10% мертвая зона

    // Предыдущее состояние кнопок зума
    bool m_zoomInPressed = false;
    bool m_zoomOutPressed = false;

    // Индексы осей и кнопок
    static constexpr int PITCH_AXIS = 4;
    static constexpr int YAW_AXIS = 5;
    static constexpr int ZOOM_IN_BUTTON = 1;
    static constexpr int ZOOM_OUT_BUTTON = 3;

    // Скорости управления
    static constexpr int MOVE_SPEED = 2000;
    static constexpr int ZOOM_SPEED = 4;
};

} // namespace gimbal
