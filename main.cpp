#include <ncurses.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

#define BACKGROUND_PAIR 1
#define BIG_BOX_PAIR 2
#define SMALL_BOX_PAIR 3
#define SELECTED_BOX_PAIR 4
#define TEXT_PAIR 5

WINDOW *create_newwin(int height, int width, int startY, int startX,
                      bool headerOn);
void destroy_win(WINDOW *local_win);
void display_table_header(WINDOW *win, int starty, int startx, int width,
                          string &label);
void showhelp(WINDOW *winOriginal);

class Utilities {
    public:

        /**
         * @brief used for taking in command's output to pipe through popen()
         * makeing use of IPC. command paramter is used for coomand to excute and get back
         * output to user.
         *
         * @param command
         * @return string
         */
        std::string get_popen(const char *command) {
            FILE *pf;
            char data[BUFFSIZE];
            std::string result;

            // Setup our pipe for reading and execute our command.
            pf = popen(command, "r");

            // Get the data from the process execution
            fgets(data, BUFFSIZE, pf);

            // the data is now in 'data'

            // Error handling
            if (pclose(pf) != 0)
                fprintf(stderr, " Error: Failed to close command stream \n");

            result = data;
            return result;
        }

        /**
         * @brief used for passing commands to pipe through popen() makeing use of
         * IPC. command paramter is used for commands to be executed and output passed.
         *
         * @param command
         * @param data
         */
        void set_popen(const char *command, const char *data) {
            FILE *pf;
            // char data[BUFFSIZE];

            // Setup our pipe for writing to file according to our command.
            pf = popen(command, "w");
            // std::cout<<data<<std::endl;
            // Set the data from the process execution
            fputs(data, pf);

            // the data is now written to file

            // Error handling
            if (pclose(pf) != 0)
                fprintf(stderr, " Error: Failed to close command stream \n");

            return;
        }
};


