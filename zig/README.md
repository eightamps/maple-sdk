# Zig Project Questions
This project is being created to help me understand how best to structure a
Zig library project that loads **and generates** static and dynamic libraries
as well as executables across Windows and Linux.

## Expectations

1) Trigger a build on a Linux build machine and generate all build artifacts.

2) Develop primarily on Linux, some manual (occasional) validation on other
machines is expected, but shouldn't be required once that platform is
implemented.

3) Automated tests for all shared features, minimize complexity and
duplication in platform-specific code. Be able to run platform specific test
executables directly on whichever platform to verify API integrations.

## Specific Zig Questions

1) How do I conditionally load platform-specific implementation files?

In C, my Makefile (or CMakeLists.txt) just includes whichever .c file is
relevant for the given platform, and there is a shared .h file that all
platforms load against.

In Zig, I'm currently doing the following:

```zig
const platform = switch (std.Target.current.os.tag) {
    .windows => @import("api_win.zig"),
    .linux => @import("api_nix.zig"),
    else => @import("api_other.zig"),
};
```

2) What is the best way to handle platform-specific variations for application
entry points? Specifically `DllMain` for Windows DLLs.

3) How do I generate a .h file from build.zig so that external C apps can
refer to my Zig-based library?

4) What is the recommended way to specifically generate a variety of
platform-specific artifacts without forcing contributors to memorize a bunch
of random build target incantations?

## TLDR;
The following includes detailed explanations of the concrete use cases that
have led to these questions.

Hopefully, this effort will also help others on their journeys into Zigland.

I've managed to get some combination of C/C++ to approximately do what we're
looking for, but I'm concerned that I've also stepped on at least a handful
of foot guns and I don't even know how to find out how bad it is.

While I'm increasingly comfortable with C, I'm less comfortable with
sophisticated C Macros, and even less comfortable with the far reaching breadth
and depth that is C++.

## Requirements
We have built a set of devices that present themselves over a USB bus.

These devices include multiple audio codecs (speaker & mic), and a composite
USB device that presents a variety of HID interfaces including mouse, keyboard
gamepad and a handful of app-specific APIs over HID.

We use libusb to interact with the HID devices.

We use native sound APIs to interact with the audio devices.

One of the features we provide is a USB telephone. This device allows a person
to connect one of those wires your grandparents had in their walls (RJ11) and
make/receive Plain Old Telephone ([POTS](https://en.wikipedia.org/wiki/Plain_old_telephone_service))
calls from a host computer.

Our telephone hardware presents itself as 3 devices:

1) USB Telephone Microphone: Represents the outbound signal
2) USB Telephone Speaker: Represents the inbound signal
3) USB Custom HID Device: Provides control and line state services

These audio devices should not be confused with actual audio input/output
devices, as they are virtual devices that represent the signals on the phone
line. These signals need to be cross-connected with the related signals that
are physically connnected to the host computer.

This means that (for Windows users), we do the following for each audio pair
(from inbound phone signal to host speakers, and from mic to outbound phone 
signal).

1) Retrieve the default device under the **[Communication Role](https://docs.microsoft.com/en-us/windows/win32/coreaudio/using-the-communication-device)**
2) [Disable Audio Ducking behavior](https://docs.microsoft.com/en-us/windows/win32/coreaudio/disabling-the-ducking-experience)
3) Connect the inbound stream to the outbound device
4) [Configure Windows DSPs](https://docs.microsoft.com/en-us/windows-hardware/drivers/audio/audio-signal-processing-modes)
to reduce noise and echo

## Artifacts
We would like to create the following build artifacts:

1) [Vendor](https://stackoverflow.com/questions/26217488/what-is-vendoring) our
non-os-provided external dependencies (libusb.so & .dll)
2) Load the appropriate os-specific libraries and expose them to application logic
3) Test executable for shared functionality
4) Test executable for each supported platform that uses platform-specific
features
5) Generate the following artifacts
* api.dll: A Windows shared library 
* console.exe: A Windows console app that loads api.dll and exercises the API
* libapi.so: A Linux shared library
* console: A Linux console app that loads api.so and exercises the API
* gui: A Linux GTK application that statically links the library

## Deployments
1) The primary client to these features is a Windows application written in C#
that runs inside of a WPF GUI on .NET 4.6.1. We are not in control of this
application or it's environment. Our library is expected to ship alongside the
application binary as a DLL. It will be linked and loaded by C#. There will be
a simple C# wrapper that loads the DLL and simplifies datatype conversion.

2) Another client to these features is an automated test harness that runs on
a Raspberry PI (Running stock Raspbian). This application is currently a
C-based GTK GUI.

3) The tertiary client is an Ubuntu workstation where most software and firmware
development takes place. This environment expects to statically link the
library into a console executable.

4) The quaternary(?!) client is a Windows test rig where manual testing will
take place outside the main application. Ideally, this would use very similar
(same?) implementation details to the Raspberry PI interactive experience.

5) We hope to eventually build and distribute a collection of GUI applications
(SDL?) that could run across **all the things** (i.e., Windows, Linux, MacOS,
Web/WASM, iOS, Android, etc.).

