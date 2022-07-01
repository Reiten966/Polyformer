$fn=144;

difference() {
    scale([1.5,1.5,1])
import("/path/to/Polyformer/[Accent Color] Spool_Body.stl");

translate([0,0,-0.1])
    # cylinder(d=160,45.1);
}

translate([0,0,44])
    cylinder(d=155,3);