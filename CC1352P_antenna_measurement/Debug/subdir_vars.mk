################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Add inputs and outputs from these tool invocations to the build variables 
CMD_SRCS += \
../cc13x2_cc26x2_tirtos7.cmd 

SYSCFG_SRCS += \
../uart2callback.syscfg 

C_SRCS += \
../RFQueue.c \
../main_tirtos.c \
../uart2callback.c \
./syscfg/ti_devices_config.c \
./syscfg/ti_radio_config.c \
./syscfg/ti_drivers_config.c \
./syscfg/ti_sysbios_config.c 

GEN_FILES += \
./syscfg/ti_devices_config.c \
./syscfg/ti_radio_config.c \
./syscfg/ti_drivers_config.c \
./syscfg/ti_sysbios_config.c 

GEN_MISC_DIRS += \
./syscfg/ 

C_DEPS += \
./RFQueue.d \
./main_tirtos.d \
./uart2callback.d \
./syscfg/ti_devices_config.d \
./syscfg/ti_radio_config.d \
./syscfg/ti_drivers_config.d \
./syscfg/ti_sysbios_config.d 

OBJS += \
./RFQueue.obj \
./main_tirtos.obj \
./uart2callback.obj \
./syscfg/ti_devices_config.obj \
./syscfg/ti_radio_config.obj \
./syscfg/ti_drivers_config.obj \
./syscfg/ti_sysbios_config.obj 

GEN_MISC_FILES += \
./syscfg/ti_radio_config.h \
./syscfg/ti_drivers_config.h \
./syscfg/ti_utils_build_linker.cmd.genlibs \
./syscfg/syscfg_c.rov.xs \
./syscfg/ti_utils_runtime_model.gv \
./syscfg/ti_utils_runtime_Makefile \
./syscfg/ti_sysbios_config.h 

GEN_MISC_DIRS__QUOTED += \
"syscfg\" 

OBJS__QUOTED += \
"RFQueue.obj" \
"main_tirtos.obj" \
"uart2callback.obj" \
"syscfg\ti_devices_config.obj" \
"syscfg\ti_radio_config.obj" \
"syscfg\ti_drivers_config.obj" \
"syscfg\ti_sysbios_config.obj" 

GEN_MISC_FILES__QUOTED += \
"syscfg\ti_radio_config.h" \
"syscfg\ti_drivers_config.h" \
"syscfg\ti_utils_build_linker.cmd.genlibs" \
"syscfg\syscfg_c.rov.xs" \
"syscfg\ti_utils_runtime_model.gv" \
"syscfg\ti_utils_runtime_Makefile" \
"syscfg\ti_sysbios_config.h" 

C_DEPS__QUOTED += \
"RFQueue.d" \
"main_tirtos.d" \
"uart2callback.d" \
"syscfg\ti_devices_config.d" \
"syscfg\ti_radio_config.d" \
"syscfg\ti_drivers_config.d" \
"syscfg\ti_sysbios_config.d" 

GEN_FILES__QUOTED += \
"syscfg\ti_devices_config.c" \
"syscfg\ti_radio_config.c" \
"syscfg\ti_drivers_config.c" \
"syscfg\ti_sysbios_config.c" 

C_SRCS__QUOTED += \
"../RFQueue.c" \
"../main_tirtos.c" \
"../uart2callback.c" \
"./syscfg/ti_devices_config.c" \
"./syscfg/ti_radio_config.c" \
"./syscfg/ti_drivers_config.c" \
"./syscfg/ti_sysbios_config.c" 

SYSCFG_SRCS__QUOTED += \
"../uart2callback.syscfg" 


