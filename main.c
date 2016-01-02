/* GNU Chess 5.0 - main.c - entry point
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

#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <errno.h>                                                              

#include "common.h"
#include "book.h"

short distance[64][64];
short taxicab[64][64];
unsigned char lzArray[65536];
BitBoard DistMap[64][8];
BitBoard BitPosArray[64];
BitBoard NotBitPosArray[64];
BitBoard MoveArray[8][64];
BitBoard Ray[64][8];
BitBoard FromToRay[64][64];
BitBoard RankBit[8];
BitBoard FileBit[8];
BitBoard Ataks[2][7];
BitBoard PassedPawnMask[2][64];
BitBoard IsolaniMask[8];
BitBoard SquarePawnMask[2][64];
BitBoard Rook00Atak[64][256];
BitBoard Rook90Atak[64][256];
BitBoard Bishop45Atak[64][256];
BitBoard Bishop315Atak[64][256];
BitBoard pinned;
BitBoard rings[4];
BitBoard boxes[2];
BitBoard stonewall[2];
BitBoard pieces[2];
BitBoard mask_kr_trapped_w[3];
BitBoard mask_kr_trapped_b[3];
BitBoard mask_qr_trapped_w[3];
BitBoard mask_qr_trapped_b[3];
BitBoard boardhalf[2];
BitBoard boardside[2];
short directions[64][64];
unsigned char BitCount[65536];
leaf Tree[MAXTREEDEPTH];
leaf *TreePtr[MAXPLYDEPTH];
int RootPV;
GameRec Game[MAXGAMEDEPTH];
int GameCnt;
int RealGameCnt;
short RealSide;
int computer;
unsigned int flags;
unsigned int preanalyze_flags;
int cboard[64];
int Mvboard[64];
Board board;
HashType hashcode[2][7][64];
HashType ephash[64];
HashType WKCastlehash;
HashType WQCastlehash;
HashType BKCastlehash;
HashType BQCastlehash;
HashType Sidehash;
HashType HashKey;
HashType PawnHashKey;
HashSlot *HashTab[2];
PawnSlot *PawnTab[2];
int Idepth;
int SxDec;
int Game50;
int lazyscore[2];
int maxposnscore[2];
int rootscore;
int lastrootscore;
unsigned long GenCnt;
unsigned long NodeCnt;
unsigned long QuiesCnt;
unsigned long EvalCnt;
unsigned long EvalCall;
unsigned long ChkExtCnt;
unsigned long OneRepCnt;
unsigned long RcpExtCnt;
unsigned long PawnExtCnt;
unsigned long HorzExtCnt;
unsigned long ThrtExtCnt;
unsigned long KingExtCnt;
unsigned long NullCutCnt;
unsigned long FutlCutCnt;
unsigned long RazrCutCnt;
unsigned long TotalGetHashCnt;
unsigned long GoodGetHashCnt;
unsigned long TotalPutHashCnt;
unsigned long CollHashCnt;
unsigned long TotalPawnHashCnt;
unsigned long GoodPawnHashCnt;
unsigned long RepeatCnt;
unsigned HashSize;
unsigned long TTHashMask;
unsigned long PHashMask;
char SANmv[SANSZ];
unsigned long history[2][4096];
int killer1[MAXPLYDEPTH];
int killer2[MAXPLYDEPTH];
int ChkCnt[MAXPLYDEPTH];
int ThrtCnt[MAXPLYDEPTH];
char id[32];
char solution[64];
double ElapsedTime;
Timer StartTime;
float SearchTime;
int SearchDepth;
int MoveLimit[2];
float TimeLimit[2];
int TCMove;
int TCinc;
float TCTime;
int castled[2];
int hunged[2];
int phase;
int Hashmv[MAXPLYDEPTH];
short RootPieces;
short RootPawns;
short RootMaterial;
short RootAlpha;
short RootBeta;
short pickphase[MAXPLYDEPTH];
short InChk[MAXPLYDEPTH];
short KingThrt[2][MAXPLYDEPTH];
short threatmv;
uint8_t threatply;
short KingSafety[2];
short pscore[64];
short bookmode;
short bookfirstlast;

char *progname;
FILE *ofp;
int myrating, opprating, suddendeath, TCionc;
char name[50];
int computerplays;		/* Side computer is playing */
int wasbookmove;		/* True if last move was book move */
int nmovesfrombook;		/* Number of moves since last book move */
int newpos, existpos;		/* For book statistics */
float maxtime;		/* Max time for the next searched move */
int n;		/* Last mobility returned by CTL */
int ExchCnt[2];	/* How many exchanges? */
int bookloaded = 0;  	/* Is the book loaded already into memory? */

