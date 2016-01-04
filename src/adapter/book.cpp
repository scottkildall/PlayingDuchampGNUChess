/* book.cpp

   GNU Chess protocol adapter

   Copyright (C) 2001-2014 Free Software Foundation, Inc.

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


// book.cpp

// includes

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "board.h"
#include "book.h"
#include "move.h"
#include "move_legal.h"
#include "san.h"
#include "util.h"

namespace adapter {

// types

struct entry_t {
   uint64 key;
   uint16 move;
   uint16 count;
   uint16 n;
   uint16 sum;
};

// variables

static FILE * BookFile;
static int BookSize;

// prototypes

static int    find_pos      (uint64 key);

static void   read_entry    (entry_t * entry, int n);
static void   write_entry   (const entry_t * entry, int n);

static uint64 read_integer  (FILE * file, int size);
static void   write_integer (FILE * file, int size, uint64 n);

// functions

// book_clear()

void book_clear() {

   BookFile = NULL;
   BookSize = 0;
}

// book_open()

void book_open(const char file_name[], int mode) {

   ASSERT(file_name!=NULL);
   ASSERT(mode==BookReadOnly || mode==BookReadWrite);

   const int MaxModeLength = 4;
   char full_file_name[MaxFileNameSize+1];
   char file_open_mode[MaxModeLength]="";
   FILE *bf;
   if ( ( bf = fopen(file_name, "r") ) != NULL ) {
      fclose(bf);
      strcpy(full_file_name,"");
   } else {
      strcpy(full_file_name,compute_pkgdatadir());
      strcat(full_file_name,"/");
   }
   strcat(full_file_name,file_name);

   if (mode == BookReadWrite) {
      strcpy(file_open_mode,"rb+");
   } else {
      strcpy(file_open_mode,"rb");
   }
   BookFile = fopen(full_file_name,file_open_mode);
   if (BookFile == NULL) {
      if (fopen(full_file_name,"rb") != NULL) {
         fclose(bf);
         my_fatal("book_open(): file \"%s\" is read only\n",full_file_name);
      } else {
         my_fatal("book_open(): can't open file \"%s\": %s\n",full_file_name,strerror(errno));
      }
   }

   if (fseek(BookFile,0,SEEK_END) == -1) {
      my_fatal("book_open(): fseek(): %s\n",strerror(errno));
   }

   BookSize = ftell(BookFile) / 16;
   if (BookSize == 0) my_fatal("book_open(): empty file\n");
}

// book_close()

void book_close() {

   if (fclose(BookFile) == EOF) {
      my_fatal("book_close(): fclose(): %s\n",strerror(errno));
   }
}

// is_in_book()

bool is_in_book(const board_t * board) {

   int pos;
   entry_t entry[1];

   ASSERT(board!=NULL);

   for (pos = find_pos(board->key); pos < BookSize; pos++) {
      read_entry(entry,pos);
      if (entry->key == board->key) return true;
   }

   return false;
}

// book_move()

int book_move(const board_t * board, bool random) {

   int best_move;
   int best_score;
   int pos;
   entry_t entry[1];
   int move;
   int score;

   ASSERT(board!=NULL);
   ASSERT(random==true||random==false);

   best_move = MoveNone;
   best_score = 0;

   for (pos = find_pos(board->key); pos < BookSize; pos++) {

      read_entry(entry,pos);
      if (entry->key != board->key) break;

      move = entry->move;
      score = entry->count;

      if (move != MoveNone && move_is_legal(move,board)) {

         // pick this move?

         ASSERT(score>0);

         if (random) {
            best_score += score;
            if (my_random_int(best_score) < score) best_move = move;
         } else {
            if (score > best_score) {
               best_move = move;
               best_score = score;
            }
         }

      } else {

         ASSERT(false);
      }
   }

   return best_move;
}

// book_move()

int book_move(const board_t * board, bool random, bool worst) {

   int worst_move;
   int worst_score;
   int best_move;
   int best_score;
   int pos;
   entry_t entry[1];
   int move;
   int score;

   ASSERT(board!=NULL);
   ASSERT(random==true||random==false);

   worst_move = MoveNone;
   worst_score = 10000;

   best_move = MoveNone;
   best_score = 0;

   for (pos = find_pos(board->key); pos < BookSize; pos++) {

      read_entry(entry,pos);
      if (entry->key != board->key) break;

      move = entry->move;
      score = entry->count;

      if (move != MoveNone && move_is_legal(move,board)) {

         // pick this move?

         ASSERT(score>0);

         if (worst) {
            if (score < worst_score) {
               worst_move = move;
               worst_score = score;
            }
         } else if (random) {
            best_score += score;
            if (my_random_int(best_score) < score) best_move = move;
         } else {
            if (score > best_score) {
               best_move = move;
               best_score = score;
            }
         }

      } else {

         ASSERT(false);
      }
   }

   if (worst) {
      best_move = worst_move;
   }

   return best_move;
}

// book_disp()

void book_disp(const board_t * board) {

   int first_pos;
   int sum;
   int pos;
   entry_t entry[1];
   int move;
   int score;
   char move_string[256];

   ASSERT(board!=NULL);

   first_pos = find_pos(board->key);

   // sum

   sum = 0;

   for (pos = first_pos; pos < BookSize; pos++) {

      read_entry(entry,pos);
      if (entry->key != board->key) break;

      sum += entry->count;
   }

   // disp

   for (pos = first_pos; pos < BookSize; pos++) {

      read_entry(entry,pos);
      if (entry->key != board->key) break;

      move = entry->move;
      score = entry->count;

      if (score > 0 && move != MoveNone && move_is_legal(move,board)) {
         move_to_san(move,board,move_string,256);
         printf(" %s (%.0f%%)\n",move_string,(double(score)/double(sum))*100.0);
      }
   }

   printf("\n");
}

// book_learn_move()

void book_learn_move(const board_t * board, int move, int result) {

   int pos;
   entry_t entry[1];

   ASSERT(board!=NULL);
   ASSERT(move_is_ok(move));
   ASSERT(result>=-1&&result<=+1);

   ASSERT(move_is_legal(move,board));

   for (pos = find_pos(board->key); pos < BookSize; pos++) {

      read_entry(entry,pos);
      if (entry->key != board->key) break;

      if (entry->move == move) {

         entry->n++;
         entry->sum += result+1;

         write_entry(entry,pos);

         break;
      }
   }
}

// book_flush()

void book_flush() {

   if (fflush(BookFile) == EOF) {
      my_fatal("book_flush(): fflush(): %s\n",strerror(errno));
   }
}

// find_pos()

static int find_pos(uint64 key) {

   int left, right, mid;
   entry_t entry[1];

   // binary search (finds the leftmost entry)

   left = 0;
   right = BookSize-1;

   ASSERT(left<=right);

   while (left < right) {

      mid = (left + right) / 2;
      ASSERT(mid>=left&&mid<right);

      read_entry(entry,mid);

      if (key <= entry->key) {
         right = mid;
      } else {
         left = mid+1;
      }
   }

   ASSERT(left==right);

   read_entry(entry,left);

   return (entry->key == key) ? left : BookSize;
}

// read_entry()

static void read_entry(entry_t * entry, int n) {

   ASSERT(entry!=NULL);
   ASSERT(n>=0&&n<BookSize);

   if (fseek(BookFile,n*16,SEEK_SET) == -1) {
      my_fatal("read_entry(): fseek(): %s\n",strerror(errno));
   }

   entry->key   = read_integer(BookFile,8);
   entry->move  = read_integer(BookFile,2);
   entry->count = read_integer(BookFile,2);
   entry->n     = read_integer(BookFile,2);
   entry->sum   = read_integer(BookFile,2);
}

// write_entry()

static void write_entry(const entry_t * entry, int n) {

   ASSERT(entry!=NULL);
   ASSERT(n>=0&&n<BookSize);

   if (fseek(BookFile,n*16,SEEK_SET) == -1) {
      my_fatal("write_entry(): fseek(): %s\n",strerror(errno));
   }

   write_integer(BookFile,8,entry->key);
   write_integer(BookFile,2,entry->move);
   write_integer(BookFile,2,entry->count);
   write_integer(BookFile,2,entry->n);
   write_integer(BookFile,2,entry->sum);
}

// read_integer()

static uint64 read_integer(FILE * file, int size) {

   uint64 n;
   int i;
   int b;

   ASSERT(file!=NULL);
   ASSERT(size>0&&size<=8);

   n = 0;

   for (i = 0; i < size; i++) {

      b = fgetc(file);

      if (b == EOF) {
         if (feof(file)) {
            my_fatal("read_integer(): fgetc(): EOF reached\n");
         } else { // error
            my_fatal("read_integer(): fgetc(): %s\n",strerror(errno));
         }
      }

      ASSERT(b>=0&&b<256);
      n = (n << 8) | b;
   }

   return n;
}

// write_integer()

static void write_integer(FILE * file, int size, uint64 n) {

   int i;
   int b;

   ASSERT(file!=NULL);
   ASSERT(size>0&&size<=8);
   ASSERT(size==8||n>>(size*8)==0);

   for (i = size-1; i >= 0; i--) {

      b = (n >> (i*8)) & 0xFF;
      ASSERT(b>=0&&b<256);

      if (fputc(b,file) == EOF) {
         my_fatal("write_integer(): fputc(): %s\n",strerror(errno));
      }
   }
}

}  // namespace adapter

// end of book.cpp