int main(int argc, char *argv[]) {
    Utilities utilities;
    int startX, startY, tableWidth, tableHeight;
    int ch;
    initscr();  // Start curses mode
    start_color();
    cbreak();  // Line buffering disabled

    // for getting keypad inputs
    keypad(stdscr, TRUE);
    getmaxyx(stdscr, tableHeight, tableWidth);

    // init_pair(1, COLOR_WHITE, COLOR_BLACK);
    init_pair(BACKGROUND_PAIR, COLOR_GREEN, COLOR_BLACK);
    init_pair(BIG_BOX_PAIR, COLOR_BLUE, COLOR_BLACK);
    init_pair(SMALL_BOX_PAIR, COLOR_WHITE, COLOR_BLACK);
    init_pair(SELECTED_BOX_PAIR, COLOR_CYAN, COLOR_BLACK);
    init_pair(TEXT_PAIR, COLOR_RED, COLOR_WHITE);

    // wbkgd(initscr(), COLOR_PAIR(BACKGROUND_PAIR));

    tableHeight /= 2;
    tableWidth /= 2;

    tableHeight = 4 * (tableHeight / 4);
    tableWidth = 5 * (tableWidth / 5);

    mvprintw(
        1, 1,
        "Hello Sir !! Welcome to the marks sheet of students (out of 9).\n");
    mvprintw(2, 1, "Have a good day sire ;)\n");
    // addstr("Press F2 for help\n");

    refresh();

    int heightOffset = 8, widthOffset = 4;
    WINDOW *table = create_newwin(
        tableHeight + heightOffset, tableWidth + widthOffset,
        (tableHeight - heightOffset) / 2, (tableWidth - widthOffset) / 2, true);
    if (table == NULL) {
        endwin();
        printf("Unable to allocate window memory to table. Exiting.\n");
        return (EXIT_FAILURE);
    }

    // 20 columns = 4 students x (4 marks + 1 total) updates realtime database
    /*
        .  A B C D Total
        S1 2 3 4 1  9
        S2 2 7 6 2  10
        S3 7 5 4 8  12
        S4 4 7 9 9  25
    */
    const int tableRows = 5, tableCols = 6;
    WINDOW *bigBox[tableRows][tableCols];

    int changeH = tableHeight / tableRows;
    int changeW = tableWidth / tableCols;
    // Calculating for a center placement of the window

    WINDOW *panel = create_newwin(3, tableWidth + widthOffset + 2,
                                  tableHeight + 11, 3, false);

    startY = (tableHeight + heightOffset / 2) / 2;
    startX = tableWidth / 2;
    int selectedRow = 0, selectedCol = 0;

    string fname = "database.txt";
    vector<vector<string>> data;
    vector<string> inputRow;
    string line, word;

    ifstream fin(fname, ios::in);
    // take input from CSV database and store in string table matrix
    if (fin.is_open()) {
        while (getline(fin, line)) {
            inputRow.clear();

            stringstream str(line);

            while (getline(str, word, ',')) inputRow.push_back(word);
            data.push_back(inputRow);
        }
    } else {
        cout << "Could not open the database file. Make sure database.txt is "
                "present and filed\n";
        exit(EXIT_FAILURE);
    }
    fin.close();

    for (int i = 0; i < tableRows; i++) {
        int sumRow = 0;
        for (int j = 0; j < tableCols; j++) {
            bigBox[i][j] =
                create_newwin(tableHeight / tableRows, tableWidth / tableCols,
                              startY, startX, false);

            if ((i == 0 || j == 0)) {
                // header and names of table
                // Info: use s.c_str() for printing the string table data, and
                // stoi for conversion to int
                if (j == tableCols - 1) {
                    mvwprintw(bigBox[i][j], changeH / 2, changeW / 4, "%s",
                              "Total");
                } else {
                    mvwprintw(bigBox[i][j], changeH / 2, changeW / 4, "%s",
                              data[i][j].c_str());
                }

            } else if (j != tableCols - 1) {
                fname = "../../osAdmin/data";
                fname +=to_string(i);
                fname += to_string(j);

                std::string content = utilities.get_popen("cat "+fname);
                sumRow += stoi(content);
                mvwprintw(bigBox[i][j], changeH / 2, changeW / 2, "%s",
                          content.c_str());

                fin.close();
            } else {
                mvwprintw(bigBox[i][j], changeH / 2, changeW / 2 - 1, "%d",
                          sumRow);  // -1 for center allign
            }
            wrefresh(bigBox[i][j]);
            startX += changeW;
        }
        startY += changeH;
        startX = tableWidth / 2;
    }

    // Instruction Panel Initial
    string panel_row = "Row no: ";
    string panel_col = " Column no: ";
    string panel_edit_on = "     EDIT MODE  ";
    string panel_edit_off = "     NORMAL MODE";
    string panel_info = panel_row + to_string(selectedRow) + panel_col +
                         to_string(selectedCol) + panel_edit_off;
    mvwprintw(panel, 1, 1, "%s", panel_info.c_str());
    wrefresh(panel);

    // Calculating for a center placement of the window
    startY = (tableHeight + heightOffset / 2) / 2;
    startX = tableWidth / 2;

    wattron(bigBox[selectedRow][selectedCol], COLOR_PAIR(SELECTED_BOX_PAIR));
    box(bigBox[selectedRow][selectedCol], 124, 45);
    wrefresh(bigBox[selectedRow][selectedCol]);
    wattroff(bigBox[selectedRow][selectedCol], COLOR_PAIR(SELECTED_BOX_PAIR));

    // my_win = create_newwin(tableHeight/4, tableWidth/5, startY,startX);

    while ((ch = getch()) != KEY_F(1)) {
        switch (ch) {
            case KEY_LEFT: {
                startX -= changeW;
                selectedCol--;
                if (selectedCol < 0) {
                    startX += changeW;
                    selectedCol++;
                }
                break;
            }
            case KEY_RIGHT: {
                startX += changeW;
                selectedCol++;
                if (selectedCol >= tableCols) {
                    startX -= changeW;
                    selectedCol--;
                }
                break;
            }
            case KEY_UP: {
                startY -= changeH;
                selectedRow--;
                if (selectedRow < 0) {
                    startY += changeH;
                    selectedRow++;
                }
                break;
            }
            case KEY_DOWN: {
                startY += changeH;
                selectedRow++;
                if (selectedRow >= tableRows) {
                    startY -= changeH;
                    selectedRow--;
                }
                break;
            }
            case (int)'\n': {
                if (selectedCol + 1 == tableCols || selectedCol == 0 ||
                    selectedRow == 0) {
                    break;
                }
                move(startY + changeH / 2, startX + changeW / 2);
                wattron(bigBox[selectedRow][selectedCol],
                        COLOR_PAIR(TEXT_PAIR));
                mvwprintw(bigBox[selectedRow][selectedCol], changeH / 2,
                          changeW / 2, " ");
                wrefresh(bigBox[selectedRow][selectedCol]);

                panel_info = panel_row + to_string(selectedRow) + panel_col +
                             to_string(selectedCol) + panel_edit_on;
                mvwprintw(panel, 1, 1, "%s", panel_info.c_str());
                wrefresh(panel);

                char inp = getch();

                wattron(bigBox[selectedRow][selectedCol],
                        COLOR_PAIR(TEXT_PAIR));
                mvwprintw(bigBox[selectedRow][selectedCol], changeH / 2,
                          changeW / 2, "%c", inp);
                wrefresh(bigBox[selectedRow][selectedCol]);
                char inp2 = getch();
                if (inp2 == '\n') {
                    if (inp >= '0' && inp <= '9') {
                        // taking int as str because the data table is stored in
                        // form of string
                        data[selectedRow][selectedCol] = inp;
                    }
                }
                break;
            }
            case KEY_F(2): {
                showhelp(stdscr);
                box(panel,0,0);
                break;
            }
        }

        panel_info = panel_row + to_string(selectedRow) + panel_col +
                     to_string(selectedCol) + panel_edit_off;
        mvwprintw(panel, 1, 1, "%s", panel_info.c_str());
        wrefresh(panel);

        // remake table after movement
        table =
            create_newwin(tableHeight + heightOffset, tableWidth + widthOffset,
                          (tableHeight - heightOffset) / 2,
                          (tableWidth - widthOffset) / 2, true);

        int tempX;
        int tempY;
        // Calculating for a center placement of the window
        tempY = (tableHeight + heightOffset / 2) / 2;
        tempX = tableWidth / 2;

        for (int i = 0; i < tableRows; i++) {
            int sumRow = 0;
            for (int j = 0; j < tableCols; j++) {
                bigBox[i][j] =
                    create_newwin(tableHeight / tableRows,
                                  tableWidth / tableCols, tempY, tempX, false);
                if ((i == 0 || j == 0)) {
                    // header and names of table
                    // Info: use s.c_str() for printing the string table data,
                    // and stoi for conversion to int
                    if (j == tableCols - 1) {
                        mvwprintw(bigBox[i][j], changeH / 2, changeW / 4, "%s",
                                  "Total");
                    } else {
                        mvwprintw(bigBox[i][j], changeH / 2, changeW / 4, "%s",
                                  data[i][j].c_str());
                    }

                } else if (j != tableCols - 1) {
                    sumRow += stoi(data[i][j]);
                    mvwprintw(bigBox[i][j], changeH / 2, changeW / 2, "%s",
                              data[i][j].c_str());
                } else {
                    // -1 for two digit center allign
                    mvwprintw(bigBox[i][j], changeH / 2, changeW / 2 - 1, "%d",
                              sumRow);
                }
                wrefresh(bigBox[i][j]);
                tempX += changeW;
            }

            // fprintf(filePointerWrite, "%d %d %d %d\n", data[i][0],
            // data[i][1], data[i][2], data[i][3]);

            tempY += changeH;
            tempX = tableWidth / 2;
        }

        // save updated data into CSV database
        ofstream fout(fname, ios::out);
        if (fout.is_open()) {
            int i = 0, j = 0;
            for (i = 0; i < data.size(); i++) {
                for (j = 0; j < data[i].size() - 1; j++) {
                    fout << data[i][j] << ",";
                }
                fout << data[i][j] << "\n";
            }
        } else {
            cout << "Could not update the database file. Make sure "
                    "database.txt is "
                    "present.\n";
            exit(EXIT_FAILURE);
        }
        fout.close();

        wbkgd(bigBox[selectedRow][selectedCol], COLOR_PAIR(4));
        wbkgd(bigBox[selectedRow][0], COLOR_PAIR(1));
        wbkgd(bigBox[0][selectedCol], COLOR_PAIR(1));

        // wattron(bigBox[selectedRow][selectedCol],
        // COLOR_PAIR(SELECTED_BOX_PAIR));
        wrefresh(bigBox[selectedRow][selectedCol]);

        // wattron(bigBox[selectedRow][0], COLOR_PAIR(SELECTED_BOX_PAIR));
        wrefresh(bigBox[selectedRow][0]);

        // wattron(bigBox[0][selectedCol], COLOR_PAIR(SELECTED_BOX_PAIR));
        wrefresh(bigBox[0][selectedCol]);

        box(bigBox[selectedRow][selectedCol], 124, 45);

        wattroff(bigBox[selectedRow][selectedCol],
                 COLOR_PAIR(SELECTED_BOX_PAIR));
        wattroff(bigBox[selectedRow][0], COLOR_PAIR(SELECTED_BOX_PAIR));
        wattroff(bigBox[0][selectedCol], COLOR_PAIR(SELECTED_BOX_PAIR));

        move(startY + changeH / 2, startX + changeW / 2);
    }

    endwin();  // End curses mode
    return 0;
}

