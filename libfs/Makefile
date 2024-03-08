# Target library
lib := libfs.a

all: $(lib)

ifneq ($(V),1)
Q=@
endif


objects := fs.o disk.o

CC := gcc
CFlags := -Wall -Werror -Wextra -MMD


#dependency tracking
deps := $(patsubst %.o, %.d, $(objects))
-include $(deps)


libfs.a: $(objects)
	@echo "CC $@"
	$(Q)ar rcs $@ $^

%.o: %.c
	@echo "CC $@"
	$(Q)$(CC) $(CFlags) -O -c $@ $<

clean: 
	$(Q)rm -f $(lib) $(objects) $(deps)

