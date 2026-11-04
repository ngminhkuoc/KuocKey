#!/bin/zsh
# Publishes the Windows build of a release that the Mac lane (release.sh <version>) already made:
#   Tools/release/release.sh --windows 1.0.14
# The tag v<version> triggers .github/workflows/windows.yml, whose "package" job uploads the installers
# (KuocKey-<ver>-x64-Setup.exe, KuocKey-<ver>-arm64-Setup.exe) and portable zips to the GitHub release.
# This script waits for them and checks that the installers are Authenticode-signed. The GitHub release is
# the only place they are published: there is no update feed and the site hosts no binaries.
set -e
VER=${1:?usage: release.sh --windows <version>}
cd "$(dirname "$0")/../.."
# the version must be the one main is at: the packages on the release were built from it
PV=$(grep MARKETING_VERSION project.yml | grep -o '"[^"]*"' | tr -d '"'); [ "$PV" = "$VER" ] || { echo "project.yml is at $PV, not $VER"; exit 1; }
REPO=ngminhkuoc/KuocKey
echo "==> waiting for the CI package job on v$VER"
for i in $(seq 1 60); do
  n=$(gh release view "v$VER" -R $REPO --json assets -q '[.assets[].name | select(endswith("-Setup.exe"))] | length' 2>/dev/null || echo 0)
  [ "$n" = "2" ] && break; sleep 30
done
[ "$n" = "2" ] || { echo "installers not on the release after 30 min: gh run list -R $REPO -w windows"; exit 1; }
rm -rf dist/windows && mkdir -p dist/windows
gh release download "v$VER" -R $REPO -p "KuocKey-$VER-*" -D dist/windows --clobber
for ARCH in x64 arm64; do
  F=dist/windows/KuocKey-$VER-$ARCH-Setup.exe
  [ -f "$F" ] || { echo "missing $F"; exit 1; }
  # CI signs with the Certum certificate (secrets CERTUM_EMAIL/CERTUM_OTP); never publish an unsigned installer
  osslsigncode verify "$F" >/dev/null 2>&1 || { echo "$F is not Authenticode-signed (CI secrets missing?)"; [ "$ALLOW_UNSIGNED" = 1 ] || exit 1; }
  [ -f "dist/windows/KuocKey-$VER-$ARCH-portable.zip" ] || { echo "missing KuocKey-$VER-$ARCH-portable.zip on the release"; exit 1; }
done
echo "==> done: https://github.com/$REPO/releases/tag/v$VER"
