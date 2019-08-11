TARGET_EXEC := OceanRendering
LOG_FILE := build-log.txt

BUILD_DIR ?= bin
SRC_DIRS ?= src

LIB_DIR := C:/ProgrammingLibraries

CC	:= "C:/MinGW-w64/bin/g++"
CXX := "C:/MinGW-w64/bin/g++"

rwildcard=$(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2) $(filter $(subst *,%,$2),$d))

SRCS := $(call rwildcard, $(SRC_DIRS)/, *.cpp *.c)
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)

LDFLAGS := -Wall -Wl,--out-implib,libNewEngine.dll.a -Wl,--major-image-version,0,--minor-image-version,0
LDLIBS := -lglu32 -lopengl32 -L"$(LIB_DIR)/Glew/lib" -lglew32 -L"$(LIB_DIR)/GLFW/lib" -lglfw3 -L"$(LIB_DIR)/GLM/lib" -lkernel32 -luser32 -lgdi32 -lwinspool -lshell32 -lole32 -loleaut32 -luuid -lcomdlg32 -ladvapi32

#INC_DIRS := $(shell find $(SRC_DIRS) -type d)
#INC_FLAGS := $(addprefix -I,$(INC_DIRS))

#CPPFLAGS ?= $(INC_FLAGS) -MMD -MP

CPPFLAGS := -std=c++14 -g -ggdb
CXXFLAGS := -I$(CURDIR)/headers -I$(CURDIR)/sources -I$(CURDIR)/src -I$(LIB_DIR)/Glew/include -I$(LIB_DIR)/GLFW/include -I$(LIB_DIR)/GLM/include -msse2

$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS)
	@echo "Building $(TARGET_EXEC)..."
	@$(CC) $(OBJS) -o $@ $(LDFLAGS) $(LDLIBS)

# assembly
$(BUILD_DIR)/%.s.o: %.s
	@$(MKDIR_P) $(dir $@)
	@$(AS) $(ASFLAGS) -c $< -o $@

# c source
$(BUILD_DIR)/%.c.o: %.c
	@$(MKDIR_P) $(dir $@)
	@$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

# c++ source
$(BUILD_DIR)/%.cpp.o: %.cpp
	@echo "Building $@..."
	@$(MKDIR_P) $(dir $@)
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

.PHONY: clean run

run:
	@echo "Running $(TARGET_EXEC)..."
	@"./$(BUILD_DIR)/$(TARGET_EXEC).exe"

clean:
	@echo "Cleaning binaries..."
	@$(RM) -r $(BUILD_DIR)/$(SRC_DIRS)


MKDIR_P ?= mkdir -p
