#pragma once

#include <QObject>
#include <memory>

#include "gimbal.h"

namespace gimbal {

/**
 * @brief Менеджер команд для управления подвесом
 * @details Предоставляет статические функции для отправки команд на подвес
 *          из различных источников (клавиатура, джойстик, UI и т.д.)
 *
 * @note使用前需要调用 init() для передачи указателя на Gimbal
 */
class CommandHandler : public QObject {
    Q_OBJECT

public:
    explicit CommandHandler(QObject* parent = nullptr);
    ~CommandHandler() override;

    /**
     * @brief Инициализировать CommandHandler
     * @param gimbal Указатель на Gimbal для отправки команд
     */
    static void init(std::shared_ptr<Gimbal> gimbal);

    /**
     * @brief Проверить, инициализирован ли CommandHandler
     */
    static bool isInitialized();

    /**
     * @brief Получить экземпляр CommandHandler
     */
    static CommandHandler& instance();

    /**
     * @brief Получить текущий тип изображения
     */
    static ImageType getCurrentImageType();

    /**
     * @brief Получить текущее состояние PIP
     */
    static bool isPipEnabled();

    // =========================================================================
    // Источник видео
    // =========================================================================

    /**
     * @brief Переключить на обычную камеру (видимый свет)
     */
    static void setVideoSourceVisible();

    /**
     * @brief Переключить на тепловизор (ИК)
     */
    static void setVideoSourceThermal();

    // =========================================================================
    // Режим "картинка в картинке" (PIP)
    // =========================================================================

    /**
     * @brief Включить режим PIP
     */
    static void enablePip();

    /**
     * @brief Выключить режим PIP
     */
    static void disablePip();

    // =========================================================================
    // Зум
    // =========================================================================

    /**
     * @brief Установить кратность зума
     * @param magnification Кратность
     */
    static void zoomTo(float magnification);

    /**
     * @brief Цифровой зум ИК (приближение)
     */
    static void irDigitalZoomIn();

    /**
     * @brief Цифровой зум ИК (отдаление)
     */
    static void irDigitalZoomOut();

    // =========================================================================
    // Трекинг целей
    // =========================================================================

    /**
     * @brief Запустить трекинг
     */
    static void startTrack();

    /**
     * @brief Остановить трекинг
     */
    static void stopTrack();

    /**
     * @brief Захватить цель в центре экрана
     * @param x Координата X центра области
     * @param y Координата Y центра области
     * @param videoWidth Ширина видео
     * @param videoHeight Высота видео
     */
    static void trackTarget(int x, int y, int videoWidth, int videoHeight);

    /**
     * @brief Захватить цель (короткое нажатие)
     * @param videoWidth Ширина видео
     * @param videoHeight Высота видео
     */
    static void targetAcquire(int videoWidth, int videoHeight);

    /**
     * @brief Отменить захват цели (долгое нажатие)
     */
    static void targetCancel();

    // =========================================================================
    // Лазерный дальномер
    // =========================================================================

    /**
     * @brief Однократное измерение лазером
     */
    static void laserSingle();

    /**
     * @brief Включить продолжительный режим дальномера
     */
    static void enableLaserRangeFinder();

    /**
     * @brief Выключить продолжительный режим дальномера
     */
    static void disableLaserRangeFinder();

    // =========================================================================
    // Внутренние функции (для обновления состояния)
    // =========================================================================

    /**
     * @brief Установить текущий тип изображения (внутренняя функция)
     * @param type Тип изображения
     */
    static void setCurrentImageType(ImageType type);

    /**
     * @brief Установить состояние PIP (внутренняя функция)
     * @param enabled Состояние PIP
     */
    static void setPipEnabled(bool enabled);

    // =========================================================================
    // Режим следования за дроном
    // =========================================================================

    /**
     * @brief Включить/выключить режим следования за дроном
     * @param enable true = включить, false = выключить
     */
    static void enableFollowMode(bool enable);

    // =========================================================================
    // OSD (On-Screen Display)
    // =========================================================================

    /**
     * @brief Включить OSD
     */
    static void enableOSD();

    /**
     * @brief Выключить OSD
     */
    static void disableOSD();

    /**
     * @brief Переключить отображение перекрестия
     * @param enable true = включить, false = выключить
     */
    static void setOSDCross(bool enable);

    /**
     * @brief Переключить отображение pitch/yaw
     * @param enable true = включить, false = выключить
     */
    static void setOSDPitchYaw(bool enable);

    /**
     * @brief Переключить отображение GPS
     * @param enable true = включить, false = выключить
     */
    static void setOSDGPS(bool enable);

    /**
     * @brief Переключить отображение времени
     * @param enable true = включить, false = выключить
     */
    static void setOSDTime(bool enable);

    /**
     * @brief Переключить отображение кратности зума (VL-MAG)
     * @param enable true = включить, false = выключить
     */
    static void setOSDVLmag(bool enable);

    // =========================================================================
    // Позиции подвеса
    // =========================================================================

    /**
     * @brief Перейти в позицию надир (home)
     */
    static void goToHome();

    /**
     * @brief Перейти в позицию фронт (0, 0)
     */
    static void goToFront();

private:
    static std::weak_ptr<Gimbal> s_gimbal;
    static CommandHandler* s_instance;
    static ImageType s_currentImageType;
    static bool s_pipEnabled;
};

} // namespace gimbal
