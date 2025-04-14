while [ "$1" ] #讀取第一個參數，不為0時執行此迴圈
do
	if [ "$1" == "-b" ];then #如果第一個參數為-b執行下式
		ob="$2" #設定ob值為第二個參數
		case $ob in #ob為16,8,2,*分別對應basesystem參數設為"Hex","Oct","bin","Unknown"
			16) basesystem="Hex";;
			8) basesystem="Oct";;
			2) basesystem="bin";;
			*) basesystem="Unknown";;
		esac
		shift 2 #將所有輸入的參數序號-2
	elif [ "$1" == "-n" ] #如果第一個參數為-n執行下式
	then
		num="$2" #設定num數值為第二個參數
		shift 2 #將所有輸入的參數序號-2
	else #第一個參數非-b or -n 執行下式
		echo "Program $0 does not recognize option $1" #回應在console 此程式無法辨識第一個參數
		exit 1 #離開程式
	fi
done
output=`echo "obase=$ob;ibase=10;$num;"|bc` #obase設定輸出進位制為$ob、ibase設定輸入進位制為10，$num為輸入數值，bc為執行前方輸出之參數帶入進行進位制轉換
echo "$num Decimal number = $output in $basesystem number system(base=$ob)" #輸出進位轉換結果至console
