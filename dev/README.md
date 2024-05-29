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