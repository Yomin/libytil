
verbose ?= 0

CC := gcc
LD := gcc
AR := ar
CF := build/util/config

VCC0 = @echo [cc] $@; $(CC) -c
VCC1 = $(CC) -c
VCC2 = $(CC) -c
VCC  = $(VCC$(verbose))
VLD0 = @echo [ld] $@; $(LD)
VLD1 = $(LD)
VLD2 = $(LD) -Wl,--trace
VLD  = $(VLD$(verbose))
VAR0 = @echo [ar] $@; $(AR) rcs
VAR1 = $(AR) rcs
VAR2 = $(AR) rcsv
VAR  = $(VAR$(verbose))
VCF0 = @echo [cf] $(if $(filter config,$@),$(config:src/%=config/$(NAME)/%.h),$@); $(CF)
VCF1 = $(CF)
VCF2 = $(CF) -v
VCF  = $(VCF$(verbose))

ifeq ($(VCC),)
    $(error "invalid verbose level")
endif

NAME     := ytil
LIBNAME  := lib$(NAME).a

FLAGS    := -std=gnu11 -march=native
PPFLAGS  := -D_GNU_SOURCE
WARNINGS := -Wall -Wpedantic -Wextra -Wno-unused-parameter -Wimplicit-fallthrough=5
WARNINGS += -Werror -Wfatal-errors
INCLUDES := -Iinclude -Iconfig -Isrc
LIBFLAGS :=
LIBS     := -l$(NAME)

ifeq ($(OS),Windows_NT)
    PPFLAGS += -D__USE_MINGW_ANSI_STDIO=1
    LIBS    += -lole32 -lssp -lshlwapi -lws2_32
endif

CFLAGS   := $(FLAGS) $(WARNINGS) $(CFLAGS)
DFLAGS   := -Og -ggdb3 $(DFLAGS)
RFLAGS   := -O2 $(RFLAGS)
CPPFLAGS := $(PPFLAGS) $(INCLUDES) $(CPPFLAGS)
DPPFLAGS := -D_FORTIFY_SOURCE=1 $(DPPFLAGS)
RPPFLAGS := -DNDEBUG=1 -DNVALGRIND=1 $(RPPFLAGS)
LDFLAGS  := $(LIBFLAGS) $(LDFLAGS)
LDLIBS   := $(LIBS) $(LDLIBS)

SOURCES  := $(shell find src -type f -name "*.c")
CONFIGS  := $(shell find src -type f -name "*.cfg")
TSOURCES := $(shell find test -type f -name "*.c")

MAKEFLAGS += --no-builtin-rules --no-builtin-variables

.SUFFIXES:
.SECONDARY:


.PHONY: all
all: debug

.PHONY: clean
clean:
	rm -rf build doc

.PHONY: distclean
distclean: clean
	rm -rf config

.PHONY: doc
doc: Doxyfile
	@doxygen $<

.PHONY: devdoc
devdoc: Doxyfile_dev
	@doxygen $<


.PHONY: config
config: build/util/config
	@$(foreach config,$(CONFIGS),$< -i \
		$(config) \
		$(patsubst src/%,config/$(NAME)/%,$(config)) \
		$(patsubst src/%,config/$(NAME)/%.h,$(config));)

build/util/config: util/config.c
	@mkdir -p $(dir $@)
	$(VLD) $(CFLAGS) -o $@ $<

config/$(NAME)/%.cfg.h: src/%.cfg build/util/config
	$(VCF) $< $(@:%.cfg.h=%.cfg) $@


.PHONY: debug
debug: CFLAGS   += $(DFLAGS)
debug: CPPFLAGS += $(DPPFLAGS)
debug: build/debug/$(LIBNAME)

.PHONY: release
release: CFLAGS   += $(RFLAGS)
release: CPPFLAGS += $(RPPFLAGS)
release: build/release/$(LIBNAME)

.PHONY: test
test: CFLAGS   += $(DFLAGS)
test: CPPFLAGS += $(DPPFLAGS)
test: build/test/$(NAME)

-include $(patsubst src/%.c,build/debug/%.d,$(SOURCES))
-include $(patsubst src/%.c,build/release/%.d,$(SOURCES))
-include $(patsubst test/%.c,build/test/%.d,$(TSOURCES))

build/debug/%.o build/release/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(VCC) $(CFLAGS) $(CPPFLAGS) -o $@ $<
	@$(CC) $(CPPFLAGS) -MM -MP -MT $@ -MF $(@:%.o=%.d) $<

build/test/%.o: test/%.c
	@mkdir -p $(dir $@)
	$(VCC) $(CFLAGS) $(CPPFLAGS) -o $@ $<
	@$(CC) $(CPPFLAGS) -MM -MP -MT $@ -MF $(@:%.o=%.d) $<

%/$(LIBNAME): $(addprefix %/,$(patsubst src/%.c,%.o,$(SOURCES)))
	$(VAR) $@ $^

build/test/$(NAME): build/debug/$(LIBNAME) $(patsubst test/%.c,build/test/%.o,$(TSOURCES))
	$(VLD) $(LDFLAGS) -o $@ $^ -Lbuild/debug $(LDLIBS)
