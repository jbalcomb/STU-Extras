





Algorithm for computer control of a digital plotter
by J. E. Bresenham
IBM SYSTEMS JOURNAL * VOL. 4 * NO. 1 . 1965

Bresenham's Line Algorithm Implemented for the NS32GX32
National Semiconductor
Application Note 611
Rick Pelleg
September 1989

BROWN UNIVERSITY
Department of Computer Science
Master's Project
CS-89-M6
"Raster Algorithms for 2D Primitives"
by Dilip Da Silva
...
RASTER ALGORITHMS FOR 2D PRIMITIVES
submitted by
Dilip Da Silva
In partial fulfillmen t of the requirements for the
Master of Science Degree
in Computer Science
Brown University
May 1989
Andries van Dam, advisor

Michael Abrash's Graphics Programming Black Book, Special Edition
Michael Abrash
1997-07-01
https://github.com/jagregory/abrash-black-book/blob/master/src/chapter-35.md
Chapter 35 -- Bresenham Is Fast, and Fast Is Good
Implementing and Optimizing Bresenham's Line-Drawing Algorithm

for a full mathematical treatment, I refer you to 
pages 433-436 of 
Foley and Van Dam's 
Fundamentals of Interactive Computer Graphics (Addison-Wesley, 1982)
...or...
pages 72-78 of the second edition of that book, which was published under the name 
Computer Graphics: Principles and Practice (Addison-Wesley, 1990)
...These sources provide the derivation of the integer-only, divide-free version of the algorithm, as well as Pascal code for drawing lines in one of the eight possible octants.



Advanced Programmers Guide to SuperVGAs
The line drawing routine shown here is based on Bresenham’s Algorithm, which is described in detail in the text Fundamentals of Interactive Computer Graphics by Foley and Van Dam.





##### vector
a vector (a quantity that has both an amount and a direction)

##### nabla  (AKA del  AKA anadelta)
symbol is called nabla (or del), and one thing it can do is calculate gradients
∇
The nabla is used in vector calculus as part of three distinct differential operators: the gradient (∇), the divergence (∇⋅), and the curl (∇×). The last of these uses the cross product and thus makes sense only in three dimensions; the first two are fully general.

##### delta
Δ

##### coordinate system


##### octants
0: M_1  0 <  y ≤  x
1: M_2  0 <  x <  y
2: M_3  0 < -x <  y
3: M_4  0 <  y ≤ -x
4: M_5  0 < -y ≤ -x
5: M_6  0 < -x < -y
6: M_7  0 <  x < -y
7: M_8  0 < -y ≤  x

in Abrash GPBB, he has the octants ordered counter-clock-wise and indexed by 0  (see Fig 35.4)
~ 0 & 3
  1 & 2
  4 & 7
  5 & 6
Octant 0:
    Increasing X, Increasing Y
    |DeltaX| > |DeltaY|
    DeltaX > 0
    DeltaY > 0
Octant 1:
    Increasing X, Increasing Y
    |DeltaY| > |DeltaX|
    DeltaX > 0
    DeltaY > 0


octant 0   X increases from start to finish
octant 3   X decreases from start to finish

 octants 1 and 2, the Y coordinate changes on every pixel

Octant0()
  * Draws a line in octant 0 or 3 ( |DeltaX| >= DeltaY ).
Octant1()
  * Draws a line in octant 1 or 2 ( |DeltaX| < DeltaY ).

8
into 4
into 2

Those four orientations form two major categories,  
orientations for which the X dimension is the major axis of the line  
orientations for which the Y dimension is the major axis of the line  

¿ ...where being "the major axis" means it is the one that gets incremented on every iteration ?  


https://github.com/jagregory/abrash-black-book/blob/master/images/35-04.jpg


if (dxabs >= dyabs) /* the line is more horizontal than vertical */
else /* the line is more vertical than horizontal */





Data Points
D_1 (x_1,y_1)
D_2 (x_2,y_2)

Δx = x2 - x1
Δy = y2 - y1

y = mx + b

x = x_i
y = y_i

slope is how much y for each x step

...slope is assumed to be between 0 and 1...
solve for m
some with (x1,y1) and (x2,y2)
    1) slope formula m = (y2 – y1)/(x2 – x1)
    2) standard line equation, y = mx + b
    The slope of the line, m can also be written as:  m = (y-b)/x

##### slope formula  m = (y2 – y1)/(x2 – x1)
m = (y2 – y1)/(x2 – x1)
m = Δy/Δx


if Δx = 1, then m = Δy/Δx reduces to m = Δy
...and Δx is 1, because the calculations are for the next x step




##### origin
screen is 320 by 200, from (0,0) to (319,199)
the origin is in the top-left
¿ means lines are always in octants 6 and 7 ?






negative gradients
reversed lines


e.g.,
Line(x1, y1, x2, y2)

