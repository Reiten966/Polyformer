# Print Guide

- Material Usage as of (9/21/2022): 1255g (2000g with panels) Main color and 500g accent color (1.25g/mm3 density). (We are constantly working on reducing the usage)

- Before you print anything, please print the [**PolyCube**](https://github.com/Reiten966/Polyformer/blob/main/STL/PolyTools/PolyCube.STL) under the Tools folder and post your result in #print_help. The cube is a test for evaluating printer's performance and accuracy, we will help you intepret the result and suggest ways you can improve the quality.

- It is recommended to use Superslicer or Cura which has individual control for overhangs, thus Prusaslicer is not recommended. You can also slow down the perimeter speed and increase fan speed if your slicer doesn't support lowering overhang speed.

- Always dry your filament before printing mechanical parts, especially PETG. The blobs and strings cause by wet filaments can cause binding and malfunctioning the gearbox.

- The parts should be printed with at least **4 perimeters, 6 top and bottom layers**, set the seam type to "aligned" in Superslicer, or set to "sharpest corner" and "smart hiding" in Cura, **20% infill and 0.2mm layer height**. Adaptive Cubic/Cubic or gyro infill are usually pretty good. If you are using clear filament, Grid and honeycomb infill looks the nicest. You can also do concentric fill for your top and bottom layers which will look very nice.

- Please also adjust the expansion rate in the slicer, you can do so by scaling the model only in XY direction by percentage, ABS has about ~0.6% shrinkage, PETG has about ~0.15%, I am not too sure about PLA (it has a lesser tendency to shrink than PETG so probably negligible) but you can do a test by printing a long bar across the plate and measure the dimension to calculate the shrinkage.


- In regards to the temperature for heatset inserts, PLA is around 225 °C, PETG is 245 °C, and ABS is 265 °C.

- All the functional parts can be printed on a 200mm x 200mm build plate. The panels can be printed on a 220mm x 200mm plate.

