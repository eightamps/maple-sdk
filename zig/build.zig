const std = @import("std");

pub fn build(b: *std.build.Builder) void {
    const version = b.version(0, 0, 1);

    // Standard target options allows the person running `zig build` to choose
    // what target to build for. Here we do not override the defaults, which
    // means any target is allowed, and the default is native. Other options
    // for restricting supported target set are available.
    var target = b.standardTargetOptions(.{});

    // Standard release options allow the person running `zig build` to select
    // between Debug, ReleaseSafe, ReleaseFast, and ReleaseSmall.
    const mode = b.standardReleaseOptions();

    // Add a simpler param for callers to build Windows artifacts
    const windows = b.option(bool, "windows", "create windows build") orelse false;

    // Default library builds for nix.
    var lib_entry = "src/sdk_nix.zig";

    if (windows) {
        target = .{
            .cpu_arch = .x86_64,
            .os_tag = .windows,
            .abi = .gnu,
        };
        lib_entry = "src/sdk_win.zig";
    }

    // Build the SDK (for provided target)
    const lib = b.addSharedLibrary("maple-sdk", lib_entry, version);
    lib.setTarget(target);
    lib.setBuildMode(mode);
    lib.setOutputDir("dist");
    lib.linkLibC();
    // lib.emit_h = true;
    lib.install();

    // Build a console client from zig
    const exe = b.addExecutable("console", "examples/console.zig");
    exe.setTarget(target);
    exe.setBuildMode(mode);
    exe.setOutputDir("dist");
    exe.linkLibC();
    // How to make this dynamically link?
    exe.addPackage(.{
        .name = "maple-sdk",
        .path = lib_entry,
    });
    exe.step.dependOn(&lib.step);
    exe.install();

    const run_cmd = exe.run();
    run_cmd.step.dependOn(b.getInstallStep());

    const run_step = b.step("run", "Run the app");
    run_step.dependOn(&run_cmd.step);

    // Build a console client from C
    // const exeC = b.addExecutable("console-win", "examples/console.c");
    // exeC.setTarget(target);
    // exeC.setBuildMode(mode);
    // exeC.setOutputDir("dist");
    // exeC.linkLibC();
    // exeC.step.dependOn(&lib.step);
    // exeC.install();

    // exe.addCSourceFile("src/lib.c", &[_][]const u8{
    // "-Wall",
    // "-Wextra",
    // "-Werror",
    // });
    // exe.linkLibrary("maple-sdk");
    // exe.addPackage(pkgs.sdl);

    // Build tests
    var stitch_tests = b.addTest("src/stitch.zig");
    // stitch_tests.addFile("src/platform/audio_test.zig");
    stitch_tests.setBuildMode(mode);

    // Run the tests
    const test_step = b.step("test", "Run library tests");
    test_step.dependOn(&stitch_tests.step);
}
