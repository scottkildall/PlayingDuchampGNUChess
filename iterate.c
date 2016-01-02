/* GNU Chess 5.0 - iterate.c - iterative deepening code
   Copyright (c) 1999-2002 Free Software Foundation, Inc.

   GNU Chess is based on the two research programs 
   Cobalt by Chua Kong-Sian and Gazebo by Stuart Cracraft.

   GNU Chess is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   GNU Chess is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GNU Chess; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

   Contact Info: 
     bug-gnu-chess@gnu.org
     cracraft@ai.mit.edu, cracraft@stanfordalumni.org, cracraft@earthlink.net
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "common.h"

#define WINDOW	75

short InChkDummy, terminal;

// internal functions -- PLAYING DUCHAMP
int playDuchampOpenings( int computerPlays );
int playDuchampOffense( int moveNum, char *moveStr );
int playDuchampDefense( int moveNum, char *moveStr );

// internal functions -- PLAYING DUCHAMP
int blunder();


//Added for web-based interface.  Need url encodeing

char to_hex(char code) {
  static char hex[] = "0123456789abcdef";
  return hex[code & 15];
}

//encodes a string for the web
void encode(char *src, char *last )
{
  char chr;
  for(; src != last; src++) {
    chr = *src;
    if( chr == ' ')
      printf( "+" );
    else if( isalnum(chr) || chr == '-' || chr == '_' || chr == '.' || chr == '~') {
        printf( "%c", chr);
    }
    else {
      printf("%%%c%c", to_hex(chr >> 4), to_hex( chr & 15) );
    }
  }
}


void Iterate (void)
/**************************************************************************
 *
 *  Begin the iterative deepening code.  The variable Idepth represents
 *  the iteration depth.
 *
 **************************************************************************/
{
   uint8_t side;
   int score, RootAlpha, RootBeta;

// PLAYING DUCHAMP
	// Play at maxIDepth vs 2100 SIGMA
	// at 7, Duchamp wins 1, loses 0
	int maxIDepth = 7;	// use this to control the level of play. 
//
	
   //dbg_printf("Entered iterate().\n");
   side = board.side;

   /* Note computer is playing side we are making move for. */
   computerplays = board.side;
	
	// PLAYING DUCHAMP -- added for openings
	if( (playDuchampOpenings(computerplays)) == 1 )
	{
		// Save the the .SANmv field of the Game array, since this is where the PGN file info gets stored
		dbg_printf("found openings and saving SANmv into array, index: = %d, move = %s\n", GameCnt, SANmv);
		strcpy (Game[GameCnt].SANmv, SANmv);
		goto skipSearch;
	}
	
	// ADD BLUNDER CODE HERE
	//duchampBlundered = 0;
	if( blunder() )
	{
		dbg_printf("BLUNDER!!\n");
		maxIDepth = 5;	// set IDepth == 5 in case of blunder
	}
	
	
   lazyscore[white] = lazyscore[black] = 150; /* was 50 */
   maxposnscore[white] = maxposnscore[black] = 150;
   GenCnt = 0;
   NodeCnt = QuiesCnt = 0;
   EvalCnt = EvalCall = 0;
   OneRepCnt = ChkExtCnt = RcpExtCnt = PawnExtCnt = HorzExtCnt = ThrtExtCnt = 0;
   KingExtCnt = 0;
   NullCutCnt = FutlCutCnt = 0;
   TotalGetHashCnt = GoodGetHashCnt = 0;
   TotalPutHashCnt = CollHashCnt = 0;
   TotalPawnHashCnt = GoodPawnHashCnt = 0;
   RootPawns = nbits (board.b[white][pawn] | board.b[black][pawn]);
   RootPieces = nbits (board.friends[white] | board.friends[black]) -
		RootPawns;
   RootMaterial = MATERIAL;
   RepeatCnt = 0;
   ElapsedTime = 0.0;
   StartTime = StartTiming();
   memset (ChkCnt, 0, sizeof (ChkCnt));
   memset (ThrtCnt, 0, sizeof (ThrtCnt));
   memset (history, 0, sizeof (history));
   memset (killer1, 0, sizeof (killer1));
   memset (killer2, 0, sizeof (killer2));
   CLEAR (flags, TIMEOUT);
   if (flags & TIMECTL)
   {
     /* 2 seconds for unknown latencies */

      SearchTime = (TimeLimit[side] - 2) / MoveLimit[side];

      /* Allocate 10% of total time as reserve */
      /* mcriley - was 9 * S /10 */

      SearchTime = 95 * SearchTime / 100;
      /* Reserve more if in sudden death */
      /* mcrikey was 8 * S / 10 */

      if (suddendeath) 
	SearchTime = 92 * SearchTime / 100;

	/* From loss against Ryo Saeba where we ran out of
	   time first move out of book and blundered pawn */
	/* Book lookups took too long */
      if ( nmovesfrombook <= 3 ){
	//printf ("Search time bonus near book\n");
	SearchTime = 1.5 * SearchTime;
      }

      /* Always target at least the increment seconds for the move! */
      /* To prevent huge surplus build-up. */

      if (TCinc != 0)
        if (SearchTime < TCinc) {
          //printf("TimeLimit[%s] = %6.2f\n",side == white ? "White" : "Black" ,TimeLimit[side]);
          if (TimeLimit[side] > 30) {   /* Only if > 15 seconds left */
            SearchTime = TCinc;
          }
        }
      ShowTime ();
   }
   Idepth = 0;
   TreePtr[2] = TreePtr[1];
   GenMoves (1);
   FilterIllegalMoves (1); 
   SortRoot ();
   
   InChk[1] = SqAtakd (board.king[side], 1^side);

   /*  Are there any legal moves? */
   if (GenCnt == 0)
   {
     if (!(flags & ENDED)) { /* Don't give result after mate as it will be muddled */
       //if (InChk[1]) {
  	//if (computerplays == black)
  	  //printf("1-0 {computer loses as black}\n");
  	//else if (computerplays == white)
  	  //printf("0-1 {computer loses as white}\n");
       //} else 
  	 //printf("1/2-1/2 {stalemate}\n");
        //fflush(stdout);
      }
      SET (flags, TIMEOUT | ENDED);
      return;
   }
   else if (GenCnt == 1)
   {
      RootPV = TreePtr[1]->move;
      SET (flags, TIMEOUT);
   }
   lastrootscore = score = Evaluate (-INFINITY, INFINITY);
      
   wasbookmove = 0;

   /* Don't look up moves in book in opponents time, think instead */

   if (bookmode != BOOKOFF && !(flags & SOLVE) && !(flags & PONDER) && nmovesfrombook <= 3) {
     //dbg_printf("Doing book lookup.\n");
     if (BookQuery(0) == BOOK_SUCCESS) {
       nmovesfrombook = 0;
       wasbookmove = 1;
       SET (flags, TIMEOUT);
     } else
       nmovesfrombook++;
   } else
   nmovesfrombook++;


/* mcriley - was 2 * searchtime */

   maxtime = 4 * SearchTime;

   if (flags & POST) {
     //printf ("Root = %d, ", score);
     //printf ("Phase = %d ", phase);
   }
   if (ofp != stdout) {
     fprintf (ofp, "Entered Iterate() for %s.\n",
		     flags & PONDER ? "pondering" : "analyzing");
     fprintf (ofp,"Root = %d\t", score);
     fprintf (ofp,"Phase = %d\t", phase);
   }
   if (SearchDepth == 0) {
      if (ofp != stdout) 
        fprintf (ofp,"\nTime = %.2f, Max = %.2f, Left = %.2f, Moves= %d\n", 
		 SearchTime,maxtime,TimeLimit[side],MoveLimit[side]);
  //    if (flags & POST) 
        //printf ("\nTime = %.2f, Max = %.2f, Left = %.2f, Moves = %d\n", 
//		SearchTime,maxtime,TimeLimit[side],MoveLimit[side]);
   } else {
      if (ofp != stdout)
        fprintf (ofp,"Depth = %d\n", SearchDepth);
      //if (flags & POST)
//	printf ("Depth = %d\n", SearchDepth);
   }

   if (flags & POST) {
     //printf("Ply   Time     Eval      Nodes   Principal-Variation\n");
     if (ofp != stdout)
       fprintf(ofp,"Ply   Time     Eval      Nodes   Principal-Variation\n");
   }
	
   while (!(flags & TIMEOUT)) 
   {
      if (score > MATE-255)
      {
	 RootAlpha = score-1;
	 RootBeta = MATE;
      }
      else if (score < -MATE+255)
      {
	 RootAlpha = -MATE;
	 RootBeta  = score+1;
      }
      else
      {
	 RootAlpha = MAX (score - WINDOW, -MATE);
	 RootBeta  = MIN (score + WINDOW,  MATE);
      }
      Idepth += 1; /* increase iteration depth */
	   
	   //dbg_printf("Interate(), iDepth = %d\n", Idepth );
	   
	   // PLAYING DUCHAMP
	   if( Idepth >  maxIDepth )
	   {   
		     dbg_printf("Max iDepth reached, time to break\n" );
		   break;
	   }
	   
      rootscore = -INFINITY-1;
      score = SearchRoot (Idepth, RootAlpha, RootBeta);
      if (score >= RootBeta && score < MATE && !(flags & TIMEOUT))
      {
         ShowLine (RootPV, score, '+');
         rootscore = -INFINITY-1;
         RootAlpha = RootBeta;
         RootBeta = INFINITY;
         score = SearchRoot (Idepth, RootAlpha, RootBeta);
      }
      /*  If we fail low, then research. */ 
      else 
      {
	if (score <= RootAlpha && !(flags & TIMEOUT))
        {
          ShowLine (RootPV, score, '-');
          rootscore = -INFINITY-1;
	  RootBeta = RootAlpha;
	  RootAlpha = -INFINITY;
          score = SearchRoot (Idepth, RootAlpha, RootBeta);
        }
      }

      /*  Print the final PV line  */
      ShowLine (RootPV, score, '.');
      lastrootscore = score;


      /* See if we have time to start another iteration */
      /* mcriley - was 2 * S / 3 */
      if (SearchDepth == 0 && (flags & TIMECTL) && ElapsedTime >= 2 * SearchTime / 3)
         SET (flags, TIMEOUT);

      if (abs(score) + Idepth >= MATE + 1) 
         SET (flags, TIMEOUT);

      if (!(flags & PONDER) && Idepth == SearchDepth) 
         break; 
   }


   /* 
    * Elapsed time is calculated in Search for timed games
    * work it out here for statistical purposes
    */
   ElapsedTime = GetElapsed (StartTime);

   /* For the moment, just bail out if pondering after search */
   if (flags & PONDER) return;
/***************************************************************************
 *
 *  We've finished the search.  Do things like update the game history,
 *  time control stuff and print the search result.
 *  mcriley - MoveLimit was = 60
 ***************************************************************************/
   SANMove (RootPV, 1);
   strcpy (Game[GameCnt+1].SANmv, SANmv);
   Game[GameCnt+1].et = ElapsedTime;
   MakeMove (side, &RootPV);
	
skipSearch:
	
   if (flags & TIMECTL)
   {
      if (suddendeath) {
	if (TimeLimit[side] > 0 && TimeLimit[side] <= 60)
	  MoveLimit[side] = 60;
	else
	  MoveLimit[side] = 35;
// 	printf("MoveLimit is %d\n",MoveLimit[side]);

 //	printf("TimeLimit is %f\n",TimeLimit[side]);
      } else
	MoveLimit[side]--;
      TimeLimit[side] -= ElapsedTime;
      if (TCinc != 0)
	TimeLimit[side] += TCinc;
      if (MoveLimit[side] == 0)
      {
         MoveLimit[side] = TCMove;
      }
   }

   if (flags & XBOARD) 
   {
     // printf ("%d. ... %s\n", GameCnt/2 + 1, AlgbrMove(RootPV));
      char *comp_move = AlgbrMove(RootPV);
      dbg_printf ("My move is: %s\n", comp_move);
      printf ("\ncomputerMove=");
      encode(comp_move, (comp_move+strlen(comp_move)) );
      fflush(stdout);
      if (ofp != stdout) {
        fprintf (ofp,"%d. ... %s\n", GameCnt/2 + 1, AlgbrMove(RootPV));
        fprintf (ofp,"My move is: %s\n", AlgbrMove(RootPV));
        fflush(ofp);
      }
   }
   else
   {
/*
      if (!wasbookmove) {
        fprintf (ofp,"\nTime = %.1f Rate=%ld Nodes=[%ld/%ld/%ld] GenCnt=%ld\n", 
		 ElapsedTime, ElapsedTime > 0 ? 
	 (unsigned long)((NodeCnt + QuiesCnt) / ElapsedTime) : 0, 
		 NodeCnt, QuiesCnt, NodeCnt+QuiesCnt, GenCnt);
        fprintf (ofp,"Eval=[%ld/%ld] RptCnt=%ld NullCut=%ld FutlCut=%ld\n",
          EvalCnt, EvalCall, RepeatCnt, NullCutCnt, FutlCutCnt);
        fprintf (ofp,"Ext: Chk=%ld Recap=%ld Pawn=%ld OneRep=%ld Horz=%ld Mate=%ld KThrt=%ld\n",
          ChkExtCnt, RcpExtCnt, PawnExtCnt, OneRepCnt, HorzExtCnt, ThrtExtCnt,
	  KingExtCnt);
        fprintf (ofp,"Material=[%d/%d : %d/%d] ", 
		 board.pmaterial[white], 
		 board.pmaterial[black], 
		 board.material[white], 
		 board.material[black]);
      fprintf (ofp,"Lazy=[%d/%d] ", lazyscore[white], lazyscore[black]);
      fprintf (ofp,"MaxPosnScore=[%d/%d]\n",maxposnscore[white],maxposnscore[black]);
        fprintf (ofp,"Hash: Success=%ld%% Collision=%ld%% Pawn=%ld%%\n",
           GoodGetHashCnt*100/(TotalGetHashCnt+1),
           CollHashCnt*100/(TotalPutHashCnt+1),
           GoodPawnHashCnt*100/(TotalPawnHashCnt+1));
      }
*/
      //if (!(flags & SOLVE)) ShowBoard ();
      dbg_printf ("\nMy move is : %s\n", SANmv);
      printf ("\ncomputerMove=", SANmv);
      encode(SANmv, (SANmv+strlen(SANmv)) );
      //fflush(stdout);
      //if (ofp != stdout) {
      //  fprintf (ofp,"\nMy move is : %s\n", SANmv);
      //  fflush(ofp);
     // }
   }

/***************************************************************************
 *
 *  Before we leave, check to see if this is mate or stalemate.
 *
 ***************************************************************************/
   TreePtr[2] = TreePtr[1];
   GenMoves (1);
   FilterIllegalMoves (1); 
   if (TreePtr[1] == TreePtr[2])
   {
     //if (SqAtakd (board.king[board.side], 1^board.side)) {
       //if (computerplays == black)
	 //printf("0-1 {computer wins as black}\n");
       //else
	 //printf("1-0 {computer wins as white}\n");
     //} else
       //printf("1/2-1/2 {stalemate}\n");
     //fflush(stdout);
     SET (flags, ENDED);
   }
   if (EvaluateDraw () || Repeat() >= 2)
   {
      //printf("1/2-1/2 {draw}\n");
 //     fflush(stdout); 
      SET (flags, ENDED);
   }
} 


