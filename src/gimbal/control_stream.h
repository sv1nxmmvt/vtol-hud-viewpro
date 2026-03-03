#pragma once

#include "gimbal.h"
#include <QThread>
#include <QAtomicInteger>
#include <memory>

namespace gimbal {

/**
 * @brief Класс для периодической отправки команд на подвес
 * @details Статический класс-шлюз для управления подвесом из разных источников.
 *          Внешние модули (мышь, клавиатура, джойстик) изменяют статические поля
 *          pitch/yaw/zoom, а поток ControlStream автоматически отправляет
 *          соответствующие команды с частотой 10 Гц.
 *
 * @note Все поля потокобезопасны (атомарные операции)
 * @note Значения интерпретируются так:
 *       - pitchSpeed/yawSpeed: -2000..+2000 (0 = остановка)
 *       - zoomSpeed: -8..+8 (0 = остановка, отрицательный = zoom out, положительный = zoom in)
 */
class ControlStream : public QThread {
    Q_OBJECT

public:
    /**
     * @brief Скорость движения pitch (наклон)
     * @details Диапазон: -2000..+2000 (0.01°/сек)
     *          0 = остановка, >0 = вверх, <0 = вниз
     */
    static QAtomicInteger<qint64> pitchSpeed;

    /**
     * @brief Скорость движения yaw (поворот)
     * @details Диапазон: -2000..+2000 (0.01°/сек)
     *          0 = остановка, >0 = вправо, <0 = влево
     */
    static QAtomicInteger<qint64> yawSpeed;

    /**
     * @brief Скорость зума
     * @details Диапазон: -8..+8
     *          0 = остановка, >0 = zoom in, <0 = zoom out
     */
    static QAtomicInteger<int> zoomSpeed;

    /**
     * @brief Получить экземпляр ControlStream
     * @param gimbal Ссылка на Gimbal для отправки команд
     * @return Единственный экземпляр ControlStream
     */
    static ControlStream& instance(Gimbal& gimbal);

    /**
     * @brief Получить экземпляр ControlStream (без инициализации)
     * @return Единственный экземпляр ControlStream
     * @warning Вызывать только после instance(gimbal)
     */
    static ControlStream& instance();

    /**
     * @brief Установить частоту отправки команд
     * @param frequency_hz Частота в Гц (по умолчанию 10)
     * @details Допустимый диапазон: 1-100 Гц
     */
    void setFrequency(int frequency_hz);

    /**
     * @brief Получить частоту отправки команд
     */
    int getFrequency() const;

    /**
     * @brief Запустить поток отправки команд
     */
    void start();

    /**
     * @brief Остановить поток отправки команд
     */
    void stop();

    /**
     * @brief Проверить, запущен ли поток
     */
    bool isRunning() const;

    /**
     * @brief Сбросить все управляющие сигналы в 0
     * @details Останавливает движение и зум
     */
    static void reset();

    ~ControlStream() override;

    // Запрет копирования
    ControlStream(const ControlStream&) = delete;
    ControlStream& operator=(const ControlStream&) = delete;

protected:
    void run() override;

private:
    explicit ControlStream(Gimbal& gimbal);

    Gimbal& m_gimbal;
    QAtomicInteger<int> m_running{0};
    int m_frequency_hz{10};  // 10 Гц по умолчанию

    // Статический экземпляр (указатель, т.к. нужен Gimbal для инициализации)
    static ControlStream* s_instance;
};

} // namespace gimbal
