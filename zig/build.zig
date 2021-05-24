const std = @import("std");

// Create shortcuts to OS tags
const windows_tag = std.Target.Os.Tag.windows;
const linux_tag = std.Target.Os.Tag.linux;

fn linkLibs(step: *std.build.LibExeObjStep, is_windows: bool, is_linux: bool) void {
    step.linkLibC();
    if (is_linux) {
        step.linkSystemLibrary("pulse");
    } else if (is_windows) {
        step.linkSystemLibrary("uuid");
        step.linkSystemLibrary("ole32");
        // step.linkSystemLibrary("kernel32");
        // step.linkSystemLibrary("gdi32");
        // step.linkSystemLibrary("user32");
        // step.linkSystemLibrary("advapi32");
        // step.linkSystemLibrary("comdlg32");
        // stesteplinkSystemLibrary("oleaut32");
    }
}

// To build for Windows, run:
// zig build -target x86_64-windows-gnu && wine64 dist/console.exe
pub fn build(b: *std.build.Builder) void {
    const version = b.version(0, 0, 1);
    var target = b.standardTargetOptions(.{});
    const mode = b.standardReleaseOptions();
    // Determine if this builder has been asked for a Windows binary.

    const curr_tag = if (target.os_tag != null) target.os_tag else std.builtin.os.tag;
    const is_windows = curr_tag == windows_tag;
    const is_linux = curr_tag == linux_tag;
    // std.debug.print("std.os.tag: {s}\n", .{std.builtin.os.tag});
    std.debug.print("current os tag: {s}\n", .{curr_tag});
    std.debug.print("Builder is_windows: {s}\n", .{is_windows});
    std.debug.print("Builder is_linux: {s}\n", .{is_linux});

    // Build a shared lib
    const lib = b.addSharedLibrary("sdk", "src/main_lib.zig", version);
    // TODO(lbayes): Figure out how to emit a .h file for external
    // project inclusion.
    lib.setTarget(target);
    lib.setBuildMode(mode);
    lib.setOutputDir("dist");
    linkLibs(lib, is_windows, is_linux);
    // lib.linkSystemLibrary("c");
    lib.install();

    // Build a console client that loads the shared lib statically
    const exe = b.addExecutable("console", "src/main_console.zig");
    exe.setTarget(target);
    exe.setBuildMode(mode);
    exe.setOutputDir("dist");
    exe.linkLibC();
    // How to make this dynamically link?
    linkLibs(exe, is_windows, is_linux);

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
    // QUESTION(lbayes): How do I include multiple files for this test run?
    // e.g.:
    // tests.addFile("src/main_console.zig");

    // Run the tests
    const test_step = b.step("test", "Run library tests");
    test_step.dependOn(&tests.step);
}
