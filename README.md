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

The resulting executable is placed in the same directory where compile.sh ran.

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
