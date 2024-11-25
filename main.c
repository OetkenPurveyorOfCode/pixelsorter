
#if defined(__linux__)
#define _POSIX_C_SOURCE 199309L
#else
#define _CRT_SECURE_NO_WARNINGS (1)
#endif 

#include <time.h>

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <memory.h>
#include <inttypes.h>
#include <math.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wall"
#pragma GCC diagnostic ignored "-Wextra"
#pragma GCC diagnostic ignored "-Wswitch"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#define STB_IMAGE_IMPLEMENTATION
#include "3rdparty/stb_image/stb_image.h"


#include "3rdparty/minifb/include/MiniFB.h"
#include "3rdparty/minifb/src/MiniFB_common.c"
#include "3rdparty/minifb/src/MiniFB_internal.c"
#include "3rdparty/minifb/src/MiniFB_timer.c"
#include "3rdparty/minifb/src/gl/MiniFB_GL.c"

#if defined(_WIN32)
#include <windows.h>
#include "3rdparty/minifb/src/windows/WindowData_Win.h"
#include "3rdparty/minifb/src/windows/WinMiniFB.c"
#elif  defined(__linux__)
#include <time.h>
#include "3rdparty/minifb/src/MiniFB_linux.c"
#include "3rdparty/minifb/src/X11/WindowData_X11.h"
#include "3rdparty/minifb/src/X11/X11MiniFB.c"
#include <linux/limits.h>
#define MAX_PATH PATH_MAX
#else 
#error "unsupported platform"
#endif 

#include "3rdparty/pcg/pcg_basic.c"
#include "3rdparty/pcg/pcg_basic.h"
#include "3rdparty/gilbert/gilbert.c"
#include "3rdparty/ffmpeg/ffmpeg.h"
#pragma GCC diagnostic pop


#include "util/atoi.h"
#include "util/array.h"
#include "util/shuffle.h"
#include "sorts/mergesort.h"
#include "sorts/insertionsort.h"
#include "sorts/quicksort.h"
#include "sorts/selectionsort.h" 
#include "sorts/shellsort.h" 
#include "sorts/heapsort.h"
#include "sorts/bubblesort.h"
#include "sorts/oddevensort.h"
#include "sorts/circlesort.h"

#ifndef MAX_PATH
#define MAX_PATH 4096
#endif

typedef ptrdiff_t ssize_t;

typedef struct {
    size_t index;
    uint8_t pixel[4];
} Pixel;

typedef struct {
    int x;
    int y;
} Mapping;

typedef Ordering (*cmp_cb)(void* userdata_void, void* elem1, void* elem2);


void shuffle_pixels(Pixel* pixels, size_t len) {
    for (size_t i = len-1; i >= 1; i--) {
        uint32_t j = pcg32_boundedrand(i+1);
        Pixel tmp = pixels[i];
        pixels[i] = pixels[j];
        pixels[j] = tmp;
    }
}

bool is_sorted(Pixel* pixels, size_t len) {
    for (size_t i = 0; i < len -1; i++) {
        if (pixels[i].index > pixels[i+1].index) {
            printf("at p[%zu] < p[%zu], p has len %zu, index cmp values %zu before %zu\n", i, i+1, len, pixels[i].index, pixels[i+1].index);
            return false;
        }
    }
    return true;
}


void pixels_move_left(Pixel* pixels, size_t len, size_t shift) {
    Pixel* tmp = malloc(sizeof(Pixel)*shift);
    memcpy(tmp, pixels, shift*sizeof(Pixel));
    memmove(pixels, pixels+shift, (len-shift)*sizeof(Pixel));
    memcpy(pixels+(len-shift), tmp, shift*sizeof(Pixel));
}

void init_pixels_hilbert(uint8_t* image, Pixel* pixels, int width, int height, int channels, Mapping* mapping) {
    for(int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int index = gilbert_xy2d(x, y, width, height);
            int i = x+y*width;
            pixels[index] = (Pixel){
                .index=index, 
                .pixel={
                    image[i*channels+0],
                    image[i*channels+1],
                    image[i*channels+2],
                    image[i*channels+3],
                }
            };
            mapping[index] = (Mapping){x, y};
        }
    }
}

void init_pixels_linear(uint8_t* image, Pixel* pixels, int width, int height, int channels, Mapping* mapping) {
    for(ptrdiff_t index = 0; index < width*height; index++) {
        pixels[index] = (Pixel){
            .index=index, 
            .pixel={
                image[index*channels+0],
                image[index*channels+1],
                image[index*channels+2],
                image[index*channels+3],
            }
        };
        mapping[index]=(Mapping){index % width, index / width};
    }
}


typedef enum {
    SPIRALSTATE_EAST,
    SPIRALSTATE_SOUTH,
    SPIRALSTATE_WEST,
    SPIRALSTATE_NORTH
} SpiralState;


void init_pixels_spiral(uint8_t* image, Pixel* pixels, int width, int height, int channels, Mapping* mapping) {
    int x = 0;
    int y = 0;
    assert(width > 2);
    assert(height > 2);
    int x_min = 0;
    int x_max = width;
    int y_min = 0;
    int y_max = height;
    SpiralState spiral_state = SPIRALSTATE_EAST;
    for(ptrdiff_t index = 0; index < width*height; index++) {
        //printf("%zu %d %d\n", index, x, y);
        assert(0 <= x && x < width);
        assert(0 <= y && y < height);
        int i = x+y*width;
        pixels[index] = (Pixel){
            .index=index, 
            .pixel={
                image[i*channels+0],
                image[i*channels+1],
                image[i*channels+2],
                image[i*channels+3],
            }
        };
        mapping[index] = (Mapping){x, y};
        if (index == width*height-1) {
            break;
        }
        switch (spiral_state) {
            break;case SPIRALSTATE_EAST: {
                if (x+1< x_max) {
                    x +=1;
                }
                else {
                    assert(y + 1 < y_max);
                    y += 1;
                    y_min += 1;
                    spiral_state = SPIRALSTATE_SOUTH;
                }
            }
            break;case SPIRALSTATE_SOUTH: {
                if (y+1< y_max) {
                    y +=1;
                }
                else {
                    assert(x - 1 >= x_min);
                    x -= 1;
                    x_max -= 1;
                    spiral_state = SPIRALSTATE_WEST;
                }
            }
            break;case SPIRALSTATE_WEST: {
                if (x-1>= x_min) {
                    x -= 1;
                }
                else {
                    assert(y - 1 >= y_min);
                    y -= 1;
                    y_max -= 1;
                    spiral_state = SPIRALSTATE_NORTH;
                }
            }
            break;case SPIRALSTATE_NORTH: {
                if (y-1 >= y_min) {
                    y -= 1;
                }
                else {
                    assert(x + 1 < x_max);
                    x += 1;
                    x_min += 1;
                    spiral_state = SPIRALSTATE_EAST;
                }
            }
        }
    }
}

void init_pixels_random(uint8_t* image, Pixel* pixels, int width, int height, int channels, Mapping* mapping) {
    size_t* indices = malloc(sizeof(size_t)*width*height);
    for (size_t i = 0; i < (size_t)(width*height); i++) {
        indices[i] = i;
    }
    for (size_t i = width*height-1; i >= 1; i--) {
        uint32_t j = pcg32_boundedrand(i+1);
        size_t tmp = indices[i];
        indices[i] = indices[j];
        indices[j] = tmp;
    }
    for(int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int i = x+y*width;
            int index = indices[i];
            pixels[index] = (Pixel){
                .index=index, 
                .pixel={
                    image[i*channels+0],
                    image[i*channels+1],
                    image[i*channels+2],
                    image[i*channels+3],
                }
            };
            mapping[index] = (Mapping){x, y};
        }
    }
    free(indices);
}

