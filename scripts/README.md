# Build and run targets

Run the script from any directory with Python 3.9 or newer:

```sh
python scripts/run_target.py src/main.cpp
python scripts/run_target.py src/test.cpp
python scripts/run_target.py tests/perft_tests.cpp
```

The default optimized flags are `-std=c++20 -O3 -march=native -flto -DNDEBUG`
plus the standard warning flags. Executables are written to the ignored `build/`
directory.

The compiler is selected from `CXX`, `g++`, then `clang++`. It can also be set
explicitly:

```sh
python scripts/run_target.py tests/perft_tests.cpp --compiler clang++
```

Build without running:

```sh
python scripts/run_target.py src/main.cpp --compile-only
```

Arguments after `--` are forwarded to the program:

```sh
python scripts/run_target.py tests/perft_tests.cpp -- --verbose
```
