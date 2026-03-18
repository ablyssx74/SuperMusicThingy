#!/bin/env bash

# HaikuHpkg
# Automated build script with user input required.
# Downloads git repos, builds, then builds Haiku hpkg file on Desktop.
# Copyright 2026 ablyss
# See the The MIT License included in this folder


# Define some vars for the script

appname="SuperMusicThingy"
description="SuperMusicThingy is a free streaming terminal media client for SomaFM. Fast, light, and fun!"

depends="haiku_devel pkgconfig cmake gcc mpv_devel curl_devel openssl3_devel nlohmann_json git"
projectmDir="/tmp/projectm"
supermusicthingyDir="/tmp/SuperMusicThingy"

# Define color codes
LIGHT_BLUE='\033[1;34m'
LIGHT_PURPLE='\033[1;35m'
NC='\033[0m' # No Color (Reset)

# Try here first else download
pathNebula="$HOME/Downloads/nebula-0.0.2-1.x86_64.hpkg"
pathlibGlvnd="$HOME/Downloads/libglvnd-1.7.0-4-x86_64.hpkg"

# Always start fresh
[[ -d ${supermusicthingyDir} ]] && rm -fr ${supermusicthingyDir}
[[ -d ${projectmDir} ]] && rm -fr ${projectmDir}

# Start

read -p "$(echo -e "${LIGHT_BLUE}${LIGHT_PURPLE}>>Option 1:${LIGHT_BLUE} Build ${LIGHT_PURPLE}${appname}Nebula${LIGHT_BLUE} with projectm visuals. 
${LIGHT_PURPLE}>>Requires${LIGHT_BLUE} libprojectm, Haiku nightly and a supported nvidia card with nebula (nvidia driver).
>>This script will try to automatically download libprojectm and nebula if not already installed.

${LIGHT_PURPLE}>>Option 2:${LIGHT_BLUE} Build ${LIGHT_PURPLE}${appname}${LIGHT_BLUE} without projectm and for normal Haiku beta5 release.
${LIGHT_PURPLE}>>Select Option: 1 or 2: ")"


	
if [[ "$REPLY" == "1" ]];then
	# Info if nebula
	ifNebula="\nAdd milk drop presets in settings\/SuperMusicThingy\/milk_presets\/"
	pkgman install ${depends} grep libsdl2_devel libx11_devel 
	appname="SuperMusicThingyNebula"
	requires=("haiku >= r1~beta5_hrev59183-1" "libglvnd >= 1.7.0-1" "nebula" "libsdl2")

elif [[ "$REPLY" == "2" ]];then
	
	pkgman install ${depends}
	requires=("haiku")
	buildspec="-DENABLE_PROJECTM=OFF -DENABLE_SDL2=OFF -DENABLE_GL=OFF"
	skipprojectm="true"

	else
		echo -e "${LIGHT_BLUE}Wrong Choice!"
		exit 1
fi


if [[ ! $skipprojectm ]];then
read -p "$(echo -e "${LIGHT_BLUE}${LIGHT_PURPLE}>>Option 1:${LIGHT_BLUE} Install projectm in /boot/home/config/non-packaged/ 
${LIGHT_PURPLE}>>Option 2:${LIGHT_BLUE} Don't install projectm because it is already installed in /boot/home/config/non-packaged/  
${LIGHT_PURPLE}>>Select Option: 1 or 2: ")" thisProjectm
fi

if [[ "$thisProjectm" == "2" ]];then 
	skipprojectm="true" 
	echo -e "${LIGHT_BLUE}"
	git clone https://github.com/ablyssx74/SuperMusicThingy.git ${supermusicthingyDir}
	cd ${supermusicthingyDir}
	mkdir -p ${supermusicthingyDir}/hpkgs/${appname}/apps
	mkdir -p ${supermusicthingyDir}/hpkgs/${appname}/bin
	mkdir -p ${supermusicthingyDir}/hpkgs/${appname}/data/${appname}/icon/
	mkdir -p ${supermusicthingyDir}/hpkgs/${appname}/data/mime_db/application
	mkdir -p ${supermusicthingyDir}/hpkgs/${appname}/data/deskbar/menu/Applications
	#[[ ! "$skipprojectm" ]] &&  mkdir -p ${supermusicthingyDir}/hpkgs/${appname}/data/projectm
	[[ "$skipprojectm" ]] && touch ${supermusicthingyDir}/hpkgs/${appname}/data/mime_db/application/x-vnd.${appname,,}
	[[ ! "$skipprojectm" ]] && touch ${supermusicthingyDir}/hpkgs/${appname}/data/mime_db/application/x-vnd.${appname,,}


