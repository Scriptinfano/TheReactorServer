#!/bin/bash
# 请在项目的根目录下执行此文件

# 重新生成构建文件
cmake -S ./ -B ./build

# 编译
cmake --build ./build
