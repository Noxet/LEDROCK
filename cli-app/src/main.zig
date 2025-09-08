const std = @import("std");
const zig_serial = @import("serial");
const clap = @import("clap");

const params = clap.parseParamsComptime(
    \\-h, --help    Display this help and exit.
    \\-p, --port <str> The device port (defaults to /dev/ttyACM0).
    \\-m, --mode <mode> Specify the mode
    \\<str>...
);

const Color = struct {
    r: u8,
    g: u8,
    b: u8,
};

pub fn main() !void {
    var gpa = std.heap.DebugAllocator(.{}){};
    defer _ = gpa.deinit();

    const Mode = enum { static, fade, pulse };
    const parsers = comptime .{
        .mode = clap.parsers.enumeration(Mode),
        .str = clap.parsers.string,
    };

    var diag = clap.Diagnostic{};
    var res = clap.parse(clap.Help, &params, parsers, .{
        .diagnostic = &diag,
        .allocator = gpa.allocator(),
    }) catch {
        std.debug.print("Failed to parse input!\n\n", .{});
        return printHelp();
    };
    defer res.deinit();

    if (res.args.help != 0)
        return printHelp();

    const c = try parseColor(res.positionals[0][0]);
    std.debug.print("color: {}\n", .{c});
    var data = [_]u8{0} ** 5;
    if (res.args.mode) |m| {
        switch (m) {
            .static => {
                std.debug.print("STATIC\n", .{});
                // if (res.positionals[0].len > 1) {}
                data[0] = '\x01';
                data[1] = c.r;
                data[2] = c.g;
                data[3] = c.b;
                data[4] = '\n';
            },
            .fade => {},
            .pulse => {},
        }
    } else {
        std.debug.print("Mode option not specified\n", .{});
    }

    const port = res.args.port orelse "/dev/ttyACM0";
    var serial = try std.fs.cwd().openFile(port, .{ .mode = .read_write });
    defer serial.close();

    try zig_serial.configureSerialPort(serial, zig_serial.SerialConfig{
        .baud_rate = 115200,
        .word_size = .eight,
        .parity = .none,
        .stop_bits = .one,
        .handshake = .none,
    });

    var writer = serial.writer(&.{});
    try writer.interface.writeAll(&data);
    std.debug.print("Data written\n", .{});
}

pub fn printHelp() !void {
    return clap.helpToFile(.stderr(), clap.Help, &params, .{});
}

pub fn parseColor(st: []const u8) !Color {
    const r = try std.fmt.parseInt(u8, st[0..2], 16);
    const g = try std.fmt.parseInt(u8, st[2..4], 16);
    const b = try std.fmt.parseInt(u8, st[4..6], 16);
    const c: Color = .{ .r = r, .g = g, .b = b };
    return c;
}
