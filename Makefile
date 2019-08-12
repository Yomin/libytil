
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
INCLUDES := $(INCLUDES) -I.
CFLAGS   := $(CFLAGS) -Wall -Wextra -Wpedantic -Woverflow -Wno-unused-parameter
CFLAGS   := $(CFLAGS) -Werror -Wfatal-errors -std=gnu11 $(INCLUDES)
DFLAGS   := $(CFLAGS) $(DFLAGS) -ggdb3 -O0
CFLAGS   := $(CFLAGS) -DNDEBUG -DNVALGRIND -O2
SOURCES  := $(shell find $(NAME) -type f -name "*.c")


.PHONY: all
all: debug

.PHONY: clean
clean:
	rm -rf debug prod


.PHONY: prod
prod: prod/$(LIBNAME)

-include $(patsubst $(NAME)/%.c, prod/%.d, $(SOURCES))

prod/%.o: $(NAME)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ $<
	@gcc -MM $(INCLUDES) $< | sed -e 's|[^:]*:|$@:|' > $(@:%.o=%.d)
	@sed -e 's/.*: *//; s/\\$$//g; s/ /:\n/g; s/$$/:/' < $(@:%.o=%.d) >> $(@:%.o=%.d)

prod/$(LIBNAME): $(patsubst $(NAME)/%.c, prod/%.o, $(SOURCES))
	$(AR) rcs $@ $^


.PHONY: debug
debug: debug/$(LIBNAME)

-include $(patsubst $(NAME)/%.c, debug/%.d, $(SOURCES))

debug/%.o: $(NAME)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(DFLAGS) -o $@ $<
	@gcc -MM $(INCLUDES) $< | sed -e 's|[^:]*:|$@:|' > $(@:%.o=%.d)
	@sed -e 's/.*: *//; s/\\$$//g; s/ /:\n/g; s/$$/:/' < $(@:%.o=%.d) >> $(@:%.o=%.d)

debug/$(LIBNAME): $(patsubst $(NAME)/%.c, debug/%.o, $(SOURCES))
	$(AR) rcs $@ $^