typedef struct {
    int x;
    int y;
    int width;
    int height;
} BlockysParams;

typedef struct {
    BlockysParams* data;
    size_t len;
    size_t capacity;
} BlockysStack;

void init_pixels_blockys(uint8_t* image, Pixel* pixels, int width, int height, int channels, Mapping* mapping) {
    BlockysParams* permutation_buffer = malloc(sizeof(BlockysParams)*4);
    BlockysStack stack = {0};
    append(&stack, ((BlockysParams){.x = 0, .y = 0, .width = width, .height = height}));
    ptrdiff_t index = 0;
    for (;;) {
        if (stack.len == 0) {
            break;
        }
        //printf("index %zu %d\n", index, width*height);
        BlockysParams params = stack.data[stack.len-1];
        stack.len -= 1;
        //printf("params %d %d %d %d\n", params.x, params.y, params.width, params.height);
        if (params.width >= 2 && params.height >= 2) {
            int new_width, new_width2;
            if (params.width % 2 == 0) {
                new_width = params.width / 2;
                new_width2 = new_width;
            }
            else {
                new_width = params.width/2;
                new_width2 = new_width +1;
            }
            //printf("%d %d %d\n", new_width2, new_width, params.width);
            assert(new_width2 + new_width == params.width);
            int new_height, new_height2;
            if (params.height % 2 == 0) {
                new_height = params.height / 2;
                new_height2 = new_height;
            }
            else {
                new_height = params.height/2;
                new_height2 = new_height + 1;
            }
            assert(new_height2 + new_height == params.height);
            BlockysParams top_left     = {.x = params.x,                  .y = params.y,                   .width = new_width, .height= new_height};
            BlockysParams top_right    = {.x = params.x + params.width/2, .y = params.y,                   .width = new_width2, .height= new_height};
            BlockysParams bottom_left  = {.x = params.x,                  .y = params.y + params.height/2, .width = new_width, .height= new_height2};
            BlockysParams bottom_right = {.x = params.x + params.width/2, .y = params.y + params.height/2, .width = new_width2, .height= new_height2};
            permutation_buffer[0] = top_left;
            permutation_buffer[1] = top_right;
            permutation_buffer[2] = bottom_left;
            permutation_buffer[3] = bottom_right;
            shuffle(permutation_buffer, sizeof(BlockysParams), 4);
            for (size_t i = 0; i < 4; i++) {
                //printf("pushed %d %d %d %d\n", permutation_buffer[i].x, permutation_buffer[i].y, permutation_buffer[i].width, permutation_buffer[i].height);
                append(&stack, permutation_buffer[i]);
            }
        }
        else if (params.height == 1 && params.width > 0) {
            for (size_t dx = 0; dx < (size_t)params.width; dx++) {
                ptrdiff_t i = (params.x+dx) + params.y*width;
                //printf("%lld\n", i);
                assert(index < width*height);
                assert(i < width*height);
                pixels[index] = (Pixel){
                    .index = index,
                    .pixel={
                        image[i*channels+0],
                        image[i*channels+1],
                        image[i*channels+2],
                        image[i*channels+3],
                    }
                };
                mapping[index] = (Mapping){params.x+dx, params.y};
                index += 1;
            }
        } else if (params.width == 1 && params.height > 0) {
            for (size_t dy = 0; dy < (size_t)params.height; dy++) {
                ptrdiff_t i = params.x + (params.y+dy)*width;
                //printf("%lld\n", i);
                assert(index < width*height);
                assert(i < width*height);
                pixels[index] = (Pixel){
                    .index = index,
                    .pixel={
                        image[i*channels+0],
                        image[i*channels+1],
                        image[i*channels+2],
                        image[i*channels+3],
                    }
                };
                mapping[index] = (Mapping){params.x, params.y+dy};
                index += 1;
            }
        }
        else {
            //printf("0 encountered\n");
        }
    }
    //printf("i %d %d\n", index, width*height);
    for (size_t li = 0; li < width*height; li++) {
        //printf("i %d --> %d %d\n", li, mapping[li].x, mapping[li].y);
    }
    //printf("inited");
    free(permutation_buffer);
}

void init_pixels_blockys_regular(uint8_t* image, Pixel* pixels, int width, int height, int channels, Mapping* mapping) {
    BlockysStack stack = {0};
    append(&stack, ((BlockysParams){.x = 0, .y = 0, .width = width, .height = height}));
    ptrdiff_t index = 0;
    for (;;) {
        if (stack.len == 0) {
            break;
        }
        //printf("index %zu %d\n", index, width*height);
        BlockysParams params = stack.data[stack.len-1];
        stack.len -= 1;
        //printf("params %d %d %d %d\n", params.x, params.y, params.width, params.height);
        if (params.width >= 2 && params.height >= 2) {
            int new_width, new_width2;
            if (params.width % 2 == 0) {
                new_width = params.width / 2;
                new_width2 = new_width;
            }
            else {
                new_width = params.width/2;
                new_width2 = new_width +1;
            }
            //printf("%d %d %d\n", new_width2, new_width, params.width);
            assert(new_width2 + new_width == params.width);
            int new_height, new_height2;
            if (params.height % 2 == 0) {
                new_height = params.height / 2;
                new_height2 = new_height;
            }
            else {
                new_height = params.height/2;
                new_height2 = new_height + 1;
            }
            assert(new_height2 + new_height == params.height);
            BlockysParams top_left     = {.x = params.x,                  .y = params.y,                   .width = new_width, .height= new_height};
            BlockysParams top_right    = {.x = params.x + params.width/2, .y = params.y,                   .width = new_width2, .height= new_height};
            BlockysParams bottom_left  = {.x = params.x,                  .y = params.y + params.height/2, .width = new_width, .height= new_height2};
            BlockysParams bottom_right = {.x = params.x + params.width/2, .y = params.y + params.height/2, .width = new_width2, .height= new_height2};
            append(&stack, top_left);
            append(&stack, top_right);
            append(&stack, bottom_left);
            append(&stack, bottom_right);
        }
        else if (params.height == 1 && params.width > 0) {
            for (size_t dx = 0; dx < (size_t)params.width; dx++) {
                ptrdiff_t i = (params.x+dx) + params.y*width;
                //printf("%lld\n", i);
                assert(index < width*height);
                assert(i < width*height);
                pixels[index] = (Pixel){
                    .index = index,
                    .pixel={
                        image[i*channels+0],
                        image[i*channels+1],
                        image[i*channels+2],
                        image[i*channels+3],
                    }
                };
                mapping[index] = (Mapping){params.x+dx, params.y};
                index += 1;
            }
        } else if (params.width == 1 && params.height > 0) {
            for (size_t dy = 0; dy < (size_t)params.height; dy++) {
                ptrdiff_t i = params.x + (params.y+dy)*width;
                //printf("%lld\n", i);
                assert(index < width*height);
                assert(i < width*height);
                pixels[index] = (Pixel){
                    .index = index,
                    .pixel={
                        image[i*channels+0],
                        image[i*channels+1],
                        image[i*channels+2],
                        image[i*channels+3],
                    }
                };
                mapping[index] = (Mapping){params.x, params.y+dy};
                index += 1;
            }
        }
        else {
            //printf("0 encountered\n");
        }
    }
    //printf("i %d %d\n", index, width*height);
    for (size_t li = 0; li < width*height; li++) {
        //printf("i %d --> %d %d\n", li, mapping[li].x, mapping[li].y);
    }
    //printf("inited");
}

