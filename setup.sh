#!/bin/bash

DIR="Source/util/include"

[ ! -d .git ] && echo "Run from repo root." && exit 1

for f in Config Presets; do
    if [ ! -f "$DIR/$f.user.h" ]; then
        cp "$DIR/$f.h" "$DIR/$f.user.h"
        echo "Created $f.user.h"
    else
        echo "$f.user.h exists"
    fi
done

echo "Done."

