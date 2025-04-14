echo "總共$#個檔案"
while [ $# -gt 0 ]
do
cp $1 $1.bak
echo "已建立$1.bak"
shift
done
