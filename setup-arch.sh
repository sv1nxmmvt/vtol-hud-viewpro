#!/bin/bash
# Скрипт установки runtime-зависимостей для hud-viewpro на Arch Linux

set -e

echo "Обновление базы пакетов..."
sudo pacman -Sy --noconfirm

echo "Установка библиотек Qt6..."
sudo pacman -S --noconfirm \
    qt6-base \
    qt6-multimedia \
    qt6-serialport

echo "Установка GStreamer и плагинов..."
sudo pacman -S --noconfirm \
    gstreamer \
    gst-plugins-base \
    gst-plugins-good \
    gst-plugins-bad \
    gst-libav

echo "Установка дополнительных библиотек..."
sudo pacman -S --noconfirm \
    ffmpeg \
    mesa \
    sdl2 \
    opencv

echo "Все зависимости установлены!"
