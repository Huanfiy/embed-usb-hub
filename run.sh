#!/bin/bash

# OpenOCD配置
OPENOCD_INTERFACE="interface/stlink.cfg"
OPENOCD_TARGET="target/stm32f4x.cfg"
OPENOCD_CMD="openocd -f $OPENOCD_INTERFACE -f $OPENOCD_TARGET"

# 工具链路径
RTT_EXEC_PATH="/home/$USER/toolchain/arm-gnu-toolchain-13.3.rel1-x86_64-arm-none-eabi/bin"
# 构建路径
BUILD_PATH="./build"
# 固件名称[不可修改]
PROJECT="rt-thread"

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

error_exit() {
    echo -e "${RED}错误: $1${NC}" >&2
    exit 1
}

success_msg() {
    echo -e "${GREEN}$1${NC}"
}

info_msg() {
    echo -e "${YELLOW}$1${NC}"
}

# 检查OpenOCD是否可用
check_openocd() {
    command -v openocd >/dev/null 2>&1 || error_exit "OpenOCD 未安装或不在 PATH 中"
}

# 检查固件
check_firmware() {
    [ -f "$BUILD_PATH/$PROJECT.bin" ] || error_exit "固件文件不存在: $BUILD_PATH/$PROJECT.bin"
}

# 检查ELF文件
check_elf() {
    [ -f "$BUILD_PATH/$PROJECT.elf" ] || error_exit "ELF文件不存在: $BUILD_PATH/$PROJECT.elf"
}

# 命令: 构建项目
cmd_build() {
    info_msg "开始构建项目..."

    [ -d "$RTT_EXEC_PATH" ] || error_exit "工具链路径不存在: $RTT_EXEC_PATH"

    # 根据参数设置编译模式
    if [ "$2" = "debug" ]; then
        info_msg "编译模式: DEBUG"
        export DEBUG=1
    else
        info_msg "编译模式: RELEASE"
        unset DEBUG
    fi

    # 设置工具链环境变量并执行 scons 命令
    export RTT_EXEC_PATH
    info_msg "工具链路径: $RTT_EXEC_PATH"

    build_output=$(bear --output compile_commands.json -- scons -j16 2>&1)
    build_result=$?

    # 输出构建日志
    echo "$build_output"

    # 检查生成的 compile_commands.json 是否有内容
    if [ -f "compile_commands.json" ] && grep -q '"output"' "compile_commands.json"; then
        # 有实际编译命令
        mkdir -p .vscode
        mv compile_commands.json .vscode/
        info_msg "检测到编译活动，compile_commands.json 已更新"
    else
        # 没有实际编译命令
        [ -f "compile_commands.json" ] && rm -f compile_commands.json
        info_msg "无编译活动，保持现有的 compile_commands.json"
    fi

    # 检查构建结果和输出中是否包含错误信息
    if [ $build_result -eq 0 ] && ! echo "$build_output" | grep -q "Error:"; then
        success_msg "构建成功！"
    else
        error_exit "构建失败"
    fi
}

# 命令: 清理项目
cmd_clean() {
    info_msg "清理项目..."
    [ -d "$RTT_EXEC_PATH" ] || error_exit "工具链路径不存在: $RTT_EXEC_PATH"
    export RTT_EXEC_PATH
    scons -c || error_exit "清理失败"
    success_msg "清理完成！"
}

# 命令: 烧录固件
cmd_flash() {
    info_msg "准备烧录固件..."
    check_openocd
    check_firmware

    # 获取固件大小
    firmware_size=$(stat -c%s "$BUILD_PATH/$PROJECT.bin")
    info_msg "固件大小: $(($firmware_size / 1024)) KB ($firmware_size 字节)"

    info_msg "开始烧录固件..."

    # 执行烧录
    $OPENOCD_CMD -c "init" \
        -c "reset halt" \
        -c "flash write_image erase $BUILD_PATH/$PROJECT.bin 0x08000000" \
        -c "reset run" \
        -c "shutdown"
    flash_result=$?

    if [ $flash_result -eq 0 ]; then
        success_msg "固件烧录成功！"
    else
        error_exit "固件烧录失败！"
    fi
}

