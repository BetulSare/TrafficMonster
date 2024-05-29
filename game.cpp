#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <ncurses.h>
#include <iostream>
#include <queue>
#include <ctime>
#include <string.h>
#define wHeight 40 /// height of the road
#define wWidth 100 /// width of the road
#define lineX 45 /// x coordinate of the middle line
#define lineLEN 10 /// distance of the middle line from the beginning and the end
#define EXITY 35 ///coordinate showing the end of the road
#define leftKeyArrow 260 /// ASCII code of the left arrow key
#define RightKeyArrow 261 /// ASCII code of the right arrow key
#define leftKeyA 97/// ASCII code of A
#define RightKeyD 100 /// ASCII code of D
#define ESC 27 /// ASCII code of the ESC key
#define ENTER 10 /// ASCII code of the ENTER key
#define KEYPUP 259 /// ASCII code of the up arrow key
#define KEYDOWN 258 /// ASCII code of the down arrow key
#define KEYERROR -1 /// ASCII code returned if an incorrect key is pressed
#define SAVEKEY 115 /// ASCII code of S
#define levelBound 300 /// To increase level after 300 points
#define MAXSLEVEL 5 /// maximum level
#define ISPEED 500000 /// initial value for game moveSpeed
#define DRATESPEED 100000 /// to decrease moveSpeed after each new level
#define MINX 5 /// minimum x coordinate value when creating cars
#define MINY 10 /// the maximum y coordinate value when creating the cars, then we multiply it by -1 and take its inverse
#define MINH 5 /// minimum height when creating cars
#define MINW 5 /// minimum width when creating cars
#define SPEEDOFCAR 3 /// speed of the car driven by the player
#define YOFCAR 34 /// y coordinate of the car used by the player
#define XOFCAR 45 /// x coordinate of the car used by the player
#define IDSTART 10 /// initial value for cars ID
#define IDMAX 20/// maximum value for cars ID
#define COLOROFCAR 3 /// color value of the car used by the player
#define POINTX 91 ///x coordinate where the point is written
#define POINTY 42 ///y coordinate where the point is written
#define MENUX 10 /// x coordinate for the starting row of the menus
#define MENUY 5 /// y coordinate for the starting row of the menus
#define MENUDIF 2 /// difference between menu rows
#define MENUDIFX 20 /// difference between menu columns
#define MENSLEEPRATE 200000 /// sleep time for menu input
#define GAMESLEEPRATE 250000 /// sleep time for player arrow keys
#define EnQueueSleep 1 /// EnQueue sleep time
#define DeQueueSleepMin 2 /// DeQueue minimum sleep time
#define numOfcolors 4 /// maximum color value that can be selected for cars
#define maxCarNumber 5 /// maximum number of cars in the queue
#define numOfChars 3 /// maximum number of patterns that can be selected for cars
#define settingMenuItem 2 /// number of options in the setting menu
#define mainMenuItem 6 /// number of options in the main menu
using namespace std;
typedef struct Car{ //
    int ID;
    int x;
    int y;
    int height;
    int width;
    int speed;
    int clr;
    bool isExist;
    char chr;
}Car;

typedef struct Game{
    int leftKey;
    int rightKey;
    queue<Car> cars;
    bool IsGameRunning;
    bool IsSaveCliked;
    int counter;
    pthread_mutex_t mutexFile;
    Car current;
    int level;
    int moveSpeed;
    int points;
}Game;

Game playingGame; /// Global variable used for new game
const char *gameTxt =  "game.txt";
const char *CarsTxt =  "cars.txt";
const char *pointsTxt =  "points.txt";

const char *settingMenu[50] = {"Play with < and > arrow keys","Play with A and D keys"};///Array with options for the Setting menu
const char *mainMenu[50] = {"New Game","Load the last game","Instructions", "Settings","Points","Exit"};///Array with options for the Main menu
void drawCar(Car c, int type, int direction); ///prints or remove the given car on the screen
void printWindow(); ///Draws the road on the screen
void *newGame(void *); /// manages new game
void initGame(); /// Assigns initial values to all control parameters for the new game
void initWindow(); ///Creates a new window and sets I/O settings
void instructions();
void settings();
void points();
void printPoint();
void drawTree(int x,int y);
void *MoveCar(void *c);
void *Enqueue(void *);
void *Dequeue(void *);
Car NewCar();
int rnd(int n);
void loadTheLastGame();

