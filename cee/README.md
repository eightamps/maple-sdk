# Maple SDK (in C)
This project is intended to initially expose Maple Command Center features 
to any Linux host specifically for automated testing. 

Hopefully, this code will evolve to become a fully cross-platform host-side 
configuration and application foundation for a variety of Eight Amps 
hardware initiatives.

## Building
### TLDR;
To build for Test run:
```bash
./build.sh test
```
To build for Debug run:
```bash
./build.sh debug
```
To build for Release run:
```bash
./build.sh release
```

I'm currently trying out Jet Brain's CLion to smooth my ramp on gaining 
proficiency in C. Unfortunately, they lean into CMake, which so far,
I find occasionally magical, but mostly opaque and frustrating.

This complicates my standard process of using regular old Makefiles for  
builds and I've introduced a script at `build.sh` to at least make it 
possible to build the software from the command line without needing to 
remember and assemble ridiculously long and obnoxious incantations.

The first issue I bumped into, was that I'm running Ubuntu 20.04, which 
ships with CMake V3.16.x, but CLion uses an included version of CMake, which 
is at V3.19.x and the generated CMakeLists file requires this version. 

I'm not sure what changed across these versions, or what's really required, 
so I just went ahead and got the latest version installed using [these 
directions](https://askubuntu.com/questions/355565/how-do-i-install-the-latest-version-of-cmake-from-the-command-line).

## Development Notes
For anyone who is interested, I'm keeping a log of notes as this software is 
developed in [this directory](notes/).
