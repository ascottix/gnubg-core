#!/bin/bash

echo "🔨 Building..."
make -f Makefile.emcc clean
make -f Makefile.emcc

cp web/gnubg-core.js dist/

echo "🎉 Done!"

