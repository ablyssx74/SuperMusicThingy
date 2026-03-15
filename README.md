[![CMake on multiple platforms](https://github.com/ablyssx74/music_thingy/actions/workflows/cmake-multi-platform.yml/badge.svg)](https://github.com/ablyssx74/music_thingy/actions/workflows/cmake-multi-platform.yml)

# <p align="center"> <img width="48" height="48" align="top" alt="Screenshot" src="https://github.com/ablyssx74/SuperMusicThingy/blob/main/icon_24px.png" />SuperMusicThingy </p>
## <p align="center"> c++ terminal app to play random music from [SomaFM](https://somafm.com/)</p>
## <p align="center"> <img width="320" height="300" align="center" alt="Screenshot" src="https://github.com/user-attachments/assets/c1a46e06-ed56-4505-aff0-e4ef507f4801" /></p>
## Includes


-	 projectM milkdrop visualizer.
-  shuffle stations.
-	 save/delete/play favorites.
-	 optional notifications.
-	 fade in/out on song change. 
-	 config manger.
-	 a CLI API backend for sending keyboard shortcuts like vol_up, shuffle, status. See SuperMusicThingy --help for details.
-	 isolated volume control.
## Tested on CachyOS and Haiku OS
- Haiku with projectm visuals requires Haiku Nightly, a supported nvidia card, [libglvnd-1.7.0.04-haiku](https://github.com/X547/nvidia-haiku/releases/download/v0.0.1/libglvnd-1.7.0-4-x86_64.hpkg) and [nvidia-haiku](https://github.com/X547/nvidia-haiku).
## Presets 
-   Download from a huge selection of [presets](https://github.com/projectM-visualizer/projectm?tab=readme-ov-file#presets) and install in SuperMusicThingy config presets folder.


## Build SuperMusicThingy
```shell
#Download the source
git clone https://github.com/ablyssx74/SuperMusicThingy.git
cd SuperMusicThingy
```

```shell
# To build without projectm, sds2 and GL, and only libcurl and libmpv.
cmake -B build -DENABLE_PROJECTM=OFF -DENABLE_SDL2=OFF -DENABLE_GL=OFF
cmake --build build
```
```shell

# To build wtih projectm, sds2 and GL.
cmake -B build
cmake --build build
```