double GetElapsed (Timer start)
{
   struct timeval t;
   gettimeofday (&t, NULL);
   return t.tv_sec - start.tv_sec + (t.tv_usec - start.tv_usec) / 1e6;
}

Timer StartTiming (void)
{
   Timer t;
   gettimeofday (&t, NULL);
   return t;
}

// PLAYING DUCHAMP
// ADD CODE HERE FOR ANALYZING MOVES, PERFORMING RANDOMNESS CHECK
int playDuchampOpenings( int computerPlays )
{
	leaf *ptr = NULL;
	char moveStr[24];
	int moveFound = 0;
	
	moveStr[0] = 0;
	dbg_printf("playDuchampOpenings(), computer plays = %d & GameCnt = %d\n", computerPlays, GameCnt);
	
	if( computerPlays == white )
	{	// GameCnt = # of moves. For Computer = Black, this will always be odd
		moveFound = playDuchampOffense( GameCnt+2, moveStr );
	}
	else
	{	// GameCnt = # of moves. For Computer = Black, this will always be even
		moveFound = playDuchampDefense( GameCnt+2, moveStr );
	}
	
	if( moveStr[0] != 0 )
	{
		dbg_printf("Move string= %s\n", moveStr);
		ptr = ValidateMove(moveStr);
		if (ptr != NULL) {
			SANMove (ptr->move, 1);
			///encode(SANmv, (SANmv+strlen(SANmv)) );	///just added
			MakeMove (board.side, &ptr->move);
			dbg_printf("Made move!\n");
			return 1;
		}
		dbg_printf("ValidateMove() returned FALSE\n");
	}
	else
		dbg_printf("no move made\n");

	return 0;
}