void init_pixels_blockys_shuffle(uint8_t* image, Pixel* pixels, int width, int height, int channels, Mapping* mapping) {
    BlockysStack stack = {0};
    append(&stack, ((BlockysParams){.x = 0, .y = 0, .width = width, .height = height}));
    ptrdiff_t index = 0;
    for (;;) {
        if (stack.len == 0) {
            break;
        }
        //printf("index %zu %d\n", index, width*height);
        size_t r = pcg32_boundedrand(stack.len);
        BlockysParams params = stack.data[r];
        stack.data[r] = stack.data[stack.len-1];
        stack.len -= 1;
        //printf("params %d %d %d %d\n", params.x, params.y, params.width, params.height);
        if (params.width >= 2 && params.height >= 2) {
            int new_width, new_width2;
            if (params.width % 2 == 0) {
                new_width = params.width / 2;
                new_width2 = new_width;
            }
            else {
                new_width = params.width/2;
                new_width2 = new_width +1;
            }
            //printf("%d %d %d\n", new_width2, new_width, params.width);
            assert(new_width2 + new_width == params.width);
            int new_height, new_height2;
            if (params.height % 2 == 0) {
                new_height = params.height / 2;
                new_height2 = new_height;
            }
            else {
                new_height = params.height/2;
                new_height2 = new_height + 1;
            }
            assert(new_height2 + new_height == params.height);
            BlockysParams top_left     = {.x = params.x,                  .y = params.y,                   .width = new_width, .height= new_height};
            BlockysParams top_right    = {.x = params.x + params.width/2, .y = params.y,                   .width = new_width2, .height= new_height};
            BlockysParams bottom_left  = {.x = params.x,                  .y = params.y + params.height/2, .width = new_width, .height= new_height2};
            BlockysParams bottom_right = {.x = params.x + params.width/2, .y = params.y + params.height/2, .width = new_width2, .height= new_height2};
            append(&stack, top_left);
            append(&stack, top_right);
            append(&stack, bottom_left);
            append(&stack, bottom_right);
        }
        else if (params.height == 1 && params.width > 0) {
            for (size_t dx = 0; dx < (size_t)params.width; dx++) {
                ptrdiff_t i = (params.x+dx) + params.y*width;
                //printf("%lld\n", i);
                assert(index < width*height);
                assert(i < width*height);
                pixels[index] = (Pixel){
                    .index = index,
                    .pixel={
                        image[i*channels+0],
                        image[i*channels+1],
                        image[i*channels+2],
                        image[i*channels+3],
                    }
                };
                mapping[index] = (Mapping){params.x+dx, params.y};
                index += 1;
            }
        } else if (params.width == 1 && params.height > 0) {
            for (size_t dy = 0; dy < (size_t)params.height; dy++) {
                ptrdiff_t i = params.x + (params.y+dy)*width;
                //printf("%lld\n", i);
                assert(index < width*height);
                assert(i < width*height);
                pixels[index] = (Pixel){
                    .index = index,
                    .pixel={
                        image[i*channels+0],
                        image[i*channels+1],
                        image[i*channels+2],
                        image[i*channels+3],
                    }
                };
                mapping[index] = (Mapping){params.x, params.y+dy};
                index += 1;
            }
        }
        else {
            //printf("0 encountered\n");
        }
    }
    //printf("i %d %d\n", index, width*height);
    for (size_t li = 0; li < width*height; li++) {
        //printf("i %d --> %d %d\n", li, mapping[li].x, mapping[li].y);
    }
    //printf("inited");
}

typedef struct {
    bool visited;
    int direction;
} MazeCell;

void maze_insert(uint8_t* image, Pixel* pixels, int width, int height, int channels, Mapping* mapping, MazeCell* grid, int x, int y, int direction, size_t* index) {
    ptrdiff_t i = x+y*width;
    assert(*index < width*height);
    assert(i < width*height);
    pixels[*index] = (Pixel){
        .index = *index,
        .pixel={
            image[i*channels+0],
            image[i*channels+1],
            image[i*channels+2],
            image[i*channels+3],
        }
    };
    mapping[*index] = (Mapping){x, y};
    grid[i].visited = true;
    grid[i].direction = direction;
    *index += 1;
}

int maze_available_directions(int width, int height, MazeCell* grid, int x, int y, int* directions) {
    int directions_count = 0;
    for (size_t direction = 0; direction < 4; direction++) {
        switch (direction) {
            break;case 0: {
                if (x + 1 < width && grid[(x+1)+y*width].visited == false) {
                    directions[directions_count++] = direction;
                }
            }
            break;case 1: {
                if (x - 1 >= 0 && grid[(x-1)+y*width].visited == false) {
                    directions[directions_count++] = direction;
                }
            }
            break;case 2: {
                if (y + 1 < height && grid[x+(y+1)*width].visited == false) {
                    directions[directions_count++] = direction;
                }
            }
            break;case 3: {
                if (y - 1 >= 0 && grid[x+(y-1)*width].visited == false) {
                    directions[directions_count++] = direction;
                }
            }
            break;default: {
                assert(false);
            }
        }
    }
    return directions_count;
}


void init_pixels_maze(uint8_t* image, Pixel* pixels, int width, int height, int channels, Mapping* mapping) {
    //printf("width: %d, height: %d\n", width, height);
    assert(width > 0);
    assert(height > 0);
    MazeCell* grid = calloc(width*height, sizeof(MazeCell));
    int directions[4] = {0};
    size_t index = 0;
    int x = 0;
    int y = 0;
    maze_insert(image, pixels, width, height, channels, mapping, grid, x, y, 5, &index);
    for(;;) {
        int directions_count = maze_available_directions(width, height, grid, x, y, directions);
        //printf("index: %zu x, y: %d, %d dircount: %d \n", index, x, y, directions_count);
        if (directions_count > 0) {
            int direction_index = pcg32_boundedrand(directions_count);
            int direction = directions[direction_index]; 
            //printf("Selected direction %d\n",direction);
            switch(direction) {
                break;case 0: x += 1;
                break;case 1: x -= 1;
                break;case 2: y += 1;
                break;case 3: y -= 1;
                break;default: assert(false);
            }
            //printf("inserting at %d %d\n", x, y);
            maze_insert(image, pixels, width, height, channels, mapping, grid, x, y, direction, &index);
            if (index == width*height) {
                free(grid);
                return;
            }
        }
        else {
            //printf("backtracking...\n");
            bool backtrack = true;
            while (backtrack) {
                //printf("backtracking %d %d\n", x, y);
                int previous_direction = grid[x+width*y].direction;
                switch(previous_direction) {
                    break;case 0: x -= 1;
                    break;case 1: x += 1;
                    break;case 2: y -= 1;
                    break;case 3: y += 1;
                    break;default: assert(false);
                }
                int directions_count = maze_available_directions(width, height, grid, x, y, directions);
                if (directions_count > 0) {
                    backtrack = false;
                }
            }
        }
    }
    assert(false);
}


int brick_mod(int x, int y) {
    while (x < 0) {
        x += y;
    }
    return x % y;
}

