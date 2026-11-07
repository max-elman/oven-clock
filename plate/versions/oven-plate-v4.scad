// Oven control-panel cover plate with a pocket for an Xteink X4 Classic/Pro e-ink reader.
// Samsung NQ70M7770 wall oven. Units: mm. Print face-down (front on the bed), ASA or PETG.
// The plate holds the reader against the glass; 4 magnet pairs hold the plate on the oven.

/* [Plate] */
plate_w     = 260.35;  // 10-1/4"
glass_h     = 101.6;   // the oven glass is 10.25 x 4"
trim_overlap = 13;     // plate runs this far onto the metal trim above and below the glass (trim is ~14 mm)
plate_h     = glass_h + 2*trim_overlap;
plate_t     = 8.0;     // total thickness
corner_r    = 6;       // corner radius seen from the front
edge_chamfer= 1.5;     // chamfer on the front edge

/* [Reader pocket] (measure the device when it arrives and adjust) */
dev_w       = 114.0;   // X4 Classic: 114 x 69 x 4.9
dev_h       = 69.0;
dev_t       = 4.9;
dev_r       = 2.5;     // pocket corner radius; the X4 Classic's corners are about 3 mm (v1 had 6, too round)
clearance   = 0.3;     // per side
pocket_x    = -5.5;    // = -win_x: pocket shifted so the window (not the reader) is centered on the plate
pocket_y    = 0;       // + = up

/* [Screen window] */
win_w       = 90.5;    // active area about 92.4 x 55.4; window a little smaller so no bezel shows
win_h       = 54;
win_x       = 5.5;     // screen sits ~5.5 mm toward the Reset/power end (5 mm bezel there, ~16 mm button chin); power end goes on the right, seen from the front
win_y       = 0;
win_chamfer = 1.2;     // bevel on the front of the window

/* [Magnets] FINDMAG 60 x 10 x 3 mm adhesive bar magnets (Walmart). One bar on the oven and one in
   the plate, both nested in the same slot so the plate sits flush. 4 pairs = 8 bars. */
mag_l       = 60;
mag_w       = 10;
mag_t       = 3;
mag_tape    = 0.4;     // adhesive foam thickness
mag_clear   = 0.5;     // added to length and width
mag_inset_y = 1.5 + (mag_w + mag_clear)/2;  // slot 1.5 mm in from the top/bottom edge, so both magnets sit on the trim, not the glass
mag_x       = 85;      // slot centers at +/- this from the plate center

/* [Pry notch] */
notch_w     = 30;
notch_d     = 2.5;

$fn = 72;
skin = plate_t - (dev_t + 0.4);              // front wall left over the reader
mag_pocket_t = 2 * (mag_t + mag_tape) + 0.2; // both magnets nest so the plate sits flush
assert(skin >= 1.2, "Front skin too thin: raise plate_t");
assert(plate_t - mag_pocket_t >= 0.8, "Magnet pocket breaks through: raise plate_t");

module rrect(w, h, r) {
  offset(r) square([w - 2*r, h - 2*r], center = true);
}

module plate_body() {
  // back face at z=0 (against the oven), front face at z=plate_t
  hull() {
    linear_extrude(plate_t - edge_chamfer) rrect(plate_w, plate_h, corner_r);
    linear_extrude(plate_t) rrect(plate_w - 2*edge_chamfer, plate_h - 2*edge_chamfer, corner_r - edge_chamfer);
  }
}

difference() {
  plate_body();

  // reader pocket, open to the back
  translate([pocket_x, pocket_y, -0.01])
    linear_extrude(dev_t + 0.4 + 0.01) rrect(dev_w + 2*clearance, dev_h + 2*clearance, dev_r);

  // screen window with a beveled front edge
  translate([pocket_x + win_x, pocket_y + win_y, 0])
    hull() {
      linear_extrude(plate_t - win_chamfer) rrect(win_w, win_h, 1);
      translate([0, 0, plate_t - 0.01]) linear_extrude(0.02) rrect(win_w + 2*win_chamfer, win_h + 2*win_chamfer, 1 + win_chamfer);
    }

  // magnet pockets
  for (sx = [-1, 1], sy = [-1, 1])
    translate([sx * mag_x, sy * (plate_h/2 - mag_inset_y), -0.01])
      linear_extrude(mag_pocket_t + 0.01) rrect(mag_l + mag_clear, mag_w + mag_clear, 1);

  // pry notch on the bottom edge, back side
  translate([0, -plate_h/2, 0])
    scale([notch_w/2, notch_d, notch_d]) sphere(r = 1, $fn = 48);
}