// Computer is white
int playDuchampOffense( int moveNum, char *moveStr )
{
	int randNum = rand();
	int foundMove = 0;
	
	dbg_printf("Analyzing offensive play move count= %d\n", moveNum);

	switch( moveNum )
	{
		// Black's opener
		case 1:
			randNum = randNum % 31;	// 31 games in opening book
			if( randNum == 0 )
				strcpy( moveStr, "e4" ); // 1/31 is e4
			else if( randNum < 7 )
				strcpy( moveStr, "Nf3" ); // 6/31 is Nf3
			else
				strcpy( moveStr, "d4" ); // 24/31 is d4
			foundMove = 1;
			break;
		
		// Black's 2nd move
		case 3:
			// IF we started with Kings Pawn
			if( strcmp( Game[0].SANmv, "e4" ) == 0 )
			{	// 1st move == e4
				if( strcmp( Game[1].SANmv, "f6" ) == 0 )
				{
					strcpy( moveStr, "e5" );
					foundMove = 1;
				}
			}
			
			// If we started with Nf3
			if( strcmp( Game[0].SANmv, "Nf3" ) == 0 )
			{	if( strcmp( Game[1].SANmv, "Nc6" ) == 0 )
				{
					strcpy( moveStr, "c4" );
					foundMove = 1;
				}
				else if( strcmp( Game[1].SANmv, "d5" ) == 0 )
				{
					strcpy( moveStr, "c4" );
					foundMove = 1;
				}
				if( strcmp( Game[1].SANmv, "Nf6" ) == 0 )
				{
					strcpy( moveStr, "c4" );
					foundMove = 1;
				}
			}
			
			// IF we started with Queen's Pawn
			if( strcmp( Game[0].SANmv, "d4" ) == 0 )
			{	// if opponent responds with Queen's pawn
				if( strcmp( Game[1].SANmv, "d5" ) == 0 )
				{
					randNum = randNum % 5;	 

					// 20% chance we respond we respond with the knight
					if( randNum == 0 )
						strcpy( moveStr, "Nf3" );
					// otherwise, respond with the gambit
					else
						strcpy( moveStr, "c4" );
					foundMove = 1;
				}
				
				// if opponent responds with Kings's pawn
				else if( strcmp( Game[1].SANmv, "e6" ) == 0 )
				{
					strcpy( moveStr, "c4" );
					foundMove = 1;
				}
				
				else if( strcmp( Game[1].SANmv, "f5" ) == 0 )
				{
					strcpy( moveStr, "g3" );
					foundMove = 1;
				}

				else if( strcmp( Game[1].SANmv, "Nf6" ) == 0 )
				{
					strcpy( moveStr, "c4" );	
					foundMove = 1;
				}
				
			}
			break;
		
		// Black's 2nd move
		case 5:
			// the e3-Nf6-e5-Nd5-c4 sequence from Kleczynski 1924 
			if( (strcmp(Game[0].SANmv,"e4")==0) && (strcmp(Game[1].SANmv,"Nf6")==0) && (strcmp(Game[2].SANmv,"e5")==0) && (strcmp(Game[3].SANmv,"Nd5")==0)  )
			{
				strcpy( moveStr, "c4" );	
				foundMove = 1;
			}
			else if( (strcmp(Game[0].SANmv,"d4")==0) && (strcmp(Game[1].SANmv,"Nf6")==0) && (strcmp(Game[2].SANmv,"c4")==0) && (strcmp(Game[3].SANmv,"e6")==0)  )
			{
				randNum = randNum % 13;	 
				
				// 10/13 or 3/13 responses with e6 (common)
				if( randNum < 4 )
				{	
					dbg_printf( "5-move e6 move 4 with Nc3 response\n" );
					strcpy( moveStr, "Nc3" );
				}
				else
				{	
					dbg_printf( "5-move e6 move 4 with Nf3 response\n" );
					strcpy( moveStr, "Nf3" );
				}
				foundMove = 1;
			}
			
			break;
	}
	
	return foundMove;
}

