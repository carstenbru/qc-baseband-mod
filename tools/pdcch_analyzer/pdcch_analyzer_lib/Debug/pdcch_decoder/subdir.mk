################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../pdcch_decoder/DciResult.cpp \
../pdcch_decoder/PdcchDecoder.cpp 

OBJS += \
./pdcch_decoder/DciResult.o \
./pdcch_decoder/PdcchDecoder.o 

CPP_DEPS += \
./pdcch_decoder/DciResult.d \
./pdcch_decoder/PdcchDecoder.d 


# Each subdirectory must supply rules for building sources it contributes
pdcch_decoder/%.o: ../pdcch_decoder/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


