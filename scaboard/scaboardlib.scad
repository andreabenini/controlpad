/*
  SCADBoard Library & SCADuino 
  -------------------------------------------
  3D Printable Breadboard Library in OpenSCAD.
 
  File: SCADBoard_Lib_0_0_13.scad
 
  Copyright (c) 2014,  J. Pagliaccio, B. Reidy. All rights reserved.
  
  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:
  
   * Redistributions of source code must retain the above copyright notice,
     this list of conditions and the following disclaimer.
  
  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED 
  TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 
  By: J. Pagliaccio, B. Reidy
  
  Version History
  -------------------------------
  v0.0.1 First printed prototype.
  v0.0.2 Wider center line fix.
  v0.0.3 Added ic module.
  v0.0.4 Underwire.
  v0.0.5 LED Module.    
  v0.0.6 Better base and cleanup.    
  v0.0.7 Parameters for all values.
  v0.0.9 Separate x and y values for each hole for 2-wires-in-a-hole.
  v0.0.10 SCADuino with 2-wire.
  v0.0.11 SCADuino with 2-wire April 2014.
  v0.0.12 SCADuino with 2-wire cleanup 1/2 deep May 2014.
  v0.0.13 Added license and library file.
 
*/
 
 
/* //Start of required variables 
//
// THESE VARIABLES ARE REQUIRED
// and are here for reference.
// put them in your main sacd file. 
// 
  
drillHoleRad = 2; 
holeDiam = 1.25; // thru hole diameter in mm
holeLenX = holeDiam;
holeLenY = holeDiam;
 
// if two Wires in One Hole
holeLenY = holeDiam + .9;
 
holeSpace=2.54; // thru hole spacing in mm
// Nore: .1 inch = .0254 centimeters
  
// thickness of the board 
materialThick = 1.5; //2.54; // mm
 
// depth of the troughs 
inset = materialThick *.5; 
deepInset = materialThick *.5; 
 
// End of required variables 
*/
 
  
//Start of SCADBoard Library 
 
pos = "Red";
neg = "Black";
trace = "Orange";
thru = "Gray";
yellowled = "Yellow";
resistor = "Purple";
cap = "Cyan";
led = "Tomato";
 
blp=1; // Bus Left Pos
bln=2; // Bus Left Neg
a=3;
b=4;
c=5;
d=6;
e=7;
//----- columns 8 9 are the IC centerline
//----- remember columns 8 and 9 are usable  
f=10;
g=11;
h=12;
i=13;
j=14;
brp=15; // Bus Right pos
brn=16; // Bus Right neg
 
 
//----------------------------------------------------------------------------
// Push Button 
//----------------------------------------------------------------------------
module pushbutton(row1, col1, row2, col2, colorx)
{   
    hole(row1,col1,thru);
    hole(row1,col2,thru); 
    hole(row2,col1,thru);
    hole(row2,col2,thru); 
 
    translate([row1*holeSpace,col1*holeSpace,materialThick-deepInset])
    color(colorx) cube([holeSpace*(row2-row1)+holeLenX,
    (col2-col1)*holeSpace+holeLenY,5]);
}
 
//----------------------------------------------------------------------------
// IC - Integrated Circuit 
//----------------------------------------------------------------------------
module ic(row1, col1, row2, col2, colorx)
{
    for(n=[row1:1:row2-1])
    {
        hole(n,e,thru); // ATMEL
        hole(n,f,thru); // ATMEL  
    }
    translate([row1*holeSpace-holeSpace/4,col1*holeSpace,materialThick-inset])
    color("grey") cube([holeSpace*(row2-row1)+holeLenX,
    (col2-col1)*holeSpace+holeLenY,5]);
 
}   
 
// ---------------------------------------------------------------------------
// LED - Specifiy the positive hole first 
// ---------------------------------------------------------------------------
module led(row1, col1, row2, col2, colorx)
{
    hole(row1,col1,pos);
    hole(row2,col2,neg); 
  