WINDOW *create_newwin(int height, int width, int startY, int startX,
                      bool headerOn) {
    WINDOW *local_win;

    local_win = newwin(height, width, startY, startX);
    box(local_win, 0, 0); /* 0, 0 gives default characters
                           * for the vertical and horizontal
                           * lines			*/
    if (headerOn) {
        wattron(local_win, COLOR_PAIR(BIG_BOX_PAIR));

        string label = "STUDENT DATABASE RECORD";
        display_table_header(local_win, 1, 0, width, label);
        label = "Move [Keys] | Edit [Enter] | Exit [F1] | Help[F2]";
        display_table_header(local_win, 4, 0, width, label);

    } else {
        wattron(local_win, COLOR_PAIR(SMALL_BOX_PAIR));
    }

    wrefresh(local_win); /* Show that box 		*/
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

/**
 * @brief adds header text (aligned centerally) and border to a window
 *
 * @param win
 * @param starty
 * @param startx
 * @param width
 * @param label
 */
void display_table_header(WINDOW *win, int starty, int startx, int width,
                          string &label) {
    init_pair(2, COLOR_WHITE, COLOR_RED);
    int length, x, y;
    float temp;

    // set stdscr as default if error in win
    if (win == NULL) win = stdscr;
    // center positioning
    getyx(win, y, x);
    if (startx != 0) x = startx;
    if (starty != 0) y = starty;
    if (width == 0) width = 80;

    length = label.size();
    temp = (width - length) / 2;
    x = startx + (int)temp;
    wattron(win, COLOR_PAIR(2));
    mvwprintw(win, y, x, "%s", label.c_str());  // add header text
    wattroff(win, COLOR_PAIR(2));

    // add horizontal division line
    mvwaddch(win, starty + 2, 0, ACS_LTEE);
    mvwhline(win, starty + 2, 1, ACS_HLINE, width - 2);
    mvwaddch(win, starty + 2, width - 1, ACS_RTEE);
    refresh();
}

/**
 * @brief displays the Help toggle window for all the info
 *
 * @param winOriginal
 */
void showhelp(WINDOW *winOriginal) {
    int width, height;
    getmaxyx(stdscr, height, width);  // get height width of stdscr
    width /= 2;
    height /= 1.2;

    // create window positioned center by width only
    WINDOW *winHelp = newwin(height, width, 0, width / 2);
    box(winHelp, (int)'|', (int)'*');

    // menu labels
    string label[] = {
        "    HELP MENU    ",
        "Arrow Keys: Navigation",
        "F1: Exit Pgm",
        "Enter: Edit Data",
        "Enter: Save edited data",
        "Press ANY key to exit help",
        "Developed By: Aditya, Amit, Anand, Sowmya",
    };
    int size = sizeof(label) / sizeof(label[0]);
    // display menu items on screen
    int i = 0;
    display_table_header(winHelp, i * 3, 0, width, label[i]);
    for (i = 1; i < size; i++) {
        string tmplabel = "(" + to_string(i) + ") " + label[i];
        display_table_header(winHelp, i * 3, 0, width, tmplabel);
    }

    // update the screen to show help menu
    touchwin(winHelp);
    wrefresh(winHelp);
    getch();                // wait for keypress
    touchwin(winOriginal);  // get back to original window

    refresh();
    wrefresh(winOriginal);
}
