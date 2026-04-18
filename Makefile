# Optimized Haiku Build Script
SHELL := /bin/bash

UNAME_M := $(shell uname -p)
ifeq ($(UNAME_M), x86)
CXX = g++-x86 -DENABLE_PROJECTM=OFF -DENABLE_SDL2=OFF -DENABLE_GL=OFF -DUSE_KONSOLE_ON_HAIKU=OFF
CC = gcc-x86
MAKE := setarch x86 $(MAKE)
ARCH = x86_gcc2
SIMD_FLAGS := -O2
else ifeq ($(UNAME_M), x86_64)
CXX = g++ -DENABLE_PROJECTM=OFF -DENABLE_SDL2=OFF -DENABLE_GL=OFF -DUSE_KONSOLE_ON_HAIKU=OFF
CC = gcc
ARCH = x86_64
SIMD_FLAGS := -O3
endif


BUILD_FLAGS = $(SIMD_FLAGS) 
LD_OPTIMIZE = -Wl,--gc-sections


EXTRA_LIBS = -lmpv -lcurl -lopenal
HAIKU_LIBS = -lnetwork -lroot -lpthread


.PHONY: build clean

all: build

build: 
	@echo "--------- Building SuperMusicThingy $(ARCH) ---------"

	$(CXX) -o SuperMusicThingy -L/boot/system/lib/x86 $(BUILD_FLAGS) $(EXTRA_LIBS) $(HAIKU_LIBS) $(LD_OPTIMIZE) SuperMusicThingy.cpp
	mimeset -f SuperMusicThingy

UNAME_M := $(shell uname -p)
ifeq ($(UNAME_M), x86)
    ARCH = x86_gcc2
else ifeq ($(UNAME_M), x86_64)
    ARCH = x86_64
endif    


PACKAGE_DIR := build/package
NAME = SuperMusicThingy
VERSION = 1.0.0

package: all
	@[ -n "$(PACKAGE_DIR)" ] || { echo "PACKAGE_DIR is undefined"; exit 1; }
	rm -rf "./$(PACKAGE_DIR)"
	mkdir -p $(PACKAGE_DIR)
	sed -e 's/$$(NAME)/$(NAME)/g' -e 's/$$(VERSION)/$(VERSION)/g' -e 's/$$(ARCH)/$(ARCH)/' -e 's/$$(YEAR)/$(shell date +%Y)/' PackageInfo.tpl > $(PACKAGE_DIR)/.PackageInfo
	mkdir -p $(PACKAGE_DIR)/apps
	mkdir -p $(PACKAGE_DIR)/bin
	mkdir -p $(PACKAGE_DIR)/data/deskbar/menu/Applications
	xres -o $(NAME) SuperMusicThingy.rsrc  
	mimeset -f $(NAME)
	cp $(NAME) $(PACKAGE_DIR)/apps/$(NAME)
	ln -s ../apps/$(NAME) $(PACKAGE_DIR)/bin/$(NAME)
	ln -s ../../../../apps/$(NAME) $(PACKAGE_DIR)/data/deskbar/menu/Applications/$(NAME)
	package create -C $(PACKAGE_DIR) $(NAME)-$(VERSION)-1-$(ARCH).hpkg


clean:
	rm -f SuperMusicThingy


