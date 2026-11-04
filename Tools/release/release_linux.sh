#!/bin/zsh
# Publishes the Linux packages of a release the Mac lane already made:
#   Tools/release/release.sh --linux 1.0.14
# The tag v<version> (or a manual run of the linux workflow with release_tag) puts kuockey_<ver>_amd64.deb,
# kuockey_<ver>_arm64.deb, the .rpm files and the Arch .pkg.tar.zst files on the GitHub release. This script
# waits for them and checks they are all there. The GitHub release is the only place they are published.
set -e
VER=${1:?usage: release.sh --linux <version>}
cd "$(dirname "$0")/../.."
# the version must be the one main is at: the packages on the release were built from it
PV=$(grep MARKETING_VERSION project.yml | grep -o '"[^"]*"' | tr -d '"'); [ "$PV" = "$VER" ] || { echo "project.yml is at $PV, not $VER"; exit 1; }
REPO=ngminhkuoc/KuocKey
echo "==> waiting for the Linux packages on v$VER"
for i in $(seq 1 60); do
  n=$(gh release view "v$VER" -R $REPO --json assets -q '[.assets[].name | select(endswith(".deb") or endswith(".rpm") or endswith(".pkg.tar.zst"))] | length' 2>/dev/null || echo 0)
  [ "$n" -ge 6 ] && break; sleep 30
done
[ "$n" -ge 6 ] || { echo "packages not on the release after 30 min: gh run list -R $REPO -w linux"; exit 1; }
rm -rf dist/linux && mkdir -p dist/linux
gh release download "v$VER" -R $REPO -p "*.deb" -p "*.rpm" -p "*.pkg.tar.zst" -D dist/linux --clobber
ls -l dist/linux
echo "==> done: https://github.com/$REPO/releases/tag/v$VER"