x_delta = x2 - x1
y_delta = y2 - y1

if(y_delta < 0)







Algorithm for computer control of a digital plotter
by J. E. Bresenham
IBM SYSTEMS JOURNAL * VOL. 4 * NO. 1 . 1965


The data to be plotted are expressed in an (x, y) rectangular
coordinate system which has been scaled with respect to the mesh;
i.e., the data points lie on mesh points and consequently have
integral coordinates.






Bresenham's Line Algorithm Implemented for the NS32GX32

National Semiconductor
Application Note 611
Rick Pelleg
September 1989

There is no method of accurately printing all of the points on the continuous line described by the equation y e mx a b.  
geometrical shapes cannot truly be implemented by their theoretical definitions because the graphics system itself is discrete, not real or continuous  
For that reason, there has been a tremendous amount of research and development in the area of discrete or raster mathematics.
Many algorithms have been developed which "map" real-world images into the discrete space of as raster device.  

Bresenham's line-drawing algorithm (and its derivatives) is one of the most commonly used algorithms today for describing a line on a raster device.  
The agorithm was first published in Bresenham's 1965 article entitled "Algorithm for Computer Control of a Digital Plotter".  

This application note describes the fundamental algorithm  
and shows an implementation specially tuned for the NS32GX32 microprocessor.  
Although given in the context of this specific application note, the assembly level optimizations are relevant to general programming for the NS32GX32.  
Timing figures are given in Appendix C.

2.0 DESCRIPTION
Bresenham's line-drawing algorithm uses an iterative scheme.  
A pixel is plotted at the starting coordinate of the line, and each iteration of the algorithm increments the pixel one unit along the major, or x-axis.  
The pixel is incremented along the minor, or y-axis, only when a decision variable (based on the slope of the line) changes sign.  
A key feature of the algorithm is that it requires only integer data and simple arithmetic.  
This makes the algorithm very efficient and fast.  









BROWN UNIVERSITY
Department of Computer Science
Master's Project
CS-89-M6
"Raster Algorithms for 2D Primitives"
by Dilip Da Silva



The basic task of scan converting a primitive involves selecting the appropriate pixels that approximate the continuous mathematical representation of the primitive on a fixed integer grid.  
In approximating the primitive, we need to choose a meaningful measure of error and then try to minimize the error.

We start out using the value of the function as a means of deciding how close a pixel is to the primitive.  
This method works well for lines and circles, but is unreliable when used with ellipses.  
A better method, called the midpoint method,  
  indicates on which side of the primitive the midpoint between two pixels lies  
  and limits the distance between the pixel chosen and the primitive to one half the distance between two pixels.  
We shall use this method as a basis for the circle, standard ellipse and general ellipse algorithms.  

NW  N  NE
W   P  W
SW  S  SE

Fig. 1. Pixel, P. surrounded by 8 adjacent pixels

In order to make the scan-conversion algorithms efficient, we avoid floating point arithmetic,  
  and use incremental techniques to minimize the calculations required for each pixel plotted.  

  The basic strategy of incremental scan-conversion algorithms  
    is to choose the next pixel from the previously chosen pixel by determining which of the adjacent pixels lies closest to the curve being drawn.  
Figure 1 illustrates a pixel, P, surrounded by its 8 adjacent pixels, where the pixels are named by their relative geographical location.  
  However, the choice of the next pixel can be reduced to only a pair of adjacent pixels depending on which octant the slope of the curve is in at the currently selected pixel.  
  We define within which octant the slope of a curve lies by the slope of the curve and the direction in which we are tracking the primitive.  

If, for example, we are tracking a line with a slope of 1/2  
  and we are tracking the line from left to right,  
  then we define the slope of the line to lie within the first octant.
For the case when the slope of the curve lies in the first octant,  
  the choice of the next pixel is reduced to the pair, E and NE.  
Furthermore, the calculations used to determine the closer pixel of the pair can be done incrementally.  
That is, the calculations done to choose the current pixel can be used to simplify the calculations for the choice of the next pixel.

1. Scan-converting Lines
The task of scan-converting a line involves selecting the pixels that best approximate the equation of the line,
  f(x,y) = y - mx - b = 0





<HR>




Michael Abrash's Graphics Programming Black Book, Special Edition  
Michael Abrash  
1997-07-01  
https://github.com/jagregory/abrash-black-book/blob/master/src/chapter-35.md  
Chapter 35 -- Bresenham Is Fast, and Fast Is Good  


Line drawing quality and speed derive from two factors: The algorithm used to draw the line and the implementation of that algorithm.

The difficulty in drawing a line lies in generating a set of pixels that, taken together, are a reasonable facsimile of a true line. Only horizontal, vertical, and 1:1 diagonal lines can be drawn precisely along the true line being represented; all other lines must be approximated from the array of pixels that a given video mode supports, as shown in Figure 35.1.

