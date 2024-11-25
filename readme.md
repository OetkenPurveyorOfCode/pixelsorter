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

