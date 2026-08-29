// Pip v0.1 - Waddl's first companion robot
// Parametric OpenSCAD concept model. Units: mm. Render: F5 / F6.
// 4x SG90 servos: 2 feet (waddle gait), 2 wings (flap). Head fixed on body shell.

$fn = 64;

// ---- parameters ----
body_r      = 70;     // body sphere radius
body_z      = 72;     // body center height
head_r      = 52;
head_off    = [0, 142, 12];
face_r      = 39;
face_off    = [0, 150, 61];
eye_r       = 9.5;
eye_dx      = 17.5;
beak_r      = 19;
beak_h      = 26;
beak_off    = [0, 125, 70];
wing_r      = 26;
wing_dx     = 70;
foot_r      = 23;
foot_dx     = 31;
tail_r      = 16;

// ---- colors ----
cream  = [0.96, 0.93, 0.86];
belly  = [0.99, 0.97, 0.93];
orange = [0.94, 0.57, 0.18];
glass  = [0.09, 0.11, 0.15];
eye_c  = [0.35, 0.88, 1.00];

module pip() {
    // body: egg
    color(cream) translate([0, body_z, 0]) scale([1.0, 0.92, 1.08]) sphere(r = body_r);
    // belly patch
    color(belly) translate([0, 64, 34]) scale([0.82, 0.78, 0.9]) sphere(r = 56);
    // head
    color(cream) translate(head_off) scale([1.05, 0.95, 1.0]) sphere(r = head_r);
    // face glass
    color(glass) translate(face_off) scale([1.0, 0.82, 0.40]) sphere(r = face_r);
    // eyes
    for (s = [-1, 1])
        color(eye_c) translate([s*eye_dx, 154, 81.5]) scale([0.8, 1.15, 0.4]) sphere(r = eye_r);
    // beak: wide flat cone
    color(orange) translate(beak_off) rotate([90, 0, 0]) scale([1.25, 0.62, 1.0])
        cylinder(r1 = beak_r, r2 = 0.5, h = beak_h);
    // wings
    for (s = [-1, 1])
        color([0.93, 0.89, 0.80]) translate([s*wing_dx, 76, 2]) rotate([0, 0, s*-16])
            scale([0.55, 1.0, 1.35]) sphere(r = wing_r);
    // feet + toes
    for (s = [-1, 1]) {
        color(orange) translate([s*foot_dx, 9, 9]) scale([1.15, 0.42, 1.5]) sphere(r = foot_r);
        for (dx = [-12, 0, 12])
            color(orange) translate([s*foot_dx + dx, 8.5, 31]) scale([1.0, 0.45, 1.4]) sphere(r = 8.5);
    }
    // tail nub
    color([0.93, 0.89, 0.80]) translate([0, 115, -66]) rotate([-40, 0, 0]) scale([1.2, 1.0, 0.8])
        cylinder(r1 = tail_r, r2 = 0.5, h = 30);
}

pip();

// print-plate layout (uncomment to lay parts flat):
// projection() pip();
