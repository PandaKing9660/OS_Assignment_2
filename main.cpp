#include <ncurses.h>

#define BACKGROUND_PAIR 1
#define BIG_BOX_PAIR 2
#define SMALL_BOX_PAIR 3
#define SELECTED_BOX_PAIR 4
#define TEXT_PAIR 5

WINDOW *create_newwin(int height, int width, int startY, int startX, int f);
void destroy_win(WINDOW *local_win);

int main(int argc, char *argv[]) {
    // WINDOW *my_win;
    int startX, startY, tableWidth, tableHeight;
    int ch;
    initscr(); /* Start curses mode 		*/
    start_color();
    cbreak();             /* Line buffering disabled, Pass on
                           * everty thing to me 		*/
    keypad(stdscr, TRUE); /* I need that nifty F1 	*/
    getmaxyx(stdscr,tableHeight,tableWidth);
    
    init_pair(BACKGROUND_PAIR, COLOR_GREEN, COLOR_YELLOW);
    init_pair(BIG_BOX_PAIR, COLOR_MAGENTA, COLOR_MAGENTA);
    init_pair(SMALL_BOX_PAIR, COLOR_BLUE, COLOR_BLUE);
    init_pair(SELECTED_BOX_PAIR, COLOR_CYAN, COLOR_CYAN);
    init_pair(TEXT_PAIR, COLOR_RED, COLOR_WHITE);

    wbkgd(initscr(), COLOR_PAIR(BACKGROUND_PAIR));
    tableHeight /= 2;
    tableWidth /= 2;

    tableHeight = 4 * (tableHeight / 4);
    tableWidth = 5 * (tableWidth / 5);

    printw("Press F1 to exit");
    refresh();

    WINDOW *table = create_newwin(tableHeight + 4,tableWidth + 4,tableHeight/2-2,tableWidth/2-2, 0);
    /*20 columns*/

    WINDOW *bigBox[4][5];

    int changeH = tableHeight / 4;
    int changeW = tableWidth / 5;
    startY = tableHeight/2; /* Calculating for a center placement */
    startX = tableWidth/2;   /* of the window		*/
    int selectedRow = 0, selectedCol = 0;
    
    FILE *filePointerRead;
    FILE *filePointerWrite;

    filePointerRead = fopen("database.txt", "r");

    int data[4][4];

    for(int i=0;i<4;i++){
        fscanf(filePointerRead, "%d %d %d %d", &data[i][0], &data[i][1],
               &data[i][2], &data[i][3]);
    }
    for(int i=0;i<4;i++){
      int sumRow=0;
      for(int j=0;j<5;j++){
        bigBox[i][j] = create_newwin(tableHeight/4,tableWidth/5,startY,startX, 1);
        if (j != 4) {
            sumRow += data[i][j];
            mvwprintw(bigBox[i][j], changeH / 2, changeW / 2, "%d", data[i][j]);
        } else {
            mvwprintw(bigBox[i][j], changeH / 2, changeW / 2, "%d", sumRow);
        }
        wrefresh(bigBox[i][j]);
        startX += changeW;
      }
      startY += changeH;
      startX = tableWidth/2;
    }

    startY = tableHeight / 2; /* Calculating for a center placement */
    startX = tableWidth / 2;  /* of the window		*/

    wattron(bigBox[selectedRow][selectedCol], COLOR_PAIR(SELECTED_BOX_PAIR));
    box(bigBox[selectedRow][selectedCol],124,45);
    wrefresh(bigBox[selectedRow][selectedCol]);
    wattroff(bigBox[selectedRow][selectedCol], COLOR_PAIR(SELECTED_BOX_PAIR));

    // my_win = create_newwin(tableHeight/4, tableWidth/5, startY, startX);

    while ((ch = getch()) != KEY_F(1)) {
        switch (ch) {
            case KEY_LEFT:
                startX -= changeW;
                selectedCol --;
                if(startX < tableWidth/2){
                  startX += changeW;
                  selectedCol ++;
                }
                break;
            case KEY_RIGHT:
                startX += changeW;
                selectedCol ++;
                if (startX >= 3 * tableWidth / 2) {
                    startX -= changeW;
                    selectedCol --;
                }
                break;
            case KEY_UP:
                startY -= changeH;
                selectedRow --;
                if (startY < tableHeight / 2) {
                    startY += changeH;
                    selectedRow ++;
                }
                break;
            case KEY_DOWN:
                startY += changeH;
                selectedRow ++;
                if (startY >= 3 * tableHeight / 2) {
                    startY -= changeH;
                    selectedRow --;
                }
                break;
            case (int)'\n':
                if(selectedCol==4){
                  break;
                }
                move(startY + changeH / 2, startX + changeW / 2);
                wattron(bigBox[selectedRow][selectedCol], COLOR_PAIR(TEXT_PAIR));
                mvwprintw(bigBox[selectedRow][selectedCol], changeH / 2, changeW / 2, " ");
                wrefresh(bigBox[selectedRow][selectedCol]);
                char inp = getch();

                
                wattron(bigBox[selectedRow][selectedCol], COLOR_PAIR(TEXT_PAIR));
                mvwprintw(bigBox[selectedRow][selectedCol], changeH / 2,
                          changeW / 2, "%c", inp);
                wrefresh(bigBox[selectedRow][selectedCol]);
                char inp2 = getch();
                if(inp2 == '\n'){
                  if(inp>='0' && inp<= '9'){
                    data[selectedRow][selectedCol] = inp - '0';
                  }
                }
                break;
        }

        table = create_newwin(tableHeight + 4, tableWidth + 4,
                              tableHeight / 2 - 2, tableWidth / 2 - 2, 0);
        int tempX;
        int tempY;
        tempY = tableHeight / 2; /* Calculating for a center placement */
        tempX = tableWidth / 2;  /* of the window		*/

        filePointerWrite = fopen("database1.txt", "a");
        for (int i = 0; i < 4; i++) {
          int sumRow = 0;
            for (int j = 0; j < 5; j++) {
                bigBox[i][j] = create_newwin(tableHeight / 4, tableWidth / 5,
                                             tempY, tempX, 1);
                if(j!=4){
                  sumRow += data[i][j];
                  mvwprintw(bigBox[i][j], changeH / 2,changeW / 2, "%d", data[i][j]);
                } else {
                    mvwprintw(bigBox[i][j], changeH / 2, changeW / 2, "%d",
                              sumRow);
                }
                wrefresh(bigBox[i][j]);
                tempX += changeW;
            }

            fprintf(filePointerWrite, "%d %d %d %d\n", data[i][0], data[i][1],
                        data[i][2], data[i][3]);
            
            tempY += changeH;
            tempX = tableWidth / 2;
        }

        wbkgd(bigBox[selectedRow][selectedCol], COLOR_PAIR(1));

        wattron(bigBox[selectedRow][selectedCol], COLOR_PAIR(SELECTED_BOX_PAIR));
        box(bigBox[selectedRow][selectedCol],124,45);
        wrefresh(bigBox[selectedRow][selectedCol]);
        wattroff(bigBox[selectedRow][selectedCol], COLOR_PAIR(SELECTED_BOX_PAIR));

        move(startY + changeH / 2,startX + changeW / 2);
    }

    endwin(); /* End curses mode		  */
    return 0;
}

