################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: dump_record_print

dependents:
	-cd /home/carsten/Uni/ma/qc-baseband/tools/pdcch_analyzer/pdcch_analyzer_lib/Release && $(MAKE) clean all

# Tool invocations
dump_record_print: $(OBJS) $(USER_OBJS) /home/carsten/Uni/ma/qc-baseband/tools/pdcch_analyzer/pdcch_analyzer_lib/Release/libpdcch_analyzer_lib.a
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C++ Linker'
	g++ -L"/home/carsten/Uni/ma/qc-baseband/tools/pdcch_analyzer/pdcch_analyzer_lib/Release" -o "dump_record_print" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(CC_DEPS)$(C++_DEPS)$(EXECUTABLES)$(C_UPPER_DEPS)$(CXX_DEPS)$(OBJS)$(CPP_DEPS)$(C_DEPS) dump_record_print
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:
/home/carsten/Uni/ma/qc-baseband/tools/pdcch_analyzer/pdcch_analyzer_lib/Release/libpdcch_analyzer_lib.a:

-include ../makefile.targets
