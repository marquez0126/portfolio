> file_heads
for file in *.html; do
echo "檔名:$file" >> file_heads
head -n 2 "$file" >> file_heads
done