int main()
{
    playingGame.leftKey = leftKeyArrow;
    playingGame.rightKey = RightKeyArrow;

    initWindow();
    return 0;
}

void *newGame(void *){
    printWindow();
    printPoint();

    drawCar(playingGame.current,2,1); /// Draw the car the player is driving on the screen
    int key;


    while (playingGame.IsGameRunning && !playingGame.IsSaveCliked) { ///continue until the game is over

    if(playingGame.points>=playingGame.level*300 && playingGame.level!=5){
        playingGame.level++;
        playingGame.moveSpeed-=100000;
    }
            key = getch(); ///Get input for the player to press the arrow keys
                if(key==ESC){
                    playingGame.IsGameRunning=false;
                }else if (key == playingGame.leftKey) { /// If the left  key is pressed
                    if(playingGame.current.x>=5){
                        drawCar(playingGame.current,1,1); /// removes player's car from screen
                        playingGame.current.x-=playingGame.current.speed; /// update position
                        drawCar(playingGame.current,2,1); /// draw player's car with new position
                    }
                }else if(key==playingGame.rightKey){
                    if(playingGame.current.x<=89){
                        drawCar(playingGame.current,1,1); /// removes player's car from screen
                        playingGame.current.x+=playingGame.current.speed; /// update position
                        drawCar(playingGame.current,2,1); /// draw player's car with new position
                    }
                }else if(key==SAVEKEY){
                     ///kaydettt
                    FILE *file=fopen(pointsTxt,"a");
                    fprintf(file,"%d\n",playingGame.points);
                    fclose(file);
                    playingGame.IsSaveCliked = true;

                    ///oyunu dosyaya yazma
                    pthread_mutex_lock(&playingGame.mutexFile);
                    FILE *filegame = fopen(gameTxt, "w+");

                    if(filegame != nullptr) {
                        fwrite(&playingGame,sizeof(playingGame),1,filegame);
                        fclose(filegame);
                    }
                    pthread_mutex_unlock(&playingGame.mutexFile);

                    ///araçları dosyaya yazma

                    playingGame.IsGameRunning = false;

                }

         usleep(GAMESLEEPRATE); ///sleep
        }

}

void *MoveCar(void *c){

    Car newc = *(Car *)c;
    while(newc.isExist){
        drawCar(newc, 1, 0);///siler
        newc.y += 1 + rand() % newc.speed;
        drawCar(newc, 2, 0);///yazar

        int carLeft = newc.x;
        int carRight = newc.x + newc.width;
        int carTop = newc.y;
        int carBottom = newc.y + newc.height;

        int playerLeft = playingGame.current.x;
        int playerRight = playingGame.current.x + playingGame.current.width;
        int playerTop = playingGame.current.y;
        int playerBottom = playingGame.current.y + playingGame.current.height;


         ///Çarpışma olursa
        if (carRight >= playerLeft && carLeft <= playerRight &&
            carBottom >= playerTop && carTop <= playerBottom) {
                playingGame.IsGameRunning = false;
                FILE *file=fopen(pointsTxt,"a");
                fprintf(file,"%d\n",playingGame.points);
                fclose(file);
                sleep(1);
                return NULL;
        }
        if(playingGame.IsSaveCliked){
            pthread_mutex_lock(&playingGame.mutexFile);
            FILE *filecars = fopen(CarsTxt, "w+");

            if(filecars != nullptr) {
                fwrite(&newc,sizeof(newc),1,filecars);
                fclose(filecars);
            }
            pthread_mutex_unlock(&playingGame.mutexFile);
        }

        if (newc.y >= 35 && newc.isExist ) {
            playingGame.points += newc.height * newc.width;
            drawCar(newc, 1, 0);
            newc.isExist = false;

        }
        usleep(playingGame.moveSpeed);
    }
    printPoint();
}

