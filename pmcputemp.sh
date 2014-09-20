#!/bin/sh

TMP=~/.tempicon
KV=`uname -r`
[ "${KV:2:1}" -lt 7 ] && HorT=head || HorT=tail
load_module_func() {
	lsmod|grep -q coretemp
	CORETEMP=$?
	lsmod|grep -q k*temp
	KTEMP=$?
	if [ "$CORETEMP" = "1" ]&& [ "$KTEMP" = "1" ];then
		for m in coretemp k10temp k8temp  
		do modprobe $m 2>/dev/null
			if [ "$?" = "0" ];then
				echo "loading $m"
				L=0
				break 
			else 
				L=1
				continue
			fi
		done
	[ "$L" = "1" ] && echo "Can't load a temperature module" && exit
	fi
	return 0
}
cputempfunc() {
     [ ! -d $TMP ] && mkdir $TMP
	 FILE=`find /sys/devices/ -name temp1_input|${HorT} -n1`
	 if [ ! "$FILE" ];then
	    FILE=`find /sys/bus/acpi/devices/ -name temp|head -n1`
	 fi
	 if [ ! "$FILE" ];then echo "not working" && exit
	 fi
	 echo "${FILE} is written to $TMP"
	 echo -n ${FILE} > $TMP/pcutemprc
	 
}
load_module_func
cputempfunc
