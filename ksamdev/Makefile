
# The main directories
MAIN      = $(KSDIR)
KSSRC     = $(MAIN)/src
KSBIN     = $(MAIN)/bin

# The name of the executable
NAME      = releaseFileCache

# List of source files
SRCFILES  = $(KSSRC)/$(NAME).cxx

# The compiler
CXX       = g++

# Rules...
$(KSBIN)/$(NAME): $(SRCFILES) 
	$(CXX) -o $@  $(SRCFILES)

clean:
	rm -fr $(KSBIN)/$(NAME)