void *Enqueue(void *){

    while(playingGame.IsGameRunning){
        if(playingGame.cars.size()<=5){
        Car newCar=NewCar();
        playingGame.cars.push(newCar);

    }
    sleep(1);
    }
}

void *Dequeue(void *){

    sleep(1);
    Car c;
    pthread_t th1;
    while(playingGame.IsGameRunning){
        c=playingGame.cars.front();
        c.isExist=true;
        pthread_create(&th1,NULL,MoveCar,(void *)&c);
        playingGame.cars.pop();
        int number=2+rnd(3);
        sleep(number);
    }
}
int rnd(int n){
    srand(time(NULL));
    int number;
    number = rand() % n;
    return number;
}

void drawCar(Car c, int type, int direction){
	///If the user does not want to exit the game and the game continues
    if(playingGame.IsSaveCliked!=true && playingGame.IsGameRunning==true)
    {
            init_pair(c.ID, c.clr, 0);

            /// Creates a color pair: init_pair(short pair ID, short foregroundcolor, short backgroundcolor);
            ///0: Black (COLOR_BLACK)
			///1: Red (COLOR_RED)
			///2: Green (COLOR_GREEN)
			///3: Yellow (COLOR_YELLOW)
			///4: Blue (COLOR_BLUE)
			attron(COLOR_PAIR(c.ID));///enable color pair
            char drawnChar;
            if (type == 1 )
               drawnChar = ' '; /// to remove car
            else
               drawnChar= c.chr;
            ///  to draw char
		    ///mvhline: used to draw a horizontal line in the window
			///shallow. : mvhline(int y, int x, chtype ch, int n)
			///y: horizontal coordinate
			///x: vertical coordinate
			///ch: character to use
			///n: Length of the line
            mvhline(c.y, c.x, drawnChar, c.width);/// top line of rectangle
            mvhline(c.y + c.height - 1, c.x, drawnChar, c.width); ///bottom line of rectangle
            if(direction == 0) /// If it is any car on the road
                mvhline(c.y + c.height, c.x, drawnChar, c.width);
            else ///player's card
                mvhline(c.y -1, c.x, drawnChar, c.width);
		    ///mvvline: used to draw a vertical line in the window
			///shallow. : mvhline(int y, int x, chtype ch, int n)
			///y: horizontal coordinate
			///x: vertical coordinate
			///ch: character to use
			///n: Length of the line
            mvvline(c.y, c.x, drawnChar, c.height); ///left line of rectangle
            mvvline(c.y, c.x + c.width - 1, drawnChar, c.height); ///right line of rectangle
            char text[5];
            if (type == 1 )
                sprintf(text,"  "); ///to remove point
            else
            sprintf(text,"%d",c.height * c.width); /// to show car's point in rectangle
            mvprintw(c.y+1, c.x +1, text);/// display car's point in rectangle
            attroff(COLOR_PAIR(c.ID));/// disable color pair
    }
}

