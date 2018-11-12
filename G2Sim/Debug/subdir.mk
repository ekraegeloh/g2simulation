################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../SRK.cpp 

OBJS += \
./SRK.o 

CPP_DEPS += \
./SRK.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -std=gnu++11 -I/home/work/Simulation/G2Sim/include -I/opt/boost_1_58_0 -I/opt/root6/include -O0 -g3 -Wall -c -fmessage-length=0 -fext-numeric-literals -Wno-misleading-indentation -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


