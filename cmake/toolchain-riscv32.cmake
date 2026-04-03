# RISC-V 32-bit Cross-Compilation Toolchain
# Requires: riscv32-unknown-elf-gcc toolchain installed
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR riscv32)

set(TOOLCHAIN_PREFIX riscv32-unknown-elf-)

find_program(CMAKE_C_COMPILER ${TOOLCHAIN_PREFIX}gcc)
find_program(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}g++)
find_program(CMAKE_ASM_COMPILER ${TOOLCHAIN_PREFIX}gcc)
find_program(CMAKE_AR ${TOOLCHAIN_PREFIX}ar)
find_program(CMAKE_OBJCOPY ${TOOLCHAIN_PREFIX}objcopy)
find_program(CMAKE_OBJDUMP ${TOOLCHAIN_PREFIX}objdump)
find_program(CMAKE_SIZE ${TOOLCHAIN_PREFIX}size)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(MCU_FLAGS "-march=rv32imac -mabi=ilp32")
set(CMAKE_C_FLAGS_INIT "${MCU_FLAGS} -fdata-sections -ffunction-sections -fno-exceptions --specs=nano.specs")
set(CMAKE_CXX_FLAGS_INIT "${MCU_FLAGS} -fdata-sections -ffunction-sections -fno-exceptions -fno-rtti --specs=nano.specs")
set(CMAKE_ASM_FLAGS_INIT "${MCU_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS_INIT "-Wl,--gc-sections --specs=nano.specs --specs=nosys.specs -nostartfiles")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
