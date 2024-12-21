#!/bin/bash
# 请在项目的根目录下执行此文件
# 删除旧的构建文件
rm -rf ./build/* ./bin/*

# 重新生成构建文件
cmake -S ./ -B ./build

# 编译
cmake --build ./build
