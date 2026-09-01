// Pip v0.3 - v0.2 shell + the parts that make it walk: rocker feet, wings, beak.
// Open in OpenSCAD, set PART, F6, export STL. Units mm. No supports at 0.2mm layers.
$fn = 72;

PART = "assembly"; // assembly | shell_front | shell_back | face_bezel | board_tray
                   // foot_left | foot_right | wing_left | wing_right | beak

// ---- dims ----
body_r   = 35;
wall     = 2.2;
body_z   = 40;

// SG90: body 22.5x12.2x22.9, tabs 32.2 wide; horn: 25T spline, ~5.5 dia neck
sg90      = [23.4, 13.2, 24];
sg90_tab  = [33.0, 2.4];
horn_boss = 6.2;     // horn neck dia + clearance
horn_spl  = 4.8;     // spline stub dia - press fit
board     = [54, 30, 15];
battery   = [52, 36, 10];

// ---- gait geometry (drives firmware too) ----
// Feet rock on a curved sole: sole radius sets the roll per degree of hip swing.
sole_r    = 55;      // rocker sole radius - bigger = calmer waddle
foot_w    = 30; foot_l = 46; foot_h = 10;
hip_x     = 14;      // foot servo offset from center

module egg(r) { scale([1.0, 0.92, 1.08]) sphere(r = r); }

module shell_half(front = true) {
    difference() {
        intersection() {
            translate([0, body_z, 0]) egg(body_r);
            translate([0, 0, front ? 0 : -100]) cube([200, 200, 100], center = true);
        }
        translate([0, body_z, 0]) egg(body_r - wall);
        translate([0, body_z, 0]) cube([body_r*3, body_r*3, 0.001], center = true);
        for (s = [-1, 1]) // foot servo pockets, horns down
            translate([s*hip_x, 12, 0]) rotate([0, 90, 0]) cube(sg90, center = true);
        for (s = [-1, 1]) // wing servo pockets, horns out
            translate([s*(body_r*0.62), body_z + 4, 0]) rotate([0, 0, 90]) cube(sg90, center = true);
        translate([0, 16, 0]) cube(battery, center = true);
        if (front) translate([0, body_z + 34, body_r - 14]) scale([1, 0.72, 0.5]) sphere(r = 18);
        if (front) translate([0, body_z + 18, body_r - 4]) rotate([90, 0, 0]) cylinder(d = 40, h = 30); // beak seat
    }
    for (x = [-1, 1]) for (y = [-1, 1])
        translate([x*(body_r*0.62), body_z + y*(body_r*0.5), 0])
            difference() { cylinder(d = 7, h = 3, center = true); cylinder(d = 1.7, h = 5, center = true); }
}

// ---- NEW IN v0.3 ----
// Rocker foot: sole is an arc of radius sole_r so a hip swing rolls the body
// onto the other foot. Servo horn presses into the horn pocket on top.
module foot(mirror = 1) {
    scale([mirror, 1, 1]) difference() {
        union() {
            intersection() {                       // rocker sole
                translate([0, sole_r - foot_h, 0]) cylinder(r = sole_r, h = foot_w, center = true);
                cube([foot_w, foot_h, foot_l], center = true);
            }
            translate([0, foot_h/2 - 1, 0]) cube([foot_w*0.8, 4, foot_l*0.7], center = true); // ankle block
        }
        translate([0, foot_h - 1, 0]) cylinder(d = horn_boss, h = 4, center = true); // horn neck seat
        translate([0, foot_h + 1, 0]) cylinder(d = horn_spl, h = 3, center = true);  // spline press fit
        translate([0, foot_h - 1, 0]) cube([20, 4, 2.2], center = true);             // horn arm slot
    }
}

// Wing: flat paddle, horn pocket at root, prints flat.
module wing(mirror = 1) {
    scale([mirror, 1, 1]) difference() {
        hull() {
            cylinder(d = 12, h = 3, center = true);
            translate([26, -6, 0]) scale([1, 1.6, 1]) cylinder(d = 14, h = 3, center = true);
        }
        cylinder(d = horn_boss, h = 5, center = true);
        cylinder(d = horn_spl, h = 3, center = true);
        cube([20, 4, 2.2], center = true);
    }
}

// Beak: two-part wedge, upper fixes to front shell, lower flaps on a 1.75mm
// filament hinge pin. Prints as one, snap apart at the score line.
module beak() {
    difference() {
        union() {
            translate([0, 0, 3])  rotate([8, 0, 0]) cube([22, 26, 5], center = true);   // upper
            translate([0, 0, -3]) rotate([-14, 0, 0]) cube([20, 24, 4], center = true); // lower
        }
        rotate([0, 90, 0]) cylinder(d = 2.0, h = 30, center = true);  // hinge bore
    }
}

module board_tray() {
    difference() {
        cube([board.x + 4, board.y + 4, 4], center = true);
        cube([board.x, board.y, 6], center = true);
        for (x = [-1, 1]) for (y = [-1, 1])
            translate([x*(board.x/2 - 3), y*(board.y/2 - 3), 0]) cylinder(d = 2, h = 8, center = true);
    }
}

module face_bezel() {
    difference() {
        scale([1, 0.72, 0.5]) sphere(r = 19.5);
        scale([1, 0.72, 0.5]) sphere(r = 17.5);
        translate([0, 0, -20]) cube([60, 60, 40], center = true);
    }
}

// ---- assembly view ----
module assembly() {
    color("Khaki")  shell_half(true);
    color("Khaki")  shell_half(false);
    color("Orange") translate([0, body_z + 18, body_r + 2]) rotate([90, 0, 0]) beak();
    color("Orange") for (s = [-1, 1]) translate([s*hip_x, 2, 0]) foot(s);
    color("Gold")   for (s = [-1, 1]) translate([s*(body_r*0.75), body_z + 4, 0]) rotate([0, s*90, 0]) wing(s);
    color("DimGray") translate([0, body_z - 12, 0]) board_tray();
}

if      (PART == "shell_front") shell_half(true);
else if (PART == "shell_back")  shell_half(false);
else if (PART == "face_bezel")  face_bezel();
else if (PART == "board_tray")  board_tray();
else if (PART == "foot_left")   foot(-1);
else if (PART == "foot_right")  foot(1);
else if (PART == "wing_left")   wing(-1);
else if (PART == "wing_right")  wing(1);
else if (PART == "beak")        beak();
else                            assembly();

// PRINT NOTES: feet print sole-down. Wings print flat. Beak prints as one
// piece; hinge pin is a 25mm length of 1.75mm filament. Shell halves print
// seam-down. All parts support-free at 0.2mm layers, ~150g total.
