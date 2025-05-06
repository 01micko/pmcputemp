#!/bin/bash

TEMP=~/.config/pmcputemp
arch=`uname -m`

load_module_func() {
	[ -z "$1" ] || mod=$1 # manually specified on cli
	#echo "test module" # COMMENT THIS IN REAL LIFE
	for m in ${mod} coretemp k10temp k8temp it87 lm85 # more can be added
	do 
		#echo "$m" # COMMENT THIS IN REAL LIFE
		lsmod|grep -q $m # check if loaded
		ret=$?
		if [ $ret -ne 0 ];then
			modprobe $m 2>/dev/null # load if not already
		fi
		check_temp_data_exists $m # returns 0 on success
		dat=$?
		if [ $dat -eq 0 ];then
			return 0 # success, break here
		else
			rmmod $m 2>/dev/null # try to unload as it is useless to us
			continue # module failed, try next module
		fi
	done		
	# if we get here it failed
	echo "Can't load a temperature module"
	return 1
}

find_func() {
	#echo "searching" # COMMENT THIS IN REAL LIFE
	FILES="$1"
	for n in `echo $FILES`; do
		read p < $n
		if [ $? = 0 ];then
			FILE=$n # found it
			break
		else
			continue # didn't find, search again
		fi
	done
	# if we get here we didn't find anything useful
}

check_temp_data_exists() {
	#echo "checking data" # COMMENT THIS IN REAL LIFE
	# the rough order is Intel, AMD, whatever else
	FILE=
	if [ -n "$1" ];then
		for a in `find /sys/devices/ -type f -name 'name'|xargs grep $1`
		do 
			for z in `find ${a/name:$1/} -name 'temp*_input'|sort`
			do
				find_func "$z"
				[ -z "$FILE" ] && continue || break # if we found data break
			done
		done
	else
		for a in `find /sys/devices/platform -type f -name 'temp*_input'|sort` \
			`find /sys/devices/pci* -type f -name 'temp*_input'|sort` \
			`find /sys/devices/virtual -type f -name 'temp'|sort`
		do find_func "$a"
			[ -z "$FILE" ] && continue || break # if we found data break
		done
	fi
	if [ ! "$FILE" ];then echo "Failed to find file, trying again." && return 1 # bail out
	fi
	# if we get here we found it, so we write to file
	[ ! -d $TEMP ] && mkdir $TEMP
	echo -e "\"${FILE}\" is written to : \n$TEMP"
	echo -n ${FILE} > $TEMP/pmcputemprc # UNCOMMENT THIS IN REAL LIFE
	return 0
}

cputempfunc() {
	#echo "starting" # COMMENT THIS IN REAL LIFE
	if [ "${arch:0:3}" != "arm" ];then
		load_module_func
		[ $? -ne 0 ] && exit 1
	else
		check_temp_data_exists
		return $?
	fi
	return 0
}

if [ -z "$1" ];then
	cputempfunc # start
else
	cli_mod=$1
	load_module_func $cli_mod # module specified on pmcputemp cli
fi
