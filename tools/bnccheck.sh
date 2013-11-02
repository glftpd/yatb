#!/bin/sh
# Crontab script for Yatb
YATBPATH=/home/users/yatb/yatb
YATBCONF=yatb.conf
YATBPIDF=yatb.pid
YATBPROG=yatb-static
YATBARGS=""
#YATBARGS="-u"
PATH=/bin:/usr/bin:/usr/local/bin
YATBPID=
cd $YATBPATH
if [ -f $YATBPIDF ]
then
        YATBPID=`cat $YATBPIDF`
        if [ `ps auwx | grep $YATBPROG | grep $YATBPID | grep -v -c grep` = 1 ]
        then
                exit
        fi
        rm -f $YATBPIDF
fi

./$YATBPROG $YATBARGS $YATBCONF >/dev/null 2>&1