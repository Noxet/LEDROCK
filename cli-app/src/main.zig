const std = @import("std");
const zig_serial = @import("serial");
const clap = @import("clap");

const params = clap.parseParamsComptime(
    \\-h, --help    Display this help and exit.
    \\-p, --port <str> The device port (defaults to /dev/ttyACM0).
    \\-m, --mode <mode> Specify the mode: static | fade | pulse
    \\
    \\                  static <color>
    \\
    \\                  fade <color from> <color to> <fade time>
    \\
    \\                  pulse <color from> <color to> <pulse time>
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

    var data = [_]u8{0} ** 20;
    var sendLen: u32 = 0;
    if (res.args.mode) |m| {
        const args = res.positionals[0];
        switch (m) {
            .static => {
                checkArgs(args, 1);
                const c = try parseColor(res.positionals[0][0]);
                // if (res.positionals[0].len > 1) {}
                data[0] = '\x01';
                data[1] = c.r;
                data[2] = c.g;
                data[3] = c.b;
                data[4] = '\n';
                sendLen = 5;
            },
            .fade => {
                checkArgs(args, 3);
                const cFrom = try parseColor(res.positionals[0][0]);
                const cTo = try parseColor(res.positionals[0][1]);
                const time = try parseTime(args[2]);
                data[0] = '\x02';
                data[1] = cFrom.r;
                data[2] = cFrom.g;
                data[3] = cFrom.b;
                data[4] = cTo.r;
                data[5] = cTo.g;
                data[6] = cTo.b;
                std.mem.writeInt(u32, data[7..][0..4], time, .little);
                data[11] = '\n';
                sendLen = 12;
            },
            .pulse => {
                checkArgs(args, 3);
                const cFrom = try parseColor(res.positionals[0][0]);
                const cTo = try parseColor(res.positionals[0][1]);
                const time = try parseTime(args[2]);
                data[0] = '\x03';
                data[1] = cFrom.r;
                data[2] = cFrom.g;
                data[3] = cFrom.b;
                data[4] = cTo.r;
                data[5] = cTo.g;
                data[6] = cTo.b;
                std.mem.writeInt(u32, data[7..][0..4], time, .little);
                data[11] = '\n';
                sendLen = 12;
            },
        }
    } else {
        std.debug.print("Mode option not specified\n", .{});
        std.process.exit(1);
    }

    const port = res.args.port orelse "/dev/ttyACM0";
    var serial = std.fs.cwd().openFile(port, .{ .mode = .read_write }) catch |err| {
        std.debug.print("Failed to open device: {}\n", .{err});
        return;
    };
    defer serial.close();

    try zig_serial.configureSerialPort(serial, zig_serial.SerialConfig{
        .baud_rate = 115200,
        .word_size = .eight,
        .parity = .none,
        .stop_bits = .one,
        .handshake = .none,
    });

    var writer = serial.writer(&.{});
    try writer.interface.writeAll(data[0..sendLen]);
}

pub fn printHelp() !void {
    return clap.helpToFile(.stderr(), clap.Help, &params, .{});
}

pub fn checkArgs(pos: []const []const u8, len: usize) void {
    if (pos.len < len) {
        std.debug.print("Not enough arguments\n", .{});
        std.process.exit(1);
    }
}

pub fn parseColor(st: []const u8) !Color {
    const r = try std.fmt.parseInt(u8, st[0..2], 16);
    const g = try std.fmt.parseInt(u8, st[2..4], 16);
    const b = try std.fmt.parseInt(u8, st[4..6], 16);
    const c: Color = .{ .r = r, .g = g, .b = b };
    return c;
}

pub fn parseTime(st: []const u8) !u32 {
    const t = try std.fmt.parseInt(u32, st, 10);
    return t;
}
