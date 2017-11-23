################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../analyzers/DataRateAnalyzer.cpp \
../analyzers/PrbCountAnalyzer.cpp \
../analyzers/SubframeAnalyzer.cpp 

OBJS += \
./analyzers/DataRateAnalyzer.o \
./analyzers/PrbCountAnalyzer.o \
./analyzers/SubframeAnalyzer.o 

CPP_DEPS += \
./analyzers/DataRateAnalyzer.d \
./analyzers/PrbCountAnalyzer.d \
./analyzers/SubframeAnalyzer.d 


# Each subdirectory must supply rules for building sources it contributes
analyzers/%.o: ../analyzers/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I../../pdcch_analyzer_lib -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


