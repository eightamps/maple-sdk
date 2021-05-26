const std = @import("std");
const print = std.debug.print;

const BuildTarget = std.build.Target;
const os_tag = std.Target.Os.Tag;

const windows_tag = os_tag.windows;
const linux_tag = os_tag.linux;

fn linkLibs(step: *std.build.LibExeObjStep, is_windows: bool, is_linux: bool) void {
    step.linkLibC();

    if (is_linux) {
        step.linkSystemLibrary("usb-1.0");
        step.linkSystemLibrary("pulse");
    } else if (is_windows) {
        step.addIncludeDir("vendor/libusb-win32/include");

        // TODO(lbayes): Copy the files found in here to dist/ when building
        step.addLibPath("vendor/libusb-win32/VS2019/MS64/dll");
        step.linkSystemLibraryName("libusb-1.0");

        step.linkSystemLibrary("uuid");
        step.linkSystemLibrary("ole32");
        step.addPackage(.{
            .name = "win32",
            .path = "vendor/win32/win32.zig",
        });
    } else {
        // is other
        print("link step for 'other'\n", .{});
    }
}

// To build for Windows, run:
// zig build -target x86_64-windows-gnu && wine64 dist/console.exe
pub fn build(b: *std.build.Builder) void {
    const version = b.version(0, 0, 1);
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
    const mode = b.standardReleaseOptions();
    // Determine if this builder has been asked for a Windows binary.

    const curr_tag = if (target.os_tag != null) target.os_tag else std.builtin.os.tag;
    const is_windows = curr_tag == windows_tag;
    const is_linux = curr_tag == linux_tag;
    // print("std.os.tag: {s}\n", .{std.builtin.os.tag});
    print("current os tag: {s}\n", .{curr_tag});
    print("Builder is_windows: {s}\n", .{is_windows});
    print("Builder is_linux: {s}\n", .{is_linux});

    // Build a shared lib
    const lib = b.addSharedLibrary("sdk", "src/main_lib.zig", version);
    // TODO(lbayes): Figure out how to emit a .h file for external
    // project inclusion.
    lib.setTarget(target);
    lib.setBuildMode(mode);
    lib.setOutputDir("dist");
    lib.force_pic = true;
    linkLibs(lib, is_windows, is_linux);
    lib.install();

    // Build a console client that loads the shared lib statically
    const exe = b.addExecutable("console", "src/main_console.zig");
    exe.setTarget(target);
    exe.setBuildMode(mode);
    exe.setOutputDir("dist");
    linkLibs(exe, is_windows, is_linux);

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

    // Build tests
    var tests = b.addTest("src/main_lib.zig");
    tests.setTarget(target);
    tests.setBuildMode(mode);

    // TODO(lbayes): Figure out how to build platform-specific test
    // containers.
    // tests.addPackage(.{
    // .name = "win32",
    // .path = "vendor/win32/win32.zig",
    // });

    linkLibs(tests, is_windows, is_linux);
    // QUESTION(lbayes): How do I include multiple files for this test run?
    // e.g.:
    // tests.addFile("src/main_console.zig");

    // Run the tests
    const test_step = b.step("test", "Run library tests");
    test_step.dependOn(&tests.step);
}
