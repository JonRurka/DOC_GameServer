#include "Linux_conio.h"

#ifdef LINUX_PLATFORM

#include <sys/select.h>
#include <sys/ioctl.h>
#include <termios.h>

static struct termios old, current;

int kbhit() {
    static const int STDIN = 0;
    static bool initialized = false;

    if (!initialized) {
        // Use termios to turn off line buffering
        termios term;
        tcgetattr(STDIN, &term);
        term.c_lflag &= ~ICANON;
        tcsetattr(STDIN, TCSANOW, &term);
        setbuf(stdin, NULL);
        initialized = true;
    }

    int bytesWaiting;
    ioctl(STDIN, FIONREAD, &bytesWaiting);
    return bytesWaiting;
}

/* Initialize new terminal i/o settings */
void initTermios(int echo)
{
    tcgetattr(0, &old); /* grab old terminal i/o settings */
    current = old; /* make new settings same as old settings */
    current.c_lflag &= ~ICANON; /* disable buffered i/o */
    if (echo) {
        current.c_lflag |= ECHO; /* set echo mode */
    }
    else {
        current.c_lflag &= ~ECHO; /* set no echo mode */
    }
    tcsetattr(0, TCSANOW, &current); /* use these new terminal i/o settings now */
}

/* Restore old terminal i/o settings */
void resetTermios(void)
{
    tcsetattr(0, TCSANOW, &old);
}

/* Read 1 character - echo defines echo mode */
char getch_(int echo)
{
    char ch;
    initTermios(echo);
    ch = getchar();
    resetTermios();
    return ch;
}

/* Read 1 character without echo */
char getch(void)
{
    return getch_(0);
}

/* Read 1 character with echo */
char getche(void)
{
    return getch_(1);
}

#endif