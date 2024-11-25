# PixelSorter

## Synopsis

A pixel sorting algorithm visualisation.

## Building

### Linux
```
.\build_release.sh
```

### Windows
```
.\build_release.bat
```

## Example

Simple example for mergesort + hilbert curve:
```
.\build\pixelsorter_win.exe <image>
```

More complex examples:
```
.\build\pixelsorter_win.exe --pattern maze --sort heapsort -n 2000 <image>
```

```
.\build\pixelsorter_win.exe -m reverse --number_of_frames 20000 -p linear -s quicksort <image>
```

## Usage

```
pixelsorter [OPTIONS] <image>

OPTIONS:
-h, --help                  Display this help message
--wait                      Wait for stdin input
-r, --record <filename>     Record video with ffmpeg
-n, --number_of_frames      Currently broken, number of write equivalents
-s <sort>, --sort <sort>    Select sorting algorithm <sort>
                                Fast sorts:
                                    quicksort
                                    heap_quicksort
                                    heapsort
                                    mergesort (default)
                                    circlesort
                                    shellsort
                                Slow sorts
                                    selectionsort
                                    insertionsort
                                    heap_insertionsort
                                Super slow sorts
                                    unopt_selectionsort
                                    double_selectionsort
                                    moving_selectionsort
                                    unopt_insertionsort
                                    middle_out_sort
                                    bubblesort
                                    heap_bubblesort
                                    oddevensort
-p <pat>, --pattern <pat>   Select pattern
                                linear
                                hilbert (default)
                                spiral
                                random
                                blockys
                                blockys_regular
                                blockys_shuffle
                                maze
                                brick
                                voronoi
                                voronoi2
                                walkers
                                rings
                                rain
-c <cmp>, --cmp <cmp>       Select comparision callback
                                index (default)
                                reverse
                                color
                                horizontal
-m <mode>, --mode <mode>    Mode
                                none
                                shuffle (default)
                                reverse
```

