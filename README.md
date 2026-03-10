[![CMake on multiple platforms](https://github.com/ablyssx74/music_thingy/actions/workflows/cmake-multi-platform.yml/badge.svg)](https://github.com/ablyssx74/music_thingy/actions/workflows/cmake-multi-platform.yml)

# <p align="center"> SuperMusicThingy </p>
## <p align="center"> c++ terminal app to play random music from [SomaFM](https://somafm.com/)</p>
## <p align="center"> <img width="320" height="300" align="center" alt="Screenshot" src="https://github.com/user-attachments/assets/d8fb1f90-a787-4c04-9035-ad499ce2734c" /></p>
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
- Haiku support is not 100% yet. The visuals don't jitter to the music ( maybe my soundcard! ) and require [nvidia-haiku](https://github.com/X547/nvidia-haiku).
## Presets 
-   Download from a huge selection of [presets](https://github.com/projectM-visualizer/projectm?tab=readme-ov-file#presets) and install in SuperMusicThingy config presets folder.

## Build SuperMusicThingy


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

