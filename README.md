# CommonLib

CommonLib is a C++23 alternative to the STL.

## Requirements

- CMake 4.1 or newer
- A compiler that supports C++23

## Build

```sh
cmake -B build && cmake --build build
```

## Demo

There's a [demo file](./demo.cpp) showcasing the complete functionality provided by this library. Feel free to look at it. To run it:

```sh
./build/demo
```

## Tests

```sh
ctest --test-dir build
```

## License

This project is licensed under the Mozilla Public License Version 2.0. For more information, look at the [LICENSE](LICENSE) file.
