# Maple SDK (in Zig)

Exploring the options for building the Maple SDK in the Zig programming language & build platform.

# Get Started

* [Zig Language Tips](https://ziglearn.org/chapter-0/)
* [Zig Language Ref](https://ziglang.org/documentation/master/)
* [Offensive Zig Tips](https://github.com/darkr4y/OffensiveZig#cross-compiling)


# Build

Linux:
```bash
zig build && ./dist/console
```

Windows:
```bash
zig build -Dwindows=true && wine64 dist/console.exe
```

