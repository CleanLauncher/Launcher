#!/bin/bash

LAUNCHER_APPID="org.prismlauncher.PrismLauncher"

svg2png() {
    input_file="$1"
    output_file="$2"
    width="$3"
    height="$4"

    inkscape -w "$width" -h "$height" -o "$output_file" "$input_file"
}

if command -v "inkscape" && command -v "icotool" && command -v "oxipng"; then
    # Windows ICO
    temp_directory=$(mktemp -d)

    svg2png ${LAUNCHER_APPID}.svg "$temp_directory/prismlauncher_16.png" 16 16
    svg2png ${LAUNCHER_APPID}.svg "$temp_directory/prismlauncher_24.png" 24 24
    svg2png ${LAUNCHER_APPID}.svg "$temp_directory/prismlauncher_32.png" 32 32
    svg2png ${LAUNCHER_APPID}.svg "$temp_directory/prismlauncher_48.png" 48 48
    svg2png ${LAUNCHER_APPID}.svg "$temp_directory/prismlauncher_64.png" 64 64
    svg2png ${LAUNCHER_APPID}.svg "$temp_directory/prismlauncher_128.png" 128 128
    svg2png ${LAUNCHER_APPID}.svg "$temp_directory/prismlauncher_256.png" 256 256

    oxipng --opt max --strip all --alpha --interlace 0 "$temp_directory/prismlauncher_"*".png"

    rm prismlauncher.ico && icotool -o prismlauncher.ico -c \
        "$temp_directory/prismlauncher_256.png"  \
        "$temp_directory/prismlauncher_128.png"  \
        "$temp_directory/prismlauncher_64.png"   \
        "$temp_directory/prismlauncher_48.png"   \
        "$temp_directory/prismlauncher_32.png"   \
        "$temp_directory/prismlauncher_24.png"   \
        "$temp_directory/prismlauncher_16.png"
else
    echo "ERROR: Windows icons were NOT generated!" >&2
    echo "ERROR: requires inkscape, icotool and oxipng in PATH"
fi

if command -v "inkscape" && command -v "iconutil" && command -v "oxipng"; then
    # macOS ICNS
    temp_directory=$(mktemp -d)

    iconset_directory="$temp_directory/prismlauncher.iconset"

    mkdir -p "$iconset_directory"

    svg2png ${LAUNCHER_APPID}.bigsur.svg "$iconset_directory/icon_16x16.png" 16 16
    svg2png ${LAUNCHER_APPID}.bigsur.svg "$iconset_directory/icon_16x16@2x.png" 32 32
    svg2png ${LAUNCHER_APPID}.bigsur.svg "$iconset_directory/icon_32x32.png" 32 32
    svg2png ${LAUNCHER_APPID}.bigsur.svg "$iconset_directory/icon_32x32@2x.png" 64 64
    svg2png ${LAUNCHER_APPID}.bigsur.svg "$iconset_directory/icon_128x128.png" 128 128
    svg2png ${LAUNCHER_APPID}.bigsur.svg "$iconset_directory/icon_128x128@2x.png" 256 256
    svg2png ${LAUNCHER_APPID}.bigsur.svg "$iconset_directory/icon_256x256.png" 256 256
    svg2png ${LAUNCHER_APPID}.bigsur.svg "$iconset_directory/icon_256x256@2x.png" 512 512
    svg2png ${LAUNCHER_APPID}.bigsur.svg "$iconset_directory/icon_512x512.png" 512 512
    svg2png ${LAUNCHER_APPID}.bigsur.svg "$iconset_directory/icon_512x512@2x.png" 1024 1024

    oxipng --opt max --strip all --alpha --interlace 0 "$iconset_directory/icon_"*".png"

    iconutil -c icns "$iconset_directory"
    cp -v "$iconset_directory/prismlauncher.icns" .
else
    echo "ERROR: macOS icons were NOT generated!" >&2
    echo "ERROR: requires inkscape, iconutil and oxipng in PATH"
fi

# replace icon in themes
cp -v ${LAUNCHER_APPID}.svg "../launcher/resources/multimc/scalable/launcher.svg"
