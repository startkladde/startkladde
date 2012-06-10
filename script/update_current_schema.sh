#!/bin/bash

# Use a temporary file in build/, so if the dumping fails, we don't overwrite
# the schema

# Call from the build directory
if [ ! -e ./startkladde ]; then
	echo "Call from the build directory and build the program first"
	exit 1
fi

make || (echo "Error during make"; exit 1)

./startkladde db:ensure_empty                       || (echo "Error: database is not empty - use ./startkladde db:clear"; false)
./startkladde_debug db:migrate                      || (echo "Error during migration"; false)
./startkladde_debug db:dump current_schema.yaml.new || (echo "Error during dump"; false)

echo "Now move current_schema.yaml.new to src/db/migrations/current_schema.yaml"

