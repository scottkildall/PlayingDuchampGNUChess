/* fen.cpp

   GNU Chess protocol adapter

   Copyright (C) 2001-2015 Free Software Foundation, Inc.

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
*/


// fen.cpp

// includes

#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "board.h"
#include "colour.h"
#include "fen.h"
#include "option.h"
#include "piece.h"
#include "square.h"
#include "util.h"

namespace adapter {

// "constants"

// const char * StartFen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w HAha - 0 1";
const char * StartFen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
static const int StringSize = 4096;

// variables

static const bool Strict = false;

// functions

// fen_fatal()

void fen_fatal(const char format[], int pos, const char fen[])
{
   char errorMsg[StringSize]="";
   char pointerStr[StringSize]="";
   for ( int i=0; i<pos; ++i ) {
      strcat( pointerStr, " " );
   }
   strcat( pointerStr, "*" );
   sprintf( errorMsg, format, pos );
   strcat( errorMsg, fen );
   strcat( errorMsg, "\n" );
   strcat( errorMsg, pointerStr );
   strcat( errorMsg, "\n" );
   my_fatal( errorMsg );
}

// board_from_fen()

bool board_from_fen(board_t * board, const char string[]) {

   int pos;
   int file, rank, sq;
   int c;
   int i, len;
   int piece;
   int king_pos[ColourNb];

   ASSERT(board!=NULL);
   ASSERT(string!=NULL);

   board_clear(board);

   king_pos[White] = SquareNone;
   king_pos[Black] = SquareNone;

   pos = 0;
   c = string[pos];

   // piece placement

   for (rank = 7; rank >= 0; rank--) {

      for (file = 0; file < 8;) {

         sq = square_make(file,rank);

         if (c >= '1' && c <= '8') { // empty square(s)

            len = c - '0';
            if (file + len > 8) fen_fatal("board_from_fen(): bad FEN (a file was expected at pos=%d)\n",pos,string);

            for (i = 0; i < len; i++) {
               board->square[sq++] = Empty;
               file++;
            }

         } else { // piece

            piece = piece_from_char(c);
            if (piece == PieceNone256) fen_fatal("board_from_fen(): bad FEN (a piece was expected at pos=%d)\n",pos,string);

            if (piece_is_king(piece)) king_pos[piece_colour(piece)] = sq;

            board->square[sq++] = piece;
            file++;
         }

         c = string[++pos];
      }

      if (rank > 0) {
         if (c != '/') fen_fatal("board_from_fen(): bad FEN ('/' was expected at pos=%d)\n",pos,string);
         c = string[++pos];
     }
   }

   // active colour

   if (c != ' ') fen_fatal("board_from_fen(): bad FEN (a space was expected at pos=%d)\n",pos,string);
   c = string[++pos];

   switch (c) {
   case 'w':
      board->turn = White;
      break;
   case 'b':
      board->turn = Black;
      break;
   default:
      fen_fatal("board_from_fen(): bad FEN (the active colour ('w' or 'b') was expected at pos=%d)\n",pos,string);
      break;
   }

   c = string[++pos];

   // castling

   if (c != ' ') fen_fatal("board_from_fen(): bad FEN (a space was expected at pos=%d)\n",pos,string);
   c = string[++pos];

   board->castle[White][SideH] = SquareNone;
   board->castle[White][SideA] = SquareNone;
   board->castle[Black][SideH] = SquareNone;
   board->castle[Black][SideA] = SquareNone;

   if (c == '-') { // no castling rights

      c = string[++pos];

   } else {

      // TODO: filter out illegal rights

      do {

         if (false) {

         } else if (c == 'K') {

            for (sq = H1; sq > king_pos[White]; sq--) {
               if (board->square[sq] == WhiteRook256) {
                  board->castle[White][SideH] = sq;
                  break;
               }
            }

         } else if (c == 'Q') {

            for (sq = A1; sq < king_pos[White]; sq++) {
               if (board->square[sq] == WhiteRook256) {
                  board->castle[White][SideA] = sq;
                  break;
               }
            }

         } else if (c == 'k') {

            for (sq = H8; sq > king_pos[Black]; sq--) {
               if (board->square[sq] == BlackRook256) {
                  board->castle[Black][SideH] = sq;
                  break;
               }
            }

         } else if (c == 'q') {

            for (sq = A8; sq < king_pos[Black]; sq++) {
               if (board->square[sq] == BlackRook256) {
                  board->castle[Black][SideA] = sq;
                  break;
               }
            }

         } else if (c >= 'A' && c <= 'H') {

            // white castling right

            sq = square_make(file_from_char(tolower(c)),Rank1);

            if (sq > king_pos[White]) { // h side
               board->castle[White][SideH] = sq;
            } else { // a side
               board->castle[White][SideA] = sq;
            }

         } else if (c >= 'a' && c <= 'h') {

            // black castling right

            sq = square_make(file_from_char(tolower(c)),Rank8);

            if (sq > king_pos[Black]) { // h side
               board->castle[Black][SideH] = sq;
            } else { // a side
               board->castle[Black][SideA] = sq;
            }

         } else {

            fen_fatal("board_from_fen(): bad FEN (invalid castling availability at pos=%d)\n",pos,string);
         }

         c = string[++pos];

      } while (c != ' ');
   }

   // en-passant

   if (c != ' ') fen_fatal("board_from_fen(): bad FEN (a space was expected at pos=%d)\n",pos,string);
   c = string[++pos];

   if (c == '-') { // no en-passant

      sq = SquareNone;
      c = string[++pos];

   } else {

      if (c < 'a' || c > 'h') fen_fatal("board_from_fen(): bad FEN (invalid en-passant rank at pos=%d)\n",pos,string);
      file = file_from_char(c);
      c = string[++pos];

      if (c < '1' || c > '8') fen_fatal("board_from_fen(): bad FEN (invalid en-passant file at pos=%d)\n",pos,string);
      rank = rank_from_char(c);
      c = string[++pos];

      sq = square_make(file,rank);
   }

   board->ep_square = sq;

   // halfmove clock

   board->ply_nb = 0;
   board->move_nb = 0; // HACK, in case of broken syntax

   if (c != ' ') {
      if (!Strict) goto update;
      fen_fatal("board_from_fen(): bad FEN (a space was expected at pos=%d)\n",pos,string);
   }
   c = string[++pos];

   if (!isdigit(c)) {
      if (!Strict) goto update;
      fen_fatal("board_from_fen(): bad FEN (a digit for the halfmove clock was expected at pos=%d)\n",pos,string);
   }

   board->ply_nb = atoi(&string[pos]);
   do c = string[++pos]; while (isdigit(c));

   // fullmove number

   board->move_nb = 0;

   if (c != ' ') {
      if (!Strict) goto update;
      fen_fatal("board_from_fen(): bad FEN (a space was expected at pos=%d)\n",pos,string);
   }
   c = string[++pos];

   if (!isdigit(c)) {
      if (!Strict) goto update;
      fen_fatal("board_from_fen(): bad FEN (a digit for the fullmove number was expected at pos=%d)\n",pos,string);
   }

   board->move_nb = atoi(&string[pos]) - 1;
   do c = string[++pos]; while (isdigit(c));

   // board update

update:
   board_init_list(board);

   return true;
}

// board_to_fen()

bool board_to_fen(const board_t * board, char string[], int size) {

   int pos;
   int file, rank;
   int sq, piece;
   int c;
   int len;
   int old_pos;

   ASSERT(board_is_ok(board));
   ASSERT(string!=NULL);
   ASSERT(size>=92);

   // init

   if (size < 92) return false;

   pos = 0;

   // piece placement

   for (rank = 7; rank >= 0; rank--) {

      for (file = 0; file < 8;) {

         sq = square_make(file,rank);
         piece = board->square[sq];
         ASSERT(piece==Empty||piece_is_ok(piece));

         if (piece == Empty) {

            len = 0;
            for (; file < 8 && board->square[square_make(file,rank)] == Empty; file++) {
               len++;
            }

            ASSERT(len>=1&&len<=8);
            c = '0' + len;

         } else {

            c = piece_to_char(piece);
            file++;
         }

         string[pos++] = c;
      }

      string[pos++] = '/';
   }

   string[pos-1] = ' '; // HACK: remove the last '/'

   // active colour

   string[pos++] = (colour_is_white(board->turn)) ? 'w' : 'b';
   string[pos++] = ' ';

   // castling

   old_pos = pos;

   if (option_get_bool("Chess960")) {

      // FEN-960

      if (board->castle[White][SideH] != SquareNone) {
         string[pos++] = toupper(file_to_char(square_file(board->castle[White][SideH])));
      }

      if (board->castle[White][SideA] != SquareNone) {
         string[pos++] = toupper(file_to_char(square_file(board->castle[White][SideA])));
      }

      if (board->castle[Black][SideH] != SquareNone) {
         string[pos++] = tolower(file_to_char(square_file(board->castle[Black][SideH])));
      }

      if (board->castle[Black][SideA] != SquareNone) {
         string[pos++] = tolower(file_to_char(square_file(board->castle[Black][SideA])));
      }

   } else {

      // FEN

      if (board->castle[White][SideH] != SquareNone) string[pos++] = 'K';
      if (board->castle[White][SideA] != SquareNone) string[pos++] = 'Q';
      if (board->castle[Black][SideH] != SquareNone) string[pos++] = 'k';
      if (board->castle[Black][SideA] != SquareNone) string[pos++] = 'q';
   }

   if (pos == old_pos) string[pos++] = '-';

   string[pos++] = ' ';

   // en-passant

   if (board->ep_square == SquareNone) {
      string[pos++] = '-';
   } else {
      if (!square_to_string(board->ep_square,&string[pos],3)) return false;
      pos += 2;
   }

   string[pos++] = ' ';

   // halfmove clock and fullmove number

   sprintf(&string[pos],"%d %d",board->ply_nb,board->move_nb+1);

   return true;
}

}  // namespace adapter

// end of fen.cpp

