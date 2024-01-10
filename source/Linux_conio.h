/**
 Linux (POSIX) implementation of _kbhit().
 Morgan McGuire, morgan@cs.brown.edu
 https://www.flipcode.com/archives/_kbhit_for_Linux.shtml
 */

#pragma once
#include "stdafx.h"

#ifdef LINUX_PLATFORM


int kbhit();

/* Initialize new terminal i/o settings */
void initTermios(int echo);

/* Restore old terminal i/o settings */
void resetTermios(void);

/* Read 1 character - echo defines echo mode */
char getch_(int echo);

/* Read 1 character without echo */
char getch(void);

/* Read 1 character with echo */
char getche(void);


#endif