#!/bin/bash
# 请在项目的根目录下执行此文件
# 删除旧的构建文件
rm -rf ./build/* ./bin/*

# 重新生成构建文件。-S用于指定源代码目录（Source Directory），也就是包含 CMakeLists.txt 文件的目录，这是告诉 CMake 去哪里找构建配置文件；-B ./build 表示生成的构建文件将存储在 build 子目录中成构建文件
cmake -S ./ -B ./build

# 编译
cmake --build ./build

# 运行可执行文件
if [ -f "./bin/MyCppExecutable" ]; then
    echo "Running the program:"
    bin/MyCppExecutable
else
    echo "Build failed. Executable not found."
fi

