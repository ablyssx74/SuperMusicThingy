# SuperMusicThingy
- c++ terminal app to play random music from somafm.com 
- SuperMusicThingy has a projectM visualizer. 
- Tested on CachyOS and Haiku OS
- Haiku support is not 100% yet. The visuals don't jitter to the music, and require https://github.com/X547/nvidia-haiku and https://github.com/projectM-visualizer/projectm 
## Build SuperMusicThingy
Build requires libcurl and libmpv
```shell

# To build without projectm, sds2 and GL support.
cmake -B build -DENABLE_PROJECTM=OFF -DENABLE_SDL2=OFF -DENABLE_GL=OFF
cmake --build build
```
```shell
# To build wtih projectm, sds2 and GL support
cmake -B build
cmake --build build
```