# 命令: 调试
cmd_debug() {
    cmd_clean
    cmd_build build debug

    info_msg "启动调试..."
    check_openocd
    check_elf

    info_msg "启动 OpenOCD 调试服务器..."
    info_msg "请在另一个终端中运行以下命令连接 GDB:"
    info_msg "arm-none-eabi-gdb $BUILD_PATH/$PROJECT.elf"
    info_msg "然后在 GDB 中执行: target remote localhost:3333"

    # 启动OpenOCD调试服务器
    $OPENOCD_CMD
}

# 命令: 启动调试服务器
cmd_debug_server() {
    info_msg "启动调试服务器..."
    check_openocd

    info_msg "启动 OpenOCD 调试服务器..."
    info_msg "GDB 连接端口: 3333"
    info_msg "Telnet 连接端口: 4444"

    # 启动OpenOCD调试服务器
    $OPENOCD_CMD
}

# 命令: GDB调试（自动启动GDB并连接）
cmd_gdb() {
    info_msg "启动 GDB 调试会话..."
    check_elf

    # 检查是否有OpenOCD在运行
    if ! pgrep -f "openocd.*stm32f4x" >/dev/null; then
        info_msg "OpenOCD 未运行，正在启动调试服务器..."
        $OPENOCD_CMD &
        OPENOCD_PID=$!
        sleep 2 # 等待OpenOCD启动
    fi

    # 创建临时的GDB脚本
    temp_gdb_script=$(mktemp)
    cat >"$temp_gdb_script" <<EOF
target remote localhost:3333
monitor reset halt
load
monitor reset halt
EOF

    info_msg "启动 GDB..."
    arm-none-eabi-gdb -x "$temp_gdb_script" "$BUILD_PATH/$PROJECT.elf"

    # 清理
    rm -f "$temp_gdb_script"
    if [ ! -z "$OPENOCD_PID" ]; then
        kill $OPENOCD_PID 2>/dev/null
    fi
}

# 命令: 重新构建并烧录
cmd_rebuild_flash() {
    cmd_clean
    cmd_build "$@"
    cmd_flash
}

# 命令: 构建并烧录
cmd_build_flash() {
    cmd_build
    cmd_flash
}

# 命令: 重新构建
cmd_rebuild() {
    cmd_clean
    cmd_build "$@"
}

# 显示帮助信息
show_help() {
    cat <<EOF
使用方法: $0 <命令> [选项]

可用命令:
    build          构建项目
    clean          清理项目
    flash          烧录固件
    build-flash    构建并烧录固件
    rebuild        清理并重新构建
    debug          清理、构建(debug模式)并启动OpenOCD调试服务器
    debug-server   仅启动OpenOCD调试服务器
    gdb            启动GDB调试会话（自动连接OpenOCD）
    rebuild-flash  清理、重新构建并烧录
    help           显示此帮助信息

编译选项:
    debug          使用 DEBUG 模式编译（添加 -g -O0 选项）
    release        使用 RELEASE 模式编译（默认，使用 -O2 选项）

OpenOCD配置:
    接口: $OPENOCD_INTERFACE
    目标: $OPENOCD_TARGET

示例:
    $0 build          # 构建项目（RELEASE 模式）
    $0 build debug    # 以 DEBUG 模式构建项目
    $0 build-flash    # 构建并烧录固件
    $0 flash          # 烧录固件
    $0 debug          # 启动完整调试流程
    $0 debug-server   # 仅启动OpenOCD调试服务器
    $0 gdb            # 启动GDB调试会话
EOF
}

# 主程序
main() {
    # 检查是否有参数
    if [ $# -eq 0 ]; then
        show_help
        exit 1
    fi

    # 处理命令
    case "$1" in
    "build")
        cmd_build "$@"
        ;;
    "clean")
        cmd_clean
        ;;
    "flash")
        cmd_flash
        ;;
    "build-flash")
        cmd_build_flash "$@"
        ;;
    "rebuild")
        cmd_rebuild "$@"
        ;;
    "debug")
        cmd_debug
        ;;
    "debug-server")
        cmd_debug_server
        ;;
    "gdb")
        cmd_gdb
        ;;
    "rebuild-flash")
        cmd_rebuild_flash "$@"
        ;;
    "help")
        show_help
        ;;
    *)
        error_exit "未知命令: $1\n使用 '$0 help' 查看可用命令"
        ;;
    esac
}

# 执行主程序
main "$@"
