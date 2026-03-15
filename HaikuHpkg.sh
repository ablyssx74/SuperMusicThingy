#!/usr/bin/env bash

# HaikuHpkg
# Automated build script with user input required.
# Downloads git repos, builds, then builds Haiku hpkg file on Desktop.
# Copyright 2026 ablyss
# See the The MIT License included in this folder


# Define some vars for the script

appname="SuperMusicThingy"
description="SuperMusicThingy is a free streaming terminal media client for SomaFM. Fast, light, and fun!"

depends="haiku_devel pkgconfig cmake gcc mpv_devel curl_devel openssl3_devel nlohmann_json git"
projectDir="/tmp/projectm"
supermusicthingyDir="/tmp/SuperMusicThingy"

# Define color codes
LIGHT_BLUE='\033[1;34m'
LIGHT_PURPLE='\033[1;35m'
NC='\033[0m' # No Color (Reset)

# Try here first else download
pathNebula="$HOME/Downloads/nebula-0.0.2-1.x86_64.hpkg"
pathlibGlvnd="$HOME/Downloads/libglvnd-1.7.0-4-x86_64.hpkg"

# Start

read -p "$(echo -e "${LIGHT_BLUE}${LIGHT_PURPLE}>>Option 1:${LIGHT_BLUE} Build ${LIGHT_PURPLE}${appname}Nebula${LIGHT_BLUE} with projectm visuals. 
${LIGHT_PURPLE}>>Requires${LIGHT_BLUE} libprojectm, Haiku nightly and a supported nvidia card with nebula (nvidia driver).
>>This script will try to automatically download libprojectm and nebula if not already installed.

${LIGHT_PURPLE}>>Option 2:${LIGHT_BLUE} Build ${LIGHT_PURPLE}${appname}${LIGHT_BLUE} without projectm and for normal Haiku beta5 release.
${LIGHT_PURPLE}>>Select Option: 1 or 2: ")"

if [[ "$REPLY" == "1" ]];then
	# Info if nebula
	ifNebula="\nAdd milk drop presets in settings\/SuperMusicThingy\/milk_presets\/"
	pkgman install ${depends} grep libsdl2_devel
	appname="SuperMusicThingyNebula"
	requires=("haiku >= r1~beta5_hrev59183-1" "libglvnd >= 1.7.0-1" "nebula" "libsdl2")

elif [[ "$REPLY" == "2" ]];then
	
	pkgman install ${depends}
	requires=("haiku")
	buildspec="-DENABLE_PROJECTM=OFF -DENABLE_SDL2=OFF -DENABLE_GL=OFF"
	skipprojectm="true"

	else
		exit 1
fi

if [[ ! $skipprojectm ]];then
read -p "$(echo -e "${LIGHT_BLUE}${LIGHT_PURPLE}>>Option 1:${LIGHT_BLUE} Install projectm in /boot/home/config/non-packaged/ 
${LIGHT_PURPLE}>>Option 2:${LIGHT_BLUE} Don't install projectm because it is already installed in /boot/home/config/non-packaged/  
${LIGHT_PURPLE}>>Select Option: 1 or 2: ")" thisProjectm
fi
[[ "$thisProjectm" == "2" ]] && skipprojectm="true" 


[[ -d ${supermusicthingyDir} ]] && rm -fr ${supermusicthingyDir}


if [[ ! -d "${supermusicthingyDir}" ]];then
	echo -e "${LIGHT_BLUE}"
	git clone https://github.com/ablyssx74/SuperMusicThingy.git ${supermusicthingyDir}
	cd ${supermusicthingyDir}
	mkdir -p ${supermusicthingyDir}/hpkgs/${appname}/apps
	mkdir -p ${supermusicthingyDir}/hpkgs/${appname}/bin
	mkdir -p ${supermusicthingyDir}/hpkgs/${appname}/data/${appname}/icon/
	mkdir -p ${supermusicthingyDir}/hpkgs/${appname}/data/mime_db/application
	mkdir -p ${supermusicthingyDir}/hpkgs/${appname}/data/deskbar/menu/Applications
	#[[ ! "$skipprojectm" ]] &&  mkdir -p ${supermusicthingyDir}/hpkgs/${appname}/data/projectm
	[[ "$skipprojectm" ]] && touch ${supermusicthingyDir}/hpkgs/${appname}/data/mime_db/application/x-vnd.supermusicthingy
	[[ ! "$skipprojectm" ]] && touch ${supermusicthingyDir}/hpkgs/${appname}/data/mime_db/application/x-vnd.supermusicthingynebula
fi

