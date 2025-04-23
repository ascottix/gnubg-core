#!/bin/bash
set -e

BUILD_BRANCH=build
BUILD_DIR=../build-tmp

echo "🔁 Cleaning old worktree (if any)..."
rm -rf "$BUILD_DIR"
git worktree prune

echo "🔁 Recreating worktree..."
git fetch origin "$BUILD_BRANCH" || git branch "$BUILD_BRANCH" origin/"$BUILD_BRANCH"
git worktree add "$BUILD_DIR" "$BUILD_BRANCH"

echo "🔨 Building..."
# make -f Makefile.emcc clean
make -f Makefile.emcc

echo "🧹 Cleaning build dir (excluding .git)..."
find "$BUILD_DIR" -mindepth 1 -maxdepth 1 ! -name '.git' -exec rm -rf {} +

echo "📁 Copying dist/ to build dir..."
find dist -type f -exec touch {} +
cp -r dist/* "$BUILD_DIR/"

#cd "$BUILD_DIR"
#echo "✅ Preparing commit..."
#git add -A
#git commit -m "Build $(date +%F_%T)" || echo "ℹ️ Nothing to commit"
#git push origin "$BUILD_BRANCH"
#cd -
cd "$BUILD_DIR"

echo "🗑️  Forcing full refresh commit (delete all files)..."
git rm -rf . > /dev/null 2>&1 || true
git commit -m "Wipe before rebuild" || echo "ℹ️ Nothing to commit (already clean)"

echo "📁 Re-copying dist/"
cp -r ../gnubg-core/dist/* .  # cambia se il path è diverso

git add -A
git commit -m "Build $(date +%F_%T)" || echo "ℹ️ Nothing to commit"
git push origin "$BUILD_BRANCH"

cd -

echo "🧼 Cleaning up worktree..."
git worktree remove "$BUILD_DIR" --force

echo "🎉 Done! Build branch updated."
