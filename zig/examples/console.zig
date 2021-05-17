const std = @import("std");
const maple = @import("maple-sdk");

pub fn main() u8 {
    std.debug.print("Loaded maple-sdk from ZIG\n", .{});
    const result = maple.add(2, 4);
    std.debug.print("RESULT: {}\n", .{result});

    var foo = maple.stitchCreateMaple();
    std.debug.print("stitchCreateMaple: {}\n", .{foo});
    return 0;
}