// PLAYING DUCHAMP
int duchampBlundered;

int slider[8] = { 0, 0, 0, 1, 1, 1, 0, 0 };
int Value[7] = { 0, ValueP, ValueN, ValueB, ValueR, ValueQ, ValueK};
int range[8] = { 0, 0, 0, 1, 1, 1, 0, 0 };
int ptype[2] = { pawn, bpawn };
char algbr[64][3] =
{ "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
  "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
  "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
  "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
  "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
  "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
  "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
  "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8" 
};
char algbrfile[9] = "abcdefgh";
char algbrrank[9] = "12345678";
  
char notation[8] = { " PNBRQK" };
char lnotation[8] = { " pnbrqk" };

short Shift00[64] =
{ 56, 56, 56, 56, 56, 56, 56, 56,
  48, 48, 48, 48, 48, 48, 48, 48,
  40, 40, 40, 40, 40, 40, 40, 40,
  32, 32, 32, 32, 32, 32, 32, 32,
  24, 24, 24, 24, 24, 24, 24, 24,
  16, 16, 16, 16, 16, 16, 16, 16,
   8,  8,  8,  8,  8,  8,  8,  8,
   0,  0,  0,  0,  0,  0,  0,  0
};

int r90[64] =
{ A8, A7, A6, A5, A4, A3, A2, A1,
  B8, B7, B6, B5, B4, B3, B2, B1,
  C8, C7, C6, C5, C4, C3, C2, C1,
  D8, D7, D6, D5, D4, D3, D2, D1,
  E8, E7, E6, E5, E4, E3, E2, E1,
  F8, F7, F6, F5, F4, F3, F2, F1,
  G8, G7, G6, G5, G4, G3, G2, G1,
  H8, H7, H6, H5, H4, H3, H2, H1 };

short Shift90[64] =
{ 0, 8, 16, 24, 32, 40, 48, 56,
  0, 8, 16, 24, 32, 40, 48, 56,
  0, 8, 16, 24, 32, 40, 48, 56,
  0, 8, 16, 24, 32, 40, 48, 56,
  0, 8, 16, 24, 32, 40, 48, 56,
  0, 8, 16, 24, 32, 40, 48, 56,
  0, 8, 16, 24, 32, 40, 48, 56,
  0, 8, 16, 24, 32, 40, 48, 56
};

int r45[64] =
{ E4, F3, H2, C2, G1, D1, B1, A1,
  E5, F4, G3, A3, D2, H1, E1, C1,
  D6, F5, G4, H3, B3, E2, A2, F1, 
  B7, E6, G5, H4, A4, C3, F2, B2,
  G7, C7, F6, H5, A5, B4, D3, G2, 
  C8, H7, D7, G6, A6, B5, C4, E3, 
  F8, D8, A8, E7, H6, B6, C5, D4, 
  H8, G8, E8, B8, F7, A7, C6, D5 };

short Shift45[64] =
{ 28, 36, 43, 49, 54, 58, 61, 63,
  21, 28, 36, 43, 49, 54, 58, 61,
  15, 21, 28, 36, 43, 49, 54, 58,
  10, 15, 21, 28, 36, 43, 49, 54,
   6, 10, 15, 21, 28, 36, 43, 49,
   3,  6, 10, 15, 21, 28, 36, 43,
   1,  3,  6, 10, 15, 21, 28, 36,
   0,  1,  3,  6, 10, 15, 21, 28 };

int Mask45[64] =
{ 0xFF, 0x7F, 0x3F, 0x1F, 0x0F, 0x07, 0x03, 0x01,
  0x7F, 0xFF, 0x7F, 0x3F, 0x1F, 0x0F, 0x07, 0x03, 
  0x3F, 0x7F, 0xFF, 0x7F, 0x3F, 0x1F, 0x0F, 0x07, 
  0x1F, 0x3F, 0x7F, 0xFF, 0x7F, 0x3F, 0x1F, 0x0F, 
  0x0F, 0x1F, 0x3F, 0x7F, 0xFF, 0x7F, 0x3F, 0x1F, 
  0x07, 0x0F, 0x1F, 0x3F, 0x7F, 0xFF, 0x7F, 0x3F, 
  0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F, 0xFF, 0x7F, 
  0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F, 0xFF };

