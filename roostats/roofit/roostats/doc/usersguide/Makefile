HEADER=header
PREFOOTER=prefooter
FOOTER=footer
CAT=cat
DATE=date
#SOURCES = CERNtips.htm atlas.htm group.htm milagro.htm HEPexp.content.htm contact.htm index.htm
SOURCES = $(wildcard *.C )
DOTOBJECTS=$(SOURCES:.C=.dot)
GENFIGURES=$(SOURCES:.C=.pdf)
FIGURES=$(wildcard *.pdf)
GUIDE=RooStats_UsersGuilde.pdf
GENDOT=$(wildcard RooStats_UsersGuide-dot2tex-*.dot)
GENTEX=$(wildcard RooStats_UsersGuide-dot2tex-*.tex)
.SUFFIXES : .C .dot .pdf

all:  $(GENFIGURES)
	latex RooStats_UsersGuide.tex
	latex RooStats_UsersGuide.tex


.C.pdf:
	root.exe -b -q $< 
clean:
	rm RooStats_UsersGuide.aux RooStats_UsersGuide.log RooStats_UsersGuide.out RooStats_UsersGuide.toc $(GENDOT) $(GENTEX)
