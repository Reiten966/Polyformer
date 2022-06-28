difference() {
  union() {
      scale([0.99,0.99,1])
    import("/Volumes/sniff/Downloads/_polyformer/[Accent Color] Spool_Shaft.stl");

    cylinder(d=60,67);

for(i = [0:11]) {

rotate([0,0,30*i]) {
    translate([23.4,0.6,1.0])
        cube([16,3,66]);
}
  }
}

    translate([0,0,-0.1])
        cylinder(d=56,69);
}