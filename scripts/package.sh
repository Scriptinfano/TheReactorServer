#!/bin/bash

# 获取当前脚本所在目录的上一级目录（即项目根目录）
PROJECT_ROOT=$(cd "$(dirname "$0")/.." && pwd)
BUILD_DIR="$PROJECT_ROOT/build_release"
OUTPUT_DIR="$PROJECT_ROOT/release_package"

echo "Project Root: $PROJECT_ROOT"

# 清理旧的构建和发布目录
rm -rf "$BUILD_DIR"
rm -rf "$OUTPUT_DIR"

# 创建发布目录
mkdir -p "$OUTPUT_DIR"

# 重新生成构建文件 (Release 模式)
echo "Configuring CMake (Release)..."
cmake -S "$PROJECT_ROOT" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release

# 编译
echo "Building..."
cmake --build "$BUILD_DIR" --config Release

# 检查编译是否成功
if [ $? -ne 0 ]; then
    echo "Build failed!"
    exit 1
fi

# 复制可执行文件到发布目录
echo "Packaging..."
if [ -d "$PROJECT_ROOT/bin" ]; then
    cp "$PROJECT_ROOT/bin/epollserver" "$OUTPUT_DIR/"
    cp "$PROJECT_ROOT/bin/epollclient" "$OUTPUT_DIR/"
else
    # 某些系统可能直接在 build 目录下生成
    cp "$BUILD_DIR/bin/epollserver" "$OUTPUT_DIR/"
    cp "$BUILD_DIR/bin/epollclient" "$OUTPUT_DIR/"
fi

# 复制必要的资源文件或配置（如果有）
cp -r "$PROJECT_ROOT/etc" "$OUTPUT_DIR/"

# 创建启动脚本
cat <<EOF > "$OUTPUT_DIR/start_server.sh"
#!/bin/bash
./epollserver 0.0.0.0 8080
EOF
chmod +x "$OUTPUT_DIR/start_server.sh"

cat <<EOF > "$OUTPUT_DIR/start_client.sh"
#!/bin/bash
if [ -z "\$1" ]; then
    echo "Usage: ./start_client.sh <SERVER_IP>"
    echo "Example: ./start_client.sh 192.168.1.100"
    exit 1
fi
./epollclient \$1 8080
EOF
chmod +x "$OUTPUT_DIR/start_client.sh"

# 打包
cd "$PROJECT_ROOT"
tar -czvf TheReactorServer_release.tar.gz release_package

echo "Package created at $PROJECT_ROOT/TheReactorServer_release.tar.gz"
echo "You can copy this tarball to another machine, extract it, and run the scripts."
