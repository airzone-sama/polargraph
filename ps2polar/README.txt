
PS2POLAR - preamble to convert PostScript to Polargraph commands
	   Andy Kinsman, Victor, NY   comments to (sneelocke at aol.com)   12/12/2011
	   This code is placed freely into the PUBLIC DOMAIN and is my original
	   work.  It leverages the power of PostScript and its ability
	   to let you inspect the path that would be rendered by that language. 

DO THIS FIRST-

FIRST install Ghostscript on your windows box.  I used version 8.63 because it was 
already installed.  Adjust batch files accordingly.  Note that the 'c' version
of the code generates output to console (stdio) and is necessary to output the
text that becomes Polargraph command files.  The non-console version is used 
to preview images.  If you have other than a windows box, you are likely good
at adjusting things.

This example prints some string art with polargraph commands and takes about
an hour to complete.  Most postcript code can go through this converter, but 
error messages might be produced in the output command file, so it might be good
to inspect it with a text editor prior to sending it to the Polargraph.  Errors
that used to be produced by curveto commands in the PostScript path are now gone
due to my automatically doing a 'flattenpath' command before rendering the path.
This is good and bad, as it might cause the path to exceed the maximum permitted
length.

BATCH-

drag/drop			- It is useful to have a shortcut to Ghostscript on your desktop
				      so you can drag and drop files for preview
gs_ps2polar_string_art_preview  - run this to preview PS files with target grid behind them
gs_ps2polar_string_art_cmds     - converts PS to Polargraph commands producing the file 'string_art_cmds'
gs_ps2polar_test_image_cmds     - converts PS images to Polargraph commands producing a cmd file
gs_ps2polar_show_origins_cmds   - converts PS show_origins to Polargraph commands producing a cmd file.
gs_ps2polar_test_text_cmds      - converts PS test_text to Polargraph commands producing a cmd file
gs_ps2polar_string_art_C17cmds  - converts PS to Polargraph commands producing the file 'string_art_C17cmds'

POSTSCRIPT-

string_art     - code to make the string art example
debug_frame    - code to make debug background target
PS2POLAR       - code to convert PS to Polargraph commands.
PS2POLAR_C17   - Same as PS2POLAR, but with new C17 server commands added. Use only if your server supports these.
show_origins   - code to make registration marks on drawings
test_image     - code to test image operator
test_text      - code to test text printing


POLARGRAPH CMD FILES-

one_motor_rev_diamond_cmds - polar cmds to test motors
show_origins_cmds          - output from gs_ps2polar_show_origins_cmds batch file
string_art_cmds            - output from gs_ps2polar_string_art_cmds batch file
string_art_C17cmds         - output from gs_ps2polar_string_art_cmds batch file - that includes new C17 commands.
test_image_cmds	   	   - output from gs_ps2polar_test_image_cmds batch file
test_text_cmds		   - output from gs_ps2polar_test_text_cmds batch file
QA versions		   - kept to diff against when regression testing code

USEFUL POSTSCRIPT WORDS ADJUSTED for the POLARGRAPH-

pen-up	   - writes command to command file to pick up the pen.
pen-down   - writes command to command file to put down the pen.

stroke     - writes commands to the command file to render the current PS path by exploiting the
	     PS pathforall command.

sloppy-stroke - Used to send a less than perfect, but much faster path to the PG.  This was the
	     original code to perform the stroke operation, but the new stroke command chops long
	     lines into shorter ones to render them much better on the device. Not supported in
	     the PS2POLAR_C17 preamble.

show       - render PS text font in outline form for the string specified.

inch	   - allows you to move about in PS, IE. "12 inch 14 inch moveto"  (moves to coord x=12",y=14" 
             on the board)
cm         - same for our metric friends.

image      - implements the PS image operator with all of the translation, scaling, and rotational 
	     capabilities of the original PS operator. It is even faithful to the transfer function.
	     This results in a collection of 'pixel' requests of specific densities to the server
	     placing the pixels in rows as specified by the PS image matrix, and PS user matrix. At the end
	     of each row of pixels the Polargraph does a <cr> and does the next row until the image is
	     complete.  Due to the nature of the Polargraph to draw pixels with a NW-to-SE orientation
             (rotated 45 degrees clockwise) you get interesting results if you orient your image in 
	     strange ways.  Images plot best if you rotate them 45 degrees clockwise before printing,
	     but interesting results are possible in all orientations.  Only 8 bit gray scale image.
	     are implemented.

LIMITATIONS-
	Paths can only be 'stroke'ed in PS for the Polargraph.
		fill operations have no way to render on the PG machine.
	Only lines the width of your current pen are rendered on the Polargraph.
		'setlinewidth' commands perform no function in PS for the Polargraph.
	Only one color is rendered.
		'setgray' commands are ignored.  So if you even draw a white line
		normally invisible on a PS page, it turns out the color of your pen.
	Only outline fonts are rendered on the Polargraph.
		because a 'fill' operation cannot be performed on the PS path.

