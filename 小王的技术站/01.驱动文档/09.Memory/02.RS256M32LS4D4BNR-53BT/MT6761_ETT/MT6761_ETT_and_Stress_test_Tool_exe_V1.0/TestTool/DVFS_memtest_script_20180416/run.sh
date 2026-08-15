#!/bin/bash
foo() { shift 1; echo "Nanamark2 pid $1"; kill $1; }


if [ "$1" == "" ]; then
	run_time=45
else
	run_time=$1
fi

i=1
while [ "$i" != 0 ]
do
	echo ===== start Nanamark2
	monkey -p se.nena.nenamark2 -v 1 
	sleep 20
	echo ===== Press run
	monkey -f /data/start 1 
	sleep $run_time
	echo ===== send back key in Monkey
	monkey -f /data/back 1 
	echo ===== end Nanamark2
	sleep 15
	echo ===== kill Nanamark2
	psline=`ps | grep se.nena.nenamark2`
	foo $psline
done
