/* solve.c

   GNU Chess frontend

   Copyright (C) 2001-2011 Free Software Foundation, Inc.

   GNU Chess is based on the two research programs
   Cobalt by Chua Kong-Sian and Gazebo by Stuart Cracraft.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

   Contact Info:
     bug-gnu-chess@gnu.org
     cracraft@ai.mit.edu, cracraft@stanfordalumni.org, cracraft@earthlink.net
*/


#include <stdio.h>
#include <string.h>
#include "common.h"

/* A line read from an EPD file */
extern char epd_line[];

void SolvePosition( char move[], const char position[] );

void Solve (char *file)
/*****************************************************************************
 *
 *
 *
 *****************************************************************************/
{
   int total, correct, found;
   char *p;
   char myMove[100]="";
   int elo;

   total = correct = 0;
   SET (flags, SOLVE);
   while (ReadEPDFile (file, 0))
   {
      NewPosition ();
      total++;
      ShowBoard ();
      SolvePosition( myMove, epd_line );
      p = solution;
      found = false;
      while (*p != '\0')
      {
         if ( (*myMove != '\0') && (!strncmp (p, myMove, strlen(myMove))) )
         {
	    correct++;
	    found = true;
	    break;
         }
         while (*p != ' ' && *p != '\0') p++;
	 while (*p == ' ' && *p != '\0') p++;
      }
      printf ("id: %s : ", id);
      printf (found ? "Correct:  " : "Incorrect:  ");
      printf ("<%s> %s\n", myMove, solution);
      printf ("Correct=%d Total=%d\n", correct, total);
   }
   CLEAR (flags, SOLVE);
}

