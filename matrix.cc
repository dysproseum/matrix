//matrix - simple matrix animation using ncurses lib
//Copyright (C) 2007  Hean Kuan Ong ( mysurface[at]gmail.com )
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either version 2
//of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

#include<curses.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
#include<unistd.h>
#include<sys/select.h>
#include<sys/time.h>
#include<sys/types.h>

#define LINES 25   // number of text flows
#define SPEED_DIFF 5 // higher speed difference larger
#define SPEED 2 // how large the text jump
#define ASCII_START 40
#define ASCII_END 50

//kbhit, Non-blocking keypress detector, when go keypress, return 1 else always return 0
int kbhit(void)
{
    int i;
    fd_set fds;
    struct timeval tv;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    tv.tv_sec = tv.tv_usec = 0;
    i = select(1, &fds, NULL, NULL, &tv);
    if (i == -1) return 0;
    if (FD_ISSET(STDIN_FILENO, &fds)) return 1;
    return 0; 
}

int Random(int max, int min)
{
    return (rand()%max)+min;
}

int SetTimer(struct timeval &tv, int usec)
{
    gettimeofday(&tv,NULL);
    tv.tv_usec+=usec;
    
    return 1;
}

int CheckTimer(struct timeval &tv, int usec)
{
    struct timeval ctv;
    gettimeofday(&ctv,NULL);

    if( (ctv.tv_usec >= tv.tv_usec) || (ctv.tv_sec > tv.tv_sec) )
    {
        gettimeofday(&tv,NULL);
        tv.tv_usec+=usec;
        return 1;
    }
    else 
        return 0;
}

//**********************************
// matrix string structure.
//**********************************
typedef struct _MSTR
{
    int x;      // it is actually a x pos for writing character on screen
    int y;      // same as above, y pos
    int startx; // the first character x pos
    int starty; // same as above, y pos
    int len;    // string length
    int color;  // 1 light green, 0 dim green
    int speed;  // speed defferences
    int count;  // counter for speed delay
    char text[100]; //text buffer.
}MSTR;

//*****************************************************
// The Random Generator to initialize all flow strings
//*****************************************************
void RandomMSTR( MSTR &mstr, int scrY, int scrX, char txt[100])
{
    int i=0,j=0;
    int rstpos=0;
    int len;
    mstr.x=Random(scrX,0);
    mstr.y=Random(scrY,0);
    mstr.startx=Random(scrX,0);
    mstr.starty=Random(scrY,0);
    mstr.color=Random(2,1)-1;
    mstr.speed=Random(SPEED_DIFF,1);
    mstr.count=mstr.speed;
    mstr.len=Random(scrY,4);
    for (i=0;i<mstr.len;i++)
        mstr.text[i]=Random(ASCII_END,ASCII_START);
    if (txt[0])// process bellow only -t
    {
        //rstpos=Random(mstr.len-1,0);
        len=strlen(txt);
        for (j=rstpos;j<mstr.len;j++)
        {
            if ((len)<=0)
                break;

            mstr.text[j]=*(txt++);
            len--;
        }
    }
    mstr.text[i+1]='\0';
}

//*******************************************
//print a standing line of matrix string and
//update the state
//*******************************************
void PrintMSTR( MSTR &mstr, int scrY, int scrX, char text[100])
{
    if (mstr.color==1)
	attron(COLOR_PAIR(1)| A_BOLD);
    else
	attron(COLOR_PAIR(1)| A_NORMAL);
    
    //eraser, clear the unwanted char
    if(mstr.count==mstr.speed)
    {
        for (int f=0;f<SPEED;f++)
            mvaddch(mstr.y-SPEED+f,mstr.x,' ');
    }
    if ( mstr.y>scrY)
    {
        RandomMSTR( mstr, scrY, scrX, text);
        mstr.y=0-mstr.len;
        mstr.x=Random(scrX,1);
    }

    mstr.starty=mstr.y;
    mstr.startx=mstr.x;
    for (int i=0;i<mstr.len;i++)
    {
        mvprintw(mstr.y,mstr.x,"%c",mstr.text[i]);
        mstr.y++;
    }
	attroff(COLOR_PAIR(1)| A_BOLD);
        if(mstr.count<=0)
        {
            mstr.starty+=SPEED;
            mstr.count=mstr.speed;
        }
        else
            mstr.count--;
        mstr.y=mstr.starty;
}

