for file in *.htm; do
mv "$file" "${file%.htm}.html"
echo "rename: $file -> ${file%.htm}.html"
done
