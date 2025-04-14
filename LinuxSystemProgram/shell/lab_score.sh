echo "Input scores:"
read score
if [ $score -ge 90 ]
then
	echo "優等"
elif [ $score -ge 80 ]
then
	echo "甲等"
elif [ $score -ge 70 ]
then 
	echo "乙等"
elif [ $score -ge 60 ]
then 
	echo "丙等"
else
	echo "不及格"
fi
