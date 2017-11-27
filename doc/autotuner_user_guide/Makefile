.PHONY: pdf dvi clean

MAINTEXFILE?=main


pdf: $(MAINTEXFILE).tex
	@latexmk -pdf $(MAINTEXFILE)
	@cp $(MAINTEXFILE).pdf margot_user_guide.pdf
	@pdffonts $(MAINTEXFILE).pdf

dvi: $(MAINTEXFILE).tex
	@latexmk -dvi $(MAINTEXFILE)

clean:
	@latexmk -C 2> /dev/null
