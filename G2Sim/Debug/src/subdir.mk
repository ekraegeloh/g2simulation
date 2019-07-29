################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/SRKBField.cpp \
../src/SRKDipoleField.cpp \
../src/SRKEField.cpp \
../src/SRKEquationOfMotion.cpp \
../src/SRKField.cpp \
../src/SRKGlobalField.cpp \
../src/SRKGradientField.cpp \
../src/SRKGraphics.cpp \
../src/SRKInterpolatedField.cpp \
../src/SRKMacroManager.cpp \
../src/SRKManager.cpp \
../src/SRKMotionState.cpp \
../src/SRKMotionTracker.cpp \
../src/SRKODEState.cpp \
../src/SRKQuadrupoleField.cpp \
../src/SRKROOTField.cpp \
../src/SRKSextupoleField.cpp \
../src/SRKSpinState.cpp \
../src/SRKSpinTracker.cpp \
../src/SRKUniformField.cpp 

OBJS += \
./src/SRKBField.o \
./src/SRKDipoleField.o \
./src/SRKEField.o \
./src/SRKEquationOfMotion.o \
./src/SRKField.o \
./src/SRKGlobalField.o \
./src/SRKGradientField.o \
./src/SRKGraphics.o \
./src/SRKInterpolatedField.o \
./src/SRKMacroManager.o \
./src/SRKManager.o \
./src/SRKMotionState.o \
./src/SRKMotionTracker.o \
./src/SRKODEState.o \
./src/SRKQuadrupoleField.o \
./src/SRKROOTField.o \
./src/SRKSextupoleField.o \
./src/SRKSpinState.o \
./src/SRKSpinTracker.o \
./src/SRKUniformField.o 

CPP_DEPS += \
./src/SRKBField.d \
./src/SRKDipoleField.d \
./src/SRKEField.d \
./src/SRKEquationOfMotion.d \
./src/SRKField.d \
./src/SRKGlobalField.d \
./src/SRKGradientField.d \
./src/SRKGraphics.d \
./src/SRKInterpolatedField.d \
./src/SRKMacroManager.d \
./src/SRKManager.d \
./src/SRKMotionState.d \
./src/SRKMotionTracker.d \
./src/SRKODEState.d \
./src/SRKQuadrupoleField.d \
./src/SRKROOTField.d \
./src/SRKSextupoleField.d \
./src/SRKSpinState.d \
./src/SRKSpinTracker.d \
./src/SRKUniformField.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -std=gnu++11 -I/home/work/Simulation/G2/G2Sim/include -I/opt/boost_1_58_0 -I/opt/root6/include -O0 -g3 -Wall -c -fmessage-length=0 -fext-numeric-literals -Wno-misleading-indentation -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


