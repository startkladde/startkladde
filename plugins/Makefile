SUBDIRS = data_format

.PHONY: all
all: $(SUBDIRS)

.PHONY: data_format
data_format:
	cd data_format && ( make all; cd .. )

.PHONY: clean
clean:
	-for i in $(SUBDIRS); do cd $$i && ( make clean; cd ..); done

