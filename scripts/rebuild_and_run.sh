#!/bin/bash
# 请在项目的根目录下执行此文件

# 重新生成构建文件
cmake -S ./ -B ./build

# 编译
cmake --build ./build

# 运行可执行文件
if [ -f "./bin/epollserver" ]; then
    echo "Running the epollserver:"
    echo "Usage: ./bin/epollserver <IP> <PORT>"
    echo "Defaulting to 127.0.0.1 8080"
    ./bin/epollserver 127.0.0.1 8080
else
    echo "Build failed. Executable not found."
fi