void init_pixels_brick(uint8_t* image, Pixel* pixels, int width, int height, int channels, Mapping* mapping) {
    int* heights = calloc(width, sizeof(int));
    int x = width / 2;
    size_t index = 0;
    for (;;) {
        bool left = (bool)pcg32_boundedrand(2);
        int dir = left ? -1 : 1; 
        for(;;) {
            x = brick_mod(x+dir, width);
            //printf("%d, %zu\n", x, index);
            if (heights[x] < height) {
                int y = heights[x];
                //printf("inserting at %d %d\n", x, y);
                ptrdiff_t i = x+y*width;
                assert(i < width*height);
                assert(index < (size_t)width*height);
                pixels[index] = (Pixel){
                    .index = index,
                    .pixel={
                        image[i*channels+0],
                        image[i*channels+1],
                        image[i*channels+2],
                        image[i*channels+3],
                    }
                };
                mapping[index] = (Mapping){x, y};
                index += 1;
                if (index == (size_t)width*height)  {
                    return;
                }
                heights[x] += 1;
                break;
            }
        }
    }
}

#define VORONOI_SEEDS 140
#define MAX(a, b) ((a) > (b) ? (a) : (b))

void init_pixels_voronoi(uint8_t* image, Pixel* pixels, int width, int height, int channels, Mapping* mapping) {
    bool* oldgrain = calloc(width*height, sizeof(bool));
    bool* newgrain = calloc(width*height, sizeof(bool));
    size_t index = 0;
    for (size_t i = 0; i < MAX(2, (size_t)width/10); i++) {
        int x = pcg32_boundedrand(width);
        int y = pcg32_boundedrand(height);
        oldgrain[x+y*width] = true;
        newgrain[x+y*width] = true;
        assert(0 <= x && x < width);
        assert(0 <= y && y < height);
        ptrdiff_t i = x+y*width;
        assert(i < width*height);
        assert(index < (size_t)width*height);
        pixels[index] = (Pixel){
            .index = index,
            .pixel={
                image[i*channels+0],
                image[i*channels+1],
                image[i*channels+2],
                image[i*channels+3],
            }
        };
        mapping[index] = (Mapping){x, y};
        index += 1;
    }
    while (index < width*height) {
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                if (
                    (
                        (x - 1 >= 0     && oldgrain[(x-1)+y    *width])
                        || (x + 1 < width  && oldgrain[(x+1)+y    *width])
                        || (y - 1 >= 0     && oldgrain[x    +(y-1)*width])
                        || (y + 1 < height && oldgrain[x    +(y+1)*width])
                    ) 
                    && (
                        !oldgrain[x+y*width]
                    )
                ) {
                    //printf("%zu %lld inserting at %d %d\n", index, (ptrdiff_t)width*height, x, y);
                    newgrain[x+y*width] = true;
                    assert(0 <= x && x < width);
                    assert(0 <= y && y < height);
                    ptrdiff_t i = x+y*width;
                    assert(i < width*height);
                    assert(index < (size_t)width*height);
                    pixels[index] = (Pixel){
                        .index = index,
                        .pixel={
                            image[i*channels+0],
                            image[i*channels+1],
                            image[i*channels+2],
                            image[i*channels+3],
                        }
                    };
                    mapping[index] = (Mapping){x, y};
                    index += 1;
                    if (index == (size_t)width*height)  {
                        return;
                    }
                }

            }
        }
        for (size_t i = 0; i < (size_t)width*height; i++) {
            if (newgrain[i]) {
                oldgrain[i] = true;
            }
        }
    }
}


void init_pixels_voronoi2(uint8_t* image, Pixel* pixels, int width, int height, int channels, Mapping* mapping) {
    bool* oldgrain = calloc(width*height, sizeof(bool));
    bool* newgrain = calloc(width*height, sizeof(bool));
    size_t index = 0;
    for (size_t i = 0; i < MAX(2, (size_t)width/10); i++) {
        int x = pcg32_boundedrand(width);
        int y = pcg32_boundedrand(height);
        oldgrain[x+y*width] = true;
        newgrain[x+y*width] = true;
        assert(0 <= x && x < width);
        assert(0 <= y && y < height);
        ptrdiff_t i = x+y*width;
        assert(i < width*height);
        assert(index < (size_t)width*height);
        pixels[index] = (Pixel){
            .index = index,
            .pixel={
                image[i*channels+0],
                image[i*channels+1],
                image[i*channels+2],
                image[i*channels+3],
            }
        };
        mapping[index] = (Mapping){x, y};
        index += 1;
    }
    while (index < width*height) {
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                if (
                    (
                           (x - 1 >= 0     && oldgrain[(x-1)+y    *width])
                        || (x + 1 < width  && oldgrain[(x+1)+y    *width])
                        || (y - 1 >= 0     && oldgrain[x    +(y-1)*width])
                        || (y + 1 < height && oldgrain[x    +(y+1)*width])
                        
                        || (x - 1 >= 0     && y - 1 >= 0     && oldgrain[(x-1)+(y-1)*width])
                        || (x - 1 >= 0     && y + 1 < height && oldgrain[(x-1)+(y+1)*width])
                        || (x + 1 < width  && y - 1 >= 0     && oldgrain[(x+1)+(y-1)*width])
                        || (x + 1 < width  && y + 1 < height && oldgrain[(x+1)+(y+1)*width])
                    ) 
                    && (
                        !oldgrain[x+y*width]
                    )
                ) {
                    //printf("%zu %lld inserting at %d %d\n", index, (ptrdiff_t)width*height, x, y);
                    newgrain[x+y*width] = true;
                    assert(0 <= x && x < width);
                    assert(0 <= y && y < height);
                    ptrdiff_t i = x+y*width;
                    assert(i < width*height);
                    assert(index < (size_t)width*height);
                    pixels[index] = (Pixel){
                        .index = index,
                        .pixel={
                            image[i*channels+0],
                            image[i*channels+1],
                            image[i*channels+2],
                            image[i*channels+3],
                        }
                    };
                    mapping[index] = (Mapping){x, y};
                    index += 1;
                    if (index == (size_t)width*height)  {
                        return;
                    }
                }

            }
        }
        for (size_t i = 0; i < (size_t)width*height; i++) {
            if (newgrain[i]) {
                oldgrain[i] = true;
            }
        }
    }
}

typedef struct {
    int x, y, ox, oy;
} Walker;

