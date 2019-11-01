
verbose ?= 0

CC_0 = @echo [cc] $(DIR)$@; gcc -c
CC_1 = gcc -c
CC   = $(CC_$(verbose))
LD_0 = @echo [ld] $(DIR)$@; gcc
LD_1 = gcc
LD   = $(LD_$(verbose))
AR_0 = @echo [ar] $(DIR)$@; ar
AR_1 = ar
AR   = $(AR_$(verbose))

NAME     := ytil
LIBNAME  := lib$(NAME).a
INCLUDES := $(INCLUDES) -Iinclude
CFLAGS   := $(CFLAGS) -Wall -Wextra -Wpedantic -Woverflow -Wno-unused-parameter
CFLAGS   := $(CFLAGS) -Werror -Wfatal-errors -std=gnu11 -march=native $(INCLUDES)
DFLAGS   := $(CFLAGS) $(DFLAGS) -ggdb3 -O0
CFLAGS   := $(CFLAGS) -DNDEBUG -DNVALGRIND -O2
SOURCES  := $(shell find src -type f -name "*.c")
TESTS    := $(shell find tests -type f -name "*.c")
TESTOBJS := $(patsubst tests/%.c, test/%.o, $(TESTS))

.PHONY: all
all: debug

.PHONY: clean
clean:
	rm -rf debug prod test


.PHONY: prod
prod: prod/$(LIBNAME)

-include $(patsubst src/%.c, prod/%.d, $(SOURCES))

prod/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ $<
	@gcc -MM $(INCLUDES) $< | sed -e 's|[^:]*:|$@:|' > $(@:%.o=%.d)
	@sed -e 's/.*: *//; s/\\$$//g; s/ /:\n/g; s/$$/:/' < $(@:%.o=%.d) >> $(@:%.o=%.d)

prod/$(LIBNAME): $(patsubst src/%.c, prod/%.o, $(SOURCES))
	$(AR) rcs $@ $^


.PHONY: debug
debug: debug/$(LIBNAME)

-include $(patsubst src/%.c, debug/%.d, $(SOURCES))

debug/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(DFLAGS) -o $@ $<
	@gcc -MM $(INCLUDES) $< | sed -e 's|[^:]*:|$@:|' > $(@:%.o=%.d)
	@sed -e 's/.*: *//; s/\\$$//g; s/ /:\n/g; s/$$/:/' < $(@:%.o=%.d) >> $(@:%.o=%.d)

debug/$(LIBNAME): $(patsubst src/%.c, debug/%.o, $(SOURCES))
	$(AR) rcs $@ $^


.PHONY: test
test: test/$(NAME)

-include $(patsubst tests/%.c, test/%.d, $(TESTS))

test/%.o: tests/%.c
	@mkdir -p $(dir $@)
	$(CC) $(DFLAGS) -o $@ $<
	@gcc -MM $(INCLUDES) $< | sed -e 's|[^:]*:|$@:|' > $(@:%.o=%.d)
	@sed -e 's/.*: *//; s/\\$$//g; s/ /:\n/g; s/$$/:/' < $(@:%.o=%.d) >> $(@:%.o=%.d)

test/$(NAME): debug/$(LIBNAME) $(TESTOBJS)
	$(LD) $(DFLAGS) -o $@ $(TESTOBJS) -Ldebug -l$(NAME)
