#!/bin/bash
find . \
     -path ./uvw -prune \
     -o -type f \( -name *.cc -o -name *.hh -o -name *.h \) \
    | sort | xargs -I '{}' \
astyle \
  --indent=spaces=2 \
  --style=gnu \
  --suffix=none \
  --max-code-length=80 \
  --indent-namespaces \
  '{}'
