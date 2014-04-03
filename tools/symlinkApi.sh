#!/bin/bash

# Display usage info
usage() {
	echo -e "Adds symlinks from compiled API2 to /usr/lib and /usr/include

Usage:
 $0 [OPTION|PATH]

 PATH	The path to where 'make && make install' was run to compile API2.

 If path is unspecified, current working directory is assumed. Existing
 symlinks will be replaced.

Options:
	--help		Display this help text
	--local		Symlink from installation in usr/local/ as source instead of
				using working directory or supplied path
	--remove	Remove symlinks
" 1>&2
}

# Removes symlinks
removelink() {
	if [ ! "$1" ]; then
		echo "removelink(): No file specified" 1>&2
		return 1
	fi

	if [ -h "$1" ]; then
		rm "$1"
		if [ $! ]; then
			echo "Problem removing $1" 1>&2
			return 1
		else
			echo "Symlink removed: $1"
		fi
	elif [ -e "$1" ]; then
		echo "$1 exists, but is not a symlink. Please remove manually." 1>&2
		exit 1;
	fi

	return 0
}

# Adds symlinks
addlink() {
	if [ ! "$2" ]; then
		echo "addlink(): Parameter(s) missing" 1>&2
		return 1
	fi

	ln -s "$1" "$2"
	
	if [ $! ]; then
		echo "Error adding symlink $2" 1>&2
		return 1
	else
		#echo -e "Symlink added:\n  $2 => $1"
		echo "Symlink added: $2"
	fi

	return 0
}


LIBLINK="/lib/librec_robotino_api2.so"
INCDIRLINK="/include/rec"
API2PATH="/install/usr/local/robotino/api2"
FULLPATH=$(pwd)
REGEXFULLPATH="^\/" 
REGEXFLAGGIVEN="^-"

# Simple option checker
if [[ $1 =~ $REGEXFLAGGIVEN ]]; then
	case $1 in 
		"--help" | "--usage" )
			usage
			exit 0
			;;

		"--local" )
			echo "Using /usr/local/robotino as source path"
			# This may be a bit ugly
			API2PATH="/usr/local/robotino/api2"
			;;

		"--remove" )
			echo "Removing symlinks"
			removelink /usr$LIBLINK
			removelink /usr$INCDIRLINK
			exit 0
			;;
		* )
			echo -e "Unknown option \"$1\"\n" 1>&2
			usage
			exit 1
	esac
fi

#Root check
if [ "$(id -u)" != "0" ]; then
	echo "This script must be run as root" 1>&2
	exit 1
fi

# If argument is given (and not caught as option), use as path
if [ "$1" ]; then
	if [ "$1" = "--local" ]; then
		FULLPATH="/"
	else
		FULLPATH=$1
	fi
fi

# New API dir check, removal of and adding links
if [[ $FULLPATH =~ $REGEXFULLPATH ]]; then
#	echo "Using API2 path: $FULLPATH"

	if [ -e $FULLPATH$API2PATH$LIBLINK ]; then
#		echo "New library found at $FULLPATH$API2PATH$LIBLINK"

		removelink /usr$LIBLINK
		addlink $FULLPATH$API2PATH$LIBLINK /usr$LIBLINK
		removelink /usr$INCDIRLINK
		addlink $FULLPATH$API2PATH$INCDIRLINK /usr$INCDIRLINK
	else
		echo "API2 not found at $FULLPATH, make sure you run the script from the base folder of the compiled API2 or supply the full path to this directory as an argument" 1>&2
		exit 1
	fi
else
	echo "Given path \"$1\" is relative, full path is required" 1>&2
	exit 1
fi

exit 0
