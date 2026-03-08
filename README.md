# Music Thingy
- a c++ terminal app to play random music from somafm.com 

# Build Basic
Build requires libcurl and libmpv
```shell
Haiku: g++ ./MusicThingy.cpp -o MusicThingy -lcurl -lmpv -lbe
Linux: g++ ./MusicThingy.cpp -o MusicThingy -lcurl -lmpv 
```
# Build Super
Build requires libcurl, libmpv, libprojectM. and GL
```shell
Haiku: g++ ./Super\ Music\ Thingy -o MusicThingy -lcurl -lmpv -lbe
Linux: g++ ./Super\ Music\ Thingy.cpp -o MusicThingy -lprojectM-4 -lmpv -lcurl     $(pkg-config --cflags --libs sdl2 gl)


