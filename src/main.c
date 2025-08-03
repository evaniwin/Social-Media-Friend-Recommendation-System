#include <curses.h>

int main()
{   
    initscr();
    printw("Hello abhishek!\n");
    refresh();
    getch();
    clear();
    endwin();
    return 0;
}
