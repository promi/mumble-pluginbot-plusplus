#!/bin/sh
echo "// Auto-generated source/build information."
echo "#define GIT_DESCRIBE \"`git describe --dirty --always --tags`\""
