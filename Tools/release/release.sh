#!/bin/zsh
# Builds, signs, notarizes and publishes a KuocKey release.
#   Tools/release/release.sh 1.0.1
# Steps: bump version → engine rule lock → universal Release build → Developer ID sign → notarize app
#        → DMG (signed, notarized, stapled) → GitHub Release → deploy site (changelog only).
# Downloads live only on GitHub Releases: no update feed, no release.json, nothing the app fetches.
set -e
if [ "$1" = "--windows" ]; then exec "$(dirname "$0")/release_windows.sh" "${@:2}"; fi   # Windows lane, after the Mac release
if [ "$1" = "--linux" ]; then exec "$(dirname "$0")/release_linux.sh" "${@:2}"; fi         # Linux lane
if [ "$1" = "--android" ]; then exec "$(dirname "$0")/release_android.sh" "${@:2}"; fi   # Android lane
VER=${1:?usage: release.sh <version> | release.sh --windows <version>}
cd "$(dirname "$0")/../.."
ROOT=$PWD
KC=~/Library/Keychains/s10w-signing.keychain-db
KCP=$(grep "^KC_PASS=" ~/.secrets/s10w_signing | cut -d= -f2-)
IDENT="Developer ID Application: Typing Technology Company Limited (3H6967F79Y)"
NOTARY=(--key ~/.secrets/AuthKey_3QNAF6H8AT.p8 --key-id 3QNAF6H8AT --issuer 6587ae6d-4945-468e-8879-d58b7a6d33ad)

echo "==> version $VER"
# the site is in Vietnamese and English (D31): both changelogs need this release
for f in site/pages/changelog.html site/pages_en/changelog.html; do
  grep -q "<!-- release $VER -->" $f || { echo "missing <!-- release $VER --> in $f"; exit 1; }
done
BUILD=$(( $(grep 'CURRENT_PROJECT_VERSION' project.yml | grep -o '[0-9]\+') + 1 ))
sed -i '' "s/MARKETING_VERSION: \".*\"/MARKETING_VERSION: \"$VER\"/; s/CURRENT_PROJECT_VERSION: \".*\"/CURRENT_PROJECT_VERSION: \"$BUILD\"/" project.yml
xcodegen generate >/dev/null

echo "==> engine rule lock"
Tools/autoswitch/check.sh >/dev/null || { echo "check.sh failed: engine behavior changed"; exit 1; }

echo "==> build (universal)"
rm -rf build/dd dist && mkdir -p dist
xcodebuild -project KuocKey.xcodeproj -scheme KuocKey -configuration Release -derivedDataPath build/dd \
  ARCHS="arm64 x86_64" ONLY_ACTIVE_ARCH=NO CODE_SIGN_IDENTITY=- CODE_SIGNING_REQUIRED=NO build | grep -E "error:|BUILD" || true
APP=build/dd/Build/Products/Release/KuocKey.app
[ "$(defaults read "$ROOT/$APP/Contents/Info.plist" CFBundleShortVersionString)" = "$VER" ] || { echo "built version mismatch"; exit 1; }
lipo -archs "$APP/Contents/MacOS/KuocKey"
cp -R "$APP" dist/

echo "==> sign + notarize app"
security unlock-keychain -p "$KCP" "$KC"
codesign --force --options runtime --timestamp --entitlements App/KuocKey.entitlements -s "$IDENT" dist/KuocKey.app
codesign --verify --deep --strict dist/KuocKey.app
ditto -c -k --keepParent dist/KuocKey.app dist/KuocKey.zip
xcrun notarytool submit dist/KuocKey.zip "${NOTARY[@]}" --wait | tail -1
xcrun stapler staple dist/KuocKey.app | tail -1
rm dist/KuocKey.zip

echo "==> DMG"
DMG=dist/KuocKey-$VER.dmg
python3 Tools/release/dmg_background.py dist/dmg_bg.png
create-dmg --volname "KuocKey" --volicon design/KuocKey.icns \
  --background dist/dmg_bg.png --window-pos 200 120 --window-size 660 420 \
  --icon-size 128 --icon "KuocKey.app" 170 190 --app-drop-link 490 190 \
  --hide-extension "KuocKey.app" --no-internet-enable \
  --codesign "$IDENT" "$DMG" dist/KuocKey.app >/dev/null || true
[ -f "$DMG" ] || { echo "dmg not created"; exit 1; }
xcrun notarytool submit "$DMG" "${NOTARY[@]}" --wait | tail -1
xcrun stapler staple "$DMG" | tail -1
spctl -a -vv -t open --context context:primary-signature "$DMG" 2>&1 | tail -1

echo "==> GitHub release"
git add project.yml && git commit -q -m "Release $VER" && git push -q origin main
gh release create "v$VER" "$DMG" -R ngminhkuoc/KuocKey --title "KuocKey $VER" --notes-file <(sed -n "/<!-- release $VER -->/,/<!-- \/release -->/p" site/pages/changelog.html | sed 's/<li>/- /; s/<[^>]*>//g' | sed '/^\s*$/d') 2>/dev/null \
  || gh release create "v$VER" "$DMG" -R ngminhkuoc/KuocKey --title "KuocKey $VER" --notes "KuocKey $VER"
echo "==> done: https://github.com/ngminhkuoc/KuocKey/releases/tag/v$VER"
