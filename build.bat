
pushd build
call clang -g -Wall -Wextra -fsanitize=address,undefined -std=c11 -I../3rdparty/minifb/include/ -I../3rdparty/minifb/src/ ../main.c ../3rdparty/ffmpeg/ffmpeg_windows.c -luser32 -lgdi32 -lwinmm -o main.exe
popd build
