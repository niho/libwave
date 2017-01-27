#
#  Makefile
#  libwave
#
#  Created by Niklas Holmgren on 2017-01-27.
#  Copyright (c) 2014 Join the conversation AB. All rights reserved.
#

LIB := digger_recorder

SRCDIR := src
EXTERNALDIR := external
PROJDIR := proj/xcode
BUILDDIR := $(PROJDIR)/build
XCODEBUILD := /usr/bin/xcodebuild
CONFIGURATION := Debug

SOURCES := $(SRCDIR)/**/**/* $(EXTERNALDIR)/**/**/*

IOS := $(BUILDDIR)/$(LIB).build/$(CONFIGURATION)-iphoneos/lib$(LIB)_ios.a
OSX := $(BUILDDIR)/$(LIB).build/$(CONFIGURATION)/lib$(LIB)_osx.a
SANDBOX := sandbox

all: $(IOS) $(OSX) $(SANDBOX)

$(SANDBOX): $(SRCDIR)/sandbox_ios/**/**/* $(SRCDIR)/sandbox_osx/**/**/*
	$(XCODEBUILD) -project $(PROJDIR)/sandbox.xcodeproj -alltargets -configuration $(CONFIGURATION)

$(IOS): $(SOURCES)
	$(XCODEBUILD) -project $(PROJDIR)/$(LIB).xcodeproj -target $(LIB)_ios -configuration $(CONFIGURATION)

$(OSX): $(SOURCES)
	$(XCODEBUILD) -project $(PROJDIR)/$(LIB).xcodeproj -target $(LIB)_osx -configuration $(CONFIGURATION)

$(EXTERNALDIR)/tinycthread:
	./clone_externals.sh

$(EXTERNALDIR)/jansson:
	./clone_externals.sh

doc: doxyfile_private doxyfile_public
	doxygen doxyfile_private
	doxygen doxyfile_public

clean:
	rm -rf $(BUILDDIR)

.PHONY : all clean doc
