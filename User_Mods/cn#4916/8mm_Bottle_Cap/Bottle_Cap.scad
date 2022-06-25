$fn = 72;

import("./../../STL/Bottle_Cutter (no supports)/Bottle_Cap.STL");

difference() {
    cylinder(d=12,12.5);
    cylinder(d=8.2,16);
}