elif [[ ! "$skipprojectm" && "$thisProjectm" == "1" ]];then 
		git clone https://github.com/projectM-visualizer/projectm.git ${projectmDir}
		cd ${projectmDir}
		git fetch --all --tags
		git submodule init
		git submodule update
		mkdir build
		cd build
		cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/boot/home/config/non-packaged ..
		cmake --build . -- -j && cmake --build . --target install 
		
		else
			echo -e "${LIGHT_BLUE}Wrong Choice!"
			exit 1
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

echo -n "resource app_flags B_SINGLE_LAUNCH;
resource app_signature \"application/x-vnd.${appname,,}\";

resource app_version {
    major  = 0,
    minor  = 1,
    middle = 0,
    variety = 0,
    internal = 0,
    short_info = \"A terminal music player\",
    long_info = \"${description}\"
};

resource vector_icon {
		$\"6E636966030400660500020016020000003C6000C000000000004C000048A000\"
        $\"0080FF280404043E404038403E4032402C2A322A262A2404043E402A382A3E2A\"
        $\"322A2C4032402640240A0340224032482A0A03403840484840090A0001011240\"
        $\"AAAA00000000000040AAAA44AAAA44AAAA01178822040A0001001240AAAA0000\"
        $\"0000000040AAAA44AAAA44AAAA01178822040A000202031240AAAA0000000000\"
        $\"0040AAAA44AAAA44AAAA01178402040A010202031240AAAA00000000000040AA\"
        $\"AA42AAAA42AAAA01178402040A0101001240AAAA00000000000040AAAA42AAAA\"
        $\"42AAAA01178822040A0201001240AAAA00000000000040AAAA42AAAA42AAAA01\"
        $\"178422040A0101011240AAAA00000000000040AAAA42AAAA42AAAA0117882204\"
        $\"0A0201011240AAAA00000000000040AAAA42AAAA42AAAA01178422040A020202\"
        $\"030240AAAA00000000000040AAAA42AAAA42AAAA\"
};
" > ${appname}.rdef 


cmake -B build_${appname} ${buildspec}
cmake --build build_${appname}    
rc -o ${appname}.rsrc ${appname}.rdef 
xres -o build_${appname}/SuperMusicThingy ${appname}.rsrc     
mv -f build_${appname}/SuperMusicThingy hpkgs/${appname}/apps/${appname}
mimeset -f hpkgs/${appname}/apps/${appname}
cp icons/icon_24px.png hpkgs/${appname}/data/${appname}/icon/
ln -sf /boot/system/apps/${appname} hpkgs/${appname}/bin/${appname}
ln -sf /boot/system/apps/${appname} hpkgs/${appname}/data/deskbar/menu/Applications/${appname}

cd ${supermusicthingyDir}/hpkgs/
package create -C ${appname} ${appname}.hpkg
mv ${supermusicthingyDir}/hpkgs/${appname}.hpkg $HOME/Desktop/${appname}.hpkg


[[ -d ${projectmDir} ]] && rm -fr ${projectmDir}
[[ -d ${supermusicthingyDir} ]] && rm -fr ${supermusicthingyDir}


if [[ "$thisProjectm" ]];then 
		# Find libglvnd
		if pkgman search libglvnd | grep -q "libglvnd"; then
			echo -e "${LIGHT_BLUE}>>>libglvnd found."
		else
			echo -e "${LIGHT_BLUE}${LIGHT_PURPLE}>>libglvnd${LIGHT_BLUE} not found. Installing..."
		fi
		
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
	
		# Find nebula
		if pkgman search nebula | grep -q "nebula"; then
   		 	echo -e "${LIGHT_BLUE}>>nebula found."
		else
			echo -e "${LIGHT_BLUE}${LIGHT_PURPLE}>>nebula${LIGHT_BLUE} not found. Installing..."
		fi
		
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
echo -e "${LIGHT_BLUE}>>>Finshed.${NC}"

