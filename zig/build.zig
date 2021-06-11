const std = @import("std");

const BuildTarget = std.build.Target;
const CrossTarget = std.zig.CrossTarget;
const bld = std.build;
const os_tag = std.Target.Os.Tag;
const print = std.debug.print;

fn linkLibs(b: *bld.Builder, artifact: *bld.LibExeObjStep, target: CrossTarget) void {
    artifact.linkLibC();
    // Activate verbose linker, useful when troubleshooting linked library issues.
    // artifact.verbose_link = true;

    const curr_tag = if (target.os_tag != null) target.os_tag else std.builtin.os.tag;
    const is_windows = curr_tag == os_tag.windows;
    const is_linux = curr_tag == os_tag.linux;

    print("---------------------------------\n", .{});
    print("Link Libraries for: {s}\n", .{artifact.name});
    print("current os tag: {s}\n", .{curr_tag});
    print("Builder is_windows: {s}\n", .{is_windows});
    print("Builder is_linux: {s}\n", .{is_linux});

    if (is_linux) {
        artifact.linkSystemLibrary("usb-1.0");
        artifact.linkSystemLibrary("pulse");

        // Link vendor/libsoundio
        artifact.addIncludeDir("vendor/libsoundio/include");
        artifact.linkSystemLibraryName("soundio");
        artifact.addLibPath("vendor/libsoundio/linux/");
    } else if (is_windows) {
        // Configure libusb
        artifact.addIncludeDir("vendor/libusb-win32/include");
        // TODO(lbayes): Copy the files found in here to dist/ when building
        artifact.addLibPath("vendor/libusb-win32/VS2019/MS64/dll");
        artifact.linkSystemLibraryName("libusb-1.0");

        // Link vendor/libsoundio
        artifact.addIncludeDir("vendor/libsoundio/include");
        artifact.addLibPath("vendor/libsoundio/win64/");
        artifact.linkSystemLibraryName("soundio");

        // Get win32 zig packages
        artifact.addPackage(.{
            .name = "win32",
            .path = "vendor/win32/win32.zig",
        });
    } else {
        // is other
        print("link artifact for 'other'\n", .{});
    }
}

// To build for Windows, run:
// zig build -target x86_64-windows-gnu && wine64 dist/console.exe
pub fn build(b: *std.build.Builder) void {
    ///////////////////////////////////////////////////////////////////////////
    // Set up shared configurations
    ///////////////////////////////////////////////////////////////////////////
    const version = b.version(0, 0, 1);
    const mode = b.standardReleaseOptions();
    var target = b.standardTargetOptions(.{
        // Found whitelist here: https://github.com/andrewrk/zig-sdl/blob/master/build.zig
        .whitelist = &[_]BuildTarget{
            .{
                .cpu_arch = .x86_64,
                .os_tag = .linux,
                .abi = .musl,
            },
            .{
                .cpu_arch = .x86_64,
                .os_tag = .windows,
                .abi = .gnu,
            },
        },
    });

    ///////////////////////////////////////////////////////////////////////////
    // Build a library
    ///////////////////////////////////////////////////////////////////////////
    const lib = b.addSharedLibrary("sdk", "src/main_lib.zig", version);
    // TODO(lbayes): Figure out how to emit a .h file for external
    // project inclusion.
    lib.setTarget(target);
    lib.setBuildMode(mode);
    lib.setOutputDir("dist");
    lib.force_pic = true;
    linkLibs(b, lib, target);
    lib.install();

    ///////////////////////////////////////////////////////////////////////////
    // Build an executable
    ///////////////////////////////////////////////////////////////////////////
    const exe = b.addExecutable("console", "src/main_console.zig");
    exe.setTarget(target);
    exe.setBuildMode(mode);
    exe.setOutputDir("dist");
    linkLibs(b, exe, target);

    // How to make this dynamically link?
    exe.addPackage(.{
        .name = "sdk",
        .path = "src/main_lib.zig",
    });

    exe.step.dependOn(&lib.step);
    exe.install();

    const run_cmd = exe.run();
    run_cmd.step.dependOn(b.getInstallStep());

    const run_step = b.step("run", "Run the app");
    run_step.dependOn(&run_cmd.step);

    ///////////////////////////////////////////////////////////////////////////
    // Build tests
    ///////////////////////////////////////////////////////////////////////////
    var tests = b.addTest("src/main_lib.zig");
    tests.emit_bin = true;
    // tests.exec_cmd = "lldb-12";
    tests.setTarget(target);
    tests.setBuildMode(mode);
    linkLibs(b, tests, target);

    // TODO(lbayes): Figure out how to build platform-specific test
    // containers.
    // tests.addPackage(.{
    // .name = "win32",
    // .path = "vendor/win32/win32.zig",
    // });

    // QUESTION(lbayes): How do I include multiple files for this test run?
    // e.g.:
    // tests.addFile("src/main_console.zig");

    // Run the tests
    const test_step = b.step("test", "Run library tests");
    test_step.dependOn(&tests.step);
}
