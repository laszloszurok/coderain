#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define SPEED 50000 //lower number -> faster generation
#define UPDATE_RATE 0.05

int counter = 0;

typedef struct screen {
    int height;
    int width;
    char** scr;
    int** visible; //visibility of characters
}screen;

// ascii codes in 33-126 interval
char generateRandomChar(){
   return (char)((rand() % 94) + 33);
}

// return 1 if column is empty, 0 otherwise
int isColumnEmpty(screen* s, int col){
   int i;
   for(i = 0; i < s->height; i++){
        if(s->visible[i][col] == 1){
            return 0;
        }
   }
   return 1;
}

int generateRandomColumn(screen* s){
    int col, counter = 0;
    while(counter < 5000){
        col = rand() % s->width;
        if(isColumnEmpty(s, col) == 1){
            return col;
        }
        counter++;
    }
    return -1;
}

void initScrWithChars(screen* s){
    int i, j;
    for(i = 0; i < s->height; i++){
        for(j = 0; j < s->width; j++){
            s->scr[i][j] = generateRandomChar();
            s->visible[i][j] = 0; //every char is invisible at first
        }
    }
}

void setColor(screen* s, int row, int col) {
    //find the first row where visibility is 0
    int i, j;
    for(i = 0; i < s->height && s->visible[i][col] != 1; i++);
    for(j = s->height - 1; j > 0 && s->visible[j][col] !=1; j--);
    
    if(i == 0){
        if(row > j - 4) {
            attron(COLOR_PAIR(2));
        } else {
            attron(COLOR_PAIR(1));
        }
    } else if(j == s->height - 1) {
        if(row >= s->height - (4 - i)){
            attron(COLOR_PAIR(2));
        } else {
            attron(COLOR_PAIR(1));
        }
    } else {
        attron(COLOR_PAIR(1));
    }
}

void drawScreen(screen* s){
    int i, j;
    for(i = 0; i < s->height; i++){
        for(j = 0; j < s->width; j++){
            if(s->visible[i][j] == 1){
                setColor(s, i, j);
                mvaddch(i, j, s->scr[i][j]);
            } else {
                mvaddch(i, j, ' '); //draw a space in place of an invisible character
            }
        }
    }
    refresh();
}

screen* init(){
    initscr();
    curs_set(0); // hide the cursor
    start_color(); // enable color
    use_default_colors();
    init_pair(1, -1, -1);
    init_pair(2, COLOR_WHITE, -1);
    //init_color(COLOR_BLACK, 0, 0, 0);
   
    //init random number generation 
    srand(time(NULL));

    screen* s = malloc(sizeof(screen));

    //get width and height of the terminal
    getmaxyx(stdscr, s->height, s->width);
    
    //allocate memory
    s->scr = (char**)malloc(sizeof(char*)*s->height);
    s->visible = (int**)malloc(sizeof(int*)*s->height);

    int i;
    for(i = 0; i < s->height; i++){
        s->scr[i] = (char*)malloc(sizeof(char) * s->width);
        s->visible[i] = (int*)malloc(sizeof(int) * s->width);
    }

    initScrWithChars(s);

    return s;
}

void finalize(screen* s){
    //free the allocated memory
    int i;
    for(i = 0; i < s->height; i++){
        free(s->scr[i]);
        s->scr[i] = NULL;

        free(s->visible[i]);
        s->visible[i] = NULL;
    }

    free(s->scr);
    s->scr = NULL;

    free(s->visible);
    s->visible = NULL;

    free(s);
    s = NULL;
    endwin();
}

void animateRain(screen* s){
    int i, j;
    for(j = 0; j < s->width; j++){
        for(i = s->height - 1; i > 0; i--){
            if(s->visible[i-1][j] == 1 && s->visible[i][j] == 0){
                s->visible[i][j] = 1;
            } else if(s->visible[i][j] == 1 && s->visible[i-1][j] == 0) {
                s->visible[i][j] = 0;
            }
        }
        if(s->visible[s->height - 1][j] == 1 && s->visible[0][j] == 1){
            s->visible[0][j] = 0;
        }
    }
}

void randomChanges(screen* s){
    int x, y;

    int counter = 0;
    while(counter < UPDATE_RATE * s->width * s->height){
        x = rand() % s->height;
        y = rand() % s->width;
        s->scr[x][y] = generateRandomChar();
        counter++;
    }
}

void updateScr(screen* s){
   if(counter % (s->height/20) == 0){
        int newCol = generateRandomColumn(s);
        if(newCol >= 0){
            s->visible[0][newCol] = 1; 
        } 
        randomChanges(s);
   }
   animateRain(s);
   counter++;
}

int main(int argc, char **argv){
    screen* s = init();
   
    while(1){
        updateScr(s);
        drawScreen(s);
        usleep(SPEED);
    }

    finalize(s);
    return 0;
}
