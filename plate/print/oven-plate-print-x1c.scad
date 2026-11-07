// X1C print orientation: face down, rotated 45 degrees so the 260 mm plate fits the 256 mm bed (~251 mm footprint)
translate([128,128,8]) rotate([0,0,45]) rotate([180,0,0]) import("../oven-plate.stl");
