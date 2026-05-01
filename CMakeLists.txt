#cmake -B build -G Ninja
#cmake --build build
#cmake --build build --target flash

cmake_minimum_required(VERSION 3.20)

# AVR cross-compile setup
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_C_COMPILER avr-gcc)
set(CMAKE_CXX_COMPILER avr-g++)
set(CMAKE_ASM_COMPILER avr-gcc)

project(main LANGUAGES C CXX ASM)

# -------------------------------------------------------------------------------------------------
# Project parameters
# -------------------------------------------------------------------------------------------------

# Project name
set(PRJ main)

# AVR MCU
set(MCU atmega328)

# MCU clock frequency
# set(CLK 16000000)
set(CLK 8000000)

# AVR programmer
set(PRG atmelice_isp)

# Fuse values
set(LFU 0xFF)
set(HFU 0xDE)
set(EFU 0x05)

# Main source file
set(SRC
    ${PRJ}.c
)

# External source/include directories
set(EXT
    Source
    Drivers
    Source/include
    Source/portable
    Source/portable/GCC/ATMega328
    Source/portable/MemMang
)

# -------------------------------------------------------------------------------------------------
# AVR tools
# -------------------------------------------------------------------------------------------------

set(AVRDUDE avrdude)

find_program(AVRDUDE_EXE avrdude REQUIRED)
find_program(OBJCOPY_EXE avr-objcopy REQUIRED)
find_program(OBJDUMP_EXE avr-objdump REQUIRED)
find_program(SIZE_EXE    avr-size REQUIRED)

# -------------------------------------------------------------------------------------------------
# Collect external .c and .cpp files
# Same behavior as your Makefile: only files directly inside each EXT directory
# -------------------------------------------------------------------------------------------------

set(EXT_SOURCES)

foreach(dir ${EXT})
    file(GLOB DIR_C_FILES CONFIGURE_DEPENDS
        "${dir}/*.c"
    )

    file(GLOB DIR_CPP_FILES CONFIGURE_DEPENDS
        "${dir}/*.cpp"
    )

    list(APPEND EXT_SOURCES
        ${DIR_C_FILES}
        ${DIR_CPP_FILES}
    )
endforeach()

# -------------------------------------------------------------------------------------------------
# Build ELF
# -------------------------------------------------------------------------------------------------

add_executable(${PRJ}
    ${SRC}
    ${EXT_SOURCES}
)

set_target_properties(${PRJ} PROPERTIES
    OUTPUT_NAME ${PRJ}
    SUFFIX ".elf"
)

target_include_directories(${PRJ} PRIVATE
    ${EXT}
)

target_compile_definitions(${PRJ} PRIVATE
    F_CPU=${CLK}
)

target_compile_options(${PRJ} PRIVATE
    -Wall
    -Os
    -mmcu=${MCU}
)

target_link_options(${PRJ} PRIVATE
    -mmcu=${MCU}
)

# Optional C++ flags
target_compile_options(${PRJ} PRIVATE
    $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>
    $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
)

# -------------------------------------------------------------------------------------------------
# Generate HEX
# -------------------------------------------------------------------------------------------------

add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/${PRJ}.hex
    COMMAND ${OBJCOPY_EXE}
            -j .text
            -j .data
            -O ihex
            $<TARGET_FILE:${PRJ}>
            ${CMAKE_BINARY_DIR}/${PRJ}.hex
    COMMAND ${SIZE_EXE}
            --format=avr
            --mcu=${MCU}
            $<TARGET_FILE:${PRJ}>
    DEPENDS ${PRJ}
    COMMENT "Generating ${PRJ}.hex and printing AVR memory usage"
)

add_custom_target(hex ALL
    DEPENDS ${CMAKE_BINARY_DIR}/${PRJ}.hex
)

# -------------------------------------------------------------------------------------------------
# Test programmer connectivity
# -------------------------------------------------------------------------------------------------

add_custom_target(test_programmer
    COMMAND ${AVRDUDE_EXE}
            -c ${PRG}
            -p ${MCU}
            -v
    COMMENT "Testing AVR programmer connectivity"
)

# -------------------------------------------------------------------------------------------------
# Flash program
# -------------------------------------------------------------------------------------------------

add_custom_target(flash
    COMMAND ${AVRDUDE_EXE}
            -c ${PRG}
            -p ${MCU}
            -U flash:w:${CMAKE_BINARY_DIR}/${PRJ}.hex:i
    DEPENDS hex
    COMMENT "Flashing ${PRJ}.hex to ${MCU}"
)

# -------------------------------------------------------------------------------------------------
# Write fuses
# -------------------------------------------------------------------------------------------------

add_custom_target(fuse
    COMMAND ${AVRDUDE_EXE}
            -c ${PRG}
            -p ${MCU}
            -U lfuse:w:${LFU}:m
            -U hfuse:w:${HFU}:m
            -U efuse:w:${EFU}:m
    COMMENT "Writing fuse values to ${MCU}"
)

# -------------------------------------------------------------------------------------------------
# Generate disassembly
# -------------------------------------------------------------------------------------------------

add_custom_target(disasm
    COMMAND ${OBJDUMP_EXE}
            -d
            $<TARGET_FILE:${PRJ}>
            > ${CMAKE_BINARY_DIR}/${PRJ}.lst
    DEPENDS ${PRJ}
    COMMENT "Generating disassembly file ${PRJ}.lst"
)

# -------------------------------------------------------------------------------------------------
# Clean generated AVR files
# CMake already has its own clean target.
# This target removes hex/lst files from the build directory.
# -------------------------------------------------------------------------------------------------

add_custom_target(clean_avr
    COMMAND ${CMAKE_COMMAND} -E rm -f
            ${CMAKE_BINARY_DIR}/${PRJ}.hex
            ${CMAKE_BINARY_DIR}/${PRJ}.lst
    COMMENT "Removing generated AVR output files"
)