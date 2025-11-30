#!/bin/bash

DIR="Source/util/include"

[ ! -d .git ] && echo "Run from repo root." && exit 1

git rm --cached "$DIR/Config.h" 2>/dev/null
git rm --cached "$DIR/Presets.h" 2>/dev/null

echo "Untracking done."
