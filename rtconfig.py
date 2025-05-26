import os

# toolchains options
ARCH = 'arm'
CPU = 'cortex-m4'
CROSS_TOOL = 'gcc'

# cross_tool provides the cross compiler
# EXEC_PATH is the compiler execute path, for example, CodeSourcery, Keil MDK, IAR
PLATFORM = 'gcc'
EXEC_PATH = ''

if os.getenv('RTT_EXEC_PATH'):
    EXEC_PATH = os.getenv('RTT_EXEC_PATH')

PREFIX = 'arm-none-eabi-'
CC = PREFIX + 'gcc'
AS = PREFIX + 'gcc'
AR = PREFIX + 'ar'
CXX = PREFIX + 'g++'
LINK = PREFIX + 'gcc'
TARGET_EXT = 'elf'
SIZE = PREFIX + 'size'
OBJDUMP = PREFIX + 'objdump'
OBJCPY = PREFIX + 'objcopy'
DEVICE = ''
CFLAGS = ''
AFLAGS = ''
LFLAGS = '-T linkscripts//STM32F446RC//link.lds'
CPATH = ''
LPATH = ''
CXXFLAGS = ''
POST_ACTION = ''


#=========================#
# my custom configuration #
#=========================#

DEVICE = 'STM32F446RCT6'

# 给编译器传递的选项
CFLAGS = [
    # 内核选项
    '-mcpu=cortex-m4',
    '-mthumb',
    '-mfloat-abi=hard',
    '-mfpu=fpv4-sp-d16',

    # STM32 芯片相关宏定义
    '-DSTM32F446xx',           # STM32F446 具体芯片型号
    '-DSTM32F4',               # ST官方F4系列标识
    '-DUSE_HAL_DRIVER',        # 使用 HAL 库
    
    # RT-Thread 系统相关宏
    '-DSOC_FAMILY_STM32',      # RT-Thread STM32家族标识
    '-DSOC_SERIES_STM32F4',    # RT-Thread F4系列标识
    
    # 时钟配置宏 (根据你的HAL配置文件，使用8MHz外部晶振)
    '-DHSE_VALUE=8000000',     # 外部高速晶振频率 8MHz
    # '-DHSI_VALUE=16000000',    # 内部高速晶振频率 16MHz
    # '-DLSE_VALUE=32768',       # 外部低速晶振频率 32.768kHz
    # '-DLSI_VALUE=32000',       # 内部低速晶振频率 32kHz
    
    # 编译选项
    '-fmessage-length=0',
    '-fsigned-char',
    '-ffunction-sections',
    '-fdata-sections',
    '-MMD -MP',
    '-Wall',
    '-std=gnu11'
]

# 给汇编器传递的选项
AFLAGS = [
    '-mcpu=cortex-m4',
    '-mthumb',
    '-mfloat-abi=hard',
    '-mfpu=fpv4-sp-d16',
]

# 给链接器传递的选项
LFLAGS = [
    '-T./linkscripts/STM32F446RC/link.lds',
    '-Wl,--gc-sections',
    '-Wl,-Map=build/output.map',
    '-Wl,--no-warn-rwx-segments',
    # '-Wl,--emit-relocs',
    '-mcpu=cortex-m4',
    '-mthumb',
    '-mfloat-abi=hard',
    '-mfpu=fpv4-sp-d16',
    '--specs=nano.specs --specs=nosys.specs'
]

# 如果设置了 DEBUG 环境变量, 则使用调试模式，可以通过脚本设置
if os.getenv('DEBUG'):
    CFLAGS.append('-DDEBUG')              # 调试模式宏
    CFLAGS.append('-g')
    CFLAGS.append('-O0')
    print('debug mode')
else:
    # CFLAGS.append('-g')
    CFLAGS.append('-O2')
    CFLAGS.append('-DNDEBUG')             # 发布模式宏
    print('release mode')

# 将列表转换为字符串, 因为最终送给 gcc 的是一串编译选项
CFLAGS = ' '.join(CFLAGS)
AFLAGS = ' '.join(AFLAGS)
LFLAGS = ' '.join(LFLAGS)

POST_ACTION = 'arm-none-eabi-objcopy -O binary $TARGET build/rt-thread.bin\n' + \
              'arm-none-eabi-objdump -D $TARGET > build/rt-thread.asm\n' + \
              'python3 post_build.py $TARGET\n'