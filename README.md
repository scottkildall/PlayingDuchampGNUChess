Playing Duchamp
================

WHERE THE gnuchess.cgi EXECUTABLE IS

The web server requires that all cgi go into the /web/turbulence/cgi-bin/duchamp directory, so that's where the gnuchess.cgi file is.  However to make things easy, I've created a link (or a shortcut) to this directory from the playing_duchamp directory.  So to access the gnuchess.cgi file go to either:

  playing_duchamp/cgi-bin (which is a link or shortcut to the real directory) or
  /web/turbulence/cgi-bin/duchamp


WHERE THE GAME FILES ARE

The games files (the pgn files) have to be reachable by the gnuchess.cgi executable, so they are in the games subdirectory of the cgi-bin directory, so either;

  playing_duchamp/cgi-bin/games (shortcut) or
  /web/turbulence/cgi-bin/duchamp/games


HOW TO REBUILD THE EXECUTABLE

To rebuild the executable, you will need to be logged into the system with a ssh terminal.
Once you are in the ssh terminal window, go into the src directory: 

 playing_duchamp/gnuchess/gnuchess-5.07/src

If you want to rebuild all of the code (required if you haven't actually changed a file)
type:

  make clean
  
To build any new changes you've made to a .c or a .h file, or after you've cleaned it all out with make clean, type:

  make

This builds an executable called gnuchess in the same directory.  

To copy the gnuchess exectuable to the proper cgi file type:

  cp gnuchess /web/turbulence/cgi-bin/duchamp/gnuchess.cgi


WHERE THE DEBUG LOG IS, IF IT EXISTS AND HOW TO MAINTAIN IT IF IT DOES

The debug log is called gnuchess.debug.  It may or may not exist, depending on how the source code is built (see HOW TO TURN DEBUG LOG OFF AND ON).   I'm going to turn off the debug log when we are done testing.  If it is turned back on, beware that it will continue to grow and should be backed up (if you care) and deleted from time to time.

Because it has to be accessible to the gnuchess.cgi executable, it's in the same directory as the games:
  
  playing_duchamp/cgi-bin/games (shortcut) or
  /web/turbulence/cgi-bin/duchamp/games


WHERE THE SOURCE CODE IS

The source code is in:

playing_duchamp/gnuchess/gnuchess-5.07/src

Look for the *.c and *.h files.  The main file is main.c


WHAT I'VE CHANGED

The original gnuchess game is run from the command line and interacts with the user allowing a full game to be played in one session.  However, this won't work with the webserver/cgi structure.  

With the webserver architecture, a cgi executable receives information from a HTTP_REQUEST, which in this case is generated by the sendAndLoad call in Flash (or whatever you are using to send variables), runs once, returns some information ant then quits.    This means that it can't keep a game active.

Because it can't keep the game active, we are storing the game in a .pgn file.  When the executable is called, it is called with a username.  This username indicates which .pgn file to use.  If there is an existing game, the gnuchess.cgi executable loads in that game, accepts a move and then outputs a move and stores the changed game in the .pgn file.

To allow all of this to happen I changed gnuchess in this way:

  - I took out all of the interactive command line code
  - Changed all of the files to ensure that the normal command line output went to the debug log. 
  - Added in code to get move, username, color and various commands. from the HTTP POST request used by flash.
  - Load a <username>.pgn file for the game when the executable first starts and save a <username>.pgn file when it finishes.
  - Output the generated move (computerMove) in a way that it can be read by Flash.
  - Added in code to load a saved game on request (on startup of the Flash code).
  - Added in code to start a new game.
  - Added in code to undo moves from the .pgn file.


VALID INFORMATION TO POST TO THE CGI EXECUTABLE

username - required for every call.  It identifies which .pgn file to use.

humanMove - Used to send a move to gnuchess and receive a new computerMove value back.

humanColor - Defines which color the human (i.e. Flash) is using, white or black ( default is white).

newGame - start a new game.  The existing .pgn file is erased and the game starts from the beginning.

undoMove - undo a move.  It is expected that Flash will send this twice, once to undo the humanMove and once to remove the computerMove

startUp - send the complete .pgn file to Flash.  Used when the flash code first starts (when the user logs in) to load in an existing game.


The actions that can be performed are:

Start a new game as the white player example

username="kildall"
humanMove="e3"
newGame="1"

returns computerMove


Start a new game as the black player

username="kildall"
humanColor="black"
newGame="1"

returns computerMove

Send a new move as either color player example
The color is kept track of in the .pgn file.

username="kildall"
humanMove="f4"
newGame="0"

returns computerMove

Get the full game from the .pgn file

username="kildall"
startUp="1"

returns if game exists:
 savedGame="yes"
 pgn=a urlencoded string containing the whole game

returns if game doesn't exist:
 savedGame="no"

Undo a move 
(note, it's expected that undo requests will come in pairs, one for black and one for white ).

username="kildall"
undo="1"

returns nothing
 

VALUES SENT BACK TO THE FLASH CODE

computerMove: 
 - returned when either newGame is started and/or a humanMove is sent
 - either a valid move or the string "illegal"

savedGame:
 - returned when startUp="1" is sent
 - is "yes" if saved game for a user exists, "no" if it doesn't

pgn
 - return when startUp="1" and a .pgn file for the user exists.
 - a urlencoded string of the full .pgn file.  


FILES OF INTEREST

main.c - This is where most of the code is
pgn.c - This is where the code is for loading and saving .pgn files
iterate.c - Where most of the move processing is done
cmd.c - not used, as it provides the command line interface, but useful for lifting out code for different commands.


HOW TO TURN DEBUG LOG ON AND OFF

Edit the debug.c file in the src directory.  At the top of the file there should be the line:

#define DEBUG 1

If it is commented out i.e. it looks like:

//#define DEBUG 1

Then the debug log is off.  If you want to turn debug back on, delete the comments, ie the // part.

If you want to turn debug off, then comment out this line, ie put the // part back.

When you've changed the code, then remake the executable and copy it to the cgi-bin directory (see HOW TO BUILD THE EXECUTABLE)


WHERE THE OLD CODE IS

There is a copy of the original command line based gnuchess here:

  playing_duchamp/gnuchess/bin

The old source code is here:

  playing_duchamp/gnuchess/gnuchess-5.07/src/orig


WAYS TO TEST THE EXECUTABLE

In order to test the executable, I created a bunch of html file to send commands to the executable.  The files are:

testnew.html - start a new game as the white player. 

testnewblack.html - start a new game as the black player.

test.html - send a move to an existing game.  If no game exists, new one is started as white player.

testundo.html - send an undo request for a specific user

teststart.html - send a startUp request for a specific user


FILE FOR SENDING EMAIL WITH EXISTING GAME WHEN NEW ONE STARTED

To send an email with the existing game (to save when new one is started) call:
  playing_duchamp/sendgame.php

It expects a username, i.e. username="kildall"


HOW TO CHANGE EMAIL ADDRESS USED TO SEND EXISTING GAME

Open the playing_duchamp/sendgame.php file and change the $to string on the second line.

LOG FILE FOR EMAILS SENT WITH EXISTING GAME

Each email sent is logged in /web/turbulence/cgi-bin/duchamp/games/sendemail.log

You'll probably have to delete it every once in a while to stop it getting too big.


PHP FILE STRUCTURE FOR LOGIN AND REGISTRATION

index.php 
  - this is the main page for everything.  It redirects to login.php, game.php or admin.php depending if and how the user is logged in.

login.php
 - Allows the user to login, either as a normal user or admin.

game.php
 - Shown once a user has logged in as a regular user, or has just registered and activated their account.  It includes the flash file of the game and is also where the loadVars of the username is done.

register.php
 - Page for registering a new user.

lostpassword.php
 - Will send a new password to the user if they've lost theirs.

changepassword.php
 - Allows the user to change their password.

More_Project_Info.php
 - Generates the more project information page.

logout.php
 - logs the user out.


activate.php
 - called from the registration email.  It's used to activate the account.


top.inc.php
 - part of the internal structure.  It includes all of the function code required for validation and talking to the database.

header.php
 - part of the internal html structure.  It creates the proper header on all of the pages.

footer.php
 - part of the internal html structure.  It closes all of the common html.

db_connect.in.php
 - called in top.inc.php.  This code is used to connect to the database.

user.functions.inc.php
 - all user related functions.  This code communicates with the database.

mail.functions.inc.php
 - all mail related functions.

validation.functions.inc.php
 - all validation functions.


HOW TO LOG IN AS ADMIN USER

On main login page, login as admin with the admin password (I'll send this in a an email).


ADMIN FUNCTIONS

The admin page lists all of the information for each user.  This includes:

- A button to delete the user.
- username
- A button with the .pgn game date and time if it exists. Use this button to download the file.
- A button for uploading a .pgn file for the user.
- The user's email.
- Their activation status 1 means activated, 0 means not activated.  This allows the admin user to activate the user manually if the registration email fails.
- Their disabled status 1 means disabled, 0 means enabled. This allows the admin user to temporarily disable a user without permanently deleting them.

Click on any of the text for that user to go to the information page for that user.  From the information page you can:

- change the users email
- activate or de-activate
- disable or enable.
- change the password.


ADMIN FUNCTIONS PHP FILES

admin.php
 - This is the main page for listing all of the users with links or buttons for all possible functions.

showplayer.php
 - This page allows admin user to activate or disable an account, or go to the change password page.  It shows all of the player information.

adminChangePassword.php
 - Page that allows admin user to change a users password.

adminUpload.php
 - code for uploading a .pgn file for a specific user.

admin.functions.inc.php
 - internal code that provides all the admin functions that communicate with the database.

download.php
 - code for downloading a specifc .pgn file.
