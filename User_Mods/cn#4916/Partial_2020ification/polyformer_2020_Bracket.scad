$fn = 72;

difference() {
union() {

import("../../STL/Main_Body (no supports unless noted)/Frame_Bracket_(Needs Supports).stl");

rotate([45,0,0]) {
translate([0,45,5])
cube([10+20,70-15,20], true);
    
*translate([5+20,45,5])
%cube([20,100,20], true);

*translate([-25,45,5])
%cube([20,100,20], true);
    
translate([-45,62.225,5])
    rotate([90,0,0])
    difference() {
        cylinder(d=13,8.5);
        translate([0,0,-1])
            cylinder(d=5.2,11);
}

translate([45,62.225,5])
    rotate([90,0,0])
    difference() {
        cylinder(d=13,8.5);
        translate([0,0,-1])
            cylinder(d=5.2,11);
}
}

}

rotate([45,0,0]) {
translate([5+20,67.5,5])
#cube(20.25, true);

translate([-25,67.5,5])
#cube(20.25, true);
}

}