int r315[64] =
{ A1, C1, F1, B2, G2, E3, D4, D5,
  B1, E1, A2, F2, D3, C4, C5, C6,
  D1, H1, E2, C3, B4, B5, B6, A7,
  G1, D2, B3, A4, A5, A6, H6, F7,
  C2, A3, H3, H4, H5, G6, E7, B8,
  H2, G3, G4, G5, F6, D7, A8, E8,
  F3, F4, F5, E6, C7, H7, D8, G8,
  E4, E5, D6, B7, G7, C8, F8, H8 };

short Shift315[64] =
{ 63, 61, 58, 54, 49, 43, 36, 28,
  61, 58, 54, 49, 43, 36, 28, 21,
  58, 54, 49, 43, 36, 28, 21, 15,
  54, 49, 43, 36, 28, 21, 15, 10,
  49, 43, 36, 28, 21, 15, 10,  6,
  43, 36, 28, 21, 15, 10,  6,  3,
  36, 28, 21, 15, 10,  6,  3,  1,
  28, 21, 15, 10,  6,  3,  1,  0 };

int Mask315[64] =
{ 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F, 0xFF,
  0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F, 0xFF, 0x7F,
  0x07, 0x0F, 0x1F, 0x3F, 0x7F, 0xFF, 0x7F, 0x3F,
  0x0F, 0x1F, 0x3F, 0x7F, 0xFF, 0x7F, 0x3F, 0x1F,
  0x1F, 0x3F, 0x7F, 0xFF, 0x7F, 0x3F, 0x1F, 0x0F,
  0x3F, 0x7F, 0xFF, 0x7F, 0x3F, 0x1F, 0x0F, 0x07,
  0x7F, 0xFF, 0x7F, 0x3F, 0x1F, 0x0F, 0x07, 0x03,
  0xFF, 0x7F, 0x3F, 0x1F, 0x0F, 0x07, 0x03, 0x01 };

int rank6[2] = { 5, 2 };
int rank7[2] = { 6, 1 };
int rank8[2] = { 7, 0 };

//Added by leslie for web
#define MAXLEN 80 //max length of input received from flash
#define EXTRA 5 //Extra chars used in transmission
#define MAXINPUT MAXLEN+EXTRA+2 //maximum length of string to receive
#define MAXGAMEFILE 50 //maximum size of game file name - based on max username being 30 chars

//Un-encodes any web specific encoded imposed on the string received from flash
void unencode(char *src, char *last, char *dest)
{
  for(; src != last; src++, dest++ )
    if(*src == '+')
      *dest = ' ';
    else if( *src == '%') {
      int code;
      if(sscanf(src+1, "%2x", &code) != 1) code = '?';
      *dest = code;
       src +=2;
    }
   else 
      *dest = *src;
    *dest = '\n';
    *++dest = '\0';
}

//General purpose routine for reporting error and exiting progream
void return_error( char* errstr ) {
  dbg_printf("Content-Type: text/plain\n\n");
  dbg_printf( errstr );
  dbg_printf("\n");
  exit(1);
}

static char data[MAXINPUT]; //Used to process data and to store username, move and color

//Gets the command sent from Flash
void get_cmd( char *lenstr, char **username, char **move, char **color, int *new, int *startUp, int *undo) {
  char input[MAXINPUT]; //Needed for getting an manipulating input data
  char *nvpair; //used to get values from input data
  char *value;
  char errstr[255]; //Space for generating error string
  long len; //Length of the input data received.
  char *request_method; //The request method used by flash to send the data - we only accept POST
  char *eqpos; //used to get values from input data
  char *crpos;

  //The next bit is checking for possible errors 
  request_method=getenv("REQUEST_METHOD");
  if( strcmp(request_method, "POST") )
    return_error("Unsupported REQUEST_METHOD.\n");

  if( strcasecmp(getenv("CONTENT_TYPE"), "application/x-www-form-urlencoded"))
    return_error("Unsupported Content-Type\n");

  if(sscanf(lenstr,"%ld",&len) != 1 || len > MAXLEN) 
    return_error("Error in invocation, bad content-length - wrong FORM probably");

  //Read the input string
  if( !fread(input, len, 1, stdin))
    return_error("Can't read CGI input from STDIN.\n");

  //Make sure the string is ended properly.
  input[len]='\0';
  dbg_printf("input %s\n", input);

  //get rid of any encoding in the string and store everything in data, which is global.
  unencode(input, input+len, data);
  dbg_printf("data %s\n", data);

  //Get the username and humanMove values from the input.  Anything else sent is ignored.
  *username=NULL;
  *move=NULL;
  *color="white";
  nvpair = strtok( data, "&");
  while( nvpair )  {
    dbg_printf("%s\n", nvpair);
    eqpos = strchr(nvpair, '=');
    *eqpos = '\0';
    value = eqpos + 1;
    crpos = strchr(value, '\n');
    if( crpos != NULL ) *crpos = '\0';
    if( !strcmp(nvpair, "username" ) ) *username = value;
    if( !strcmp(nvpair, "humanMove") ) *move = value;
    if( !strcmp(nvpair, "humanColor") ) *color = value;
    if( !strcmp(nvpair, "startUp") ) {
      if( !strcmp(value, "1" ) ) *startUp =  true;
    }
    if( !strcmp(nvpair, "newGame") ) {
      if( !strcmp(value, "1" ) ) *new = true;
    }
    if( !strcmp(nvpair, "undoMove") ) {
      if( !strcmp(value, "1" ) ) *undo = true;
    }
    nvpair = strtok(NULL, "&" );
  }
 
  if( *username == NULL ) {
    dbg_printf("no username received\n");
    exit(1);
  }
}

