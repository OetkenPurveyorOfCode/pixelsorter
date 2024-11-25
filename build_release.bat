mkdir build
pushd build
call clang -Wall -Wextra -std=c11 -O3 -I../3rdparty/minifb/include/ -I../3rdparty/minifb/src/ ../main.c ../3rdparty/ffmpeg/ffmpeg_windows.c -luser32 -lgdi32 -lwinmm -o main.exe
popd build
