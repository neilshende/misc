BUILD_TYPE ?= $(shell ./check_day.sh)
ifeq ($(BUILD_TYPE),EVEN)
CXX  = gcc -fsanitize=address -g
else
CXX  = gcc -fsanitize=thread -g
endif
odd_build:
	#$(info Building for odd day)
	# build commands for odd days
	#$(CXX) a.c -o a.o1

even_build:
	#$(info Building for even day)
	# build commands for even days
	#$(CXX) a.c -o a.o2

#all: odd_build even_build
all:
#	$(if $(filter EVEN,$(BUILD_TYPE)),even_build,odd_build)
	@echo $(BUILD_TYPE)
