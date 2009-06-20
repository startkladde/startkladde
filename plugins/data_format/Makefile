PLUGINS = lsv_albgau

.PHONY: all
all: $(PLUGINS)

.PHONY: lsv_albgau
lsv_albgau:
	cd lsv_albgau && ( make all; cd .. )

.PHONY: clean
clean:
	-for i in $(PLUGINS); do cd $$i && ( make clean; cd ..); done

