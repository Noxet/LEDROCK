const std = @import("std");

// Although this function looks imperative, it does not perform the build
// directly and instead it mutates the build graph (`b`) that will be then
// executed by an external runner. The functions in `std.Build` implement a DSL
// for defining build steps and express dependencies between them, allowing the
// build runner to parallelize the build automatically (and the cache system to
// know when a step doesn't need to be re-run).
pub fn build(b: *std.Build) void {
    const windows = b.option(bool, "windows", "Target Windows") orelse false;

    const exe = b.addExecutable(.{
        .name = "cli-app",
        .root_module = b.createModule(.{ .root_source_file = b.path("src/main.zig"), .target = b.resolveTargetQuery(.{
            .os_tag = if (windows) .windows else .linux,
        }) }),
    });

    const serial_dep = b.dependency("serial", .{});
    exe.root_module.addImport("serial", serial_dep.module("serial"));

    const clap = b.dependency("clap", .{});
    exe.root_module.addImport("clap", clap.module("clap"));

    b.installArtifact(exe);

    const run_exe = b.addRunArtifact(exe);

    const run_step = b.step("run", "Run the app");
    run_step.dependOn(&run_exe.step);
}
