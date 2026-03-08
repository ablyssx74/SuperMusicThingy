# MusicThingy amd SuperMusicThingy
- c++ terminal app to play random music from somafm.com 
- SuperMusicThingy has a projectM visualizer. 
- Haiku support is not 100% yet. The visuals don't jitter to the music, and requires https://github.com/X547/nvidia-haiku and https://github.com/projectM-visualizer/projectm 
## Build MusicThingy
Build requires libcurl and libmpv
```shell
#Haiku
g++ ./MusicThingy.cpp -o MusicThingy -lcurl -lmpv -lbe
```
```shell
#Linux
g++ ./MusicThingy.cpp -o MusicThingy -lcurl -lmpv 
```
## Build SuperMusicThingy
Build requires libcurl, libmpv, libsdl2, libprojectM and GL
```shell

#Haiku
# libprojectM-4 installed in /boot/home/config/non-packaged/
g++ ./SuperMusicThingy.cpp -o SuperMusicThingy \
    $(pkg-config --cflags --libs sdl2) \
    -I/boot/home/config/non-packaged/include \
    -L/boot/home/config/non-packaged/lib \
    -lprojectM-4 -lmpv -lcurl -lGL -lopenal

```
```shell

#Linux
g++ ./SuperMusicThingy.cpp -o SuperMusicThingy \
-lprojectM-4 -lmpv -lcurl $(pkg-config --cflags --libs sdl2 gl)
```