void PrintVersion()
{
    printf("matrix 0.1a\n");
}

void PrintHelp()
{
    PrintVersion();
    printf("simple matrix animation using ncurses lib\n");
    printf("Usage: matrix [option]... \n");
    printf("       matrix --version\n");
    printf("       matrix --help\n");
    printf("\n");
    printf("       -c [1-7]     Color value from 1 to 7, default is 2\n");
    printf("       -t [string]  Text heading for each flow string\n");
    printf("       -s [1-9]     The flow speed, 1 fastest, 9 slowest\n");
    printf("       -l [1-50]    Default have 25 flow strings, you can add more\n");
    printf("\n");
    printf("       --version    Print out the version\n");
    printf("       --help       Print this page\n");
}

void square ( int y, int x , int leny, int lenx, int color)
{
    for(int a=y;a<(y+leny);a++)
    {
        mvchgat(a, x, lenx, A_STANDOUT, color, NULL);        
    }
}

int main(int argc, char * argv[])
{
    MSTR mstr[100];
    int scrX, scrY;
    int i,j=0;
    struct timeval tv;
    char text[100];
    int delay=3;
    int morelines=0;

    initscr();     //in ncurses
    curs_set(0);   //hide cursor
    noecho();
    start_color(); //start color
    init_pair(1, COLOR_GREEN, COLOR_BLACK);
    init_pair(2, COLOR_BLUE, COLOR_BLACK);
    memset(text,0,sizeof(text));

    //----check for options------
    // I know it is a dump way,
    for (int a=0;a<argc-1;a++)
    {
       if(strncmp(argv[a+1],"-v",2)==0)
        {
            endwin();    // just check version only
            PrintVersion();
            return 0;
        }
       if(strncmp(argv[a+1],"--version",9)==0)
        {
            endwin();    // just check version only
            PrintVersion();
            return 0;
        }
       if(strncmp(argv[a+1],"--help",6)==0)
        {
            endwin();    
            PrintHelp();
            return 0;
        }
       if(strncmp(argv[a+1],"-c",2)==0)
        {
            if (argc>(a+2))
            {
                if (atoi(argv[a+2])>0 && atoi(argv[a+2])<8)
                    init_pair(1, atoi(argv[a+2]), COLOR_BLACK);
            }        
        }
       if(strncmp(argv[a+1],"-l",2)==0)
        {
            if (argc>(a+2))
            {
                if (atoi(argv[a+2])>0 && atoi(argv[a+2])<=50)
                    morelines=atoi(argv[a+2]);
            }        
        }
       if(strncmp(argv[a+1],"-s",2)==0)
       {
           if (argc>(a+2))
           {
               if (atoi(argv[a+2])>0 && atoi(argv[a+2])<10)
                   delay=atoi(argv[a+2]);
           }        
       }
       if(strncmp(argv[a+1],"-t",2)==0)
       {
            if (argc>(a+2))
            {
                if (strlen(argv[a+2])<100)
                    sprintf(text,"%s",argv[a+2]);
                else
                {
                    strncpy(text,argv[a+2],99);
                    text[99]='\0';
                }

            }        
        
       }

    }

    srand(time(NULL));          //randomized
    getmaxyx(stdscr,scrY,scrX); //get screen resolution

    //init matrix
    for (i=0;i<LINES+morelines;i++)
        RandomMSTR(mstr[i],scrY,scrX,text);

    SetTimer(tv,delay*10000); //set up a delay timer

    while(!kbhit()) //will quit if detects key press.
    {
        usleep(1);
        if (CheckTimer(tv,delay*10000))
        {
            getmaxyx(stdscr,scrY,scrX);       //get screen resolution
            for (j=0;j<LINES+morelines;j++)
                PrintMSTR(mstr[j],scrY,scrX,text);
            //testing square
            //square(scrY-5,scrX-10,5,10,1);
            refresh();
        }
    }
    endwin(); // out ncurses
    return 0;
}
