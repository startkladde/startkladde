.PHONY: all
all: $(SUBDIRS)

.PHONY: clean
clean:
	-for i in $(SUBDIRS); do cd $$i && ( make clean; cd ..); done

