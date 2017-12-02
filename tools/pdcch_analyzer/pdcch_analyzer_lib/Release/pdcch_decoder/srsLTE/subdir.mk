################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../pdcch_decoder/srsLTE/bit.c \
../pdcch_decoder/srsLTE/convcoder.c \
../pdcch_decoder/srsLTE/crc.c \
../pdcch_decoder/srsLTE/dci.c \
../pdcch_decoder/srsLTE/debug.c \
../pdcch_decoder/srsLTE/parity.c \
../pdcch_decoder/srsLTE/phy_common.c \
../pdcch_decoder/srsLTE/ra.c \
../pdcch_decoder/srsLTE/rm_conv.c \
../pdcch_decoder/srsLTE/sequence.c \
../pdcch_decoder/srsLTE/vector.c \
../pdcch_decoder/srsLTE/viterbi.c \
../pdcch_decoder/srsLTE/viterbi37_avx2.c \
../pdcch_decoder/srsLTE/viterbi37_neon.c \
../pdcch_decoder/srsLTE/viterbi37_port.c \
../pdcch_decoder/srsLTE/viterbi37_sse.c 

OBJS += \
./pdcch_decoder/srsLTE/bit.o \
./pdcch_decoder/srsLTE/convcoder.o \
./pdcch_decoder/srsLTE/crc.o \
./pdcch_decoder/srsLTE/dci.o \
./pdcch_decoder/srsLTE/debug.o \
./pdcch_decoder/srsLTE/parity.o \
./pdcch_decoder/srsLTE/phy_common.o \
./pdcch_decoder/srsLTE/ra.o \
./pdcch_decoder/srsLTE/rm_conv.o \
./pdcch_decoder/srsLTE/sequence.o \
./pdcch_decoder/srsLTE/vector.o \
./pdcch_decoder/srsLTE/viterbi.o \
./pdcch_decoder/srsLTE/viterbi37_avx2.o \
./pdcch_decoder/srsLTE/viterbi37_neon.o \
./pdcch_decoder/srsLTE/viterbi37_port.o \
./pdcch_decoder/srsLTE/viterbi37_sse.o 

C_DEPS += \
./pdcch_decoder/srsLTE/bit.d \
./pdcch_decoder/srsLTE/convcoder.d \
./pdcch_decoder/srsLTE/crc.d \
./pdcch_decoder/srsLTE/dci.d \
./pdcch_decoder/srsLTE/debug.d \
./pdcch_decoder/srsLTE/parity.d \
./pdcch_decoder/srsLTE/phy_common.d \
./pdcch_decoder/srsLTE/ra.d \
./pdcch_decoder/srsLTE/rm_conv.d \
./pdcch_decoder/srsLTE/sequence.d \
./pdcch_decoder/srsLTE/vector.d \
./pdcch_decoder/srsLTE/viterbi.d \
./pdcch_decoder/srsLTE/viterbi37_avx2.d \
./pdcch_decoder/srsLTE/viterbi37_neon.d \
./pdcch_decoder/srsLTE/viterbi37_port.d \
./pdcch_decoder/srsLTE/viterbi37_sse.d 


# Each subdirectory must supply rules for building sources it contributes
pdcch_decoder/srsLTE/%.o: ../pdcch_decoder/srsLTE/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


