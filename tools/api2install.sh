#!/bin/bash

# This script will download, build and install the Robotino API2.



### "Constants"

DEPEND="subversion build-essential cmake" # git checkinstall wget
API2REPOSITORY="http://svn.openrobotino.org/api2/trunk"
API2BRANCH="source/api2"
SUPERAPIDIR="/home/$USER/opt"
APIDIR="$SUPERAPIDIR/api2"
SUPERTMPDIR="/home/$USER/tmp"
TMPDIR="$SUPERTMPDIR/robotino"

LIBLINK="/lib/librec_robotino_api2.so"
INCDIRLINK="/include/rec"
API2PATH="/install/usr/local/robotino/api2"

### Variables
DONTASK=0
EXITAFTERPARSE=0
DOCLEANUP=0
DOUNINSTALL=0
DOREMOVELINKS=0



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
	while true; do
		read ANSWER
		case "$ANSWER" in
			"" | "y" | "Y" )
				echo "Please enter 'yes' to continue"
				echo -n "Do you wish to continue? (yes/no) "
				;;
			"yes" | "YES" )
				return 0
				;;
			* )
				return 1
		esac
	done
}

# Reads and checks an y/n/yes/no answer, defaults to yes
defaultyes()
{
	read ANSWER
	case "$ANSWER" in
		"y" | "Y" | "yes" | "YES" | "Yes" | "" )
			return 1;
			;;
	esac
	return 0;
}

# Reads and checks an y/n/yes/no answer, defaults to no
defaultno()
{
	read ANSWER
	case "$ANSWER" in
		"y" | "Y" | "yes" | "YES" | "Yes" )
			return 1;
			;;
	esac
	return 0;
}

# Removes symlink
removelink() {
	if [ ! "$1" ]; then
		echo "removelink(): No file specified" 1>&2
		return 1
	fi

	if [ -h "$1" ]; then
		sudo rm "$1"
		if [ "$!" ]; then
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

# Adds symlink
addlink() {
	if [ ! "$2" ]; then
		echo "addlink(): Parameter(s) missing" 1>&2
		return 1
	fi

	sudo ln -s "$1" "$2"
	
	if [ "$!" ]; then
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
	rm -rf "$TMPDIR"
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

# Installs dependencies, warns if not Debian-based
installdepends()
{
	# Verify that apt-get is available
	if [ -e "/usr/bin/apt-get" ]; then
		# Install dependencies
		echo "
Installing dependencies using the systems package manager, you might be asked
for a password.
"
		sudo apt-get -y install "$DEPEND"
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
	removelink "/usr$LIBLINK"
	removelink "/usr$INCDIRLINK"
	if [ -e "$APIDIR" ]; then
		rm -rf "$APIDIR"
		rmdir --ignore-fail-on-non-empty "$SUPERAPIDIR"
		echo "Uninstall complete"
	else
		echo"
The installation directory was not found."
		exit 1
	fi
}

# Print usage information
usage()
{
	echo "USAGE: $0 [OPTIONS]

OPTIONS
   --help,--usage   Display this help text
   --dontask        Perform install no questions asked (we take no
                    responsibility any damage caused by this script)
   --cleanup        Remove temporary files
   --uninstall      Remove installation (includes symlinks)
   --removelinks    Remove symlinks
"
}

# Simple option checker
parseargs()
{
	for ARG in "$@"; do
		case "$ARG" in 
			"--help" | "--usage" )
				usage
				exit 0 ;;
			"--dontask" )
				DONTASK=1
				echo "Installing without prompting"
				shift ;;
			"--cleanup" )
				DOCLEANUP=1
				EXITAFTERPARSE=1
				shift ;;
			"--uninstall" )
				DOUNINSTALL=1
				EXITAFTERPARSE=1
				shift ;;
			"--removelinks" )
				DOREMOVELINKS=1
				EXITAFTERPARSE=1
				shift ;;
			"" )
				break ;;
			* )
				echo -e "Unknown option \"$ARG\"\n" 1>&2
				usage
				exit 1 ;;
		esac
	done
}

# Perform special actions detected by parseargs
performactions()
{
	if [ $DOCLEANUP -gt 0 ]; then
		cleantmp
	fi
	if [ $DOUNINSTALL -gt 0 ]; then
		uninstall
	else
		if [ $DOREMOVELINKS -gt 0 ]; then
			removelink /usr$LIBLINK
			removelink /usr$INCDIRLINK
		fi
	fi

	if [ $EXITAFTERPARSE -gt 0 ]; then
		exit 0
	fi
}



### Run start
parseargs "$@"
rootcheck
performactions
askconsent
installdepends


# Get the API2 source
echo "Fetching the api source code
"
if [ -e "$TMPDIR/$API2BRANCH/.svn" ]; then
	# We already have source, just update repo instead!
	cd "$TMPDIR/$API2BRANCH"
	svn up || {
		echo "
Subversion returned an error while updating the API2 source code. Please check
any errors, fix the problem and try again."
		exit 1
	}
else
	mkdir -p "$TMPDIR"
	cd "$TMPDIR"
	svn co "$API2REPOSITORY" "$API2BRANCH" || {
		echo "
Subversion returned an error while downloading the API2 source code. Please
check any error messanges provided, fix the problem and try again."
		exit 1
	}
	# The following line is from the Robotino wiki, it does not seem to be needed
	#find source/api2/external -wholename "*/bin/*" -exec chmod +x {} \;
fi


# Create build directory and build the api
echo "Building the api
"
if [ -e "$APIDIR" ]; then
	rm -rf "$APIDIR"
fi
mkdir -p "$APIDIR"
cd "$APIDIR"
# The following line is from the Robotino wiki, it does not seem to be needed
#export ROBOTINOAPI2_32_DIR=/home/$USER/build/api2/install/usr/local/robotino/api2/
cmake "$TMPDIR/$API2BRANCH" -Wno-dev
make install -j4 || {
	echo "
Error during compilation. Please check any error messages."
	exit 1
}


# Symlink api into the folders the compiler and linker will look
echo "
We will now create symlinks to the newly compiled api installation. This
requires root privileges, you might be asked for a password.
"
removelink "/usr$LIBLINK"
removelink "/usr$INCDIRLINK"
addlink "$APIDIR$API2PATH$LIBLINK" "/usr$LIBLINK"
addlink "$APIDIR$API2PATH$INCDIRLINK" "/usr$INCDIRLINK"


# Cleanup?
if [ $DONTASK -gt 0 ]; then
	echo "
Temporary files have not been removed. They are useful for upgrading the
installation, but can be removed by re-running the script with the
\"--cleanup\" parameter or by manually deleting $TMPDIR."
else
	echo -n "
API installation has completed, would you like to keep the temporary files
(this will reduce future downloads when this script re-run to update the api
installation) ? (y) "
	if defaultyes; then
		cleantmp
	else
		echo "Temporary files kept"
	fi
fi


# End message
echo "

Installation completed. Any installed dependecies has been kept.

To report bugs or look at some Robotino code, go to
http://github.org/larsoyvind/robotino-brain
"

