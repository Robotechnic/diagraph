#!/usr/bin/sh

echo "Generating images..."
awk -f ./generate_images.awk README.md > .README.temp
mv .README.temp README.md
rm images/*2.png