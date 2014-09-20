#!/bin/sh

TMP=~/.config/pmcputemp
KV=`uname -r`
arch=`uname -m`
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
	[ "$L" = "1" ] && echo "Can't load a temperature module" && return 1
	fi
	return 0
}
cputempfunc() {
	if [ "${arch:0:3}" != "arm" ];then
		load_module_func
		[ $? -ne 0 ] && exit 1
	fi
	[ ! -d $TMP ] && mkdir $TMP
	FILE=`find /sys/devices/ -type f -name temp1_input|${HorT} -n1`
	if [ ! "$FILE" ];then
		FILE=`find /sys/devices/platform/ -type f -name 'temp*input'|tail -n1`
	fi
	if [ ! "$FILE" ];then
		FILE=`find /sys -type f -name 'temp*'|head -n1` #last try
	fi
	if [ ! "$FILE" ];then echo "Failed to find file" && exit 1
	fi
	echo "${FILE} is written to $TMP"
	echo -n ${FILE} > $TMP/pcutemprc
	 
}

cputempfunc
