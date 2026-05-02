# Headers live next to each owner under src/<name>/ (see docs/TEAMS.md). Build output under build/.

CXX       ?= g++
CXXFLAGS  ?= -std=c++17 -Wall -Wextra -O2
CPPFLAGS  ?= -Isrc/scott -Isrc/wu-yiming -Isrc/enya -Isrc/francis -Isrc/victor -Isrc/zhang-peihan
LDFLAGS   ?=
LDLIBS    ?=

BUILD_DIR := build
SRC_DIR   := src
TARGET    := $(BUILD_DIR)/gp

# Dependency order: foundation → data/shop → monsters → heroes → battle view → combat → hell/post → UI → main
SRCS := \
	$(SRC_DIR)/scott/utils.cpp \
	$(SRC_DIR)/scott/console_ui.cpp \
	$(SRC_DIR)/wu-yiming/game_data.cpp \
	$(SRC_DIR)/wu-yiming/shop_system.cpp \
	$(SRC_DIR)/enya/monster_system.cpp \
	$(SRC_DIR)/francis/hero_system.cpp \
	$(SRC_DIR)/francis/battle_view.cpp \
	$(SRC_DIR)/victor/battle_system.cpp \
	$(SRC_DIR)/enya/hell_mode.cpp \
	$(SRC_DIR)/zhang-peihan/ui_flow.cpp \
	$(SRC_DIR)/scott/main.cpp

OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRCS))

all: $(TARGET)

$(BUILD_DIR):
	mkdir -p $@

$(TARGET): $(OBJS) | $(BUILD_DIR)
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)

rebuild: clean all

.PHONY: all clean rebuild
