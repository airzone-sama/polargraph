/*
 * PG_preview
 *
 * convert command file into a PostScript for preview with Ghostscript
 * drag and drop your command file onto this executable.  The output
 * file "preview.ps" will appear several directories above in your file system.
 * No clue why it appears there, or how to make it appear in the current directory,
 * but it lands where it lands in "C:\Documents and Settings\username" onmy XP system.
 * Double click on it and GhostScript will provide a preview of the command file.
 * 
 * The bottom of the preview.ps file includes some statistics about the command file.
 *
 * Someday I will add a calculation on estimated plotting time for the command file.
 *
 * A. Kinsman 12/23/11
 *
 */

#include "stdafx.h"
#include "stdio.h"
#include "string.h"
#include "math.h"

// convert step count back to cartesian values
void to_cartesian(int *x, int *y, int Alen, int Blen)
{
	double steps_per_mm = 8.33333333;	// steps per mm on motor strings
	double mwidth = 1116.0; // space between motor pulleys
	double a,b;
	a=Alen/steps_per_mm;  // convert counts to mm
	b=Blen/steps_per_mm;
	*x=int( (pow(mwidth,2)-pow(b,2)+pow(a,2))/(mwidth*2) );
	*y=int( sqrt( pow(a,2) - pow(*x,2) ) );
}


int main(int argc, char* argv[])
{
	char line[256];		// input line from file
	char cmd[6];		// input command
	int	 x,y;			// output values
	int  ia,ib;           	// input values - length of a motor string and b motor string
	int  pen_up = true;	// pen state 1=up, 0=down
	int  mina = 20000;	// keep min/max on step counts
	int  minb = 20000;
	int  maxa = 0;
	int  maxb = 0;
	int  minx = 20000;	// keep min/max on coords
	int  miny = 20000;
	int  maxx = 0;
	int  maxy = 0;
	int  size,shade;		// size and shade of image pixels
	FILE *input_file;	
	FILE *output_file;
	int path_exists =false;
	
	// debug argc=2;
	// debug argv[1]=&"show_origins_cmds.txt";

    /* validate args */
    if( argc != 2 )
    {
       printf("usage: %s polargraph_command_file",argv[0]);
       return(1);
    }

    /* open file */
    if( (input_file = fopen(argv[1],"r")) == NULL )
    {
       printf("can't open input file\n");
       return(1);
    }
	if( (output_file = fopen("preview.ps","w")) == NULL )
	{
       printf("can't open output file\n");
       return(1);
    }

	/* some PS preamble */
	fprintf(output_file,"/inch {72 mul} def\n");
	fprintf(output_file,"0 7 inch translate\n");
	fprintf(output_file,"1 -1 scale\n");
	fprintf(output_file,".5 dup scale\n");
	fprintf(output_file,"\n");

	/* read a line */
	fgets(line,sizeof(line),input_file);
	while( !feof(input_file) )
	{

		// debug fprintf(output_file,"%s",line);

		/* check for commands */

		// pen up
		if( strncmp(line,"C14",3) == 0)
		{
			pen_up = true;		// pen goes up
			if( path_exists) 
			{
				fprintf(output_file,"stroke\n");	// stroke anything in path
				path_exists = false;
			}

		}
		// pen down
		else if( strncmp(line,"C13",3) == 0)
		{
			pen_up = false;
		}
		// move or new move straight
		else if( (strncmp(line,"C01",3) == 0) || (strncmp(line,"C17",3) == 0) )
		{
			/* obtain the parameters */
			sscanf(line,"%3s,%d,%d",cmd,&ia,&ib);

			/* convert them to cartesian values */
			to_cartesian(&x,&y,ia,ib);

			/* analyze them */
			if( ia < mina ) mina=ia;
			if( ia > maxa ) maxa=ia;
			if( ib < minb ) minb=ib;
			if( ib > maxb ) maxb=ib;

			if( x < minx ) minx=x;
			if( x > maxx ) maxx=x;
			if( y < miny ) miny=y;
			if( y > maxy ) maxy=y;

			/* process them */
			if(pen_up)
				fprintf(output_file,"%d %d moveto\n",x,y);
			else
				fprintf(output_file,"%d %d lineto\n",x,y);
			path_exists=true;
		}
		// place image pixel
		else if( (strncmp(line,"C05",3) == 0) || (strncmp(line,"C06",3) == 0) )
		{
			/* obtain the parameters */
			sscanf(line,"%3s,%d,%d,%d,%d",cmd,&ia,&ib,&size,&shade);

			/* convert them to cartesian values */
			to_cartesian(&x,&y,ia,ib);

			/* analyze them */
			if( ia < mina ) mina=ia;
			if( ia > maxa ) maxa=ia;
			if( ib < minb ) minb=ib;
			if( ib > maxb ) maxb=ib;

			if( x < minx ) minx=x;
			if( x > maxx ) maxx=x;
			if( y < miny ) miny=y;
			if( y > maxy ) maxy=y;

			/* process them */
			fprintf(output_file,"%4.2f setgray\n",shade/255.);
			fprintf(output_file,"%d %d moveto\n",x,y);
			fprintf(output_file,"%d %d %d 0 360 arc fill\n",x,y,size/10);
		}

		/* read another line */
	    fgets(line,sizeof(line),input_file);
	}

	fprintf(output_file,"%% string length stats A: %d to %d\n",mina,maxa);
	fprintf(output_file,"%% string length stats B: %d to %d\n\n",minb,maxb);

	fprintf(output_file,"%%       extents stats X: %d to %d\n",minx,maxx);
	fprintf(output_file,"%%       extents stats Y: %d to %d\n\n",miny,maxy);

	fprintf(output_file,"showpage\n\n");
	fclose(output_file);

	return 0;
}


