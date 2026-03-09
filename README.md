# SuperMusicThingy
- c++ terminal app to play random music from somafm.com 
- SuperMusicThingy has a projectM visualizer. 
- Tested on CachyOS and Haiku OS
- Haiku support is not 100% yet. The visuals don't jitter to the music, and require [nvidia-haiku](https://github.com/X547/nvidia-haiku ) and [projectM](https://github.com/projectM-visualizer/projectm)
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
