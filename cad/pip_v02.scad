// Pip v0.2 - printable shell with real mounts
// Open in OpenSCAD, set PART below, F6 to render, File > Export > Export as STL.
// Units: mm. Parts print without supports at 0.2mm layers.

$fn = 72;

// ---- choose part ----
PART = "assembly";   // "shell_front", "shell_back", "face_bezel", "board_tray", "assembly"

// ---- dims ----
body_r   = 35;        // outer body sphere radius (v0.2 scale: ~14cm tall, fits most printers)
wall     = 2.2;
body_z   = 40;
split_x  = 0;         // shell splits into front/back along YZ plane

// SG90 servo: body 22.5 x 12.2 x 22.9, tabs 32.2 wide, 2mm deep
sg90     = [23.4, 13.2, 24];   // pocket w/ 0.4mm clearance
sg90_tab = [33.0, 2.4];

// ESP32 devkit 52 x 28 x 13 over pins; LiPo 1000mAh ~ 50 x 34 x 8
board    = [54, 30, 15];
battery  = [52, 36, 10];

module egg(r) { scale([1.0, 0.92, 1.08]) sphere(r = r); }

module shell_half(front = true) {
    // hollowed egg half with cutouts for servos/board/battery
    difference() {
        intersection() {
            translate([0, body_z, 0]) egg(body_r);
            // keep only front (z>0) or back (z<0)
            translate([0, 0, front ? 0 : -100]) cube([200, 200, 100], center = true);
        }
        translate([0, body_z, 0]) egg(body_r - wall);       // hollow
        translate([0, body_z, 0]) cube([body_r*3, body_r*3, 0.001], center = true); // seam face
        // foot servo pockets: horns face DOWN into feet, bodies in lower shell
        for (s = [-1, 1])
            translate([s*14, 12, 0]) rotate([0, 90, 0]) cube(sg90, center = true);
        // wing servo pockets: horns face OUT the sides
        for (s = [-1, 1])
            translate([s*(body_r*0.62), body_z + 4, 0]) rotate([0, 0, 90]) cube(sg90, center = true);
        // battery bay, low for center of mass
        translate([0, 16, 0]) cube(battery, center = true);
        // face window (front half only)
        if (front) translate([0, body_z + 34, body_r - 14]) scale([1, 0.72, 0.5]) sphere(r = 18);
    }
    // screw bosses at the seam, 4x M2 self-tap
    for (x = [-1, 1]) for (y = [-1, 1])
        translate([x * (body_r*0.62), body_z + y * (body_r*0.5), 0])
            difference() {
                cylinder(r = 3.2, h = 8, center = true);
                cylinder(r = 1.1, h = 10, center = true);
            }
}

module board_tray() {
    difference() {
        cube([board[0] + 6, board[1] + 6, 3], center = true);            // floor
        translate([0, 0, 2]) cube(board, center = true);                  // recess
    }
    // corner posts with M2 holes
    for (x = [-1, 1]) for (y = [-1, 1])
        translate([x * (board[0]/2 + 1.5), y * (board[1]/2 + 1.5), 4])
            difference() {
                cylinder(r = 2.6, h = 8, center = true);
                cylinder(r = 1.0, h = 10, center = true);
            }
}

module face_bezel() {
    // holds the 27x27mm OLED board behind the face window
    difference() {
        scale([1, 0.72, 0.5]) sphere(r = 19);
        cube([27.8, 27.8, 30], center = true);
        translate([0, 0, 12]) scale([1, 0.72, 0.5]) sphere(r = 17.5);
    }
}

if (PART == "shell_front") shell_half(true);
if (PART == "shell_back")  shell_half(false);
if (PART == "board_tray")  board_tray();
if (PART == "face_bezel")  face_bezel();
if (PART == "assembly") {
    color([0.96, 0.93, 0.86]) shell_half(true);
    color([0.93, 0.89, 0.80]) shell_half(false);
    translate([0, 16, 0]) color([0.4, 0.4, 0.45]) cube(battery, center = true);
}
