# cbld

cbld is a small and simple build tool for C and C++ projects.

It is designed around a plain-text project configuration and a small set of build commands, without requiring generated build files.

## Status

cbld is experimental and under development.

The project format and command interface may change.

## Dependencies

cbld requires:

* A C compiler
* A C++ compiler for C++ projects

## Building

Clone the repository and build cbld:

```sh
cbld build
```

The resulting executable is placed in the same directory where `cbld build` ran.

## Running

cbld uses a `cbld/` directory in the project root for configuration:

```text
cbld/
├── sources
├── cflags
├── cxxflags
├── ldflags
├── target
├── compiler
├── linker
└── commands/
```

A basic project can define its source files in `cbld/sources`:

```text
src/main.c
src/foo.c
src/bar.c
```

Build the project with:

```sh
cbld build
```

Custom commands can be placed in `cbld/commands/` and invoked with:

```sh
cbld cmd <command>
```

For more information:

```sh
cbld help
```

## Contributing

Contributions, bug reports, and ideas are welcome.

## License

See `LICENSE`.
