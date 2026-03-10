#!/usr/bin/env fish
# Скрипт установки runtime-зависимостей для hud-viewpro на Arch Linux

echo "Обновление базы пакетов..."
sudo pacman -Sy --noconfirm

echo ""
echo "Установка библиотек Qt6..."
sudo pacman -S --noconfirm \
    qt6-base \
    qt6-multimedia \
    qt6-serialport

echo ""
echo "Установка GStreamer и плагинов..."
sudo pacman -S --noconfirm \
    gstreamer \
    gst-plugins-base \
    gst-plugins-good \
    gst-plugins-bad \
    gst-libav

echo ""
echo "Установка дополнительных библиотек..."
sudo pacman -S --noconfirm \
    ffmpeg \
    mesa \
    sdl2 \
    opencv

echo ""
echo "========================================"
echo "Все зависимости установлены."
echo "========================================"
echo ""
