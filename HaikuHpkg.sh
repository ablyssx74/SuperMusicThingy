#!/usr/bin/env bash

# HaikuHpkg
# Automated build script with user input required.
# Downloads git repos, builds, then builds Haiku hpkg file on Desktop.
# Copyright 2026 ablyss
# See the The MIT License included in this folder


appname="SuperMusicThingy"
depends="haiku_devel pkgconfig cmake gcc mpv_devel curl_devel openssl3_devel nlohmann_json git"
projectDir="/tmp/projectm"
supermusicthingyDir="/tmp/SuperMusicThingy"

read -p "
Option 1: Build ${appname} with projectm visuals. 
Requires libprojectm, Haiku nightly and a supported nvidia card with nebula (nvidia driver).
This script will automatically download libprojectm and nebula if not installed.


Option 2: Build SuperMusicThingy without projectm and for normal Haiku beta5 release.  (select 1 or 2): "

if [[ "$REPLY" == "1" ]];then
	
	pkgman install ${depends} grep
	appname="SuperMusicThingyNebula"
	requires=("haiku >= r1~beta5_hrev59451-1" "libglvnd >= 1.7.0-1" "nebula" "libsdl2")

elif [[ "$REPLY" == "2" ]];then
	
	pkgman install ${depends}
	requires=("haiku")
	buildspec="-DENABLE_PROJECTM=OFF -DENABLE_SDL2=OFF -DENABLE_GL=OFF"
	skipprojectm="true"

	else
		exit 1
fi




if [[ ! -d "${supermusicthingyDir}" ]];then
	read -p "${appname} source not found. Download and install in ${supermusicthingyDir} y/n: " choice1
else
	read -p "${supermusicthingyDir} found. Deleteing this might help build problems. Delete and reinstall? y/n: " choice2
fi

	if [[ "$choice2" == "y" ]];then
		rm -fr ${supermusicthingyDir}
		choice1=y
	fi

if [[ "$choice1" == "y" ]];then	
	git clone https://github.com/ablyssx74/SuperMusicThingy.git ${supermusicthingyDir}
	cd ${supermusicthingyDir}
	mkdir -p ${supermusicthingyDir}/hpkgs/${appname}/apps
	mkdir -p ${supermusicthingyDir}/hpkgs/${appname}/bin
	mkdir -p ${supermusicthingyDir}/hpkgs/${appname}/data/mime_db/application
	mkdir -p ${supermusicthingyDir}/hpkgs/${appname}/data/deskbar/menu/Applications
	#[[ ! "$skipprojectm" ]] &&  mkdir -p ${supermusicthingyDir}/hpkgs/${appname}/data/projectm
	[[ "$skipprojectm" ]] && touch ${supermusicthingyDir}/hpkgs/${appname}/data/mime_db/application/x-vnd.supermusicthingy
	[[ ! "$skipprojectm" ]] && touch ${supermusicthingyDir}/hpkgs/${appname}/data/mime_db/application/x-vnd.supermusicthingynebula
fi

if [[ ! "$skipprojectm" ]];then 

	if [[ ! -d ${projectDir} ]];then
		read -p "Required ${projectDir}  source not found. Download, build add link to SuperMusicThingy? y/n: " choice1
	else
		read -p "${projectDir}  found. Deleteing this might help build problems. Delete and reinstall? y/n: " choice2
	fi
	if [[ "$choice2" == "y" ]];then
		rm -fr ${projectDir} 
		choice1=y
	fi

	if [[ "$choice1" == "y" ]];then
		pkgman install cmake libsdl2_devel libx11_devel
				git clone https://github.com/projectM-visualizer/projectm.git ${projectDir} 
		cd ${projectDir} 
		git fetch --all --tags
		git submodule init
		git submodule update
		mkdir build
		cd build
        #cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/boot/home/config/non-packaged ..
	
		cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=${supermusicthingyDir}/hpkgs/${appname}/ ..
		cmake --build . -- -j && cmake --build . --target install 
 fi
fi

if [[ ! -e ${supermusicthingyDir}/hpkgs/${appname}/.PackageInfo ]];then
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
" > ${supermusicthingyDir}/hpkgs/${appname}/.PackageInfo
	 
fi

    


cd ${supermusicthingyDir}
cmake -B build_${appname} ${buildspec}
cmake --build build_${appname}    
rc -o ${appname}.rsrc ${appname}.rdef 
xres -o build_${appname}/SuperMusicThingy ${appname}.rsrc     
mv -f build_${appname}/SuperMusicThingy hpkgs/${appname}/apps/${appname}
mimeset -f hpkgs/${appname}/apps/${appname}
ln -sf /boot/system/apps/${appname} hpkgs/${appname}/bin/${appname}
ln -sf /boot/system/apps/${appname} hpkgs/${appname}/data/deskbar/menu/Applications/${appname}


cd ${supermusicthingyDir}/hpkgs/
package create -C ${appname} ${appname}.hpkg
mv ${supermusicthingyDir}/hpkgs/${appname}.hpkg $HOME/Desktop/${appname}.hpkg

if [[ ! "$skipprojectm" ]];then 
	if [[ -d ${projectDir}  ]];then
		read -p "Delete ${projectDir}  source? y/n: "
	fi
	if [[ $REPLY == y ]];then
		rm -fr ${projectDir} 
	fi
fi

if [[ -d ${supermusicthingyDir} ]];then
	read -p "Delete ${supermusicthingyDir}  source? y/n: "
fi

if [[ "$REPLY" == "y" ]];then
	rm -fr ${supermusicthingyDir}
fi

if [[ ! "$skipprojectm" ]];then 
	if pkgman search libglvnd | grep -q "libglvnd"; then
    	echo "libglvnd found."
		else
    		read -p "libglvnd not found. Download and install? y/n: " glvnd
	fi
	if [[ "$glvnd" == "y" ]];then
		TMP_PKG=$(mktemp /tmp/libglvnd.XXXXXX.hpkg)
		echo "Downloading libglvnd..."
		curl -L -o "$TMP_PKG" "https://github.com/X547/nvidia-haiku/releases/download/v0.0.1/libglvnd-1.7.0-4-x86_64.hpkg"
		if [ -s "$TMP_PKG" ]; then
    		read -p "Download complete. Would you like to install this package now? (y/n): " 
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
	if [[ "$nebula" == "y" ]];then
		TMP_PKG=$(mktemp /tmp/nvidia_driver.XXXXXX.hpkg)
		echo "Downloading nebula driver..."
		curl -L -o "$TMP_PKG" "https://github.com/X547/nvidia-haiku/releases/download/v0.0.2/nebula-0.0.2-1.x86_64.hpkg"
		if [ -s "$TMP_PKG" ]; then
    		read -p "Download complete. Would you like to install this package now? (y/n): " 
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
