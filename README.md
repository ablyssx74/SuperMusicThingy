[![CMake on multiple platforms](https://github.com/ablyssx74/music_thingy/actions/workflows/cmake-multi-platform.yml/badge.svg)](https://github.com/ablyssx74/music_thingy/actions/workflows/cmake-multi-platform.yml)

## <p align="center"> <img width="48" height="48" align="top" alt="icon" src="https://github.com/user-attachments/assets/40b6432d-33e5-4946-8eed-9f0508000b23" />SuperMusicThingy </p>
### <p align="center"> SuperMusicThingy is a free streaming terminal media client for [SomaFM](https://somafm.com/)<br> Fast, light, and fun! </p>
#### <p align="center">Screenshots
<p align="center">&nbsp;&nbsp;&nbsp; Konsole &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Terminology &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; &nbsp;&nbsp;&nbsp;&nbsp; Haiku &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
<br><img width="200" height="180" align="center" alt="Screenshot1" src="https://github.com/user-attachments/assets/cfdaf54e-b502-4317-89b9-365cdd11dd6c" /><img width="180" height="160" align="center" alt="Screenshot2" src="https://github.com/user-attachments/assets/7661c7c9-f107-4485-ac68-a593bb09ca9e" /><img width="200" height="180" align="center" alt="Screenshot3" src="https://github.com/user-attachments/assets/d3bfab0e-8bcf-4b9f-a9d5-dfbb1363796f" /></p>

### Includes


-	 projectM milkdrop visualizer.
-    shuffle stations.
-	 save/delete/play favorites.
-	 optional notifications.
-	 fade in/out on song change. 
-	 config manger.
-	 a CLI API backend for sending keyboard shortcuts like vol_up, shuffle, status.
### Tested on CachyOS and Haiku OS x86_64

### Presets 
-   Download from a huge selection of [presets](https://github.com/projectM-visualizer/projectm?tab=readme-ov-file#presets) and install in SuperMusicThingy config presets folder.<br> I recommend [projectm_presets](http://spiegelmc.com/pub/projectm_presets.zip).
  


### Build Latest SuperMusicThingy
```shell
#Download the source
git clone https://github.com/ablyssx74/SuperMusicThingy.git
cd SuperMusicThingy
```
```shell

# To build wtih projectm
# Default install prefix is /usr/local/bin
cmake -B build
cmake --build build
sudo cmake --install build
```
```shell
# To build without projectm visualizer
# Default install prefix is /usr/local/bin
cmake -B build -DENABLE_PROJECTM=OFF -DENABLE_SDL2=OFF -DENABLE_GL=OFF
cmake --build build
sudo cmake --install build
```
### Preferred Method For Haiku
-	 This will create SuperMusicThingy.hpkg package on the Desktop<br>
-	 Download: [HaikuHpkg.zip](https://github.com/ablyssx74/SuperMusicThingy/releases/download/v1.0.0/HaikuHpkg.zip) <br>
-	 If building without projectm visuals select option 2 after running ./HaikuHpkg.sh <br>
-	 If building with projectm select option 1. <br>
-	 Visuals require [Haiku Nightly](https://download.haiku-os.org/nightly-images/x86_64/), a Turing+ GPU supported Nvidia card, [libglvnd-1.7.0-4-x86_64.hpkg](https://github.com/X547/nvidia-haiku/releases/download/v0.0.1/libglvnd-1.7.0-4-x86_64.hpkg) and [nebula-0.0.2-1.x86_64.hpkg](https://github.com/X547/nvidia-haiku/releases/download/v0.0.2/nebula-0.0.2-1.x86_64.hpkg). 
```shell
cd Haiku
bash ./HaikuHpkg.sh
```

