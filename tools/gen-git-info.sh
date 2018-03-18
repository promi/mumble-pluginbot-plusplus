#!/bin/sh
echo "Input = $1"
echo "// Auto-generated source/build information." > "$1.hh"
echo "#define GIT_DESCRIBE \"`git describe --dirty --always --tags`\"" >> "$1.hh"
