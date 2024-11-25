mkdir build
cd build
gcc -ggdb -Wall -Wextra -fsanitize=address,undefined -std=c11 -O2 -I../3rdparty/minifb/include/ \
    -I../3rdparty/minifb/src/ ../main.c ../3rdparty/ffmpeg/ffmpeg_linux.c -lX11 -lm -o main_linux.exe
cd ..
