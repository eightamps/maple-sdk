const std = @import("std");

const Allocator = std.mem.Allocator;
const talloc = std.testing.allocator;

pub const Stitch = struct {
    allocator: *Allocator,

    pub fn init(a: *Allocator) !*Stitch {
        var instance = try a.create(Stitch);
        instance.* = Stitch{
            .allocator = a,
        };
        return instance;
    }

    pub fn deinit(self: *Stitch) void {
        self.allocator.destroy(self);
    }
};

test "Stitch is instantiable" {
    const s = try Stitch.init(talloc);
    defer s.deinit();
}
