################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../records/PdcchAddCellInfoRecord.cpp \
../records/PdcchDataRecord.cpp \
../records/PdcchDumpRecord.cpp \
../records/PdcchGpsRecord.cpp \
../records/PdcchMainCellInfoRecord.cpp \
../records/PdcchTimeRecord.cpp 

OBJS += \
./records/PdcchAddCellInfoRecord.o \
./records/PdcchDataRecord.o \
./records/PdcchDumpRecord.o \
./records/PdcchGpsRecord.o \
./records/PdcchMainCellInfoRecord.o \
./records/PdcchTimeRecord.o 

CPP_DEPS += \
./records/PdcchAddCellInfoRecord.d \
./records/PdcchDataRecord.d \
./records/PdcchDumpRecord.d \
./records/PdcchGpsRecord.d \
./records/PdcchMainCellInfoRecord.d \
./records/PdcchTimeRecord.d 


# Each subdirectory must supply rules for building sources it contributes
records/%.o: ../records/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