void initWindow(){

    initscr();            /// initialize the ncurses window
	start_color();        /// enable color manipulation
	keypad(stdscr, true); /// enable the keypad for the screen

	curs_set(0);          /// hide the cursor
	cbreak();             /// disable line buffering
	noecho();             /// don't echo characters entered by the user

    int x =10, y = 5;
    init_pair(1, COLOR_GREEN, COLOR_BLACK);
    init_pair(2, COLOR_RED, COLOR_BLACK);

    pthread_t th1; ///create new thread
    pthread_t th2; ///create new thread enqueue
    pthread_t th3; ///create new thread dequeue


    int choice;
    int highlight=0;

    while(1){
        for (int i = 0; i < 6; i++)
        {
             if(i==highlight){
                char str[200];
                strcpy(str,"->");
                strcat(str,mainMenu[i]);


                attron(COLOR_PAIR(2));
                mvprintw(y, x-2,str );///mainMenu[i]
                attroff(COLOR_PAIR(2));
                y += 2;
            }else{
                attron(COLOR_PAIR(1));
                mvprintw(y, x, mainMenu[i]);
                attroff(COLOR_PAIR(1));
                y += 2;
            }

        }

        choice=getch();
        switch (choice){
            case KEYPUP:
                highlight--;
                if(highlight==-1){
                    highlight=0;
                }
                break;
            case KEYDOWN:
                highlight++;
                if(highlight==6){
                    highlight=5;
                }
                break;
            case 10:
                 switch(highlight){
                    case 0:

                        initGame();
                        clear();
                        refresh();
                        sleep(1);
                        endwin();

                        pthread_create(&th1, NULL, newGame,NULL);/// Run newGame function with thread
                        pthread_create(&th2, NULL, Enqueue,NULL);/// Run ENQUEUE
                        pthread_create(&th3, NULL, Dequeue,NULL);/// Run DEQUEUE


                        pthread_join(th3,NULL);
                        pthread_join(th2,NULL);
                        pthread_join(th1,NULL); ///Wait for the thread to finish, when the newGame function finishes, the thread will also finish.


                        ///enqueue ve dequeue burada newgame threadinden önce join edilecek

                        nodelay(stdscr, false);
                    break;

                    case 1:
                        loadTheLastGame();

                        ///load the last game
                    break;


                    case 2:
                        ///instructions
                        instructions();
                    break;

                    case 3:
                        ///settings
                        settings();
                    break;

                    case 4:
                        ///points
                        points();
                    break;

                    case 5:
                        ///exit
                        goto end;
                    break;

                 }
            break;
        }
         x =10, y = 5;
        clear();
    }


    end:
    refresh();

    endwin();

    sleep(1);
}

void loadTheLastGame(){
    pthread_t th1,th2,th3,th4;
    pthread_mutex_lock(&playingGame.mutexFile);
    FILE *filegame = fopen(gameTxt, "r+");
    Game lastGame;

    if(filegame != nullptr) {
        fread(&lastGame,sizeof(Game),1,filegame);
        fclose(filegame);
    }
        pthread_mutex_unlock(&playingGame.mutexFile);
        lastGame.IsSaveCliked=false;
        lastGame.IsGameRunning=true;
        playingGame=lastGame;

        ///araçları dosyadan okuma
        pthread_mutex_lock(&playingGame.mutexFile);
        FILE *filecars = fopen(CarsTxt, "r+");
        Car lastCars;
        if(filecars != nullptr) {
            fread(&lastCars,sizeof(Car),1,filecars);
            pthread_create(&th4,NULL,MoveCar,(void *)&lastCars);
            while (!feof(filecars)){
                fread(&lastCars,sizeof(Car),1,filecars);
                //playingGame.cars.push(lastCars);
                pthread_create(&th4,NULL,MoveCar,(void *)&lastCars);

            }
            fclose(filecars);
        }

        pthread_mutex_unlock(&playingGame.mutexFile);

    clear();
    refresh();
    sleep(1);


    pthread_create(&th1, NULL, newGame,NULL);/// Run newGame function with thread
    pthread_create(&th2, NULL, Enqueue,NULL);/// Run ENQUEUE
    pthread_create(&th3, NULL, Dequeue,NULL);/// Run DEQUEUE

    pthread_join(th1,NULL);
    pthread_join(th3,NULL);
    pthread_join(th2,NULL);
    nodelay(stdscr, false);

}

void printPoint(){
    int x=91,y=42;
    init_pair(3, COLOR_WHITE, COLOR_BLACK);
    char text[100];
    snprintf(text, sizeof(text), "POINT: %d    Level: %d", playingGame.points,playingGame.level);
    attron(COLOR_PAIR(3));
    mvprintw(y, x,text);
    attroff(COLOR_PAIR(3));
}

