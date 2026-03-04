#!/bin/bash
# Скрипт установки runtime-зависимостей для hud-viewpro на Ubuntu

set -e

echo "Обновление списков пакетов..."
sudo apt update

echo "Установка библиотек Qt6..."
sudo apt install -y \
    libqt6core6 \
    libqt6gui6 \
    libqt6widgets6 \
    libqt6multimedia6 \
    libqt6serialport6

echo "Установка GStreamer и плагинов..."
sudo apt install -y \
    libgstreamer1.0-0 \
    gstreamer1.0-plugins-base \
    gstreamer1.0-plugins-good \
    gstreamer1.0-plugins-bad \
    gstreamer1.0-libav \
    gstreamer1.0-tools

echo "Установка дополнительных библиотек..."
sudo apt install -y \
    libavcodec60 \
    libavformat60 \
    libswscale7 \
    libgl1 \
    libsdl2-2.0-0 \
    libopencv4.5

echo "Все зависимости установлены!"
