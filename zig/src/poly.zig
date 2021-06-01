const std = @import("std");

const print = std.debug.print;

pub fn Abstract(comptime T: type, comptime R: type, val: u32) type {
    return struct {
        const Self = @This();

        impl: *T = T{},

        pub fn exec(self: *const Self) R {
            print("Abstract.exec with val: {d}\n", .{val});
            return self.impl.exec();
        }
    };
}

pub const ConcreteOneItem = struct {
    name: []const u8 = "one",
};

pub const ConcreteOne = struct {
    pub fn exec(self: *ConcreteOne) ConcreteOneItem {
        print("ConcreteOne.exec\n", .{});
        return ConcreteOneItem{};
    }
};

pub const ConcreteTwoItem = struct {
    name: []const u8 = "two",
};

pub const ConcreteTwo = struct {
    pub fn exec(self: *ConcreteTwo) ConcreteTwoItem {
        print("ConcreteTwo.exec\n", .{});
        return ConcreteTwoItem{};
    }
};

test "Create Abstract" {
    print("\n\n", .{});
    const one = Abstract(ConcreteOne, ConcreteOneItem, 42){};
    const oneItem = one.exec();
    print(">> one name: {s}\n", .{oneItem.name});

    const two = Abstract(ConcreteTwo, ConcreteTwoItem, 24){};
    const twoItem = two.exec();
    print(">> two name: {s}\n", .{twoItem.name});
}
