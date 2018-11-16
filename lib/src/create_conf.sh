#!/bin/bash

LOG="mtlog.conf"
> $LOG

for((i = 1; i < 1000; i++))
do
    echo "[ log$i  ]" >> $LOG
    echo "name   = \"/var/log/eBMC/mt.log$i\"" >> $LOG
    echo "size   = 1024$i" >> $LOG
    echo "rotate = $i" >> $LOG
    echo "" >> $LOG
done