void printWindow(){



    initscr();            /// initialize the ncurses window
	start_color();        /// enable color manipulation
	keypad(stdscr, true); /// enable the keypad for the screen
	nodelay(stdscr, true);/// set the getch() function to non-blocking mode
	curs_set(0);          /// hide the cursor
	cbreak();             /// disable line buffering
	noecho();             /// don't echo characters entered by the user

    init_pair(1, COLOR_GREEN, COLOR_BLACK);
    init_pair(2, COLOR_RED, COLOR_BLACK);

    for (int i = 1; i < wHeight - 1; ++i) {
		///mvprintw: Used to print text on the window, paramters order: y , x , string
        mvprintw(i, 2, "*"); ///left side of the road
        mvprintw(i, 0, "*");
        mvprintw(i, wWidth - 1, "*");/// right side of the road
        mvprintw(i, wWidth - 3, "*");
    }

    for (int i = lineLEN; i < wHeight -lineLEN ; ++i) { ///line in the middle of the road
        mvprintw(i, lineX, "#");
    }
    drawTree(110,5);
    drawTree(110,15);
    drawTree(110,25);

}
void points(){
    FILE *file = fopen(pointsTxt, "r");
    if (file != NULL) {
        initscr();            // ncurses penceresini başlat
        start_color();        // Renk kullanımını etkinleştir
        keypad(stdscr, true); // Ekran için tuş takımını etkinleştir
        curs_set(0);          // İmleci gizle
        cbreak();             // Satır tamponlamayı devre dışı bırak
        noecho();             // Kullanıcı tarafından girilen karakterleri ekrana yazdırma
        init_pair(1, COLOR_GREEN, COLOR_BLACK); // Renk çiftini tanımla
        clear();
        refresh();
        sleep(1);

        int x = 10, y = 5;
        int countInteger = 0;
        char text[100];

        int currentInteger = 0;


        while (fscanf(file, "%d", &currentInteger) == 1) {
            countInteger++;

            snprintf(text, sizeof(text), "Game %d: %d", countInteger, currentInteger);


            attron(COLOR_PAIR(1));
            mvprintw(y, x, "%s", text);
            attroff(COLOR_PAIR(1));

            if (countInteger % 10 == 0) {
                x += 20;
                y = 5;
            } else {
                y += 2;
            }
            refresh();
        }

        fclose(file);
        refresh();
        sleep(7);
        endwin();
    }
}

void drawTree(int x,int y){



    init_pair(1, COLOR_GREEN, COLOR_BLACK);
    init_pair(2, COLOR_RED, COLOR_BLACK);

    attron(COLOR_PAIR(1));
    mvprintw(y, x, "*");
    attroff(COLOR_PAIR(1));

    attron(COLOR_PAIR(1));
    mvprintw(y+1, x-1, "*");
    attroff(COLOR_PAIR(1));

    attron(COLOR_PAIR(1));
    mvprintw(y+1, x+1, "*");
    attroff(COLOR_PAIR(1));

    attron(COLOR_PAIR(1));
    mvprintw(y+2, x-2, "*");
    attroff(COLOR_PAIR(1));

    attron(COLOR_PAIR(1));
    mvprintw(y+2, x, "*");
    attroff(COLOR_PAIR(1));

    attron(COLOR_PAIR(1));
    mvprintw(y+2, x+2, "*");
    attroff(COLOR_PAIR(1));

    attron(COLOR_PAIR(2));
    mvprintw(y+3, x, "#");
    attroff(COLOR_PAIR(2));

    attron(COLOR_PAIR(2));
    mvprintw(y+4, x, "#");
    attroff(COLOR_PAIR(2));

}
void settings(){
    init_pair(1, COLOR_GREEN, COLOR_BLACK);
    init_pair(2, COLOR_RED, COLOR_BLACK);
    int choiceSettings;
    int highlightSettings=0;

    clear();
    refresh();
    sleep(1);
    endwin();

    initscr();            /// initialize the ncurses window
    start_color();        /// enable color manipulation
    keypad(stdscr, true); /// enable the keypad for the screen
    ///nodelay(stdscr, true);/// set the getch() function to non-blocking mode
    curs_set(0);          /// hide the cursor
    cbreak();             /// disable line buffering
    noecho();             /// don't echo characters entered by the user

    char str2[200];
    int  x =10, y = 5;
    while(1){

        for(int i=0;i<2;i++){
            if(i==highlightSettings){
                char str2[200];
                strcpy(str2,"->");
                strcat(str2,settingMenu[i]);

                attron(COLOR_PAIR(2));
                mvprintw(y,x-2,str2);
                attroff(COLOR_PAIR(2));
                y += 2;
            }else{
                 attron(COLOR_PAIR(1));
                 mvprintw(y, x,settingMenu[i]);
                 attroff(COLOR_PAIR(1));
                 y+=2;
            }

        }

        choiceSettings=getch();


        switch (choiceSettings){
            case KEYPUP:
                highlightSettings--;
                if(highlightSettings==-1){
                    highlightSettings=0;
                }
            break;

            case KEYDOWN:
                highlightSettings++;
                    if(highlightSettings==2){
                        highlightSettings=1;
                    }
            break;

            case 10:
                switch(highlightSettings){
                    case 0:
                        playingGame.leftKey=leftKeyArrow;
                        playingGame.rightKey=RightKeyArrow;
                        clear();
                        return ;
                    break;

                    case 1:
                        playingGame.leftKey=leftKeyA;
                        playingGame.rightKey=RightKeyD;
                        clear();
                        return ;

                    break;

                }
            break;
        }
        clear();
        refresh();
        x =10, y = 5;
    }


}

