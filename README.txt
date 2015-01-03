
BrundleFab - A Prototype Poweder Bed Printer

Axes & Tools
------------

X axis is from feed to part bins.
Y axis holds the print carriage.
Z axis is the part bin.
E axis is the powder feed bin.

Tool 0 is the null tool
Tools 1..16 are the ink sprayers
Tool 20 is the heat lamp

GCode
-----

The 

The following GCode commands are supported:

G0 Xn Yn Zn En			Uncontrolled move
G1 Xn Yn Zn En			Controlled move
G20				Set units to inches
G21				Set units to mm
G28				Home all axes
G90				Absolute positioning
G91				Relative positioning
M0				Stop
M1				Sleep
M17				Enable motors
M18				Disable motors
M114				Get current position
M115				Get firmware version
M124				Emergency stop
T0				Select null tool
T1 Pn Qn Rn Sn			Select ink tool
				  P: (x) Width (in or mm) of tool pattern
				  Q: (y) Length (in or mm) of tool pattern
				  R: (z) Height (in or mm) of tool pattern
				  S: Size (in bytes) of tool pattern
				The next line immediately following is the
				Base64 encoded pattern.
T2 .. T16			Additional ink heads
T20				Select heat lamp tool
