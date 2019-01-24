# Augdwarf

(i.e. "Augment with DWARF")
Module for adding DWARF v4 debugging symbols to translation chain of new LYaPAS compiler.

## Build

```bash
mkdir build && cd build
cmake -D LIBDWARF_LOCATION="<absolute path to libdwarf>" ..
cmake --build .
```
A note: libdwarf should be compliled with `./configure && make` beforehand.