void instructions(){
init_pair(1, COLOR_GREEN, COLOR_BLACK);
    init_pair(2, COLOR_RED, COLOR_BLACK);
    clear();
    refresh();
    sleep(1);
    endwin();
    initscr();            /// initialize the ncurses window
    start_color();        /// enable color manipulation
    keypad(stdscr, true); /// enable the keypad for the screen
    ///nodelay(stdscr, true);/// set the getch() function to non-blocking mode
    curs_set(0);          /// hide the cursor
    cbreak();             /// disable line buffering
    noecho();             /// don't echo characters entered by the us
    int x =10, y = 5;

    char *instructions[100]={"< or A: moves the car to the left","> or D: moves the car to the right","ESC: exists the game without saving","S: saves and exists the game"};

    for(int i=0;i<4;i++){
        attron(COLOR_PAIR(1));
        mvprintw(y, x,instructions[i]);
        attroff(COLOR_PAIR(1));
        y += 2;

    }

    refresh();
    sleep(7);
    endwin();
}

void initGame(){
    playingGame.cars = queue<Car>();
    playingGame.counter =IDSTART;
    playingGame.mutexFile = PTHREAD_MUTEX_INITIALIZER; ///assigns the initial value for the mutex
    playingGame.level = 1;
    playingGame.moveSpeed = ISPEED;
    playingGame.points = 0;
    playingGame.IsSaveCliked = false;
    playingGame.IsGameRunning = true;
    playingGame.current.ID = IDSTART-1;
    playingGame.current.height = MINH;
    playingGame.current.width = MINW;
    playingGame.current.speed = SPEEDOFCAR;
    playingGame.current.x = XOFCAR;
    playingGame.current.y = YOFCAR;
    playingGame.current.clr = COLOROFCAR;
    playingGame.current.chr = '*';
}

Car NewCar(){
    Car newCar;
    newCar.height=5+rnd(3);
    if(playingGame.counter==20){
        playingGame.counter=10;
    }
    newCar.ID=IDSTART+playingGame.counter;
    playingGame.counter++;

    newCar.isExist=false;
    newCar.clr=1+rnd(numOfcolors + 1);
    newCar.chr=1+rnd(numOfChars + 1);

    if(newCar.chr==1){
        newCar.chr='+';
    }else if(newCar.chr==2){
        newCar.chr='*';
    }else if(newCar.chr==3){
        newCar.chr='#';
    }else{
        newCar.chr='+';
    }


    newCar.speed=newCar.height/2;
    int xint;
    xint=5+rnd(86);
    while(xint>38 && xint<46){
        xint=5+rnd(86);
    }
    newCar.x=xint;
    newCar.y=-10+rnd(1);///-10 dan baþlayacak
    newCar.width=5+rnd(3);
    return newCar;
}
