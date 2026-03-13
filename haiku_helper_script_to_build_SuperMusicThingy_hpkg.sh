#!/usr/bin/env bash
#
# Automated build script
# Downloads git repo, builds, then builds Haiku hpkg file on Desktop

appname="SuperMusicThingy"
read -p "Build ${appname} with projectm visuals? Requires building projectm from source and nebula (nvidia driver). y/n: "
if [[ "$REPLY" == y ]];then
	appname="SuperMusicThingyNebula"
	requires=("haiku >= r1~beta5_hrev59451-1" "libglvnd >= 1.7.0-1" "nebula" "libsdl2")

else
	appname="SuperMusicThingy"
	requires=("haiku")
	buildspec="-DENABLE_PROJECTM=OFF -DENABLE_SDL2=OFF -DENABLE_GL=OFF"
	skipprojectm="true"
fi


projectm_source_directory="/tmp/projectm"

if [[ ! -d "/tmp/SuperMusicThingy" ]];then
	read -p "SuperMusicThingy source not found. Download and install in /tmp/SuperMusicThingy y/n: "
else
	read -p "/tmp/SuperMusicThingy found. Deleteing this might help build problems. Delete and reinstall? y/n: "
fi

if [[ "$REPLY" == y ]];then
	[[ -e /tmp/SuperMusicThingy ]] && rm -fr /tmp/SuperMusicThingy
	cd /tmp/
	git clone git@github.com:ablyssx74/SuperMusicThingy.git
	mkdir -p /tmp/SuperMusicThingy/	
	mkdir -p /tmp/SuperMusicThingy/hpkgs
	mkdir -p /tmp/SuperMusicThingy/hpkgs/${appname}
	mkdir -p /tmp/SuperMusicThingy/hpkgs/${appname}/apps
	mkdir -p /tmp/SuperMusicThingy/hpkgs/${appname}/bin
	mkdir -p /tmp/SuperMusicThingy/hpkgs/${appname}/data/mime_db/application
	touch /tmp/SuperMusicThingy/hpkgs/${appname}/data/mime_db/application/x-vnd.supermusicthingy
fi

if [[ ! "$skipprojectm" ]];then 
	if [[ ! -d "$projectm_source_directory" ]];then
		read -p "Required libprojectM-4 source not found. Download, build add link to SuperMusicThingy? y/n: " choice1
	else
		read -p "/tmp/libprojectm found. Deleteing this might help build problems. Delete and reinstall? y/n: " chorice2
	fi
	if [[ $choice2 == y ]];then
		rm -fr $projectm_source_directory
	fi

	if [[ $choice1 == y ]];then
		pkgman install cmake libsdl2_devel libx11_devel
		cd /tmp
		git clone https://github.com/projectM-visualizer/projectm.git
		cd projectm
		git fetch --all --tags
		git submodule init
		git submodule update
		mkdir build
		cd build
		cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/tmp/SuperMusicThingy/hpkgs/${appname}/ ..
		cmake --build . -- -j && cmake --build . --target install 
 fi
fi

if [[ ! -e /tmp/SuperMusicThingy/hpkgs/${appname}/.PackageInfo ]];then
echo -n "name		${appname}
version			1.0.0-1
architecture	x86_64
summary			\"Portable streaming media client\"
description		\"SuperMusicThingy is a free streaming media client. Fast, light, and fun!\n
Add milk drop presets in settings/SuperMusicThingy/milk_presets/\"

packager		\"ablyss <jb@epluribusunix.net>\"
vendor			\"Haiku Project\"
licenses {
	\"MIT\"
}
copyrights {
	\"2026 ablyss\"
}
provides {
	${appname} = 1.0.0
	app:${appname}
}
requires {
	${requires[0]}
	nlohmann_json
	mpv 
	curl 
	openal
	${requires[1]}
	${requires[2]}
	${requires[3]}
	
}
urls {
	\"https://github.com/ablyssx74/SuperMusicThingy\"
}
source-urls {
# Download
	\"https://github.com/ablyssx74/SuperMusicThingy/archive/refs/tags/v1.0.0.tar.gz\"
}
" > /tmp/SuperMusicThingy/hpkgs/${appname}/.PackageInfo
	 
fi

    
pkgman install mpv_devel

cd /tmp/SuperMusicThingy
cmake -B build_${appname} ${buildspec}
cmake --build build_${appname}    
rc -o ${appname}.rsrc ${appname}.rdef 
xres -o build_${appname}/SuperMusicThingy ${appname}.rsrc     
mv -f build_${appname}/SuperMusicThingy hpkgs/${appname}/apps/${appname}
mimeset -f hpkgs/${appname}/apps/${appname}
ln -s /boot/system/apps/${appname} hpkgs/${appname}/bin/${appname}


cd /tmp/SuperMusicThingy/hpkgs/
package create -C ${appname} ${appname}.hpkg
mv /tmp/SuperMusicThingy/hpkgs/${appname}.hpkg $HOME/Desktop/${appname}.hpkg

if [[ ! "$skipprojectm" ]];then 
	if [[ -d "$projectm_source_directory" ]];then
		read -p "Delete projectm source? y/n: "
	fi
	if [[ $REPLY == y ]];then
		rm -fr "${projectm_source_directory}*"
	fi
fi

if [[ -d /tmp/SuperMusicThingy ]];then
	read -p "Delete SuperMusicThing source? y/n: "
fi

if [[ $REPLY == y ]];then
	rm -fr "/tmp/SuperMusicThingy*"
fi

if [[ ! "$skipprojectm" ]];then 
	if pkgman search libglvnd | grep -q "libglvnd"; then
    	echo "libglvnd found."
		else
    		read -p "libglvnd not found. Download and install? y/n: " glvnd
	fi
	if [[ "$glvnd" == y ]];then
		TMP_PKG=$(mktemp /tmp/libglvnd.XXXXXX.hpkg)
		echo "Downloading libglvnd..."
		curl -L -o "$TMP_PKG" "https://github.com/X547/nvidia-haiku/releases/download/v0.0.1/libglvnd-1.7.0-4-x86_64.hpkg"
		if [ -s "$TMP_PKG" ]; then
    		echo "Successfully downloaded to: $TMP_PKG"
    		read -p "Would you like to install this package now? (y/n): "
    		if [ "$REPLY" == "y" ]; then
       			pkgman install "$TMP_PKG"
       		rm "$TMP_PKG"
    		fi
			else
    			echo "Download failed!"
   		 		rm "$TMP_PKG"
		fi
	fi



	if pkgman search nebula | grep -q "nebula"; then
   		 echo "nebula found."
		else
   		 read -p "nebula not found. Download and install? y/n: " nebula
	fi
	if [[ "$nebula" == y ]];then
		TMP_PKG=$(mktemp /tmp/nvidia_driver.XXXXXX.hpkg)
		echo "Downloading nebula driver..."
		curl -L -o "$TMP_PKG" "https://github.com/X547/nvidia-haiku/releases/download/v0.0.2/nebula-0.0.2-1.x86_64.hpkg"
		if [ -s "$TMP_PKG" ]; then
    		echo "Successfully downloaded to: $TMP_PKG"
    		read -p "Would you like to install this package now? (y/n): " 
    		if [ "$REPLY" == "y" ]; then
       			pkgman install "$TMP_PKG"
       			rm "$TMP_PKG"
    		fi
			else
    			echo "Download failed!"
    			rm "$TMP_PKG"
		fi
	fi
fi


