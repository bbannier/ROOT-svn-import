
GUIDE	:= RooStats_UsersGuide
FIGURES	:= $(filter-out $(GUIDE).pdf, $(wildcard *.pdf))

SOURCES 	= $(wildcard *.C)
DOTOBJECTS	= $(SOURCES:.C=.dot)

GENDOT	= $(wildcard $(GUIDE)-dot2tex-*.dot)
GENTEX	= $(wildcard $(GUIDE)-dot2tex-*.tex)

$(GUIDE).pdf: $(GUIDE).tex $(SOURCES) $(FIGURES) $(DOTOBJECTS)
	latex --shell-escape $<
	latex $<

%.dot: %.C
	root.exe -b -q -l $< 

clean:
	rm -f $(GUIDE).aux $(GUIDE).log $(GUIDE).out $(GUIDE).toc $(GENDOT) $(GENTEX)