WINDOW *create_newwin(int height, int width, int startY, int startX, int f) {
    WINDOW *local_win;
    local_win = newwin(height, width, startY, startX);
    if (f)
        wattron(local_win, COLOR_PAIR(SMALL_BOX_PAIR));
    else
        wattron(local_win, COLOR_PAIR(BIG_BOX_PAIR));
    box(local_win, 0, 0); /* 0, 0 gives default characters
                           * for the vertical and horizontal
                           * lines			*/
    wrefresh(local_win);  /* Show that box 		*/
    wattroff(local_win, COLOR_PAIR(BIG_BOX_PAIR));

    return local_win;
}

void destroy_win(WINDOW *local_win) {
    /* box(local_win, ' ', ' '); : This won't produce the desired
     * result of erasing the window. It will leave it's four corners
     * and so an ugly remnant of window.
     */
    wborder(local_win, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
    /* The parameters taken are
     * 1. win: the window on which to operate
     * 2. ls: character to be used for the left side of the window
     * 3. rs: character to be used for the right side of the window
     * 4. ts: character to be used for the top side of the window
     * 5. bs: character to be used for the bottom side of the window
     * 6. tl: character to be used for the top left corner of the window
     * 7. tr: character to be used for the top right corner of the window
     * 8. bl: character to be used for the bottom left corner of the window
     * 9. br: character to be used for the bottom right corner of the window
     */
    wrefresh(local_win);
    delwin(local_win);
}

