################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../writers/FrameAverageWriter.cpp \
../writers/ResultWriter.cpp \
../writers/SfnIterationAverageWriter.cpp \
../writers/TimeAverageWriter.cpp 

OBJS += \
./writers/FrameAverageWriter.o \
./writers/ResultWriter.o \
./writers/SfnIterationAverageWriter.o \
./writers/TimeAverageWriter.o 

CPP_DEPS += \
./writers/FrameAverageWriter.d \
./writers/ResultWriter.d \
./writers/SfnIterationAverageWriter.d \
./writers/TimeAverageWriter.d 


# Each subdirectory must supply rules for building sources it contributes
writers/%.o: ../writers/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I../../pdcch_analyzer_lib -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


