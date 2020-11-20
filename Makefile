
verbose ?= 0

CC := gcc
LD := gcc
AR := ar

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

ifeq ($(VCC), )
    $(error "invalid verbose level")
endif

NAME     := ytil
LIBNAME  := lib$(NAME).a

FEATURES := -std=gnu11 -march=native -D_GNU_SOURCE
WARNINGS := -Wall -Wpedantic -Wextra -Wno-unused-parameter -Wimplicit-fallthrough=5
WARNINGS += -Werror -Wfatal-errors
INCLUDES := -Iinclude -Isrc
LIBFLAGS :=
LIBS     := -l$(NAME)

ifeq ($(OS), Windows_NT)
    FEATURES += -D__USE_MINGW_ANSI_STDIO=1
    LIBS     += -lole32
endif

CFLAGS   := $(FEATURES) $(WARNINGS) $(INCLUDES) $(CFLAGS)
DFLAGS   := -Og -ggdb3 -D_FORTIFY_SOURCE=1 $(DFLAGS)
PFLAGS   := -O2 -DNDEBUG -DNVALGRIND $(PFLAGS)
LDFLAGS  := $(LIBFLAGS) $(LDFLAGS)
LDLIBS   := $(LIBS) $(LDLIBS)

SOURCES  := $(shell find src -type f -name "*.c")
TSOURCES := $(shell find tests -type f -name "*.c")


.PHONY: all
all: debug

.PHONY: clean
clean:
	rm -rf debug prod test doc

.PHONY: doc
doc: Doxyfile
	@doxygen $<

.PHONY: devdoc
devdoc: Doxyfile_dev
	@doxygen $<


.PHONY: prod
prod: prod/$(LIBNAME)

-include $(patsubst src/%.c, prod/%.d, $(SOURCES))

prod/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(VCC) $(CFLAGS) $(PFLAGS) -o $@ $<
	@$(CC) -MM -MP -MT $@ -MF $(@:%.o=%.d) $(INCLUDES) $<

prod/$(LIBNAME): $(patsubst src/%.c, prod/%.o, $(SOURCES))
	$(VAR) $@ $^


.PHONY: debug
debug: debug/$(LIBNAME)

-include $(patsubst src/%.c, debug/%.d, $(SOURCES))

debug/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(VCC) $(CFLAGS) $(DFLAGS) -o $@ $<
	@$(CC) -MM -MP -MT $@ -MF $(@:%.o=%.d) $(INCLUDES) $<

debug/$(LIBNAME): $(patsubst src/%.c, debug/%.o, $(SOURCES))
	$(VAR) $@ $^


.PHONY: test
test: test/$(NAME)

-include $(patsubst tests/%.c, test/%.d, $(TSOURCES))

test/%.o: tests/%.c
	@mkdir -p $(dir $@)
	$(VCC) $(CFLAGS) $(DFLAGS) -o $@ $<
	@$(CC) -MM -MP -MT $@ -MF $(@:%.o=%.d) $(INCLUDES) $<

test/$(NAME): debug/$(LIBNAME) $(patsubst tests/%.c, test/%.o, $(TSOURCES))
	$(VLD) $(LDFLAGS) -o $@ $^ -Ldebug $(LDLIBS)
