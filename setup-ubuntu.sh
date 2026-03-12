#!/bin/bash
# Скрипт установки зависимостей для разработки hud-viewpro на Ubuntu

set -e

echo "Обновление списков пакетов..."
sudo apt update

echo "Установка инструментов разработки..."
sudo apt install -y \
    cmake \
    ninja-build \
    build-essential \
    pkg-config

echo "Установка библиотек Qt6..."
sudo apt install -y \
    libqt6core6 \
    libqt6gui6 \
    libqt6widgets6 \
    libqt6multimedia6 \
    libqt6serialport6 \
    qt6-base-dev \
    qt6-multimedia-dev \
    qt6-serialport-dev

echo "Установка GStreamer и плагинов..."
sudo apt install -y \
    libgstreamer1.0-0 \
    gstreamer1.0-plugins-base \
    gstreamer1.0-plugins-good \
    gstreamer1.0-plugins-bad \
    gstreamer1.0-libav \
    gstreamer1.0-tools \
    libgstreamer1.0-dev \
    libgstreamer-plugins-base1.0-dev

echo "Установка дополнительных библиотек..."
sudo apt install -y \
    libavcodec-dev \
    libavformat-dev \
    libswscale-dev \
    libgl1 \
    libsdl2-2.0-0 \
    libsdl2-dev \
    libopencv-dev

echo "Все зависимости установлены."
