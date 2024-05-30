# Bluepy

## Build

```shell
g++ -o bletoolkit bletoolkit.cc -L. -lbluepy $(pkg-config --cflags --libs glib-2.0)
```

```shell
export LD_LIBRARY_PATH=.:$LD_LIBRARY_PATH
sudo ./bletoolkit
```


```shell
g++ -o bletoolkit bletoolkit.cc -L. -lbluepy $(pkg-config --cflags --libs glib-2.0) -Wl,-rpath=.
sudo ./bletoolkit
```

cmake -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain.cmake -B build-arm64 -S source-directory

