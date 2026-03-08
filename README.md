# Music Thingy
- c++ terminal app to play random music from somafm.com 

## Build Basic
Build requires libcurl and libmpv
```shell
#Haiku
g++ ./MusicThingy.cpp -o MusicThingy -lcurl -lmpv -lbe

#Linux
g++ ./MusicThingy.cpp -o MusicThingy -lcurl -lmpv 
```
## Build Super
Build requires libcurl, libmpv, libsdl2, libprojectM and GL
```shell

#Haiku
# libprojectM-4 installed in /boot/home/config/non-packaged/
g++ ./SuperMusicThingy.cpp -o SuperMusicThingy \
    $(pkg-config --cflags --libs sdl2) \
    -I/boot/home/config/non-packaged/include \
    -L/boot/home/config/non-packaged/lib \
    -lprojectM-4 -lmpv -lcurl -lGL -lopenal

#Linux
g++ ./SuperMusicThingy.cpp -o SuperMusicThingy \
-lprojectM-4 -lmpv -lcurl $(pkg-config --cflags --libs sdl2 gl)
```



