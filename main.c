#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <ncurses.h>


#define MAX(x,y) ((x)>(y)?(x):(y))
#define MIN(x,y) ((x)>(y)?(y):(x))

#define MWIDTH 80
#define MHEIGHT 30
#define MSNAKEL 1200

#define UPDATE_INTERV 20000

//moves per second
#define MPS 30

#define INITFOOD 20

#define FOOD 4
#define SNAKE 7
#define EMPTY 0

int width, height, A[MHEIGHT+2][MWIDTH+2], beg_time, cur_time;
char autoplay;
WINDOW *board;

struct brick{
    int x, y;
};

struct snake{
    struct brick body[MSNAKEL], out;
    int beg, end;
    char dir;
};

struct snake* nsnake(int x, int y, char dir){
    struct snake* temp = (struct snake*)malloc(sizeof(struct snake));
    temp->beg=0;
    temp->end=0;
    temp->dir=dir;
    temp->body[0].x=x;
    temp->body[0].y=y;
    temp->out.x=(-1);
    temp->out.y=(-1);
    return temp;
}

struct brick rand_brick(){
    struct brick temp;
    temp.x = 1+rand()%(width-2);
    temp.y = 1+rand()%(height-2);
    return temp;
}

void add_food(){
    struct brick temp;
    do{
        temp = rand_brick();
    }while(A[temp.y][temp.x]==FOOD);
    mvwaddch(board, temp.y, temp.x, 'x');
    A[temp.y][temp.x] = FOOD;
}

//return elapsed time in 1/100 seconds
int get_time(){
    struct timeval temp;
    gettimeofday(&temp, 0);
    return temp.tv_sec*100+temp.tv_usec/10000;
}

char collision(struct brick a){
    return A[a.y][a.x]==SNAKE;
}

char out_of_bounds(struct brick a){
    return a.x<1||a.x>(width-2)||a.y<1||a.y>(height-2);
}

char make_move(struct snake *a){
    struct brick temp = a->body[a->beg];
    switch(a->dir){
        case 1:
            temp.y -= 1;
            break;
        case 2:
            temp.x += 1;
            break;
        case 3:
            temp.y += 1;
            break;
        case 4:
            temp.x -= 1;
            break;
    }


    a->beg = (a->beg+1)%MSNAKEL;
    a->body[a->beg] = temp;
    if(collision(temp)){
        return 2;
    }
    if(A[temp.y][temp.x]!=FOOD){
        a->out = a->body[a->end];
        A[a->out.y][a->out.x] = EMPTY;
        a->end = (a->end+1)%MSNAKEL;
        A[temp.y][temp.x] = SNAKE;
    }
    else{
        A[temp.y][temp.x] = SNAKE;
        add_food();
        a->out.y = (-1);
    }
    if(out_of_bounds(a->body[a->beg]))
        return (-42);
    else
        return 0;
}

char update(struct snake *a){
    int temp = get_time()-cur_time, i, ticks = (float)temp*MPS/100;
    for(i=0; i<ticks; i++)
        if(make_move(a)!=0){
            return 1;
        }
        else{
            if(a->out.y!=(-1)){
                mvwaddch(board, a->out.y, a->out.x, ' ');
            }
            mvwaddch(board, a->body[a->beg].y,
                            a->body[a->beg].x, 'o');
        }
    cur_time += ticks*100/MPS;
    wrefresh(board);
    return 0;
}

int can_move(struct brick a){
    return !out_of_bounds(a)&&A[a.y][a.x]!=SNAKE;
}

int get_rand_dir(struct snake *a){
    struct brick loc = a->body[a->beg];
    int i=0, temp[4];
    loc.y -= 1;
    if(can_move(loc))
        temp[i++] = KEY_UP;
    loc.y += 1;
    loc.x += 1;
    if(can_move(loc))
        temp[i++] = KEY_RIGHT;
    loc.x -= 1;
    loc.y += 1;
    if(can_move(loc))
        temp[i++] = KEY_DOWN;
    loc.y -= 1;
    loc.x -= 1;
    if(can_move(loc))
        temp[i++] = KEY_LEFT;
    loc.x += 1;
    if(i==0)
        return 1+(rand()%4);
    return temp[rand()%i];
}

