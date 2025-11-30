#!/bin/bash

DIR="Source/util/include"

[ ! -d .git ] && echo "Run from repo root." && exit 1

[ ! -f "$DIR/ConfigUser.h" ] && cp "$DIR/Config.h" "$DIR/ConfigUser.h" && echo "Created ConfigUser.h"
[ ! -f "$DIR/PresetsUser.h" ] && cp "$DIR/Presets.h" "$DIR/PresetsUser.h" && echo "Created PresetsUser.h"

echo "Done."