void init_pixels_walkers(uint8_t* image, Pixel* pixels, int width, int height, int channels, Mapping* mapping) {
    Walker* walkers = calloc(width*height, sizeof(Walker));
    size_t index = 0;
    bool* oldgrain = calloc(width*height, sizeof(bool));
    bool* newgrain = calloc(width*height, sizeof(bool));
    size_t num_seeds = MAX(2, (size_t)(height*width)/100000);
    for (size_t i = 0; i < num_seeds; i++) {
        int x = pcg32_boundedrand(width);
        int y = pcg32_boundedrand(height);
        oldgrain[x+y*width] = true;
        newgrain[x+y*width] = true;
        assert(0 <= x && x < width);
        assert(0 <= y && y < height);
        ptrdiff_t i = x+y*width;
        assert(i < width*height);
        assert(index < (size_t)width*height);
        pixels[index] = (Pixel){
            .index = index,
            .pixel={
                image[i*channels+0],
                image[i*channels+1],
                image[i*channels+2],
                image[i*channels+3],
            }
        };
        mapping[index] = (Mapping){x, y};
        index += 1;
    }
    ptrdiff_t wi = 0;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            if (!oldgrain[x+y*width]) {
                walkers[wi++] = (Walker){x, y, x,  y};
            }
        }
    }
    ptrdiff_t walker_count = wi;
    
    while (index < (size_t)width*height) {
        for (wi = 0; wi < walker_count; wi++) {
            int dir = pcg32_boundedrand(4);
            switch(dir) {
                break;case 0: if (walkers[wi].x + 1 < width ) {walkers[wi].x += 1;};
                break;case 1: if (walkers[wi].x - 1 >= 0    ) {walkers[wi].x -= 1;};
                break;case 2: if (walkers[wi].y + 1 < height) {walkers[wi].y += 1;};
                break;case 3: if (walkers[wi].y - 1 >= 0    ) {walkers[wi].y -= 1;};
                break;default: assert(false);
            }
            int x = walkers[wi].x;
            int y = walkers[wi].y;
            if (
                (
                       (x - 1 >= 0     && oldgrain[(x-1)+y    *width])
                    || (x + 1 < width  && oldgrain[(x+1)+y    *width])
                    || (y - 1 >= 0     && oldgrain[x    +(y-1)*width])
                    || (y + 1 < height && oldgrain[x    +(y+1)*width])
                    
                    || (x - 1 >= 0     && y - 1 >= 0     && oldgrain[(x-1)+(y-1)*width])
                    || (x - 1 >= 0     && y + 1 < height && oldgrain[(x-1)+(y+1)*width])
                    || (x + 1 < width  && y - 1 >= 0     && oldgrain[(x+1)+(y-1)*width])
                    || (x + 1 < width  && y + 1 < height && oldgrain[(x+1)+(y+1)*width])
                ) 
                && (
                    !oldgrain[x+y*width]
                )
            ) {
                //printf("%zu %lld inserting at %d %d %lld\n", index, (ptrdiff_t)width*height, x, y, walker_count);
                newgrain[x+y*width] = true;
                assert(0 <= x && x < width);
                assert(0 <= y && y < height);
                ptrdiff_t i = walkers[wi].ox+walkers[wi].oy*width;
                assert(i < width*height);
                assert(index < (size_t)width*height);
                pixels[index] = (Pixel){
                    .index = index,
                    .pixel={
                        image[i*channels+0],
                        image[i*channels+1],
                        image[i*channels+2],
                        image[i*channels+3],
                    }
                };
                mapping[index] = (Mapping){walkers[wi].ox, walkers[wi].oy};
                index += 1;
                walkers[wi] = walkers[walker_count];
                walker_count -= 1;
                if (index == (size_t)width*height)  {
                    return;
                }
            }   
        }
        for (size_t i = 0; i < (size_t)width*height; i++) {
            if (newgrain[i]) {
                oldgrain[i] = true;
            }
        }
    }
}

void init_pixels_rings(uint8_t* image, Pixel* pixels, int width, int height, int channels, Mapping* mapping) {
    Walker* walkers = calloc(width*height, sizeof(Walker));
    int r = 100;
    size_t index = 0;
    bool* oldgrain = calloc(width*height, sizeof(bool));
    bool* newgrain = calloc(width*height, sizeof(bool));
    size_t num_seeds = MAX(2, (size_t)(height*width)/100000);
    for (size_t i = 0; i < num_seeds; i++) {
        int x = pcg32_boundedrand(width);
        int y = pcg32_boundedrand(height);
        oldgrain[x+y*width] = true;
        newgrain[x+y*width] = true;
        assert(0 <= x && x < width);
        assert(0 <= y && y < height);
        ptrdiff_t i = x+y*width;
        assert(i < width*height);
        assert(index < (size_t)width*height);
        pixels[index] = (Pixel){
            .index = index,
            .pixel={
                image[i*channels+0],
                image[i*channels+1],
                image[i*channels+2],
                image[i*channels+3],
            }
        };
        mapping[index] = (Mapping){x, y};
        index += 1;
    }
    ptrdiff_t wi = 0;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            if (!oldgrain[x+y*width]) {
                walkers[wi++] = (Walker){x, y, x, y};
            }
        }
    }
    ptrdiff_t walker_count = wi;
    
    while (index < (size_t)width*height) {
        for (wi = 0; wi < walker_count; wi++) {
            int dir = pcg32_boundedrand(4);
            switch(dir) {
                break;case 0: if (walkers[wi].x + 1 < width) {walkers[wi].x += 1;};
                break;case 1: if (walkers[wi].x - 1 >= 0   ) {walkers[wi].x -= 1;};
                break;case 2: if (walkers[wi].y + 1 < height) {walkers[wi].y += 1;};
                break;case 3: if (walkers[wi].y - 1 >= 0) {walkers[wi].y -= 1;};
                break;default: assert(false);
            }
            int x = walkers[wi].x;
            int y = walkers[wi].y;
            if (
                (
                       (x - 1 >= 0     && oldgrain[(x-1)+y    *width])
                    || (x + 1 < width  && oldgrain[(x+1)+y    *width])
                    || (y - 1 >= 0     && oldgrain[x    +(y-1)*width])
                    || (y + 1 < height && oldgrain[x    +(y+1)*width])
                    
                    || (x - r >= 0     && y - r >= 0     && oldgrain[(x-r)+(y-r)*width])
                    || (x - r >= 0     && y + r < height && oldgrain[(x-r)+(y+r)*width])
                    || (x + r < width  && y - r >= 0     && oldgrain[(x+r)+(y-r)*width])
                    || (x + r < width  && y + r < height && oldgrain[(x+r)+(y+r)*width])
                ) 
                && (
                    !oldgrain[x+y*width]
                )
            ) {
                //printf("%zu %lld inserting at %d %d %lld\n", index, (ptrdiff_t)width*height, x, y, walker_count);
                newgrain[x+y*width] = true;
                assert(0 <= x && x < width);
                assert(0 <= y && y < height);
                ptrdiff_t i = walkers[wi].ox+walkers[wi].oy*width;
                assert(i < width*height);
                assert(index < (size_t)width*height);
                pixels[index] = (Pixel){
                    .index = index,
                    .pixel={
                        image[i*channels+0],
                        image[i*channels+1],
                        image[i*channels+2],
                        image[i*channels+3],
                    }
                };
                mapping[index] = (Mapping){walkers[wi].ox, walkers[wi].oy};
                index += 1;
                walkers[wi] = walkers[walker_count];
                walker_count -= 1;
                if (index == (size_t)width*height)  {
                    return;
                }
            }   
        }
        for (size_t i = 0; i < (size_t)width*height; i++) {
            if (newgrain[i]) {
                oldgrain[i] = true;
            }
        }
    }
}

typedef struct {
    int x, y;
} RainCell;

#define MIN(a, b) ((a) < (b) ? (a) : (b))

void init_pixels_rain(uint8_t* image, Pixel* pixels, int width, int height, int channels, Mapping* mapping) {
    RainCell* unused = malloc(width*height*sizeof(RainCell));
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            unused[x + y*width] = (RainCell){x, y};
        }
    }
    ssize_t unused_count = width*height;
    ssize_t index = 0;
    ssize_t rain_length = 100;
    while (index < width*height) {
        ssize_t it = pcg32_boundedrand(unused_count); // FIXME for larger that u32 pixels
        for (int j = it; j < MIN(unused_count, it+rain_length); j++) {
            int x = unused[j].x;
            int y = unused[j].y;
            assert(0 <= x && x < width);
            assert(0 <= y && y < height);
            ptrdiff_t i = x+ y*width;
            assert(i < width*height);
            assert(index < width*height);
            //printf("index %lld it %lld unused %lld it-j %lld at %d %d\n", index, it, unused_count, j-it, x, y);
            pixels[index] = (Pixel){
                .index = index,
                .pixel={
                    image[i*channels+0],
                    image[i*channels+1],
                    image[i*channels+2],
                    image[i*channels+3],
                }
            };
            mapping[index] = (Mapping){x, y};
            index += 1;
            unused[j] = unused[unused_count-1];
            unused_count -= 1;
        }
    }    
    assert(unused_count == 0);
    free(unused);
}

