# SuperMusicThingy
<p align="center">
## c++ terminal app to play random music from [SomaFM](https://somafm.com/)
## <img width="320" height="300" align="center" alt="Screenshot" src="https://github.com/user-attachments/assets/6334a669-472f-4d92-8841-0542d56f06d6" /></p>
## Includes
-	 projectM milkdrop visualizer.
-    shuffle stations.
-	 save/delete/play favorites.
-	 config manger.
-	 a backend for sending keyboard shortcuts like vol_up, shuffle, status, --help
-	 isoalted volume control.
## Tested on CachyOS and Haiku OS
- Haiku support is not 100% yet. The visuals don't jitter to the music, and require [nvidia-haiku](https://github.com/X547/nvidia-haiku) and building [projectM](https://github.com/projectM-visualizer/projectm) from source.
## Presets 
-   Download from a huge selection of [presets](https://github.com/projectM-visualizer/projectm?tab=readme-ov-file#presets) and intall in SuperMusicThingy config presets folder.

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

