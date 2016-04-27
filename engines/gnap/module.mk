MODULE := engines/gnap

MODULE_OBJS := \
	datarchive.o \
	debugger.o \
	detection.o \
	gamesys.o \
	gnap.o \
	grid.o \
	menu.o \
	resource.o \
	sound.o \
	scenes/groupcs.o \
	scenes/group0.o \
	scenes/group1.o \
	scenes/group2.o \
	scenes/intro.o \
	scenes/scene30.o \
	scenes/scene31.o \
	scenes/scene32.o \
	scenes/scene33.o \
	scenes/scene38.o \
	scenes/scene39.o \
	scenes/scene40.o \
	scenes/scene41.o \
	scenes/scene42.o \
	scenes/scene43.o \
	scenes/scene44.o \
	scenes/scene45.o \
	scenes/scene46.o \
	scenes/scene49.o \
	scenes/scene50.o \
	scenes/scene51.o \
	scenes/scene52.o \
	scenes/scene53.o \
	scenes/scenecore.o

# This module can be built as a plugin
ifeq ($(ENABLE_GNAP), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
