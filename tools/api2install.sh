#!/bin/bash

# This script will download, build and install the Robotino API2.


### 	"Constants"

# Important!!   The dependency lists must match up, $DEPEND contains the
# package name, $DEPENDBINS contains (one of) the command(s) of the package
DEPEND=('subversion' 'cmake')
DEPENDBINS=('svn' 'cmake')

API2REPOSITORY="http://svn.openrobotino.org/api2/trunk"
API2BRANCH="source/api2"
SUPERAPIDIR="/home/$USER/opt"
APIDIR="$SUPERAPIDIR/api2"
SUPERTMPDIR="/home/$USER/tmp"
TMPDIR="$SUPERTMPDIR/robotino"

LIBLINK="/lib/librec_robotino_api2.so"
INCDIRLINK="/include/rec"
API2PATH="/install/usr/local/robotino/api2"

DOCPATH="$APIDIR$API2PATH/doc"
CDOC="$DOCPATH/rec_robotino_api2_c/index.html"
CPPDOC="$DOCPATH/rec_robotino_api2/index.html"


###		Variables

dontAsk=0
exitAfterParse=0
doCleanup=0
doUninstall=0
doRemoveLinks=0
doCreateLinks=0


###		Utility functions

# Echo error msg to stderr
echoErr() {
	echo "$@" 1>&2;
}

# Reads and checks a dialog requiring a full "yes" to continue
requireYes() {
	while true; do
		read answer
		case "$answer" in
			"" | "y" | "Y" )
				echoErr -e "\e[1mPlease enter 'yes' to continue\e[21m"
				echoErr -ne "Do you wish to continue? \e[1m(yes/no)\e[21m "
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
defaultYes() {
	read answer
	case "$answer" in
		"y" | "Y" | "yes" | "YES" | "Yes" | "" )
			return 0;
			;;
	esac
	return 1;
}

# Reads and checks an y/n/yes/no answer, defaults to no
defaultNo() {
	read answer
	case "$answer" in
		"y" | "Y" | "yes" | "YES" | "Yes" )
			return 0;
			;;
	esac
	return 1;
}


### Other functions

# Don't run if root
rootCheck() {
	if [ "$(id -u)" = "0" ]; then
		echoErr "You are currently the root user, please run the script as a regular user."
		exit 1
	fi
}

