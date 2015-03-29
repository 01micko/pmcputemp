#!/bin/sh

TMP=~/.config/pmcputemp
arch=`uname -m`
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
	FILES=`find /sys/devices -type f -name 'temp*_input'|sort`
	for n in `echo $FILES`; do
		read p < $n
		if [ $? = 0 ];then
			FILE=$n
			break
		else
			continue
		fi
	done
	if [ ! "$FILE" ];then echo "Failed to find file" && exit 1
	fi
	echo "${FILE} is written to $TMP"
	echo -n ${FILE} > $TMP/pmcputemprc
}

cputempfunc
