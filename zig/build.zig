const std = @import("std");

// To build for Windows, run:
// zig build -target x86_64-windows-gnu && wine64 dist/console.exe
pub fn build(b: *std.build.Builder) void {
    const version = b.version(0, 0, 1);
    var target = b.standardTargetOptions(.{});
    const mode = b.standardReleaseOptions();

    // Build a shared lib
    const lib = b.addSharedLibrary("sdk", "src/main_lib.zig", version);
    // TODO(lbayes): Figure out how to emit a .h file for external
    // project inclusion.
    lib.setTarget(target);
    lib.setBuildMode(mode);
    lib.setOutputDir("dist");
    lib.linkLibC();
    lib.install();

    // Build a console client that loads the shared lib statically
    const exe = b.addExecutable("console", "src/main_console.zig");
    exe.setTarget(target);
    exe.setBuildMode(mode);
    exe.setOutputDir("dist");
    exe.linkLibC();
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
    // QUESTION(lbayes): How do I include multiple files for this test run?
    // e.g.:
    // tests.addFile("src/main_console.zig");

    // Run the tests
    const test_step = b.step("test", "Run library tests");
    test_step.dependOn(&tests.step);
}
