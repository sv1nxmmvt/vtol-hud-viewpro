#pragma once

#include <QObject>
#include <QKeyEvent>

namespace gimbal {

class ControlStream;

/**
 * @brief Обработчик событий клавиатуры для управления подвесом
 * @details Перехватывает события нажатия/отпускания клавиш и управляет
 *          сигналами ControlStream для движения подвеса и зума.
 *
 * Управление:
 * - Движение (Pan/Tilt): W/A/S/D или стрелки
 *   - W/↑: pitch вверх (+2000)
 *   - S/↓: pitch вниз (-2000)
 *   - A/←: yaw влево (-2000)
 *   - D/→: yaw вправо (+2000)
 *
 * - Зум: R/F
 *   - R: zoom in (скорость 4)
 *   - F: zoom out (скорость -4)
 *
 * - Команды (через CommandHandler):
 *   - V: обычная камера
 *   - T: тепловизор
 *   - P: включить PIP
 *   - O: выключить PIP
 *   - I: IR zoom in
 *   - K: IR zoom out
 *   - Space: laser single
 *   - L: включить/выключить дальномер
 *   - G: включить/выключить режим следования
 */
class KeyboardHandler : public QObject {
    Q_OBJECT

public:
    explicit KeyboardHandler(QObject* parent = nullptr);
    ~KeyboardHandler() override;

    /**
     * @brief Обработать событие нажатия клавиши
     * @param event Событие клавиатуры
     * @param controlStream Поток управления для отправки команд
     * @return true если событие обработано, false иначе
     */
    bool handleKeyPress(QKeyEvent* event, ControlStream* controlStream);

    /**
     * @brief Обработать событие отпускания клавиши
     * @param event Событие клавиатуры
     * @param controlStream Поток управления для отправки команд
     * @return true если событие обработано, false иначе
     */
    bool handleKeyRelease(QKeyEvent* event, ControlStream* controlStream);

    /**
     * @brief Проверить, является ли клавиша управляющей
     * @param key Код клавиши
     * @return true если клавиша управляет подвесом
     */
    static bool isControlKey(int key);

private:
    // Скорости управления
    static constexpr int MOVE_SPEED = 2000;     // Скорость движения осей
    static constexpr int ZOOM_SPEED = 4;        // Скорость зума

    // Состояние переключателей
    bool m_laserRangeFinderEnabled = false;
    bool m_followModeEnabled = false;
};

} // namespace gimbal
