# bcbld

bcbld is a small and simple build tool for C and C++ projects.

It is designed around a plain-text project configuration and a small set of build commands, without requiring generated build files.

## Status

bcbld is experimental and under development.

The project format and command interface may change.

## Dependencies

bcbld requires:

* A C compiler
* A C++ compiler for C++ projects

## Building

Clone the repository and run compile.sh:

```sh
./compile.sh
```

On Windows, run compile.bat from the repository root instead. It uses `clang` if it is on `PATH`, and otherwise MSVC `cl` (from a Developer Command Prompt):

```bat
.\compile.bat
```

The resulting executable is placed in the same directory where the script ran.

### Windows notes

* bcbld passes GCC-style flags (`-c`, `-o`, `-I`) to the compiler and linker, so use `clang`/`clang++` or MinGW `gcc`/`g++` in `bcbld/compiler` and `bcbld/linker`. MSVC `cl` is not supported as the project compiler.
* Include the `.exe` extension in `bcbld/target` if you want it (e.g. `app.exe`).
* Custom commands in `bcbld/commands/` may be `.exe`, `.com`, `.bat` or `.cmd` files; `bcbld cmd run` finds `run.bat`, `run.exe`, etc.

## Running

bcbld uses a `bcbld/` directory in the project root for configuration:

```text
bcbld/
├── sources
├── cflags
├── cxxflags
├── ldflags
├── target
├── compiler
├── linker
└── commands/
```

A basic project can define its source files in `bcbld/sources`:

```text
src/main.c
src/foo.c
src/bar.c
```

Build the project with:

```sh
bcbld build
```

Custom commands can be placed in `bcbld/commands/` and invoked with:

```sh
bcbld cmd <command>
```

For more information:

```sh
bcbld help
```

## Contributing

Contributions, bug reports, and ideas are welcome.

## License

See `LICENSE`.
