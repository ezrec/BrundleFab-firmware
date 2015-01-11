
BrundleFab - A Prototype Poweder Bed Printer

Axes & Tools
------------

X axis is from feed to part bins.
Y axis holds the print carriage.
Z axis is the part bin.
E axis is the powder feed bin.

Build area size is 175x260x230mm

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
G28 Xn Yn En			Home selected axes
G90				Absolute positioning
G91				Relative positioning

M0				Stop
M1				Sleep
M17				Enable motors
M18				Disable motors
M20 dirname			List SD files
M23 filename			Select SD file
M24				Start SD print
M25				Pause SD print
M26 Sn				Set position in SD file
M27				Report SD print position
M30 filename			Delete file from SD
M32 filename			Select SD and and printf
M36 filename			Return file information
M111 Sn				Set debug flags
M114				Get current position
M115				Get firmware version
M117 message			Display message
M124				Emergency stop

T0				Select null tool
T1 Pn Qn Rn Sn			Select ink tool
				  P: bitmap (bits 23..0) of nozzle
				  Q: bitmap (bits 47..24) of nozzle
				  R: bitmap (bits 71..48) of nozzle
				  S: Spray rate (dots / minute)
T2 .. T16			Additional ink heads
T20				Select heat lamp tool

Menu Tree
---------

Left - Back one menu
Right - Select/Down

0 Monitor
1 Status
2 Axis

1.0 X: xxxxx/xxxxx
1.1 Y: yyyyy/yyyyy
1.2 Z: zzzzz/zzzzz
1.3 E: eeeee/eeeee
1.4 T: n 

2.0 X Axis 
2.1 Y Axis
2.2 Z Axis
2.3 E Axis

2.n.0 Target: xxxx  (up-down selects increment/decrement)
2.n.1 Position: xxxx

