#!/bin/bash

# This script will download, build and install the Robotino API2.



### "Constants"

DEPEND="subversion build-essential cmake wget" # git checkinstall
API2REPOSITORY="http://svn.openrobotino.org/api2/trunk"
API2BRANCH="source/api2"
SUPERAPIDIR="/home/$USER/opt"
APIDIR="$SUPERAPIDIR/api2"
SUPERTMPDIR="/home/$USER/tmp"
TMPDIR="$SUPERTMPDIR/robotino"
SYMLINKSCRIPTNAME="symlinkApi.sh"
SYMLINKSCRIPTURL="https://raw.githubusercontent.com/larsoyvind/robotino-brain/master/tools/$SYMLINKSCRIPTNAME"

LIBLINK="/lib/librec_robotino_api2.so"
INCDIRLINK="/include/rec"
API2PATH="/install/usr/local/robotino/api2"

### Variables
DONTASK=0
EXITAFTERPARSE=0


### Functions

# Don't run if root
rootcheck() {
	if [ "$(id -u)" = "0" ]; then
		echo "You are currently the root user, please run the script as a regular user."
		exit 1
	fi
}

# Reads and checks a dialog requiring a full "yes" to continue
requireyes()
{
	read ANSWER
	while [[ "$ANSWER" == "" || "$ANSWER" == "y" || "$ANSWER" == "Y" ]]; do
		echo "Please enter 'yes' to continue"
		echo -n "Do you wish to continue? (yes/no) "
		read ANSWER
	done
	if [[ "$ANSWER" != "yes" && "$ANSWER" != "YES" ]]; then
		return 1
	fi
}

# Reads and checks an y/n/yes/no answer, defaults to no
defaultno()
{
	read ANSWER
	case $ANSWER in
		"y" )
			;&
		"Y" )
			;&
		"yes" ) ;&
		"YES" ) ;&
		"Yes" )
			return 1;
			;;
	esac
}

# Removes symlinks
removelink() {
	if [ ! "$1" ]; then
		echo "removelink(): No file specified" 1>&2
		return 1
	fi

	if [ -h "$1" ]; then
		sudo rm "$1"
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

	sudo ln -s "$1" "$2"
	
	if [ $! ]; then
		echo "Error adding symlink $2" 1>&2
		return 1
	else
		#echo -e "Symlink added:\n  $2 => $1"
		echo "Symlink added: $2"
	fi

	return 0
}

# Clean out the temporary directory
cleantmp()
{
	rm -rf $TMPDIR
#	rmdir --ignore-fail-on-non-empty $SUPERTMPDIR
	echo "Temporary files removed"
}

# Be nice, ask for consent
askconsent()
{
	if [ $DONTASK -gt 0 ]; then
		return 0
	fi

	echo -n "
The api2 for Robotino will now be installed:

Install dir: $APIDIR
Temp dir: $TMPDIR

After the installation symlinks will be created in system folders making
compilation and linking work as if the API was installed from a system package.
This will require root privileges.

During the installation, the following dependencies and any dependecies they
might have will also be installed:
$DEPEND

We take no responsibility for any harm caused by this script.
Do you wish to continue? (yes/no) "

	if requireyes; then
		return 0
	fi
	exit 1
}

# Check distribution
installdepends()
{
	# Verify that apt-get is available
	if [ -e "/usr/bin/apt-get" ]; then
		# Install dependencies
		echo "
We will now install dependencies using the package manager, you might be
asked for a password.
"
		sudo apt-get -y install $DEPEND
	else
		if [ $DONTASK -gt 0 ]; then
			return 0
		fi

		# Unsupported distro warning
		echo -n "
Unsupported distribution!

This installation script has been written for and tested on Ubuntu (14.04 LTS),
and will possibly work for other Debian/Ubuntu derivatives (which should not
generate this message).

You can run the script anyway, but for the script to work you will have to
provide the dependencies yourself (as described in the previous dialog).
(This has been tested to work on Arch Linux.)

Do you wish to continue anyway? (n) "

		if defaultno; then
			exit 1	# Recieved "yes"
		fi
		echo ""
		return 0
	fi
	return 0
}

