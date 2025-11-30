#!/bin/bash

DIR="Source/util/include"

[ ! -d .git ] && echo "Run from repo root." && exit 1

for f in Config Presets; do
    if [ ! -f "$DIR/$f.h" ]; then
        cp "$DIR/$f.default.h" "$DIR/$f.h"
        echo "Created $f.h"
    else
        echo "$f.h exists"
    fi
done

echo "Done."

