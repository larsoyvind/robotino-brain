#!/bin/bash

# This script will download, build and install the Robotino API2.

DEPEND="subversion build-essential checkinstall cmake wget" # git

API2REPOSITORY="http://svn.openrobotino.org/api2/trunk"
API2BRANCH="source/api2"
APIDIR="/home/$USER/opt/api2"
SUPERTMPDIR="/home/$USER/tmp"
TMPDIR="$SUPERTMPDIR/robotino"
CURDIR=`pwd`
SYMLINKSCRIPTNAME="symlinkApi.sh"
#SYMLINKSCRIPTURL="https://github.com/larsoyvind/robotino-brain/raw/master/tools/$SYMLINKSCRIPTNAME"
SYMLINKSCRIPTURL="https://raw.githubusercontent.com/larsoyvind/robotino-brain/master/tools/$SYMLINKSCRIPTNAME"

# Don't run if root
if [ "$(id -u)" = "0" ]; then
	echo "You are currently the root user, please run the script as a regular user."
	exit 1
fi

# Verify that apt-get is available
if [ ! -e "/usr/bin/apt-get" ]; then
	echo -n "
Unsupported distribution!

This installation script has been written for and tested on Ubuntu (14.04 LTS),
and will possibly work for other Debian/Ubuntu derivatives (which should not
generate this message).

You can run the script anyway, but for the script to work you will have to
provide the dependencies yourself (these will be listed in the next dialog).
(This has been tested and is working on Arch Linux.)

Do you wish to continue anyway? (n) "
	read CONTINUE
	case $CONTINUE in
		"y" )
			;&
		"Y" )
			;&
		"yes" ) ;&
		"YES" ) ;&
		"Yes" )
			;;
		*)
			exit 1
			;;
	esac
	echo "";
fi


# Be nice, ask for consent
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

read ANSWER
while [[ "$ANSWER" == "y" || "$ANSWER" == "Y" ]]; do
	echo "Please enter 'yes' to continue"
	read ANSWER
done
if [[ "$ANSWER" != "yes" && "$ANSWER" != "YES" ]]; then
	exit 1
fi


# Install dependencies
if [[ ! "$CONTINUE" ]]; then
	echo "
We will now install dependencies using the package manager, you might be
asked for a password.
"
	sudo apt-get -y install $DEPEND
fi


# Get the API2 source
if [ -e "$TMPDIR/$API2BRANCH/.svn" ]; then
	# We already have source, just update repo instead!
	cd $TMPDIR/$API2BRANCH
	svn up
else
	mkdir -p $TMPDIR
	cd $TMPDIR
	svn co $API2REPOSITORY $API2BRANCH
	# The following line is from the Robotino wiki, does not seem to be needed
	#find source/api2/external -wholename "*/bin/*" -exec chmod +x {} \;
fi


# This is from the Robotino wiki, does not seem to be needed
#export ROBOTINOAPI2_32_DIR=/home/$USER/build/api2/install/usr/local/robotino/api2/


# Create build directory and build the api
if [ -e "$APIDIR" ]; then
	rm -rf $APIDIR
fi
mkdir -p $APIDIR
cd $APIDIR
cmake $TMPDIR/$API2BRANCH
make
make install


# Symlink api into the folders the compiler and linker will look
cd $TMPDIR
if [ -e "$TMPDIR/$SYMLINKSCRIPTNAME" ]; then
	rm $TMPDIR/$SYMLINKSCRIPTNAME
fi
wget $SYMLINKSCRIPTURL
echo "
We will now run a script to creates symlinks to the newly compiled api
installation. This script needs to run as root, so you might be asked for a
password.
"
sudo bash $SYMLINKSCRIPTNAME $APIDIR


# Cleanup?
echo -n "
API installation has completed, would you like us to remove the temporary
files (reduces downloads when this script re-run to update installation) ? (n) "
read REMOVETMP
case $REMOVETMP in
	"y" )
		;&
	"Y" )
		;&
	"yes" ) ;&
	"YES" ) ;&
	"Yes" )
		rm -rf $TMPDIR
		rmdir --ignore-fail-on-non-empty $SUPERTMPDIR
		echo "Temporary files removed"
		;;
esac

echo "

Installation completed. Any installed dependecies has been kept.


To report bugs or look at some Robotino code, go to
http://github.org/larsoyvind/robotino-brain
"