// TODO init sorted after brightness
// TODO sort after pattern image brightness
// TODO init julia set
// TODO bias walker speed for one direction

typedef enum {
    SA_SELECTION_SORT,
    SA_UNOPT_SELECTION_SORT,
    SA_DOUBLE_SELECTION_SORT,
    SA_MOVING_SELECTION_SORT,
    ///////////////
    SA_LINEAR,
    ///////////////
    SA_HEAP_QUICKSORT,
    SA_SHELL_SORT, //technically not logarithmic, so sue me.
    SA_HEAP_SORT,
    SA_MERGE_SORT,
    SA_QUICK_SORT,
    SA_CIRCLE_SORT,
    ///////////////
    SA_LOGARITHMIC,
    ///////////////
    SA_INSERTION_SORT,
    SA_UNOPT_INSERTION_SORT,
    SA_MIDDLE_OUT_SORT,
    SA_HEAP_INSERTION_SORT,
    SA_ODD_EVEN_SORT,
    SA_BUBBLE_SORT,
    SA_HEAP_BUBBLE_SORT,
    ///////////////
    SA_QUADRATIC,

} SortingAlgorithm;

typedef enum {
    PAT_LINEAR,
    PAT_HILBERT,
    PAT_SPIRAL,
    PAT_RANDOM,
    PAT_BLOCKYS,
    PAT_BLOCKYS_REGULAR,
    PAT_BLOCKYS_SHUFFLE,
    PAT_MAZE,
    PAT_BRICK,
    PAT_VORONOI,
    PAT_VORONOI2,
    PAT_WALKERS,
    PAT_RINGS,
    PAT_RAIN,
} Pattern;

typedef enum {
    IM_NONE,
    IM_SHUFFLE,
    IM_REVERSE,
} InitMode;

bool streq(char* s1, char* s2) {
    return (strcmp(s1, s2) == 0);
}

typedef struct {
    struct mfb_window* window;
    uint32_t* window_buffer;
    Pixel* pixels;
    int width;
    int height;
    ptrdiff_t modulus;
    int write_counter;
    bool record_with_ffmpeg;
    FFMPEG* ffmpeg;
    Pattern pattern;
    Mapping* mapping;
} Userdata;


static inline __attribute__((always_inline))
void new_write_callback(void* userdata_void, void* elems, size_t elem_begin, size_t elem_end) {
    Userdata* userdata = (Userdata*)userdata_void;
    Pixel* pixels = (Pixel*)elems;
    for (size_t i = elem_begin; i < elem_end; i++) {
        int x = userdata->mapping[i].x;
        int y = userdata->mapping[i].y;
        
        userdata->window_buffer[x + y*userdata->width] = (uint32_t)pixels[i].pixel[3] << UINT32_C(24) 
            | (uint32_t)pixels[i].pixel[0] << UINT32_C(16) 
            | (uint32_t)pixels[i].pixel[1] << UINT32_C(8) 
            | (uint32_t)pixels[i].pixel[2];
    }
    if (userdata->ffmpeg && userdata->record_with_ffmpeg) {
        assert(ffmpeg_send_frame(userdata->ffmpeg, userdata->window_buffer, userdata->width, userdata->height));
    }
    if(mfb_update_ex(userdata->window, userdata->window_buffer, userdata->width, userdata->height) < 0) {
        userdata->window = NULL;
    }
    if (!mfb_wait_sync(userdata->window)) {
        exit(0);
    };
}

static inline __attribute__((always_inline))
Ordering compare_index_cb(void* userdata, void* p1v, void* p2v) {
    (void)userdata;
    Pixel* p1 = (Pixel*)p1v;
    Pixel* p2 = (Pixel*)p2v;
    if (p1->index < p2 ->index) {
        return Ordering_LessThan;
    }
    else if (p1->index > p2 ->index) {
        return Ordering_GreaterThan;
    }
    else {
        return Ordering_Equal;
    } 
}

static inline __attribute__((always_inline))
Ordering compare_index_horizontal(void* userdata_v, void* p1v, void* p2v) {
    Userdata* userdata = (Userdata*)userdata_v;
    Pixel* p1 = (Pixel*)p1v;
    Pixel* p2 = (Pixel*)p2v;
    int x1 = p1->index % userdata->width;
    int x2 = p2->index % userdata->width;
    if (x1 < x2) {
        return Ordering_LessThan;
    }
    else if (x1 > x2) {
        return Ordering_GreaterThan;
    }
    else {
        return Ordering_Equal;
    } 
}

static inline __attribute__((always_inline))
Ordering compare_index_xor2(void* userdata_v, void* p1v, void* p2v) {
    Userdata* userdata = (Userdata*)userdata_v;
    Pixel* p1 = (Pixel*)p1v;
    Pixel* p2 = (Pixel*)p2v;
    int x1 = p1->index % userdata->width;
    int x2 = p2->index % userdata->width;
    int y1 = p1->index / userdata->width;
    int y2 = p2->index / userdata->width;
    x1 = x1 ^ y1;
    x2 = x2 ^ y2;
    if (x1 < x2) {
        return Ordering_LessThan;
    }
    else if (x1 > x2) {
        return Ordering_GreaterThan;
    }
    else {
        return Ordering_Equal;
    } 
}

static inline __attribute__((always_inline))
Ordering compare_index_xor(void* userdata_v, void* p1v, void* p2v) {
    Userdata* userdata = (Userdata*)userdata_v;
    Pixel* p1 = (Pixel*)p1v;
    Pixel* p2 = (Pixel*)p2v;
    int x1 = p1->index + p1->pixel[2];
    int x2 = p2->index + p2->pixel[2];
    if (x1 < x2) {
        return Ordering_LessThan;
    }
    else if (x1 > x2) {
        return Ordering_GreaterThan;
    }
    else {
        return Ordering_Equal;
    } 
}

static inline __attribute__((always_inline))
Ordering compare_index_rev_cb(void* userdata, void* p1v, void* p2v) {
    (void)userdata;
    Pixel* p1 = (Pixel*)p1v;
    Pixel* p2 = (Pixel*)p2v;
    if (p1->index < p2 ->index) {
        return Ordering_GreaterThan;
    }
    else if (p1->index > p2 ->index) {
        return Ordering_LessThan;
    }
    else {
        return Ordering_Equal;
    } 
}

static inline __attribute__((always_inline))
Ordering compare_color_cb(void* userdata, void* p1v, void* p2v) {
    (void)userdata;
    Pixel* p1 = (Pixel*)p1v;
    Pixel* p2 = (Pixel*)p2v;
    int brightness1 = p1->pixel[1];
    int brightness2 = p2->pixel[1];
    if (brightness1 < brightness2) {
        return Ordering_LessThan;
    }
    else if (brightness1 > brightness2) {
        return Ordering_GreaterThan;
    }
    else {
        return Ordering_Equal;
    } 
}

