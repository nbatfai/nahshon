all:	clean validate_xmllint_samu samu-SRS-hu.pdf 
epub:	validate_xmllint_samu samu-SRS-hu.epub

samu-SRS-hu.pdf: samu-SRS-hu.xml samu-hu.xls
	dblatex samu-SRS-hu.xml -p samu-hu.xls

samu-SRS-hu.epub: samu-SRS-hu.xml
	dbtoepub -o samu-SRS-hu.epub samu-SRS-hu.xml

.PHONY: validate_jing_samu
validate_jing_samu:
	@if [ ! -e jing.jar ]; then \
		wget https://jing-trang.googlecode.com/files/jing-20091111.zip; \
		unzip jing-20091111.zip > /dev/null; \
		mv jing-20091111/bin/jing.jar jing.jar; \
		rm -rf jing-20091111*; \
	fi

	xmllint --xinclude samu-SRS-hu.xml --output output.xml
	java -jar jing.jar -c http://docbook.org/xml/5.1CR4/rng/docbookxi.rnc output.xml
	@rm -f output.xml

.PHONY: validate_xmllint_samu
validate_xmllint_samu:
	xmllint --xinclude samu-SRS-hu.xml --output output.xml
	xmllint --relaxng http://docbook.org/xml/5.1CR4/rng/docbookxi.rng output.xml --noout
	rm -f output.xml

.PHONY: clean
clean:
	rm -f samu-SRS-hu.pdf samu-SRS-hu.epub
	rm -f *~