# Uninstallation
uninstall()
{
	echo "Uninstalling..."
	removelink /usr$LIBLINK
	removelink /usr$INCDIRLINK
	rm -rf $APIDIR
	rmdir --ignore-fail-on-non-empty $SUPERAPIDIR
	echo "Uninstall complete"
}

# Print usage information
usage()
{
	echo "USAGE: $0 [OPTIONS]

OPTIONS
	--help,--usage	Display this help text
	--dontask	Perform install no questions asked
	--cleanup	Remove temporary files
	--uninstall	Remove installation (includes symlinks)
	--removelinks	Remove symlinks
"
}

# Simple option checker
parseargs()
{
	THIS="$0"
	for ARG in $@; do
		case $ARG in 
			"--help" | "--usage" )
				usage $THIS
				exit 0 ;;

			"--dontask" )
				DONTASK=1
				echo "Installing without prompting"
				shift ;;

			"--cleanup" )
				cleantmp
				EXITAFTERPARSE=1
				shift ;;

			"--uninstall" )
				uninstall
				EXITAFTERPARSE=1
				shift ;;

			"--removelinks" )
				removelink /usr$LIBLINK
				removelink /usr$INCDIRLINK
				exit 0 ;;
			
			"" )
				break ;;

			* )
				echo -e "Unknown option \"$ARG\"\n" 1>&2
				usage
				exit 1 ;;
		esac
	done
	if [ $EXITAFTERPARSE -gt 0 ]; then
		exit 0
	fi
}



### Run start
parseargs $@
rootcheck
askconsent
installdepends


# Get the API2 source
if [ -e "$TMPDIR/$API2BRANCH/.svn" ]; then
	# We already have source, just update repo instead!
	cd $TMPDIR/$API2BRANCH
	svn up
else
	mkdir -p $TMPDIR
	cd $TMPDIR
	svn co $API2REPOSITORY $API2BRANCH
	# The following line is from the Robotino wiki, it does not seem to be needed
	#find source/api2/external -wholename "*/bin/*" -exec chmod +x {} \;
fi


# Create build directory and build the api
if [ -e "$APIDIR" ]; then
	rm -rf $APIDIR
fi
mkdir -p $APIDIR
cd $APIDIR
# The following line is from the Robotino wiki, it does not seem to be needed
#export ROBOTINOAPI2_32_DIR=/home/$USER/build/api2/install/usr/local/robotino/api2/
cmake $TMPDIR/$API2BRANCH --no-warn-unused-cli
# make
make install


# Symlink api into the folders the compiler and linker will look
echo "
We will now run a script to creates symlinks to the newly compiled api
installation. This script needs to run as root, you might be asked for a
password.
"
#wget -q -O - $SYMLINKSCRIPTURL | sudo bash
removelink /usr$LIBLINK
removelink /usr$INCDIRLINK
addlink $APIDIR$API2PATH$LIBLINK /usr$LIBLINK
addlink $APIDIR$API2PATH$INCDIRLINK /usr$INCDIRLINK


# Cleanup?
if [ $DONTASK -gt 0 ]; then
	echo "
Temporary files have not been removed. They are useful for upgrading the
installation, but can be removed by re-running the script with the
\"--cleanup\" parameter or by manually deleting $TMPDIR."
else
	echo -n "
API installation has completed, would you like us to remove the temporary
files (reduces downloads when this script re-run to update installation) ? (n) "
	if defaltno; then
		cleantmp
	fi
fi


# End message
echo "

Installation completed. Any installed dependecies has been kept.


To report bugs or look at some Robotino code, go to
http://github.org/larsoyvind/robotino-brain
"

