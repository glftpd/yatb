#!/bin/bash

#full command to start yatb (needs -u or password compiled to yatb)
command="yatb -u yatb.conf"

#full path to yatb dir
startdir="/root/yatb"

# add something like this to crontab
# 0,5,10,15,20,25,30,35,40,45,50,55 * * * * /root/yatb/bnctest.sh 1>/dev/null 2>&1

################################################################
res=`ps ax|grep "$command" | grep -v grep`

if test -n "$res" 
then
	echo "yatb running"
else
	echo "yatb not running"
	cd $startdir
	./$command
fi