    translate([row1*holeSpace+(row2-row1)*holeSpace/2+holeLenX/2,
    col1*holeSpace+((col2-col1)*holeSpace)/2+holeLenY/2, materialThick-inset])
    color(colorx) cylinder(h = materialThick*2, r = 6/2);
}
 
// ---------------------------------------------------------------------------
// HOLE - Thru pin hole for a trace
// ---------------------------------------------------------------------------
module hole(row, col, colorx)
{
    translate([row*holeSpace,col*holeSpace,-materialThick/2])
    color(colorx) cube([holeLenX,holeLenY,materialThick*2]);
}
  
// ---------------------------------------------------------------------------
// HULLHOLE - This is a hole at the end of the wire inset 
// This is used for making the wires with the hull module. 
//----------------------------------------------------------------------------
module hullhole(row, col, colorx)
{
    translate([row*holeSpace,col*holeSpace,materialThick/1.5])
    color(colorx)cube([holeLenX,holeLenY,materialThick]);
}
 
 
//----------------------------------------------------------------------------
// UNDER HULL HOLE - this should be replaced with hullhole and a new
// depth argument
//----------------------------------------------------------------------------
module underhullhole(row, col, colorx)
{
    translate([row*holeSpace,col*holeSpace,-.5*(materialThick)])
    color(colorx)cube([holeLenX,holeLenY,materialThick/1.25]);
}
  
//----------------------------------------------------------------------------
// WIRE - A trace form point A to point B
//----------------------------------------------------------------------------
module wire(row1, col1, row2, col2, colorx)
{
    hole(row1,col1,colorx);
    hole(row2,col2,colorx);
    color(colorx)
    hull()
    {
        hullhole(row1,col1,colorx);
        hullhole(row2,col2,colorx);
    }
}
 
//----------------------------------------------------------------------------
// UNDER WIRE - A trace form point A to point B
//----------------------------------------------------------------------------
module underwire(row1, col1, row2, col2, colorx)
{
    hole(row1,col1,colorx);
    hole(row2,col2,colorx);
    color(colorx)
    hull()
    {
        underhullhole(row1,col1,colorx);
        underhullhole(row2,col2,colorx);
    }
}
  
//----------------------------------------------------------------------------
// CREATE BASE BOARD
//----------------------------------------------------------------------------
module createboard(rows, colorx){
  
    pad = 2; // holeSpaces
    x=rows + pad; // rows + 1 + padding 
    y=17 + pad; // columns + 1 + padding 
   hoff = 1.2; // hole inset from corners  
 
    color(colorx, 1)
  
    difference() {
        // the base 
        translate([-holeSpace*pad/2,-holeSpace*pad/2,0])
        cube([x*holeSpace+holeLenX,y*holeSpace+holeLenY,materialThick]);
 
        union() {
            // holes lower left  
            translate([hoff,hoff,materialThick])
            cylinder (h = materialThick*2.5, r=drillHoleRad, center = true, $fn=20);
            // lower right 
            translate([holeSpace*x-drillHoleRad*2-hoff,hoff,materialThick])
            cylinder (h = materialThick*2.5, r=drillHoleRad, center = true, $fn=20);
            // upper left 
            translate([hoff,holeSpace*y-drillHoleRad*2-hoff,materialThick])
            cylinder (h = materialThick*2.5, r=drillHoleRad, center = true, $fn=20);
            // upper right 
            translate([holeSpace*x-drillHoleRad*2-hoff,holeSpace*y-drillHoleRad*2-hoff,materialThick])
            cylinder (h = materialThick*2.5, r=drillHoleRad, center = true, $fn=20);
        } 
    }
}
  
 
//----------------------------------------------------------------------------
// Standard Breadboard eg: All Holes 
//----------------------------------------------------------------------------
module standardBreadboard(x, y)
{
    for(n=[0:1:x])
    {
        for(ii=[0:1:y]) { hole(n,ii); }
    }
}
//End of SCADBoard Library 
