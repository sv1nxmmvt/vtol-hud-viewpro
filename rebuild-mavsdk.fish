#!/usr/bin/env fish
# Скрипт сборки MAVSDK для проекта hud-viewpro
# Использование: ./rebuild-mavsdk.fish [--clean] [--install-prefix PATH]

set -l SCRIPT_DIR (cd (dirname (status -f)); and pwd)
set -l MAVSDK_DIR "$SCRIPT_DIR/MAVSDK"
set -l BUILD_DIR "$MAVSDK_DIR/cpp/build"
set -l INSTALL_DIR "$MAVSDK_DIR/cpp/install"

# Парсинг аргументов
set -l DO_CLEAN 0
set -l CUSTOM_INSTALL ""

for arg in $argv
    switch $arg
        case '--clean'
            set DO_CLEAN 1
        case '--install-prefix'
            # Следующий аргумент - путь
            set CUSTOM_INSTALL $argv[(math (contains -i -- $arg $argv) + 1)]
        case '*'
            echo "Неизвестный аргумент: $arg"
            echo "Использование: $status-filename [--clean] [--install-prefix PATH]"
            exit 1
    end
end

# Если указан кастомный префикс, используем его
if test -n "$CUSTOM_INSTALL"
    set INSTALL_DIR "$CUSTOM_INSTALL"
end

echo "========================================"
echo "Сборка MAVSDK"
echo "========================================"
echo "MAVSDK_DIR:    $MAVSDK_DIR"
echo "BUILD_DIR:     $BUILD_DIR"
echo "INSTALL_DIR:   $INSTALL_DIR"
echo "========================================"

# Очистка, если указано
if test $DO_CLEAN -eq 1
    echo "[Очистка] Удаление директории сборки..."
    rm -rf "$BUILD_DIR"
end

# Создание директории сборки
mkdir -p "$BUILD_DIR"

# Конфигурация
echo "[Конфигурация] Запуск cmake..."
cd "$BUILD_DIR" && \
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR" \
    -DBUILD_SHARED_LIBS=ON \
    -DBUILD_TESTING=OFF \
    -DBUILD_MAVSDK_SERVER=OFF

if test $status -ne 0
    echo "========================================"
    echo "❌ ОШИБКА: Конфигурация не удалась!"
    echo "========================================"
    exit 1
end

# Сборка
echo ""
echo "[Сборка] Запуск сборки (это может занять несколько минут)..."
cd "$BUILD_DIR" && \
cmake --build . -j(nproc)

if test $status -ne 0
    echo "========================================"
    echo "❌ ОШИБКА: Сборка не удалась!"
    echo "========================================"
    exit 1
end

# Установка
echo ""
echo "[Установка] Копирование файлов в $INSTALL_DIR..."
cd "$BUILD_DIR" && \
cmake --install .

if test $status -ne 0
    echo "========================================"
    echo "❌ ОШИБКА: Установка не удалась!"
    echo "========================================"
    exit 1
end

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