int main(int argc, char** argv) {
    pcg32_srandom(234, 789);
    char filename[MAX_PATH+1] = {0};
    char videoname[MAX_PATH+1] = {0};
    SortingAlgorithm sort = SA_MERGE_SORT;
    Pattern pattern = PAT_HILBERT;
    bool record_with_ffmpeg = false;
    cmp_cb compare_callback = compare_index_cb;
    InitMode init_mode = IM_SHUFFLE;
    bool wait_for_getchar = false;
    size_t video_hold = 120;
    int n_every = 1000;
    for (int arg_it = 1; arg_it < argc; arg_it++) {
        if (streq(argv[arg_it], "-h")) {
            printf("Help page not implemented! Sorry!");
            exit(0);
        }
        else if (streq(argv[arg_it], "--wait")) {
            wait_for_getchar = true;
        }
        else if (streq(argv[arg_it], "-s") || streq(argv[arg_it], "--sort")) {
            if (arg_it + 1 < argc) {
                arg_it += 1;
                if (streq(argv[arg_it], "quicksort")) {
                    sort = SA_QUICK_SORT;
                }
                else if (streq(argv[arg_it], "heap_insertionsort")) {
                    sort = SA_HEAP_INSERTION_SORT;
                }
                else if (streq(argv[arg_it], "mergesort")) {
                    sort = SA_MERGE_SORT;
                }
                else if (streq(argv[arg_it], "insertionsort")) {
                    sort = SA_INSERTION_SORT;
                }
                else if (streq(argv[arg_it], "middle_out_sort")) {
                    sort = SA_MIDDLE_OUT_SORT;
                }
                else if (streq(argv[arg_it], "selectionsort")) {
                    sort = SA_SELECTION_SORT;
                }
                else if (streq(argv[arg_it], "circlesort")) {
                    sort = SA_CIRCLE_SORT;
                }
                else if (streq(argv[arg_it], "heap_bubblesort")) {
                    sort = SA_HEAP_BUBBLE_SORT;
                }
                else if (streq(argv[arg_it], "unopt_selectionsort")) {
                    sort = SA_UNOPT_SELECTION_SORT;
                }
                else if (streq(argv[arg_it], "double_selectionsort")) {
                    sort = SA_DOUBLE_SELECTION_SORT;
                }
                else if (streq(argv[arg_it], "moving_selectionsort")) {
                    sort = SA_MOVING_SELECTION_SORT;
                }
                else if (streq(argv[arg_it], "shellsort")) {
                    sort = SA_SHELL_SORT;
                }
                else if (streq(argv[arg_it], "heap_quicksort")) {
                    sort = SA_HEAP_QUICKSORT;
                }
                else if (streq(argv[arg_it], "heapsort")) {
                    sort = SA_HEAP_SORT;
                }
                else if (streq(argv[arg_it], "oddevensort")) {
                    sort = SA_ODD_EVEN_SORT;
                }
                else if (streq(argv[arg_it], "bubblesort")) {
                    sort = SA_BUBBLE_SORT;
                }
                else if (streq(argv[arg_it], "unopt_insertionsort")) {
                    sort = SA_UNOPT_INSERTION_SORT;
                }
                else {
                    fprintf(stderr, "Invalid option for sort %s\n", argv[arg_it]);
                    exit(0);
                }
            }
            else {
                fprintf(stderr, "Option `-s` missing 1 required positional argument");
                exit(0);
            }
        }
        
        else if (streq(argv[arg_it], "-r") || streq(argv[arg_it], "--record")) {
            record_with_ffmpeg = true;
            if (arg_it+1 < argc) {
                arg_it += 1;
                assert(strlen(argv[arg_it]) < MAX_PATH && "Filename too long");
                strncpy(videoname, argv[arg_it], MAX_PATH);
                videoname[MAX_PATH-1]='\0';
            }
        }
        else if (streq(argv[arg_it], "-n")) {
            if (arg_it + 1 < argc) {
                arg_it += 1;
                ParseIntResult result = sv_parse_int((sv){.data=argv[arg_it], .len=strlen(argv[arg_it])});
                if (result.err == PARSE_INT_OK && result.val >= 1) {
                    n_every = result.val;
                }
                else {
                    fprintf(stderr, "Invalid number for n\n");
                    exit(0);
                }
            }
            else {
                fprintf(stderr, "Option `-n` missing 1 required positional argument\n");
                exit(0);
            }
        }
        else if (streq(argv[arg_it], "-p") || streq(argv[arg_it], "--pattern")) {
            if (arg_it + 1 < argc) {
                arg_it += 1;
                if (streq(argv[arg_it], "linear")) {
                    pattern = PAT_LINEAR;
                }
                else if (streq(argv[arg_it], "hilbert")) {
                    pattern = PAT_HILBERT;
                }
                else if (streq(argv[arg_it], "spiral")) {
                    pattern = PAT_SPIRAL;
                }
                else if (streq(argv[arg_it], "random")) {
                    pattern = PAT_RANDOM;
                }
                else if (streq(argv[arg_it], "blockys")) {
                    pattern = PAT_BLOCKYS;
                }
                else if (streq(argv[arg_it], "blockys_regular")) {
                    pattern = PAT_BLOCKYS_REGULAR;
                }
                else if (streq(argv[arg_it], "blockys_shuffle")) {
                    pattern = PAT_BLOCKYS_SHUFFLE;
                }
                else if (streq(argv[arg_it], "maze")) {
                    pattern = PAT_MAZE;
                }
                else if (streq(argv[arg_it], "brick")) {
                    pattern = PAT_BRICK;
                }
                else if (streq(argv[arg_it], "voronoi")) {
                    pattern = PAT_VORONOI;
                }
                else if (streq(argv[arg_it], "voronoi2")) {
                    pattern = PAT_VORONOI2;
                }
                else if (streq(argv[arg_it], "walkers")) {
                    pattern = PAT_WALKERS;
                }
                else if (streq(argv[arg_it], "rings")) {
                    pattern = PAT_RINGS;
                }
                else if (streq(argv[arg_it], "rain")) {
                    pattern = PAT_RAIN;
                }
                else {
                    fprintf(stderr, "Invalid option for pattern: %s\n", argv[arg_it]);
                    exit(0);
                }
            }
            else {
                fprintf(stderr, "Option `--pattern` missing 1 required positional argument\n");
                exit(0);
            }
        }
        else if (streq(argv[arg_it], "--cmp_color")) {
            compare_callback = compare_color_cb;
        }
        else if (streq(argv[arg_it], "--cmp_rev_index")) {
            compare_callback = compare_index_rev_cb;
        }
        else if (streq(argv[arg_it], "--cmp_index_horiz")) {
            compare_callback = compare_index_horizontal;
        }
        else if (streq(argv[arg_it], "--cmp_index_xor")) {
            compare_callback = compare_index_xor;
        }
        else if (streq(argv[arg_it], "--no_shuffle")) {
            init_mode = IM_NONE;
        }
        else {
            if (strlen(argv[arg_it]) > MAX_PATH) {
                fprintf(stderr, "Filename too long\n");
                exit(0);
            }
            else {
                strncpy(filename, argv[arg_it], MAX_PATH);
                filename[MAX_PATH-1]='\0';
            }
        }
    }
    assert(sort != SA_QUADRATIC);
    int width, height;
    int channels = 4;
    uint8_t* image = stbi_load(filename, &width, &height, NULL, channels);
    if (!image) {
        printf("ERROR: Could not load image `%s`\n", filename);
        exit(0);
    }
    Pixel* pixels = malloc(sizeof(Pixel)*width*height);
    assert(pixels && "OOM");
    Mapping* mapping = malloc(sizeof(Mapping)*width*height);
    assert(mapping && "OOM");
    FFMPEG* ffmpeg = (void*)0;
    if (record_with_ffmpeg) {
        ffmpeg = ffmpeg_start_rendering(width, height, 60, videoname);
        assert(ffmpeg != NULL);
    }
    switch (pattern) {
        break;case PAT_HILBERT: {
            init_pixels_hilbert(image, pixels, width, height, channels, mapping);
        }
        break;case PAT_LINEAR: {
            init_pixels_linear(image, pixels, width, height, channels, mapping);
        }
        break;case PAT_SPIRAL: {
            init_pixels_spiral(image, pixels, width, height, channels, mapping);
        }
        break;case PAT_RANDOM: {
            init_pixels_random(image, pixels, width, height, channels, mapping);
        }
        break;case PAT_BLOCKYS: {
            init_pixels_blockys(image, pixels, width, height, channels, mapping);
        }
        break;case PAT_BLOCKYS_REGULAR: {
            init_pixels_blockys_regular(image, pixels, width, height, channels, mapping);
        }
        break;case PAT_BLOCKYS_SHUFFLE: {
            init_pixels_blockys_shuffle(image, pixels, width, height, channels, mapping);
        }
        break;case PAT_MAZE: {
            init_pixels_maze(image, pixels, width, height, channels, mapping);
        }
        break;case PAT_BRICK: {
            init_pixels_brick(image, pixels, width, height, channels, mapping);
        }
        break;case PAT_VORONOI: {
            init_pixels_voronoi(image, pixels, width, height, channels, mapping);
        }
        break;case PAT_VORONOI2: {
            init_pixels_voronoi2(image, pixels, width, height, channels, mapping);
        }
        break;case PAT_WALKERS: {
            init_pixels_walkers(image, pixels, width, height, channels, mapping);
        }
        break;case PAT_RINGS: {
            init_pixels_rings(image, pixels, width, height, channels, mapping);
        }
        break;case PAT_RAIN: {
            init_pixels_rain(image, pixels, width, height, channels, mapping);
        }
        break;default: assert(false);
    }
    struct mfb_window* window = mfb_open_ex("sorting visualisation", width, height, WF_RESIZABLE);
    if (!window) {
        printf("ERROR: Could not open window!");
        exit(0);
    }
    uint32_t* window_buffer = malloc(width*height*4);
    assert(window_buffer && "Buy more ram");
    if (init_mode == IM_SHUFFLE) {
        shuffle_pixels(pixels, width*height);
    }
    if (init_mode == IM_REVERSE) {
        // reverse pixels
    }
    Userdata userdata = {
        .window = window,
        .window_buffer = window_buffer,
        .pixels = pixels,
        .width = width,
        .height = height,
        .modulus = n_every,
        .record_with_ffmpeg = record_with_ffmpeg,
        .ffmpeg = ffmpeg,
        .pattern = pattern,
        .mapping = mapping,
    };
    if (wait_for_getchar) {
        printf("Waiting for input...");
        getchar();
    }
    
    if (sort < SA_LINEAR) {
        userdata.modulus = userdata.modulus;
    }
    else if (sort < SA_LOGARITHMIC) {
        userdata.modulus = userdata.modulus *logf(width*height);
    }
    else if (sort < SA_QUADRATIC) {
        userdata.modulus = userdata.modulus*width*height;
    }
    else {
        assert(0 && "unknown complexity");
    }
    new_write_callback(&userdata, pixels, 0, width*height);
    while (mfb_wait_sync(window)) {
        if (sort == SA_MERGE_SORT) {
            mergesort((void*)&userdata, pixels, sizeof(pixels[0]), width*height, userdata.modulus,  new_write_callback, compare_callback);
        }
        if (sort == SA_QUICK_SORT) {
            quicksort((void*)&userdata, pixels, sizeof(pixels[0]), width*height, userdata.modulus,  new_write_callback, compare_callback);
        }
        if (sort == SA_HEAP_QUICKSORT) {
            heap_quicksort((void*)&userdata, pixels, sizeof(pixels[0]), width*height, userdata.modulus,  new_write_callback, compare_callback);
        }
        if (sort == SA_INSERTION_SORT) {
            optimized_insertionsort3((void*)&userdata, pixels, sizeof(pixels[0]), width*height, userdata.modulus,  new_write_callback, compare_callback);
        }
        if (sort == SA_MIDDLE_OUT_SORT) {
            middle_out_sort((void*)&userdata, pixels, sizeof(pixels[0]), width*height, userdata.modulus,  new_write_callback, compare_callback);
        }
        if (sort == SA_HEAP_INSERTION_SORT) {
            heap_insertionsort((void*)&userdata, pixels, sizeof(pixels[0]), width*height, userdata.modulus,  new_write_callback, compare_callback);
        }
        if (sort == SA_SELECTION_SORT) {
            optimized_selectionsort55((void*)&userdata, pixels, sizeof(pixels[0]), width*height, userdata.modulus,  new_write_callback, compare_callback);
        }
        if (sort == SA_UNOPT_SELECTION_SORT) {
            selectionsort((void*)&userdata, pixels, sizeof(pixels[0]), width*height, userdata.modulus,  new_write_callback, compare_callback);
        }
        if (sort == SA_DOUBLE_SELECTION_SORT) {
            double_selectionsort((void*)&userdata, pixels, sizeof(pixels[0]), width*height, userdata.modulus,  new_write_callback, compare_callback);
        }
        if (sort == SA_MOVING_SELECTION_SORT) {
            moving_selectionsort((void*)&userdata, pixels, sizeof(pixels[0]), width*height, userdata.modulus,  new_write_callback, compare_callback);
        }
        if (sort == SA_ODD_EVEN_SORT) {
            odd_even_sort((void*)&userdata, pixels, sizeof(pixels[0]), width*height, userdata.modulus,  new_write_callback, compare_callback);
        }
        if (sort == SA_SHELL_SORT) {
            shellsort((void*)&userdata, pixels, sizeof(pixels[0]), width*height, userdata.modulus,  new_write_callback, compare_callback);
        }
        if (sort == SA_CIRCLE_SORT) {
            circlesort((void*)&userdata, pixels, sizeof(pixels[0]), width*height, userdata.modulus,  new_write_callback, compare_callback);
        }
        if (sort == SA_HEAP_SORT) {
            heapsort((void*)&userdata, pixels, sizeof(pixels[0]), width*height, userdata.modulus,  new_write_callback, compare_callback);
        }
        if (sort == SA_BUBBLE_SORT) {
            bubblesort((void*)&userdata, pixels, sizeof(pixels[0]), width*height, userdata.modulus,  new_write_callback, compare_callback);
        }
        if (sort == SA_HEAP_BUBBLE_SORT) {
            heap_bubblesort((void*)&userdata, pixels, sizeof(pixels[0]), width*height, userdata.modulus,  new_write_callback, compare_callback);
        }
        if (sort == SA_UNOPT_INSERTION_SORT) {
            insertionsort((void*)&userdata, pixels, sizeof(pixels[0]), width*height, userdata.modulus,  new_write_callback, compare_callback);
        }
        new_write_callback(&userdata, pixels, 0, width*height);
        if (userdata.record_with_ffmpeg) {
            for (size_t i = 0; i < video_hold; i++) {
                new_write_callback(&userdata, pixels, 0, width*height);
            }
            printf("Ending recording");
            assert(ffmpeg_end_rendering(ffmpeg, false));
            userdata.record_with_ffmpeg = false;
        }
        assert(is_sorted(pixels, width*height));
        printf("Sorted!");
    }
    return 0;
}
