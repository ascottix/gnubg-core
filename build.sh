#!/bin/bash

set -e

BUILD_BRANCH=build
BUILD_DIR=../build-tmp

# echo "➡️ Building..."
make -f Makefile.emcc clean
make -f Makefile.emcc

echo "➡️ Preparing build branch..."
git worktree remove $BUILD_DIR --force || true
git worktree add $BUILD_DIR $BUILD_BRANCH

echo "➡️ Copying dist/ to $BUILD_DIR..."
rm -rf $BUILD_DIR/*
cp -r dist/* $BUILD_DIR/

cd $BUILD_DIR
git add .
git commit -m "Build $(date +%F_%T)" || echo "Nothing to commit"
git push origin $BUILD_BRANCH
cd -

git worktree remove $BUILD_DIR
echo "✅ Build branch updated!"
