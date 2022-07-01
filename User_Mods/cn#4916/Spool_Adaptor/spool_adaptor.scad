$fn=72;

difference() {
  union() {
      scale([0.99,0.99,1])
      difference() {
    import("[Accent Color] Spool_Shaft.stl");
       translate([0,0,-0.1])
          cylinder(d=78,75);
      }

    cylinder(d=56,67);

for(i = [0:6]) {

rotate([0,0,51.4285*i]) {
    translate([23.4,-1,0])
        cube([16,2,67]);
}
  }
}

    translate([0,0,-0.1])
        cylinder(d=52,69);
}