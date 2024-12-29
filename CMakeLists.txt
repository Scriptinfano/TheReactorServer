# 要求的CMake最小版本
cmake_minimum_required(VERSION 3.10)

# 项目名称
project(MyCppProject)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 23)

# 关闭编译器的返回值优化
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-elide-constructors")

# 添加调试信息，再生成的目标文件中添加符号表，以便调试器可以解析变量名称和函数调用等信息
set(CMAKE_BUILD_TYPE Debug)

# 定义一些路径的名字
set(INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)
set(IMPL_DIR ${CMAKE_SOURCE_DIR}/src/impl)
set(BIN_DIR ${CMAKE_SOURCE_DIR}/bin)
set(LIB_DIR ${CMAKE_SOURCE_DIR}/libs)

# 设置全局可执行文件输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BIN_DIR})

# 包含头文件路径
include_directories(${INCLUDE_DIR})
link_directories(${LIB_DIR})

# 获取 src/impl 目录下的所有 .cpp 文件
file(GLOB IMPL_FILES "${IMPL_DIR}/*.cpp")

# 获取 src 目录下的所有 .cpp 文件（主程序文件，包LIB_DIR函数）
file(GLOB SRC_FILES "${CMAKE_SOURCE_DIR}/src/*.cpp")

# 获取动态库文件
file(GLOB DYNAMIC_LIBS "${LIB_DIR}/*.so")

# 为每个 src/*.cpp 创建一个可执行文件
foreach(CPP_FILE ${SRC_FILES})
    # 获取文件名（不含路径和扩展名）
    get_filename_component(EXE_NAME ${CPP_FILE} NAME_WE)
    
    # 添加可执行文件，将 IMPL_FILES 作为公共依赖链接到所有可执行文件
    add_executable(${EXE_NAME} ${CPP_FILE} ${IMPL_FILES})

    # 可选：为每个目标设置单独的输出目录
    set_target_properties(${EXE_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${BIN_DIR}
        BUILD_RPATH ${LIB_DIR}
        INSTALL_RPATH ${LIB_DIR}
    )
    target_link_libraries(${EXE_NAME} ${DYNAMIC_LIBS})
endforeach()
