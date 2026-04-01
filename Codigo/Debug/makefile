################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include source/mcal/subdir.mk
-include source/hal/subdir.mk
-include source/app/subdir.mk
-include SDK/startup/subdir.mk
-include SDK/CMSIS/subdir.mk
ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

OPTIONAL_TOOL_DEPS := \
$(wildcard ../makefile.defs) \
$(wildcard ../makefile.init) \
$(wildcard ../makefile.targets) \


BUILD_ARTIFACT_NAME := controlDeAccesos
BUILD_ARTIFACT_EXTENSION := axf
BUILD_ARTIFACT_PREFIX :=
BUILD_ARTIFACT := $(BUILD_ARTIFACT_PREFIX)$(BUILD_ARTIFACT_NAME)$(if $(BUILD_ARTIFACT_EXTENSION),.$(BUILD_ARTIFACT_EXTENSION),)

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all:
	+@$(MAKE) --no-print-directory main-build && $(MAKE) --no-print-directory post-build

# Main-build Target
main-build: controlDeAccesos.axf

# Tool invocations
controlDeAccesos.axf: $(OBJS) $(USER_OBJS) makefile $(OPTIONAL_TOOL_DEPS)
	@echo 'Building target: $@'
	@echo 'Invoking: MCU Linker'
	@echo "Template not found for name "memory.ldt"." && echo "The name was interpreted by this TemplateLoader: com.crt.advproject.linkertemplate.FMLinkerFileTemplateLoader@5c5be3d0." && false
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) controlDeAccesos.axf
	-@echo ' '

post-build:
	-@echo 'Performing post-build steps'
	-arm-none-eabi-size "controlDeAccesos.axf"; # arm-none-eabi-objcopy -v -O binary "controlDeAccesos.axf" "controlDeAccesos.bin" ; # checksum -p MK64FN1M0xxx12 -d "controlDeAccesos.bin";
	-@echo ' '

.PHONY: all clean dependents main-build post-build

-include ../makefile.targets
