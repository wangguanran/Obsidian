#!/system/bin/sh

check_num=$1
fail=0

echo "check_sum=${check_num}"

while [ "${fail}" != "1" ]
do
  status=`cat /d/memtest/result`
  if [ "${status}" = "fail" ] ; then
    fail=1
  else
  	num=`pidof memtester | wc -w`
  	if [ "${check_num}" != "${num}" ] ; then
	  	echo "[DRAM_MEMTEST] Error: Detect memtester killed, memtester num ${num} should be ${check_num}" > /dev/kmsg
#  		fail=1
		fi
  fi
  
  if [ "${fail}" != "0" ] ; then
    echo "[DRAM_MEMTEST] Error detected, exit"
    killall -9 memtester
  	exit 1
  else
  	sleep 30
  fi
done

exit 0