int playDuchampDefense( int moveNum, char *moveStr )
{
	int randNum = rand();
	int foundMove = 0;
	
	dbg_printf("Analyzing defensive play, move count= %d\n", GameCnt);
	
	switch( moveNum )
	{
		case 2:
			// opner is Nf3
			if( (strcmp(Game[0].SANmv,"Nf3")==0))
			{
				strcpy( moveStr, "Nf6" );
				foundMove = 1;
			}
			// opner is c4
		   else if( (strcmp(Game[0].SANmv,"c4")==0))
			{
				randNum = randNum % 4;
				if( randNum == 0 )
					strcpy( moveStr, "e5" );		// 25% of time respond with e5
				else
					strcpy( moveStr, "Nf6" );		// 75% of the respond with Nf6
				foundMove = 1;
			}
			else if( (strcmp(Game[0].SANmv,"d4")==0))
			{
				randNum = randNum % 18;
				if( randNum < 2 )
					strcpy( moveStr, "d5" );		// 2/18 of time respond with d5
				else
					strcpy( moveStr, "Nf6" );		// 16/18 of the respond with Nf6
				foundMove = 1;
			}
			else if( (strcmp(Game[0].SANmv,"e4")==0))
			{
				randNum = randNum % 17;
				if( randNum < 2 )
					strcpy( moveStr, "c5" );		// 2/17 of time respond with c5
				else if( randNum < 9 )
					strcpy( moveStr, "Nf6" );		// 6/17 of the respond with Nf6
				else
					strcpy( moveStr, "c6" );		// 9/17 of the respond with Nf6
				foundMove = 1;
			}
			break;
			
			case 4:
			if( (strcmp(Game[0].SANmv,"c4")==0) && (strcmp(Game[1].SANmv,"Nf6")==0) && (strcmp(Game[2].SANmv,"Nc3")==0) )
			{
				strcpy( moveStr, "e6" );
				foundMove = 1;
			}
			else if( (strcmp(Game[0].SANmv,"d4")==0) && (strcmp(Game[1].SANmv,"Nf6")==0) && (strcmp(Game[2].SANmv,"Nf3")==0) )
			{
				randNum = randNum % 9;
				if( randNum < 2 )
					strcpy( moveStr, "e6" );	// 2/9 respond with e6
				else
					strcpy( moveStr, "b6" );	// 7/9 respond with b6
				foundMove = 1;
			}
			else if( (strcmp(Game[0].SANmv,"d4")==0) && (strcmp(Game[1].SANmv,"Nf6")==0) && (strcmp(Game[2].SANmv,"c4")==0) )
			{
				strcpy( moveStr, "e6" );
				foundMove = 1;
			}
			else if( (strcmp(Game[0].SANmv,"e4")==0) && (strcmp(Game[1].SANmv,"c6")==0) && (strcmp(Game[2].SANmv,"d4")==0) )
			{
				strcpy( moveStr, "d5" );
				foundMove = 1;
			}
			break;
	}
	
	return foundMove;
}