//Open the file and read it's contents.  Used by startUp and Undo
int get_file( char *gamefile, FILE **gfh, char **buffer, long *lSize ) {
  size_t result;

  *gfh = fopen(gamefile, "r" );
  if( *gfh==NULL) {
    dbg_printf("can't open %s\n", gamefile);
    return false;
  }

  fseek( *gfh, 0, SEEK_END);
  *lSize = ftell( *gfh );
  rewind( *gfh );

  long bufsize = sizeof(char)*(*lSize);
  *buffer = (char*) malloc(bufsize);
  if( *buffer == NULL ) {
    dbg_printf("malloc error\n" );
    return false;
  }

  result = fread(*buffer,1,*lSize, *gfh);
  if( result != *lSize) {
    dbg_printf("Reading error\n");
    return false;
  }
  return true;
}

//Undo a move.  This function expects the 
//gamefile is open already and expects to be passed it's info
int undo_move( char *gamefile, FILE *gfh, char *buffer, long lSize) {
  char *src;
  int dotpos, spacepos, bracepos, lfpos, i;
      
  dbg_printf("undo\n");  
  fclose( gfh ); //close file for reading and reopen for writing
  gfh = fopen(gamefile, "w" );
  if( gfh==NULL) {
    dbg_printf("can't open %s\n", gamefile);
    return false; 
  }
  
  //Go to the end of the file contents and
  //strip all linesfeeds and spaces from the end.    
  src = buffer+lSize-1;
  i = lSize-1;
  for(; src != buffer; src-- ) {
    if( (*src != 0 ) && ( *src != 10 ) && ( *src != ' ' ) ) break;
    i--;
  }
  i -= 2;
      
  //Starting from the end (minus linefeeds and spaces)
  //Get the positions of the last dot(.), brace(]) and space ( )
  //These tell use where the last move is.
  src = buffer+i;
  dotpos = spacepos = bracepos = lfpos = -1; 
  for(; src != buffer; src-- ) {
    if(( *src == 10) && (lfpos == -1) && (src < ((buffer+lSize)-3)) ) lfpos = i;

    if(( *src == '.' ) && ( dotpos == -1 )) dotpos = i;
    if(( *src == ' ' ) && ( spacepos == -1 )) spacepos = i;
    if(( *src == ']' ) && ( bracepos == -1 )) bracepos = i;
    if(( dotpos != -1 ) && (spacepos != -1) && 
       ( bracepos != -1 ) && ( lfpos != -1 ) ) break;
    i--;
  }
      
  //If there is no dot(.) after the initial braces, then there is no move, the file is empty
  if( ( dotpos == -1 ) || ( bracepos > dotpos ) ) {
    dbg_printf("can't undo, empty file\n");
    return true;
  }

  //The file wraps, so move could be on the edge of a linefeed not a space, 
  //This looks for this situation and if true uses the linefeed as the last space.
  if( lfpos > spacepos ) spacepos = lfpos;
  buffer[spacepos+1] = '\0';

  //Find the linefeed and space positions just before the last move.
  //This tells us if the move that is removed is the first or second move.
  //If it's the secomnd move, then also remove the sequence number.
  src = buffer+(spacepos-2);
  i = spacepos - 2; 
  spacepos = lfpos = -1; 
  for( ; src != buffer; src-- ) {
    if(( *src == 0x0A ) && ( lfpos == -1 )) lfpos = i;
    if(( *src == ' ' ) && ( spacepos == -1 )) spacepos = i;
    if((spacepos != -1) && (lfpos != -1) ) break;
    i--;
  }
  if( lfpos > spacepos ) spacepos = lfpos;
  if( spacepos < dotpos ) {
    buffer[spacepos] = '\0';
  }
  fprintf(gfh,"%s\n", buffer );

  return true;
}