char five_percent(){
    return rand()%100<5;
}

int get_dir(struct snake *a){
    struct brick loc = a->body[a->beg];
    switch(a->dir){
        case 1:
            loc.y -= 1;
            break;
        case 2:
            loc.x += 1;
            break;
        case 3:
            loc.y += 1;
            break;
        case 4:
            loc.x -= 1;
            break;
    }
    if(!can_move(loc)||five_percent())
        return get_rand_dir(a);
    else
        return ERR;
}


int initialize(){
    initscr();
    cbreak();
    noecho();
    nodelay(stdscr, TRUE);
    keypad(stdscr, TRUE);
    curs_set(0);
    srand(time(NULL));
    width = MIN(MWIDTH,COLS);
    height = MIN(MHEIGHT,LINES);
    board = newwin(height, width, 0, 0);
    box(board, 0, 0);
    refresh();
    wrefresh(board);

    return 0;
}

char pause(){
    int ch, i;
    wmove(board, 10, 10);
    wprintw(board, "Pause");
    wrefresh(board);
    while(true){
        usleep(UPDATE_INTERV*100);
        ch = getch();
        if(ch=='p')
            break;
    }
    wmove(board, 10, 10);
    wprintw(board, "       ");
    for(i=0; i<20; i++)
        if(A[10][i+10]==SNAKE)
            mvwaddch(board, 10, 10+i, 'o');
        else if(A[10][i+10]==FOOD)
            mvwaddch(board, 10, 10+i, 'x');
    wrefresh(board);
    beg_time += get_time() - cur_time;
    cur_time = get_time();
}

int play_game(){
    int ch, i, j;
    struct snake *joe = nsnake(width/2, height/2, 1);
    beg_time = get_time();
    cur_time = get_time();
    A[height/2][width/2] = SNAKE;
    for(i=0; i<INITFOOD; i++)
        add_food();
    while(update(joe)==0){
        usleep(UPDATE_INTERV);
        ch = getch();
        if(autoplay&&ch!='p')
            ch = get_dir(joe);
        switch(ch){
            case ERR:
                break;
            case KEY_UP:
                if(joe->dir!=3)
                    joe->dir = 1;
                break;
            case KEY_RIGHT:
                if(joe->dir!=4)
                    joe->dir = 2;
                break;
            case KEY_DOWN:
                if(joe->dir!=1)
                    joe->dir = 3;
                break;
            case KEY_LEFT:
                if(joe->dir!=2)
                    joe->dir = 4;
                break;
            case KEY_F(1):
                return 1;
            case 'p':
                pause();
                break;
        }

    }
    free(joe);
    for(i=0; i<MHEIGHT+2; i++)
        for(j=0; j<MWIDTH+2; j++)
            A[i][j] = EMPTY;
    wmove(board, 10, 10);
    wprintw(board, "You survived for %d seconds, good job.",
            (cur_time-beg_time)/100);
    wrefresh(board);
    sleep(1);
    for(i=1; i<height-1; i++)
        for(j=1; j<width-1; j++)
            mvwaddch(board, i, j, ' ');
    wrefresh(board);
    cur_time = get_time();
    return 0;
}

int main(int argc, char *argv[]){
    char loop=false, *s;
    while(--argc>0 && (*++argv)[0]=='-')
        for(s=argv[0]+1; *s!='\0'; s++){
            switch(*s){
                case 'h':
                    printf("Avilible command line arguments:\n");
                    printf("    -h, print this message\n");
                    printf("    -l, loop the game forever\n");
                    printf("    -a, some stupid ai\n");
                    return 0;
                    break;
                case 'a':
                    autoplay = true;
                    break;
                case 'l':
                    loop = true;
                    break;
                default:
                    printf("Invalid option\n");
                    return 1;
            }
        }

    usleep(500000);

    initialize();
    do{
        play_game();
    }while(loop);

    endwin();


    return 0;
}
