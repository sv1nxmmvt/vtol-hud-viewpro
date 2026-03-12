#!/usr/bin/env bash
# Скрипт сборки MAVSDK для проекта hud-viewpro
# Использование: ./rebuild-mavsdk.sh [--clean] [--install-prefix PATH]

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
MAVSDK_DIR="$SCRIPT_DIR/MAVSDK"
BUILD_DIR="$MAVSDK_DIR/cpp/build"
INSTALL_DIR="$MAVSDK_DIR/cpp/install"

DO_CLEAN=0
CUSTOM_INSTALL=""

# Parse arguments
while [[ $# -gt 0 ]]; do
    case "$1" in
        --clean)
            DO_CLEAN=1
            shift
            ;;
        --install-prefix)
            CUSTOM_INSTALL="$2"
            shift 2
            ;;
        *)
            echo "Неизвестный аргумент: $1"
            echo "Использование: $0 [--clean] [--install-prefix PATH]"
            exit 1
            ;;
    esac
done

if [[ -n "$CUSTOM_INSTALL" ]]; then
    INSTALL_DIR="$CUSTOM_INSTALL"
fi

echo "========================================"
echo "Сборка MAVSDK"
echo "========================================"
echo "MAVSDK_DIR:    $MAVSDK_DIR"
echo "BUILD_DIR:     $BUILD_DIR"
echo "INSTALL_DIR:   $INSTALL_DIR"
echo "========================================"

if [[ $DO_CLEAN -eq 1 ]]; then
    echo "[Очистка] Удаление директории сборки..."
    rm -rf "$BUILD_DIR"
fi

mkdir -p "$BUILD_DIR"

echo "[Конфигурация] Запуск cmake..."
cd "$BUILD_DIR" || exit 1
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR" \
    -DBUILD_SHARED_LIBS=ON \
    -DBUILD_TESTING=OFF \
    -DBUILD_MAVSDK_SERVER=OFF

if [[ $? -ne 0 ]]; then
    echo "========================================"
    echo "❌ ОШИБКА: Конфигурация не удалась!"
    echo "========================================"
    exit 1
fi

echo ""
echo "[Сборка] Запуск сборки (это может занять несколько минут)..."
cd "$BUILD_DIR" || exit 1
cmake --build . -j"$(nproc)"

if [[ $? -ne 0 ]]; then
    echo "========================================"
    echo "❌ ОШИБКА: Сборка не удалась!"
    echo "========================================"
    exit 1
fi

echo ""
echo "[Установка] Копирование файлов в $INSTALL_DIR..."
cd "$BUILD_DIR" || exit 1
cmake --install .

if [[ $? -ne 0 ]]; then
    echo "========================================"
    echo "❌ ОШИБКА: Установка не удалась!"
    echo "========================================"
    exit 1
fi

echo ""
echo "========================================"
echo "✅ Сборка MAVSDK завершена успешно!"
echo "========================================"
echo ""
echo "Библиотеки установлены в: $INSTALL_DIR"
echo ""
echo "Для использования в вашем проекте добавьте в CMakeLists.txt:"
echo "  set(CMAKE_PREFIX_PATH \"\$CMAKE_SOURCE_DIR/MAVSDK/cpp/install\" \${CMAKE_PREFIX_PATH})"
echo "  find_package(MAVSDK REQUIRED)"
echo "  target_link_libraries(your_target PRIVATE MAVSDK::mavsdk)"
echo ""
echo "Примеры подключения к ArduPilot:"
echo "  - UDP:  udp://:14540 (слушать порт 14540)"
echo "  - TCP:  tcp://127.0.0.1:5760"
echo "  - Serial: serial:///dev/ttyUSB0:5760"
echo ""