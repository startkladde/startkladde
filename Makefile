# The debug version is called startkladde_debug. The release version is called
# startkladde, not startkladde_release, due to qmake stinkage (see
# startkladde.pro).

# Note: the program (startkladde) is built by another Makefile which is
# generated by qmake. Any dependency of startkladde which is built by this
# Makefile has to be stated explicitly, even though it is also present in the
# generated Makefile. These dependencies are in the startkladde_autogenerated
# target.
# For example: startkladde depends on CurrentSchema.cpp which is autogenerated
# from current_schema.yaml by this Makefile.
# If current_schema.yaml is changed, CurrentSchema.cpp has to be rebuilt. This
# only happens if CurrentSchema.cpp is a prerequisite of startkladde in *this*
# Makefile. Also, the generated Makefile would not know how to build
# CurrentSchema.cpp

##############
## Settings ##
##############

# Set to qmake-qt4 if the default qmake is qmake-qt3 on this system
QMAKE = qmake -Wall


####################
## Common targets ##
####################

.PHONY: default all startkladde
default: startkladde

startkladde: release

startkladde_debug: debug

all: startkladde plugins


#########################
## Generic build rules ##
#########################

# A Makefile ist created from the corresponding project files by invoking qmake
# This will also generate the .Debug and .Release Makefiles if required by the
# .pro file.
Makefile_%: %.pro
	$(QMAKE) $<

# A .Debug or .Release Makefile is generated with the corresponding Makefile.
# Note that this rule requires a command (even if it is empty), or it will not
# work (i. e. the File will not be reubuilt if it is out of date.
Makefile_%.Debug Makefile_%.Release: Makefile_%
	#


########################
## Individual targets ##
########################

# The startkladde Makefiles depend on the migration list
Makefile_startkladde: build/migrations.pro

# The migrations list depends on the migrations directory
build/migrations.pro: migrations.pro.erb src/db/migrations
	mkdir -p build
	erb $< >$@ || rm $@


#########################
## Autogenerated files ##
#########################

# See the comment at the beginning of this file
startkladde_autogenerated: build/migrations.h build/migrations_headers.h build/CurrentSchema.cpp version/version.h build/migrations.pro

build/migrations.h build/migrations_headers.h: build/%.h: src/db/migration/%.h.erb src/db/migrations/*.h src/db/migrations
	mkdir -p build
	./script/build/generate_migration_headers.rb

build/CurrentSchema.cpp: src/db/schema/CurrentSchema.cpp.erb src/db/migrations/current_schema.yaml
	mkdir -p build
	erb -T 1 src/db/schema/CurrentSchema.cpp.erb >$@ || rm $@

version/version.h: version/version version/make_version
	cd version; ./make_version; cd ..



####################
## Subdirectories ##
####################

.PHONY: plugins
plugins:
	$(MAKE) -C $@


#############
## Cleanup ##
#############

.PHONY: clean
clean: Makefile_startkladde
	$(MAKE) -f Makefile_startkladde distclean
	$(MAKE) -C plugins clean
	rm -f Makefile_startkladde Makefile_startkladde.Debug Makefile_startkladde.Release
	rm -f version/version.h
	rm -rf build debug release
	rm -f object_script.startkladde.Debug object_script.startkladde.Release # For Windows
	rm -f startkladde startkladde_release startkladde_debug


##########
## Misc ##
##########

.PHONY: run
run: startkladde
	./startkladde -q --no-full-screen

# Use a temporary file in build/, so if the dumping fails, we don't overwrite
# the schema
.PHONY: update_current_schema
update_current_schema: startkladde
	mkdir -p build
	./startkladde db:ensure_empty || (echo "Error: database is not empty - use ./startkladde db:clear"; false)
	./startkladde db:migrate
	./startkladde db:dump build/current_schema.yaml
	mv build/current_schema.yaml src/db/migrations/current_schema.yaml
	echo "Schema definition updated successfully"

# Forward to Makefile_startkladde
.PHONY: release debug install uninstall
release debug install uninstall debug-% release-%: Makefile_startkladde startkladde_autogenerated
	$(MAKE) -f $< $@