# Removes symlink
removeLink() {
	if [ ! "$1" ]; then
		echo "removeLink(): No file specified" 1>&2
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
addLink() {
	if [ ! "$2" ]; then
		echo "addLink(): Parameter(s) missing" 1>&2
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

# Symlink api into the folders the compiler and linker will look
symlinkApi() {
	echo -e "
\e[1mWe will now create symlinks to the newly compiled api installation. This
requires root privileges, you might be asked for a password.\e[21m"
	removeLink "/usr$LIBLINK" || return 1
	removeLink "/usr$INCDIRLINK" || return 1
	addLink "$APIDIR$API2PATH$LIBLINK" "/usr$LIBLINK" || return 1
	addLink "$APIDIR$API2PATH$INCDIRLINK" "/usr$INCDIRLINK" || return 1

	return 0
}

# Clean out the temporary directory
cleanTmp() {
	rm -rf "$TMPDIR"
	echo -e "\e[1mTemporary files removed\e[21m"
}

# Be nice, ask for consent
askConsent() {
	[ $dontAsk -gt 0 ] &&
		return 0

	echoErr -ne "
\e[1mThe api2 for Robotino will now be installed:\e[21m

Install dir: \e[1m$APIDIR\e[21m
Temp dir: \e[1m$TMPDIR\e[21m

During the installation, some build dependencies may need to be installed.
After the installation symlinks will be created in system folders making
compilation and linking work as if the API was installed from a system package.
\e[1mThese actions will require root privileges\e[21m, you will be notified when they are
taken.

\e[1mWe take no responsibility for any harm caused by this script.\e[21m

Do you wish to continue? (yes/no) "
	requireYes && {
		echoErr ""
		return 0
	}

	exit 1
}

# Installs dependencies, exit with error if unmet and unsupported
installDepends() {

	missingDepends=()
	packageManager=""

	# Check build dependencies
	for i in ${!DEPEND[@]}; do
		[ ! -e "/usr/bin/${DEPENDBINS[$i]}" ] && {
			# echo "${DEPEND[$i]} missing"
			missingDepends=("${missingDepends[@]}" "${DEPEND[$i]} ")
		}
	done
	[ "${#missingDepends[@]}" == 0 ] && {
		# Build dependencies are satisfied
		echo "Build dependencies satisfied"
		return 0
	}

	# Install dependencies
	if [ -e "/usr/bin/apt-get" ]; then
		# Debian-base, use apt-get
		packageManager="apt-get -y install"
	elif [ -e "/usr/bin/pacman" ]; then
		# Arch-base, use pacman
		packageManager="pacman --noconfirm -S"
	else
		# Unsupported distro, advise manual install
		echoErr -e "
\e[1mUnsupported distribution!\e[21m

We are unable to automatically install the necessary build dependencies.
\e[1mYou can still use the script,\e[21m but you need to install the following
build dependencies manually first:
\e[1m${missingDepends[@]}\e[21m"
		echoErr ""
		exit 1
	fi

	if [ "$packageManager" != "" ]; then
		# A package manager was found, install dependencies
		echoErr -e "
\e[1mInstalling build dependencies using the systems package manager, you might
be asked for a password:
${missingDepends[@]}\e[21m"
		sudo $packageManager ${missingDepends[@]} || {
			echoErr -e "
\e[1mError installing dependencies, fix the reported problem and try again.\e[21m"
			exit 1
		}
		echo ""
	fi

	return 0
}

# Uninstallation
uninstall() {
	echo -e "\e[1mUninstalling...\e[21m"
	if [ -e "$APIDIR" ]; then
		echo "Removing symlinks"
		removeLink "/usr$LIBLINK"
		removeLink "/usr$INCDIRLINK"
		echo "Removing files from $SUPERAPIDIR"
		rm -rf "$APIDIR"
		rmdir --ignore-fail-on-non-empty "$SUPERAPIDIR"
		echo -e "\e[1mUninstall complete\e[21m"
		if [ -e "$TMPDIR" ]; then
			echo "To remove temporary files from installation, please run
$0 --cleanup"
		fi
	else
		echoErr "Error: The installation directory was not found."
		exit 1
	fi
}

# Rudementary check to see if the api is installed
isInstalled() {
	if [ -e "$CPPDOC" ]; then
		return 0
	else
		return 1
	fi
}

# Print usage information
usage() {
	echo "USAGE: $0 [OPTIONS]

Installs RobotinoAPI2 in the user account and symlinks the api into system
folders for native inclusion during compilation and running.

The script can be run again to update the installation.


OPTIONS
   --help,--usage   Display this help text

   --dontask        Perform install no questions asked (we take no
                    responsibility any damage caused by this script)

   --doc            Opens C++ api documentation in browser (if available)
   --cdoc           Opens C api documentation in browser (if available)

   --createlinks    Creates symlinks (done automatically during install)

   --cleanup        Remove temporary files
   --uninstall      Remove installation (includes symlinks)
   --removelinks    Remove symlinks
"
#   --verbose        Display full output of child processes
#   --force          Re-install even if repository is up to date and build exist
}

# Simple option checker
parseArgs() {
	for ARG in "$@"; do
		case "$ARG" in 
			"--help" | "--usage" )
				usage
				exit 0 ;;
			"--doc" )
				isInstalled || {
					echoerr "Please install first"
					exit 1
				}
				xdg-open "$CPPDOC" || {
					echoerr -e "
Unable to open browser (using xdg), the documentation is available here:
$CPPDOC
"
					exit 1
				}
				exit 0 ;;
			"--cdoc" )
				isInstalled || {
					echoerr "Please install first"
					exit 1
				}
				xdg-open "$CDOC" || {
					echoerr -e "
Unable to open browser (using xdg), the documentation is available here:
$CDOC
"
					exit 1
				}
				exit 0 ;;
			"--dontask" )
				dontAsk=1
				echo "Installing without prompting"
				shift ;;
			"--cleanup" )
				doCleanup=1
				exitAfterParse=1
				shift ;;
			"--uninstall" )
				doUninstall=1
				exitAfterParse=1
				shift ;;
			"--removelinks" )
				doRemoveLinks=1
				exitAfterParse=1
				shift ;;
			"--createlinks" )
				doCreateLinks=1
				shift ;;
#			"--verbose" )
#				shift ;;
			"" )
				break ;;
			* )
				echoErr -e "Unknown option \"$ARG\"\n"
				usage
				exit 1 ;;
		esac
	done
}

# Perform special actions detected by parseArgs
performActions() {
	if [ $doCleanup -gt 0 ]; then
		cleanTmp
	fi
	if [ $doUninstall -gt 0 ]; then
		uninstall
	else
		if [ $doRemoveLinks -gt 0 ]; then
			removeLink /usr$LIBLINK
			removeLink /usr$INCDIRLINK
		fi
		if [ $doCreateLinks -gt 0 ]; then
			symlinkApi || exit 1
		fi
	fi

	if [ $exitAfterParse -gt 0 ]; then
		exit 0
	fi
}



### Run start

parseArgs "$@"
rootCheck
performActions
askConsent
installDepends


# Get the API2 source
echo -en "\e[1mFetching the api source code, \e[21m"
if [ -e "$TMPDIR/$API2BRANCH/.svn" ]; then
	# We already have source, just update repo instead!
	echo -e "\e[1mfound existing repository - updating\e[21m"
	cd "$TMPDIR/$API2BRANCH"
	svn up > /dev/null || {
		echoErr -e "
\e[1mSubversion returned an error while updating the API2 source code. Please check
any errors, fix the problem and try again.\e[21m
"
		exit 1
	}
else
	mkdir -p "$TMPDIR"
	cd "$TMPDIR"
	echo -e "\e[1mcloning api2 repository\e[21m
(this can take some time)"
	svn co "$API2REPOSITORY" "$API2BRANCH" > /dev/null || {
		echoErr -e "
\e[1mSubversion returned an error while downloading the API2 source code. Please
check any error messanges provided, fix the problem and try again.\e[21m
"
		exit 1
	}
	# The following line is from the Robotino wiki, it does not seem to be needed
	#find source/api2/external -wholename "*/bin/*" -exec chmod +x {} \;
fi


# Create build directory and build the api
echo -e "
\e[1mBuilding the api\e[21m
"
if [ -e "$APIDIR" ]; then
	rm -rf "$APIDIR"
fi
mkdir -p "$APIDIR"
cd "$APIDIR"
# The following line is from the Robotino wiki, it does not seem to be needed
#export ROBOTINOAPI2_32_DIR=/home/$USER/build/api2/install/usr/local/robotino/api2/
echo -e "\e[1mGenerating build files\e[21m
"
cmake "$TMPDIR/$API2BRANCH" -Wno-dev > /dev/null
echo -e "\e[1mCompiling\e[21m
(This may take some time. Any warning messages can usually be ignored.)
"
make install -j4 > /dev/null || {
	echoErr -e "
\e[1mError during compilation. Please check any error messages.\e[21m
"
	exit 1
}


# Symlink the api
symlinkApi || {
	echo -e "
\e[1mError during symlink creation. Please check any error messages.\e[21m
"
	exit 1
}


# Cleanup?
if [ $dontAsk -gt 0 ]; then
	echo "
Temporary files have not been removed. These are useful for upgrading the
installation, but can be removed by running the script with the \"--cleanup\"
parameter or by manually deleting $TMPDIR."
else
	echoErr -ne "
\e[1mWould you like to keep the local copy of the api repository\e[21m, this
will reduce future downloads if this script is re-run to update the api
installation \e[1m? (Y/n) \e[21m"
	if defaultYes; then
		echo "Temporary files kept"
	else
		cleanTmp
	fi
fi


# End message
echo -e "

\e[1mInstallation completed. Any installed dependecies has been kept.\e[21m

To report bugs or look at some Robotino code, go to
http://github.org/larsoyvind/robotino-brain
"

