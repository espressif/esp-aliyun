#!/bin/bash
set -e

# 安装软件
sudo apt-get install git wget make libncurses-dev flex bison gperf python python-serial

# 下载工具链
mkdir -p ~/esp-toolchain
cd ~/esp-toolchain
wget https://dl.espressif.com/dl/xtensa-esp32-elf-linux64-1.22.0-61-gab8375a-5.2.0.tar.gz
tar -xzf xtensa-esp32-elf-linux64-1.22.0-61-gab8375a-5.2.0.tar.gz

# 配置环境变量
echo 'export IDF_PATH=$(pwd)/esp-idf' >> ~/.profile
echo 'export PATH=$PATH:$HOME/esp-toolchain/xtensa-esp32-elf/bin' >> ~/.profile
source ~/.profile