if [[ ! "$skipprojectm" && "$thisProjectm" == "1" ]];then 

	if [[ ! -d ${projectDir} ]];then
		read -p "$(echo -e "${LIGHT_BLUE}${LIGHT_PURPLE}>>>Required ${projectDir}${LIGHT_BLUE} source not found. Download, build add link to SuperMusicThingy? y/n: ")" choice1
	else
		read -p "$(echo -e "${LIGHT_BLUE}${LIGHT_PURPLE}>>${projectDir}${LIGHT_BLUE} found. Deleteing this might help build problems. Delete and reinstall? y/n: ")" choice2
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
		cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/boot/home/config/non-packaged ..
		cmake --build . -- -j && cmake --build . --target install 
 fi
fi

if [[ ! -e ${supermusicthingyDir}/hpkgs/${appname}/.PackageInfo ]];then
echo -n "name	${appname}
version			1.0.0-1
architecture	x86_64
summary			\"Portable streaming terminal media client for SomaFM\"
description		\"${description} ${ifNebula}\"

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
cp icon_24px.png hpkgs/${appname}/data/${appname}/icon/
ln -sf /boot/system/apps/${appname} hpkgs/${appname}/bin/${appname}
ln -sf /boot/system/apps/${appname} hpkgs/${appname}/data/deskbar/menu/Applications/${appname}

cd ${supermusicthingyDir}/hpkgs/
package create -C ${appname} ${appname}.hpkg
mv ${supermusicthingyDir}/hpkgs/${appname}.hpkg $HOME/Desktop/${appname}.hpkg

if [[ "$thisProjectm" == "1" ]];then 
	if [[ -d ${projectDir}  ]];then
		read -p "$(echo -e "${LIGHT_BLUE}>>Delete ${LIGHT_PURPLE}${projectDir}${LIGHT_BLUE} source? y/n: ")"
	fi
	if [[ $REPLY == y ]];then
		rm -fr ${projectDir} 
	fi
fi

#if [[ -d ${supermusicthingyDir} ]];then
#	read -p "$(echo -e "${LIGHT_BLUE}>>Delete ${LIGHT_PURPLE}${supermusicthingyDir}${LIGHT_BLUE} source? y/n: ")"
#fi
[[ -d ${supermusicthingyDir} ]] && rm -fr ${supermusicthingyDir}
#if [[ "$REPLY" == "y" ]];then
#	rm -fr ${supermusicthingyDir}
#fi

if [[ "$thisProjectm" ]];then 

		# Find libglvnd
		if pkgman search libglvnd | grep -q "libglvnd"; then
			echo -e "${LIGHT_BLUE}>>>libglvnd found."
		else
			read -p "$(echo -e "${LIGHT_BLUE}${LIGHT_PURPLE}>>libglvnd${LIGHT_BLUE} not found. Download/Install? y/n: ")" glvnd
	fi
	
	if [[ "$glvnd" == "y" ]];then
			if [[ -e "$pathlibGlvnd" ]];then
				echo -e "${LIGHT_BLUE}"
				pkgman install "$pathlibGlvnd"				
			else
				TMP_PKG=$(mktemp /tmp/libglvnd.XXXXXX.hpkg)
				echo -e "${LIGHT_BLUE}>>Installing libglvnd..."
				curl -L -o "$TMP_PKG" "https://github.com/X547/nvidia-haiku/releases/download/v0.0.1/libglvnd-1.7.0-4-x86_64.hpkg"
				if [ -s "$TMP_PKG" ]; then
					echo -e "${LIGHT_BLUE}"
       				pkgman install "$TMP_PKG" -y
       				rm "$TMP_PKG"    		
					else
    					echo -e "${LIGHT_BLUE}Download failed!"
   		 				rm "$TMP_PKG"
				fi
			fi	
	fi	
	
	# Find nebula
	if pkgman search nebula | grep -q "nebula"; then
   		 echo -e "${LIGHT_BLUE}>>nebula found."
		else
			read -p "$(echo -e "${LIGHT_BLUE}${LIGHT_PURPLE}>>nebula${LIGHT_BLUE} not found. Download/Install? y/n: ")" nebula
	fi
	if [[ "$nebula" == "y" ]];then
		if [[ -e "$pathNebula" ]];then
				echo -e "${LIGHT_BLUE}"
				pkgman install "$pathNebula"				
			else
				TMP_PKG=$(mktemp /tmp/nvidia_driver.XXXXXX.hpkg)
		
				echo -e "${LIGHT_BLUE}>>Installing nebula driver..."
				curl -L -o "$TMP_PKG" "https://github.com/X547/nvidia-haiku/releases/download/v0.0.2/nebula-0.0.2-1.x86_64.hpkg"
				if [ -s "$TMP_PKG" ]; then
					echo -e "${LIGHT_BLUE}"
       				pkgman install "$TMP_PKG"
       				rm "$TMP_PKG"    		
					else
    					echo -e "${LIGHT_BLUE}Download failed!"
    					rm "$TMP_PKG"
				fi
		fi		
	fi
	
	
fi
echo -e "${LIGHT_BLUE}>>>Finshed.${NC}"