TUNING-
	Edit the PS2POLAR.PS file to correct the width between your pulleys on your Polargraph, the number
	of steps per revolution, and mm per revolution of your stepper motorsmotors.  Run the motor test 
	pattern command file to check these if you are uncertain of these values and measure the plotted
	pattern.

	Now assuming you are plotting in portrait mode-		

	Run a postscript sequence that makes a square, say  4 inches on a side.  Measure the square after
	you plot it.  If the top to bottom direction is taller or shorter than 4 inches, adjust the definition
	of 'inch' in the PS2POLAR.PS file.  Once you nail this dimension down, check the width of the square.
	If the square is wider or narrower than 4 inches, adjust the 'aspect_ratio' parameter in the file. 
	Make this value smaller to make your square more narrow, larger to make it wider.  Now you should be
	able to plot in inches as in the program below and have near perfect aspect ratios for circles that
	are round.

	%! Postscript to make 4 inch square
	10 inch dup moveto	% move to center of the Polargraph board
	4 inch 0 inch rlineto	% relative lineto 4 inches right
	0 inch 4 inch rlineto	% relative lineto 4 inches up
	-4 inch 0 inch rlineto  % relative lineto 4 inches left
	closepath		% close path
	stroke			% draw the path
	showpage
	

FAQ-

How do I use the resulting command file?   

The F4 option on the Polargraph controller code allows you import this command file.

How did you think of doing this?  

I wrote a similar front end for controlling a CNC wood router for a customer in town two decades ago.
Last Easter when I spotted the 'Eggbot' I considered improving its interface in a similar fashion.
When my brother recently built a Polargraph, my old retired fingers were twitching.  

What types of files are best to plot? 

Any vector type art you like is best.  Remember this machine is slow, so be willing to wait for it
to finish.  The accuracy is pretty good at about 200 steps in the x and y directions if you install
800 steps/rev stepper motors.  Artwork that includes filled paths can't be rendered with this code.
You can make signs with the font capabilites, and cut out the letters to make stencils as well.  Low
resolution images with high density changes are best to view.  You can doctor images with Photosshop
to increase the contrast, or better clip the levels to maximize the presentation on the Polargraph.
If your board is roughly 24x24 inches, expect to print images in the 60x60 pixel size with pixels about
1/4 inch in width and height.  Resize images to this size and view them on your screen to get an idea
as to how they will appear when rendered, or use the previewing technique shown here with Ghostscript.
This will save paper.

Brief- Limitations include-

Fonts are always rendered as outline fonts.
Any color requested gets the color of the current pen installed in the Polargraph.
Any line width requested gets the width of the curren pen installed in the Polargraph.
Images other than 8 bit grayscale are not rendered.


How do I get my image into PostScript so I can send it to the Polargraph in PostScript?

Good question.  On my computer I installed a local black and white PostScript printer.  Any one of the
older HP laserjet printers which did PostScript works fine.  I used a HP LaserJet IIIP.  When the widget
asks which printer port your printer is on scroll down the list (LPT1:, LPT2:,etc)  and find the option
to 'print to a file' at the bottom.  Now when you print to this newly installed device a box pops up
and asks you the name of the output file.  That will be a Postscript file.   Edit all the junk out of
the data, add in headers that look like my example and ship it to the Polargraph through the PS2POLAR
conversion code.  Many files generated by PS drivers only need to have PS2POLAR.PS pasted into the very
beginning of the file for them to run under Ghostscript and produce the correct Polargraph commands.
Some will require doctoring to remove all the obfuscated PS code and insert only the necessary around
image data. 



RELEASE NOTES-

V1.0

Keep text sent with 'show' commands to some reasonable length as all text is
converted to outline fonts which can possible exceed the maximum path length in Ghostscript.
Select any font that Ghostscript supports in the beginning of the PS2POLAR file.  Scale
it to your heart's content as well.

Eventually I may add a routine that clips all paths to the surface area of your plotting board.
I've scrapped this idea because long motions can walk outside of the drawing frame without
Ghostscript's knowledge, so clipping does no good in these cases. 

V1.1

Made a second pass on the aspect ratio.  Since I don't have two machines to test this on, your
aspect ratio factor may differ.  I think it may have to do with the spacing between motors.  
Mine has been calibrated and seems to make very round circles if you make a couple iterations 
on determining your aspect ratio factor.

Code added to support printing of PostScript images by redefining the PS 'image' operator.
Added a batch file to provide an example of converting a PostScript image for rendering.

Added a batch file to perform a QA test regression to make certain changes to the code don't
produce unpredictable results.  It compares cached known outputs against new outputs for three
specific output command files.

V1.2

Fixed text rendering that was broken in V1.1.  I've added a regression test to make certain it doesn't
break in the future.

This version encompasses a philosophy change.  I should be more loyal to rendering exactly
as postscript intended, processing the resulting path after it is manufactured with a tailored 'stroke' 
command.  This will be slow since many long lines will be chopped to shorter lines.   I now  
provide a 'sloppy-stroke' call for those who want the artistic droopy (but faster) lines the 
Polargraph would normally draw if I don't chop paths.  

Removed all words redefined except 'stroke', 'show', and 'image'.   Left 'sloppy-stroke' in for 
those who want to be artistic.

V1.3

Fixed sloppy line added by closepath operators, now it is a properly segmented line.
Which explained why text rendered always had a last line drawn for a letter just too sloppy.
Enhanced the 'motor test' command script to show 2 and 3 revolutions.

V1.4

Removed the targets included in PS2POLAR.PS, use show_origins.ps instead if you need them.
Removed unnecessary bind in lineto-seg routine.  
Removed a few lines of code in the Lineto-seg routine.
Added a 'tuning' section to this file.

V1.5

Remove support for sloppy-stroke in PS2POLAR_C17.
Add support for C17 commands to server - cuts down serial port traffic and cmd file size.
If you need wish to use C17 commands to control your server, find the file PS2POLAR_C17.ps,
delete the file PS2POLAR.ps (or rename it to something), and rename PS2POLAR_C17.ps to 
PS2POLAR.ps to replace the original PS2POLAR.ps file with the new version that includes C17 command
calls.  
