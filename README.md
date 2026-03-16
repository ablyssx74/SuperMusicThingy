[![CMake on multiple platforms](https://github.com/ablyssx74/music_thingy/actions/workflows/cmake-multi-platform.yml/badge.svg)](https://github.com/ablyssx74/music_thingy/actions/workflows/cmake-multi-platform.yml)

# <p align="center"> <img width="48" height="48" align="top" alt="Screenshot" src="https://github.com/ablyssx74/SuperMusicThingy/blob/main/icon_24px.png" />SuperMusicThingy </p>
## <p align="center"> SuperMusicThingy is a free streaming terminal media client for [SomaFM](https://somafm.com/).<br> Fast, light, and fun! </p>
## <p align="center"> <img width="320" height="300" align="center" alt="Screenshot" src="https://github.com/user-attachments/assets/70a3adb1-d92a-4742-8406-a645bbe8823b" /><img width="320" height="300" align="center" alt="Screenshot" src="https://github.com/user-attachments/assets/7bb3f158-8077-402a-9537-d9111f23ad3a" /></p>
## Includes


-	 projectM milkdrop visualizer.
-  shuffle stations.
-	 save/delete/play favorites.
-	 optional notifications.
-	 fade in/out on song change. 
-	 config manger.
-	 a CLI API backend for sending keyboard shortcuts like vol_up, shuffle, status. See SuperMusicThingy --help for details.
-	 isolated volume control.
## Tested on CachyOS and Haiku OS x86_64
- Haiku with projectm visuals requires [Haiku Nightly](https://download.haiku-os.org/nightly-images/x86_64/), a Turing+ GPU supported Nvidia card, [libglvnd-1.7.0-4-x86_64.hpkg](https://github.com/X547/nvidia-haiku/releases/download/v0.0.1/libglvnd-1.7.0-4-x86_64.hpkg) and [nebula-0.0.2-1.x86_64.hpkg](https://github.com/X547/nvidia-haiku/releases/download/v0.0.2/nebula-0.0.2-1.x86_64.hpkg).
## Presets 
-   Download from a huge selection of [presets](https://github.com/projectM-visualizer/projectm?tab=readme-ov-file#presets) and install in SuperMusicThingy config presets folder.<br> I recommend [projectm_presets](http://spiegelmc.com/pub/projectm_presets.zip).
  


## Build Latest SuperMusicThingy
```shell
#Download the source
git clone https://github.com/ablyssx74/SuperMusicThingy.git
cd SuperMusicThingy
```

```shell
# To build without projectm, sds2 and GL, and only libcurl and libmpv.
# Default install prefix is /usr/local/bin
cmake -B build -DENABLE_PROJECTM=OFF -DENABLE_SDL2=OFF -DENABLE_GL=OFF
cmake --build build
sudo cmake --install build
```
```shell

# To build wtih projectm, sds2 and GL.
# Default install prefix is /usr/local/bin
cmake -B build
cmake --build build
sudo cmake --install build
```

