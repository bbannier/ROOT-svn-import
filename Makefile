# ===============================================================
# Makefile 
# 
# To built 'name' with or without debugging ...
# > gmake 'name' or  > gmake 'name' DEB=-g
#
# ================================================================

# Run make DEB=-g to build in debug mode
DEB       =

# the executable will be in $(HOME)/$(OS)
OS        = .

# the main directory
MAIN      = .

# specific code

# The name of the executable
NAME      = xrdping

# *.F files are stored in SRCDIR
SRCDIR    = $(MAIN)

# Special libs
ULIB     = -lresolv -lm -ldl

# set C flags
CC         = g++
CCFLAGS    = $(DEB) -c -pipe -Wall -W -Woverloaded-virtual -fPIC

# vpath %.cc $(SRCDIR)

# *.o files are stored in $(OBJDIR)

OBJDIR  = .
vpath %.o $(OBJDIR)

O_FILES  := $(CCFILES:%.cc=%.o)
OBJFILES := $(addprefix $(OBJDIR)/,$(notdir $(O_FILES)))

# Rules...

all: $(NAME)
	@ echo "Making all ..."

xrdping: $(MAIN)/xrdping.o
	@ echo "Linking ... $@"
	$(CC) $(DEB) -o $@  $(MAIN)/xrdping.o $(ULIB)

$(OBJDIR)/%.o: %.cc
	@echo "CCFLAGS ... $(CCFLAGS)"
	$(CC) $(DEB) $(CCFLAGS) $< -o $@

clean:
	rm -f $(MAIN)/*.o $(OBJDIR)/xrdping