int main (int argc, char *argv[])
{
  int i;
  char *username;
  char *move;
  char *color;
  int new = false;
  int startUp = false;
  int undo = false;
  char gamefile[MAXGAMEFILE]; //Holds the name of the .pgn file
 
  //Used for dbg_print statements.  Ignored if DEBUG is not defined 
  dbg_open(NULL);

  dbg_printf("in gnuchess\n");

//Check to see if request has come through the web or the command line.
  char *lenstr = getenv("CONTENT_LENGTH");

  if(lenstr == NULL) {
//request has come through command line
//This is just here for testing, it should probably be replaced by some kind of error once development is done.
    printf("not cgi, print options\n");
    username="kildall3";
    //move="e5";
    //color="black";
    startUp = true;
    //new = true;
    //undo = true;
  }
  else {
//request has come through the web
    get_cmd( lenstr, &username, &move, &color, &new, &startUp, &undo );
  }

  dbg_printf("username is %s\n", username);

//Create the game file name from the username
  sprintf( gamefile, "games/%s.pgn", username );

//required for the proper header in the pgn file.
  strcpy( name, username );

  if( startUp || undo )  {
    FILE *gfh;
    char *buffer;
    long lSize;

    dbg_printf("startup or undo\n");
    if( !get_file( gamefile, &gfh, &buffer, &lSize ) ) {
      if( startUp ) {printf("\nsavedGame=no"); }
      exit(1);
    }
    
    if( startUp ) {
      printf("\nsavedGame=yes&pgn="); 
      encode( buffer, buffer+lSize );
    }
    else {
      undo_move( gamefile, gfh, buffer, lSize); 
    }

    fclose( gfh );
    free(buffer);

    return 0;
  }

  /*
   * Parse command line arguments conforming with getopt_long syntax
   * Note: we have to support "xboard" and "post" as bare strings
   * for backward compatibility.
   */
 
  int c;

  //not sure if this is need, but was left over from original gnuchess
  progname = argv[0]; /* Save in global for cmd_usage */

  /* Initialize random number generator */
  srand((unsigned int) time(NULL));
  
  /* initialize control flags, these are normally set through the command line, but we can set the in the code if needed */
  flags = ULL(0);

  /* output for thinking, We don't really use this, but other routines check it, so I'm leaving it in. */
  ofp = stdout;


  HashSize = 0 ; /* Set HashSize zero */
  //Commented out because I think the default state is opt_hash = 0, so we wouldn't use this.
  //if ( opt_hash != 0)
  //  CalcHashSize(opt_hash);

  Initialize ();

//Default state.  make sure that program generates computerMove and doesn't post out information messages
  SET( flags, HARD );
  SET( flags, POST );

//I'm not sure how this is used or if we need it, so it's left in.
  bookmode = BOOKPREFER;
  bookfirstlast = 3;

  if( new ) dbg_printf("new game\n");

//This loads the <usename>.pgn file if it exists.  If it doesn't, nothting is done and code proceeds as a new game.
  if( !new ) PGNReadFromFile( gamefile );

//Set the color -I'm using the code from cmd_go
   if( new && !strcmp( color, "black" ) ) {
    dbg_printf("new and color is black\n");
    SET( flags, THINK );
    CLEAR( flags, MANUAL );
    CLEAR( flags, TIMEOUT );
    CLEAR( flags, ENDED );
    computer = board.side;
    Iterate();

    PGNSaveToFile(gamefile, "");
  }
  else {
    if( move != NULL ) {
      if( !strcmp( color, "black") ) {
        computer=white;
        if ( board.side == white ) {
          board.side = black;
          board.ep = -1;
         }
       }
//This code is pulled straight out of the parse_input method in cmd.c
      leaf *ptr;

      ptr = ValidateMove (move);

      if( ptr != NULL ) {
        SANMove( ptr->move, 1);
        MakeMove(board.side, &ptr->move);
        strcpy (Game[GameCnt].SANmv, SANmv);
        fflush(stdout);
        SET( flags, THINK);
        Iterate ();
        CLEAR(flags, THINK);

        
//Added this to save the game to the <username>.pgn file.    
        PGNSaveToFile(gamefile, "");
      } else {
        printf("\ncomputerMove=Illegal");
        fflush(stdout);
      }
    }
  }

  /*  Some cleaning up  */
  free (HashTab[0]);
  free (HashTab[1]);

  dbg_close();
  return 0;
}