Considerable thought has gone into the design of line-drawing algorithms, and a number of techniques for drawing high-quality lines have been developed. Unfortunately, most of these techniques were developed for powerful, expensive graphics workstations and require very high resolution, a large color palette, and/or floating-point hardware. 

Bresenham's line-drawing algorithm, on the other hand, is uniquely suited to microcomputer implementation in that it requires no floating-point operations, no divides, and no multiplies inside the line-drawing loop. Moreover, it can be implemented with surprisingly little code.

The key to grasping Bresenham's algorithm is to understand that when drawing an approximation of a line on a finite-resolution display, each pixel drawn will lie either exactly on the true line or to one side or the other of the true line. The amount by which the pixel actually drawn deviates from the true line is the error of the line drawing at that point. As the drawing of the line progresses from one pixel to the next, the error can be used to tell when, given the resolution of the display, a more accurate approximation of the line can be drawn by placing a given pixel one unit of screen resolution away from its predecessor in either the horizontal or the vertical direction, or both.

Let's examine the case of drawing a line where the horizontal, or X length of the line is greater than the vertical, or Y length, and both lengths are greater than 0.

The approach to approximation that Bresenham's algorithm takes is to move exactly 1 pixel along the major dimension of the line each time a new pixel is drawn, while moving 1 pixel along the minor dimension each time the line moves more than halfway between pixels along the minor dimension.

 The algorithm is a process of drawing a pixel at each possible coordinate along the major dimension of the line, each with the closest possible coordinate along the minor dimension. 
 The running error is used to keep track of when the coordinate along the minor dimension must change in order to remain as close as possible to the true line. 
 
 The above description of the case where X is the major dimension, Y is the minor dimension, and both dimensions are greater than zero is readily generalized to all eight octants in which lines could be drawn, as we will see in the C implementation.

for a full mathematical treatment, I refer you to pages 433-436 of Foley and Van Dam's Fundamentals of Interactive Computer Graphics (Addison-Wesley, 1982), or pages 72-78 of the second edition of that book, which was published under the name Computer Graphics: Principles and Practice (Addison-Wesley, 1990)
These sources provide the derivation of the integer-only, divide-free version of the algorithm, as well as Pascal code for drawing lines in one of the eight possible octants.

Strengths and Weaknesses
The overwhelming strength of Bresenham's line-drawing algorithm is speed. With no divides, no floating-point operations, and no need for variables that won't fit in 16 bits, it is perfectly suited for PCs.

The weakness of Bresenham's algorithm is that it produces relatively low-quality lines by comparison with most other line-drawing algorithms. In particular, lines generated with Bresenham's algorithm can tend to look a little jagged. On the PC, however, jagged lines are an inevitable consequence of relatively low resolution and a small color set, so lines drawn with Bresenham's algorithm don't look all that much different from lines drawn in other ways. Besides, in most applications, users are far more interested in the overall picture than in the primitive elements from which that picture is built. As a general rule, any collection of pixels that trend from point A to point B in a straight fashion is accepted by the eye as a line. Bresenham's algorithm is successfully used by many current PC programs, and by the standard of this wide acceptance the algorithm is certainly good enough.

Then, too, users hate waiting for their computer to finish drawing. By any standard of drawing performance, Bresenham's algorithm excels.


Looking at EVGALine

EVGALine next performs a simple check to cut in half the number of line orientations that must be handled separately. Figure 35.4 shows the eight possible line orientations among which a Bresenham's algorithm implementation must distinguish. (In interpreting Figure 35.4, assume that lines radiate outward from the center of the figure, falling into one of eight octants delineated by the horizontal and vertical axes and the two diagonals.) The need to categorize lines into these octants falls out of the major/minor axis nature of the algorithm; the orientations are distinguished by which coordinate forms the major axis and by whether each of X and Y increases or decreases from the line start to the line end.

 A moment of thought will show, however, that four of the line orientations are redundant.  
 Each of the four orientations for which DeltaY, the Y component of the line, is less than 0  
   (that is, for which the line start Y coordinate is greater than the line end Y coordinate)  
   can be transformed into one of the four orientations for which the line start Y coordinate is less than the line end Y coordinate simply by reversing the line start and end coordinates,  
   so that the line is drawn in the other direction.  
EVGALine does this by swapping (X0,Y0) (the line start coordinates) with (X1,Y1) (the line end coordinates) whenever Y0 is greater than Y1.


This accomplished, EVGALine must still distinguish among the four remaining line orientations.  
Those four orientations form two major categories, orientations for which the X dimension is the major axis of the line and orientations for which the Y dimension is the major axis.  







counter-clock-wise  (CCW)  
anti-clock-wise  (ACW)  





∵   Because
∴   Therefore
Δ   Delta
