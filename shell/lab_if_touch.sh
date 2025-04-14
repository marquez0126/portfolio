filename=logical
file=~/test/${filename}
test ! -d ~/test && echo "未發現test資料夾，建立test資料夾" && mkdir ~/test
test ! -e ${file} && echo "${filename}檔案不存在，建立${filename}檔案
路徑:"${file}"" && touch ${file} && exit 0
test -f ${file} && echo -e "刪除${filename}檔案，建立${filename}資料夾\n路徑:"${file}"" && rm ${file} && mkdir ${file} && exit 0
test -d ${file} && echo -e "刪除${filename}資料夾\n路徑:${file}" && rmdir ${file} && exit 0