// PLAYING DUCHAMP -- ADDED
int blunder()
{
	int randNum = rand();
	float randPercentage = (float)randNum/(float)RAND_MAX;
	float blunderThreshold = 0.0;
	duchampBlundered = 0;
	
	dbg_printf("blunder percentage = %5.2f\n", randPercentage );
	
	if( (GameCnt+2) < 40 )
		blunderThreshold = .02;
	else if( (GameCnt+2) < 56 )
	{
		blunderThreshold = .05;
	}
	else
	{
		blunderThreshold = .10;
	}
	
	if( randPercentage <= blunderThreshold )		
		duchampBlundered = 1;
	
	// 2% of the time, Duchamp will commit a blunder before move 40
	// 5% of the time, Duchamp will commit a blunder after move 40
	// 10% of the time, Duchamp will commit a blunder after move 50
	
	
	return duchampBlundered;
}

/*
 for (i = 0; i <= GameCnt; i++)
 {
 if (! (i % 2)) {
 sprintf (s, "%s%d. ", s, i/2 + 1);
 }
 sprintf (s, "%s%s ", s, Game[i].SANmv);
 if (strlen (s) > 80)
 {
 p = s + 79;
 while (*p-- != ' ');
 *++p = '\0'; 
 fprintf (fp, "%s\n", s);
 strcpy (s, p+1);
 }
 if (Game[i].comments) {
 fprintf (fp, "%s\n", s);
 fprintf (fp, "%s", Game[i].comments);
 if (Game[i].comments[0] &&
 Game[i].comments[strlen(Game[i].comments)-1] != '\n')
 fprintf(fp, "\n");
 s[0] = '\0';
 }
 */


