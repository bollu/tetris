#include <iostream>
#include <windows.h>
#include <iomanip>
#include <conio.h>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <ctime>
#include <algorithm>
#include <cstring>
#include <cctype>
#include <sys/timeb.h>




using namespace std;

COORD coord={0,0};
int board_x=2;
int board_y=5;
class board;
class block;
void framework();
board *b;
class mess;
mess *m;
int axis_shift_x=0,axis_shift_y=0;


/*
    1.  Scoring system refinement
    4.  Start game animation
    6.  Hall of fame
    7.  Pause animation
    14. Control refining
    15. Score bonus for dead drops depending upon the distance dropped
    16. Clean up code
    17. Reprint the falling brick and it's ghost after resuming from pause
    18. Massive bonus for completely clearing the board
    19. Animate clearrow() function towards the score
    21. The last losing brick doesnt show in the end-game animation
    22. Animate the last losing brick
    23. Refine placement of upcoming and current
*/


void gotoxy(int x,int y)
{
    x+=axis_shift_x;
    y+=axis_shift_y;
    if(x<0||y<0)
    {
        return;
    }
    coord.X=x;
    coord.Y=y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE),coord);
}

int getTime()
{
    timeb tb;
    ftime(&tb);
    int nCount=tb.millitm+((tb.time&0xfffff)*1000);
    return nCount;
}

int diffTime(int StartTime)
{
    int nSpan=getTime()-StartTime;
    if(nSpan<0)
    {
        nSpan+=0x100000*1000;
    }
    return nSpan;
}

int main();
void start_animation();
void marquee();




class highscore
{
private:
    int score;
    char name[30];
public:
    highscore()
    {
        score=2000;
        strcpy(name,"Anonymous");
    }
    highscore(char n[30],int s)
    {
        strcpy(name,n);
        score=abs(s);
    }
    void set_values(int s,char n[30])
    {
        score=abs(s);
        strcpy(name,n);
    }
    int get_score()
    {
        return score;
    }
};

class hall_of_fame
{
private:
    highscore *h[10];
    hall_of_fame()
    {
        for(int i=0;i<10;i++)
        {
            h[i]=new highscore;
        }
    }
    void arrange()
    {
        for(int i=0;i<10;i++)
        {
            for(int j=i;j<10;j++)
            {
                if(h[j]->get_score()>h[i]->get_score())
                {
                    highscore *temp;
                    temp=h[i];
                    h[i]=h[j];
                    h[j]=temp;
                    delete temp;
                }
            }
        }
    }
    bool is_new_highscore(highscore *n)
    {
        if(n->get_score()>h[9]->get_score())
        {
            return true;
        }
        return false;
    }
    void add_highscore(highscore *n)
    {
        for(int i=0;i<10;i++)
        {
            if(h[i]->get_score()<n->get_score())
            {
                delete h[9];
                for(int j=9;j>i;j--)
                {
                    h[j]=h[j-1];
                }
                h[i]=n;
                return;
            }
        }
    }
};

class symbols
{
    public:
    struct single
    {
        struct corner
        {
            char t_l,t_r,b_l,b_r;
        }c;
        struct horizontal
        {
            char u_b,t_b,b_b;
        }h;
        struct vertical
        {
            char u_b,l_b,r_b;
        } v;
        char centre;
    }s;
    struct twice
    {
        struct corner
        {
            char t_l,t_r,b_l,b_r;
        }c;
        struct horizontal
        {
            char u_b,t_b,b_b;
        }h;
        struct vertical
        {
            char u_b,l_b,r_b;
        } v;
        char centre;
    }d;
    struct mixed
    {
        struct horizontal
        {
            char t_b,b_b,centre;
        }h;
        struct vertical
        {
            char l_b,r_b,centre;
        } v;
    }m;
    struct shade
    {
        struct solid
        {
            char l,r,t,b,centre,full;
        }s;
        struct light
        {
            char l1,l2,l3;
        }l;
    }sh;
    symbols()
    {
        this->m.h.t_b=207;
        this->m.h.b_b=209;
        this->m.h.centre=216;
        this->m.v.l_b=182;
        this->m.v.r_b=199;
        this->m.v.centre=215;

        this->d.centre=206;
        this->d.c.t_l=201;
        this->d.c.t_r=187;
        this->d.c.b_l=200;
        this->d.c.b_r=188;
        this->d.h.u_b=205;
        this->d.h.t_b=202;
        this->d.h.b_b=203;
        this->d.v.u_b=186;
        this->d.v.l_b=185;
        this->d.v.r_b=204;

        this->s.centre=197;
        this->s.c.t_l=218;
        this->s.c.t_r=191;
        this->s.c.b_l=192;
        this->s.c.b_r=217;
        this->s.h.u_b=196;
        this->s.h.t_b=193;
        this->s.h.b_b=194;
        this->s.v.u_b=179;
        this->s.v.l_b=180;
        this->s.v.r_b=195;

        this->sh.s.full=219;
        this->sh.s.b=220;
        this->sh.s.l=221;
        this->sh.s.r=222;
        this->sh.s.t=223;
        this->sh.s.centre=254;

        this->sh.l.l1=178;
        this->sh.l.l2=177;
        this->sh.l.l3=176;
    }
}sym;

struct def
{
    char shape;
    int inc_x[4];
    int inc_y[4];
    int index;
    def *link;
    def()
    {
        this->link=NULL;
    }
}*I,*J,*L,*O,*S,*T,*Z;

class board
{
private:
    class block
    {
        public:
        int x_index,y_index;
        void assign_new()
        {
            int r=rand();
            r%=7;
            switch(r)
            {
            case 0:
                {
                    d=I;
                    break;
                }
            case 1:
                {
                    d=J;
                    break;
                }
            case 2:
                {
                    d=L;
                    break;
                }
            case 3:
                {
                    d=O;
                    break;
                }
            case 4:
                {
                    d=S;
                    break;
                }
            case 5:
                {
                    d=T;
                    break;
                }
            case 6:
                {
                    d=Z;
                    break;
                }
            }
        }
        def *d;
        block()
        {
            d=NULL;
            y_index=0;
            x_index=6;
        }
        block(char x)
        {
            x=toupper(x);
            switch(x)
            {
            case 'I':
                {
                    d=I;
                    break;
                }
            case 'J':
                {
                    d=J;
                    break;
                }
            case 'L':
                {
                    d=L;
                    break;
                }
            case 'O':
                {
                    d=O;
                    break;
                }
            case 'S':
                {
                    d=S;
                    break;
                }
            case 'T':
                {
                    d=T;
                    break;
                }
            case 'Z':
                {
                    d=Z;
                    break;
                }
            case 'A':
                {
                    assign_new();
                }
            }
            y_index=0;
            x_index=6;
        }
        block(block *p)
        {
            x_index=p->x_index;
            y_index=p->y_index;
            d=p->d;
        }
        bool rotate()
        {
            d=d->link;
            if(is_valid())
            {
                return true;
            }
            else
            {
                for(int i=0;i<3;i++)
                {
                    d=d->link;
                }
                return false;
            }
        }
        bool move_up()
        {
            y_index--;
            if(is_valid())
            {
                return true;
            }
            else
            {
                y_index++;
                return false;
            }
        }
        bool move_down()
        {
            y_index++;
            if(is_valid())
            {
                return true;
            }
            else
            {
                y_index--;
                return false;
            }
        }
        bool move_left()
        {
            x_index--;
            if(is_valid())
            {
                return true;
            }
            else
            {
                x_index++;
                return false;
            }
        }
        bool move_right()
        {
            x_index++;
            if(is_valid())
            {
                return true;
            }
            else
            {
                x_index--;
                return false;
            }
        }
        void erase()
        {
            for(int i=0;i<4;i++)
            {
                if((y_index+d->inc_y[i])<0||(y_index+d->inc_y[i])>24)
                {
                    continue;
                }
                gotoxy((board_x+(x_index*3)+(d->inc_x[i]*3)),(board_y+(y_index*2)+(d->inc_y[i]*2)));
                cout<<"   ";
                gotoxy((board_x+(x_index*3)+(d->inc_x[i]*3)),(board_y+(y_index*2)+(d->inc_y[i]*2))+1);
                cout<<"   ";
            }
        }
        void print()
        {
            if(!is_valid())
            {
                return;
            }
            for(int i=0;i<4;i++)
            {
                if((x_index+d->inc_x[i])>24)
                {
                    return;
                }
            }
            for(int i=0;i<4;i++)
            {
                if((y_index+d->inc_y[i])<0||(y_index+d->inc_y[i])>24)
                {
                    continue;
                }
                bool is_above=false,is_below=false,is_left=false,is_right=false;
                for(int j=0;j<4;j++)
                {
                    if(i==j)
                    {
                        continue;
                    }
                    if(d->inc_x[i]==d->inc_x[j])
                    {
                        int val=d->inc_y[i]-d->inc_y[j];
                        if(val==1)
                        {
                            is_above=true;
                        }
                        if(val==-1)
                        {
                            is_below=true;
                        }
                    }
                    if(d->inc_y[i]==d->inc_y[j])
                    {
                        int val=d->inc_x[i]-d->inc_x[j];
                        if(val==1)
                        {
                            is_left=true;
                        }
                        if(val==-1)
                        {
                            is_right=true;
                        }
                    }
                }
                char cube[2][4];
                cube[0][3]='\0';
                cube[1][3]='\0';
                if(is_left)
                {
                    if(is_above)
                    {
                        cube[0][0]=sym.d.c.b_r;
                        cube[0][1]=' ';
                        if(is_right)
                        {
                            cube[0][2]=sym.d.c.b_l;
                        }
                        else
                        {
                            cube[0][2]=sym.d.v.u_b;
                        }
                    }
                    else
                    {
                        cube[0][0]=sym.d.h.u_b;
                        cube[0][1]=sym.d.h.u_b;
                        if(is_right)
                        {
                            cube[0][2]=sym.d.h.u_b;
                        }
                        else
                        {
                            cube[0][2]=sym.d.c.t_r;
                        }
                    }
                    if(is_below)
                    {
                        cube[1][0]=sym.d.c.t_r;
                        cube[1][1]=' ';
                        if(is_right)
                        {
                            cube[1][2]=sym.d.c.t_l;
                        }
                        else
                        {
                            cube[1][2]=sym.d.v.u_b;
                        }
                    }
                    else
                    {
                        cube[1][0]=sym.d.h.u_b;
                        cube[1][1]=sym.d.h.u_b;
                        if(is_right)
                        {
                            cube[1][2]=sym.d.h.u_b;
                        }
                        else
                        {
                            cube[1][2]=sym.d.c.b_r;
                        }
                    }
                }
                else
                {
                    if(is_above)
                    {
                        cube[0][0]=sym.d.v.u_b;
                        cube[0][1]=' ';
                        if(is_right)
                        {
                            cube[0][2]=sym.d.c.b_l;
                        }
                        else
                        {
                            cube[0][2]=sym.d.v.u_b;
                        }
                    }
                    else
                    {
                        cube[0][0]=sym.d.c.t_l;
                        cube[0][1]=sym.d.h.u_b;
                        if(is_right)
                        {
                            cube[0][2]=sym.d.h.u_b;
                        }
                        else
                        {
                            cube[0][2]=sym.d.c.t_r;
                        }
                    }
                    if(is_below)
                    {
                        cube[1][0]=sym.d.v.u_b;
                        cube[1][1]=' ';
                        if(is_right)
                        {
                            cube[1][2]=sym.d.c.t_l;
                        }
                        else
                        {
                            cube[1][2]=sym.d.v.u_b;
                        }
                    }
                    else
                    {
                        cube[1][0]=sym.d.c.b_l;
                        cube[1][1]=sym.d.h.u_b;
                        if(is_right)
                        {
                            cube[1][2]=sym.d.h.u_b;
                        }
                        else
                        {
                            cube[1][2]=sym.d.c.b_r;
                        }
                    }
                }
                gotoxy((board_x+(x_index*3)+(d->inc_x[i]*3)),(board_y+(y_index*2)+(d->inc_y[i]*2)));
                cout<<cube[0];
                gotoxy((board_x+(x_index*3)+(d->inc_x[i]*3)),(board_y+(y_index*2)+(d->inc_y[i]*2)+1));
                cout<<cube[1];
            }
            gotoxy(0,0);
        }
        void print_ghost()
        {
            if(!is_valid())
            {
                return;
            }
            for(int i=0;i<4;i++)
            {
                if((x_index+d->inc_x[i])>24)
                {
                    return;
                }
            }
            for(int i=0;i<4;i++)
            {
                if((y_index+d->inc_y[i])<0||(y_index+d->inc_y[i])>24)
                {
                    continue;
                }
                bool is_above=false,is_below=false,is_left=false,is_right=false;
                for(int j=0;j<4;j++)
                {
                    if(i==j)
                    {
                        continue;
                    }
                    if(d->inc_x[i]==d->inc_x[j])
                    {
                        int val=d->inc_y[i]-d->inc_y[j];
                        if(val==1)
                        {
                            is_above=true;
                        }
                        if(val==-1)
                        {
                            is_below=true;
                        }
                    }
                    if(d->inc_y[i]==d->inc_y[j])
                    {
                        int val=d->inc_x[i]-d->inc_x[j];
                        if(val==1)
                        {
                            is_left=true;
                        }
                        if(val==-1)
                        {
                            is_right=true;
                        }
                    }
                }
                char cube[2][4];
                cube[0][3]='\0';
                cube[1][3]='\0';
                if(is_left)
                {
                    if(is_above)
                    {
                        cube[0][0]=sym.s.c.b_r;
                        cube[0][1]=' ';
                        if(is_right)
                        {
                            cube[0][2]=sym.s.c.b_l;
                        }
                        else
                        {
                            cube[0][2]=sym.s.v.u_b;
                        }
                    }
                    else
                    {
                        cube[0][0]=sym.s.h.u_b;
                        cube[0][1]=sym.s.h.u_b;
                        if(is_right)
                        {
                            cube[0][2]=sym.s.h.u_b;
                        }
                        else
                        {
                            cube[0][2]=sym.s.c.t_r;
                        }
                    }
                    if(is_below)
                    {
                        cube[1][0]=sym.s.c.t_r;
                        cube[1][1]=' ';
                        if(is_right)
                        {
                            cube[1][2]=sym.s.c.t_l;
                        }
                        else
                        {
                            cube[1][2]=sym.s.v.u_b;
                        }
                    }
                    else
                    {
                        cube[1][0]=sym.s.h.u_b;
                        cube[1][1]=sym.s.h.u_b;
                        if(is_right)
                        {
                            cube[1][2]=sym.s.h.u_b;
                        }
                        else
                        {
                            cube[1][2]=sym.s.c.b_r;
                        }
                    }
                }
                else
                {
                    if(is_above)
                    {
                        cube[0][0]=sym.s.v.u_b;
                        cube[0][1]=' ';
                        if(is_right)
                        {
                            cube[0][2]=sym.s.c.b_l;
                        }
                        else
                        {
                            cube[0][2]=sym.s.v.u_b;
                        }
                    }
                    else
                    {
                        cube[0][0]=sym.s.c.t_l;
                        cube[0][1]=sym.s.h.u_b;
                        if(is_right)
                        {
                            cube[0][2]=sym.s.h.u_b;
                        }
                        else
                        {
                            cube[0][2]=sym.s.c.t_r;
                        }
                    }
                    if(is_below)
                    {
                        cube[1][0]=sym.s.v.u_b;
                        cube[1][1]=' ';
                        if(is_right)
                        {
                            cube[1][2]=sym.s.c.t_l;
                        }
                        else
                        {
                            cube[1][2]=sym.s.v.u_b;
                        }
                    }
                    else
                    {
                        cube[1][0]=sym.s.c.b_l;
                        cube[1][1]=sym.s.h.u_b;
                        if(is_right)
                        {
                            cube[1][2]=sym.s.h.u_b;
                        }
                        else
                        {
                            cube[1][2]=sym.s.c.b_r;
                        }
                    }
                }
                gotoxy((board_x+(x_index*3)+(d->inc_x[i]*3)),(board_y+(y_index*2)+(d->inc_y[i]*2)));
                cout<<cube[0];
                gotoxy((board_x+(x_index*3)+(d->inc_x[i]*3)),(board_y+(y_index*2)+(d->inc_y[i]*2)+1));
                cout<<cube[1];
            }
            gotoxy(0,0);
        }
        void print_xy(int x=0,int y=0)
        {
            for(int i=0;i<4;i++)
            {
                bool is_above=false,is_below=false,is_left=false,is_right=false;
                for(int j=0;j<4;j++)
                {
                    if(i==j)
                    {
                        continue;
                    }
                    if(d->inc_x[i]==d->inc_x[j])
                    {
                        int val=d->inc_y[i]-d->inc_y[j];
                        if(val==1)
                        {
                            is_above=true;
                        }
                        if(val==-1)
                        {
                            is_below=true;
                        }
                    }
                    if(d->inc_y[i]==d->inc_y[j])
                    {
                        int val=d->inc_x[i]-d->inc_x[j];
                        if(val==1)
                        {
                            is_left=true;
                        }
                        if(val==-1)
                        {
                            is_right=true;
                        }
                    }
                }
                char cube[2][4];
                cube[0][3]='\0';
                cube[1][3]='\0';
                if(is_left)
                {
                    if(is_above)
                    {
                        cube[0][0]=sym.d.c.b_r;
                        cube[0][1]=' ';
                        if(is_right)
                        {
                            cube[0][2]=sym.d.c.b_l;
                        }
                        else
                        {
                            cube[0][2]=sym.d.v.u_b;
                        }
                    }
                    else
                    {
                        cube[0][0]=sym.d.h.u_b;
                        cube[0][1]=sym.d.h.u_b;
                        if(is_right)
                        {
                            cube[0][2]=sym.d.h.u_b;
                        }
                        else
                        {
                            cube[0][2]=sym.d.c.t_r;
                        }
                    }
                    if(is_below)
                    {
                        cube[1][0]=sym.d.c.t_r;
                        cube[1][1]=' ';
                        if(is_right)
                        {
                            cube[1][2]=sym.d.c.t_l;
                        }
                        else
                        {
                            cube[1][2]=sym.d.v.u_b;
                        }
                    }
                    else
                    {
                        cube[1][0]=sym.d.h.u_b;
                        cube[1][1]=sym.d.h.u_b;
                        if(is_right)
                        {
                            cube[1][2]=sym.d.h.u_b;
                        }
                        else
                        {
                            cube[1][2]=sym.d.c.b_r;
                        }
                    }
                }
                else
                {
                    if(is_above)
                    {
                        cube[0][0]=sym.d.v.u_b;
                        cube[0][1]=' ';
                        if(is_right)
                        {
                            cube[0][2]=sym.d.c.b_l;
                        }
                        else
                        {
                            cube[0][2]=sym.d.v.u_b;
                        }
                    }
                    else
                    {
                        cube[0][0]=sym.d.c.t_l;
                        cube[0][1]=sym.d.h.u_b;
                        if(is_right)
                        {
                            cube[0][2]=sym.d.h.u_b;
                        }
                        else
                        {
                            cube[0][2]=sym.d.c.t_r;
                        }
                    }
                    if(is_below)
                    {
                        cube[1][0]=sym.d.v.u_b;
                        cube[1][1]=' ';
                        if(is_right)
                        {
                            cube[1][2]=sym.d.c.t_l;
                        }
                        else
                        {
                            cube[1][2]=sym.d.v.u_b;
                        }
                    }
                    else
                    {
                        cube[1][0]=sym.d.c.b_l;
                        cube[1][1]=sym.d.h.u_b;
                        if(is_right)
                        {
                            cube[1][2]=sym.d.h.u_b;
                        }
                        else
                        {
                            cube[1][2]=sym.d.c.b_r;
                        }
                    }
                }
                gotoxy((x+(d->inc_x[i]*3)),(y+(d->inc_y[i]*2)));
                cout<<cube[0];
                gotoxy((x+(d->inc_x[i]*3)),(y+(d->inc_y[i]*2)+1));
                cout<<cube[1];
            }
            gotoxy(0,0);
        }
        bool settle()
        {
            if(y_index<0||y_index>24)
            {
                return false;
            }
            if(x_index<0||x_index>14)
            {
                return false;
            }
            for(int i=0;i<4;i++)
            {
                if(b->is_occupied((x_index+d->inc_x[i]),(y_index+d->inc_y[i])))
                {
                    return false;
                }
            }
            for(int i=0;i<4;i++)
            {
                if((y_index+d->inc_y[i])<0||(y_index+d->inc_y[i])>24)
                {
                    continue;
                }
                if((x_index+d->inc_x[i])<0||(x_index+d->inc_x[i])>14)
                {
                    continue;
                }
                gotoxy((board_x+(x_index*3)+(d->inc_x[i]*3)),(board_y+(y_index*2)+(d->inc_y[i]*2)));
                cout<<sym.sh.s.full<<sym.sh.s.full<<sym.sh.s.full;
                gotoxy((board_x+(x_index*3)+(d->inc_x[i]*3)),(board_y+(y_index*2)+(d->inc_y[i]*2)+1));
                cout<<sym.sh.s.full<<sym.sh.s.full<<sym.sh.s.full;
            }
            return true;
        }
        bool is_valid()
        {
            for(int i=0;i<4;i++)
            {
                if((x_index+d->inc_x[i])<0||(x_index+d->inc_x[i])>14)
                {
                    return false;
                }
                if((y_index+d->inc_y[i])>24)
                {
                    return false;
                }
                if(b->is_occupied(int(x_index+d->inc_x[i]),int(y_index+d->inc_y[i])))
                {
                    return false;
                }
            }
            return true;
        }
    }*current=NULL,*upcoming=NULL,*shadow=NULL;
    int score;
    bool is_highscore;
    int rows_cleared;
    int level;
    int level_index[13];
    int level_time[12];
    bool cont;
    bool game_on;
    bool occupied[15][25];
    char screen[57][81];
    void init_pieces()
    {
        def *temp;
        {
            temp=new def;
            I=temp;
            temp->shape='I';
            temp->index=0;
            temp->inc_x[0]=0;
            temp->inc_x[1]=0;
            temp->inc_x[2]=0;
            temp->inc_x[3]=0;
            temp->inc_y[0]=0;
            temp->inc_y[1]=-1;
            temp->inc_y[2]=-2;
            temp->inc_y[3]=-3;
            temp->link=new def;
            temp=temp->link;
            temp->shape='I';
            temp->index=1;
            temp->inc_x[0]=0;
            temp->inc_x[1]=1;
            temp->inc_x[2]=2;
            temp->inc_x[3]=3;
            temp->inc_y[0]=0;
            temp->inc_y[1]=0;
            temp->inc_y[2]=0;
            temp->inc_y[3]=0;
            temp->link=I;
        }
        {
            temp=new def;
            J=temp;
            temp->shape='J';
            temp->index=0;
            temp->inc_x[0]=0;
            temp->inc_x[1]=1;
            temp->inc_x[2]=1;
            temp->inc_x[3]=1;
            temp->inc_y[0]=0;
            temp->inc_y[1]=0;
            temp->inc_y[2]=-1;
            temp->inc_y[3]=-2;
            temp->link=new def;
            temp=temp->link;
            temp->shape='J';
            temp->index=1;
            temp->inc_x[0]=0;
            temp->inc_x[1]=0;
            temp->inc_x[2]=1;
            temp->inc_x[3]=2;
            temp->inc_y[0]=-1;
            temp->inc_y[1]=0;
            temp->inc_y[2]=0;
            temp->inc_y[3]=0;
            temp->link=new def;
            temp=temp->link;
            temp->shape='J';
            temp->index=2;
            temp->inc_x[0]=0;
            temp->inc_x[1]=0;
            temp->inc_x[2]=0;
            temp->inc_x[3]=1;
            temp->inc_y[0]=0;
            temp->inc_y[1]=-1;
            temp->inc_y[2]=-2;
            temp->inc_y[3]=-2;
            temp->link=new def;
            temp=temp->link;
            temp->shape='J';
            temp->index=3;
            temp->inc_x[0]=0;
            temp->inc_x[1]=1;
            temp->inc_x[2]=2;
            temp->inc_x[3]=2;
            temp->inc_y[0]=-1;
            temp->inc_y[1]=-1;
            temp->inc_y[2]=-1;
            temp->inc_y[3]=0;
            temp->link=J;
        }
        {
            temp=new def;
            L=temp;
            temp->shape='L';
            temp->index=0;
            temp->inc_x[0]=0;
            temp->inc_x[1]=0;
            temp->inc_x[2]=0;
            temp->inc_x[3]=1;
            temp->inc_y[0]=0;
            temp->inc_y[1]=-1;
            temp->inc_y[2]=-2;
            temp->inc_y[3]=0;
            temp->link=new def;
            temp=temp->link;
            temp->shape='L';
            temp->index=1;
            temp->inc_x[0]=0;
            temp->inc_x[1]=1;
            temp->inc_x[2]=2;
            temp->inc_x[3]=2;
            temp->inc_y[0]=0;
            temp->inc_y[1]=0;
            temp->inc_y[2]=0;
            temp->inc_y[3]=-1;
            temp->link=new def;
            temp=temp->link;
            temp->shape='L';
            temp->index=2;
            temp->inc_x[0]=0;
            temp->inc_x[1]=1;
            temp->inc_x[2]=1;
            temp->inc_x[3]=1;
            temp->inc_y[0]=-2;
            temp->inc_y[1]=-2;
            temp->inc_y[2]=-1;
            temp->inc_y[3]=0;
            temp->link=new def;
            temp=temp->link;
            temp->shape='L';
            temp->index=3;
            temp->inc_x[0]=0;
            temp->inc_x[1]=0;
            temp->inc_x[2]=1;
            temp->inc_x[3]=2;
            temp->inc_y[0]=0;
            temp->inc_y[1]=-1;
            temp->inc_y[2]=-1;
            temp->inc_y[3]=-1;
            temp->link=L;
        }
        {
            temp=new def;
            O=temp;
            temp->shape='O';
            temp->index=0;
            temp->inc_x[0]=0;
            temp->inc_x[1]=1;
            temp->inc_x[2]=0;
            temp->inc_x[3]=1;
            temp->inc_y[0]=0;
            temp->inc_y[1]=0;
            temp->inc_y[2]=-1;
            temp->inc_y[3]=-1;
            temp->link=O;
        }
        {
            temp=new def;
            S=temp;
            temp->shape='S';
            temp->index=0;
            temp->inc_x[0]=0;
            temp->inc_x[1]=0;
            temp->inc_x[2]=1;
            temp->inc_x[3]=1;
            temp->inc_y[0]=-2;
            temp->inc_y[1]=-1;
            temp->inc_y[2]=-1;
            temp->inc_y[3]=0;
            temp->link=new def;
            temp=temp->link;
            temp->shape='S';
            temp->index=1;
            temp->inc_x[0]=0;
            temp->inc_x[1]=1;
            temp->inc_x[2]=1;
            temp->inc_x[3]=2;
            temp->inc_y[0]=0;
            temp->inc_y[1]=0;
            temp->inc_y[2]=-1;
            temp->inc_y[3]=-1;
            temp->link=S;
        }
        {
            temp=new def;
            T=temp;
            temp->shape='T';
            temp->index=0;
            temp->inc_x[0]=0;
            temp->inc_x[1]=1;
            temp->inc_x[2]=2;
            temp->inc_x[3]=1;
            temp->inc_y[0]=-1;
            temp->inc_y[1]=-1;
            temp->inc_y[2]=-1;
            temp->inc_y[3]=0;
            temp->link=new def;
            temp=temp->link;
            temp->shape='T';
            temp->index=1;
            temp->inc_x[0]=0;
            temp->inc_x[1]=1;
            temp->inc_x[2]=1;
            temp->inc_x[3]=1;
            temp->inc_y[0]=-1;
            temp->inc_y[1]=0;
            temp->inc_y[2]=-1;
            temp->inc_y[3]=-2;
            temp->link=new def;
            temp=temp->link;
            temp->shape='T';
            temp->index=2;
            temp->inc_x[0]=1;
            temp->inc_x[1]=0;
            temp->inc_x[2]=1;
            temp->inc_x[3]=2;
            temp->inc_y[0]=-1;
            temp->inc_y[1]=0;
            temp->inc_y[2]=0;
            temp->inc_y[3]=0;
            temp->link=new def;
            temp=temp->link;
            temp->shape='T';
            temp->index=3;
            temp->inc_x[0]=0;
            temp->inc_x[1]=0;
            temp->inc_x[2]=0;
            temp->inc_x[3]=1;
            temp->inc_y[0]=0;
            temp->inc_y[1]=-1;
            temp->inc_y[2]=-2;
            temp->inc_y[3]=-1;
            temp->link=T;
        }
        {
            temp=new def;
            Z=temp;
            temp->shape='Z';
            temp->index=0;
            temp->inc_x[0]=0;
            temp->inc_x[1]=0;
            temp->inc_x[2]=1;
            temp->inc_x[3]=1;
            temp->inc_y[0]=-0;
            temp->inc_y[1]=-1;
            temp->inc_y[2]=-1;
            temp->inc_y[3]=-2;
            temp->link=new def;
            temp=temp->link;
            temp->shape='Z';
            temp->index=1;
            temp->inc_x[0]=0;
            temp->inc_x[1]=1;
            temp->inc_x[2]=1;
            temp->inc_x[3]=2;
            temp->inc_y[0]=-1;
            temp->inc_y[1]=-1;
            temp->inc_y[2]=0;
            temp->inc_y[3]=0;
            temp->link=Z;
        }
    }
    void clear_rows()
    {
        bool to_be_cleared[25];
        for(int i=0;i<25;i++)
        {
            to_be_cleared[i]=true;
            for(int j=0;j<15;j++)
            {
                if(!occupied[j][i])
                {
                    to_be_cleared[i]=false;
                    break;
                }
            }
        }
        for(int i=24;i>=0;i--)
        {
            if(to_be_cleared[i])
            {
                for(int j=0;j<33;j++)
                {
                    int l1=j-5;
                    int l2=l1-5;
                    int l3=l2-5;
                    int l4=l3-5;
                    int l5=l4-5;
                    if(j<8)
                    {
                        gotoxy((board_x+21+(3*j)),(board_y+(2*i)));
                        cout<<sym.d.c.t_l<<sym.d.h.u_b<<sym.d.c.t_r;
                        gotoxy((board_x+21+(3*j)),(board_y+(2*i)+1));
                        cout<<sym.d.c.b_l<<sym.d.h.u_b<<sym.d.c.b_r;
                        gotoxy((board_x+21-(3*j)),(board_y+(2*i)));
                        cout<<sym.d.c.t_l<<sym.d.h.u_b<<sym.d.c.t_r;
                        gotoxy((board_x+21-(3*j)),(board_y+(2*i)+1));
                        cout<<sym.d.c.b_l<<sym.d.h.u_b<<sym.d.c.b_r;
                    }
                    if(l1>=0&&l1<8)
                    {
                        gotoxy((board_x+21+(3*l1)),(board_y+(2*i)));
                        cout<<sym.s.c.t_l<<sym.s.h.u_b<<sym.s.c.t_r;
                        gotoxy((board_x+21+(3*l1)),(board_y+(2*i)+1));
                        cout<<sym.s.c.b_l<<sym.s.h.u_b<<sym.s.c.b_r;
                        gotoxy((board_x+21-(3*l1)),(board_y+(2*i)));
                        cout<<sym.s.c.t_l<<sym.s.h.u_b<<sym.s.c.t_r;
                        gotoxy((board_x+21-(3*l1)),(board_y+(2*i)+1));
                        cout<<sym.s.c.b_l<<sym.s.h.u_b<<sym.s.c.b_r;
                    }
                    if(l2>=0&&l2<8)
                    {
                        gotoxy((board_x+21+(3*l2)),(board_y+(2*i)));
                        cout<<sym.sh.l.l3<<sym.sh.l.l3<<sym.sh.l.l3;
                        gotoxy((board_x+21+(3*l2)),(board_y+(2*i)+1));
                        cout<<sym.sh.l.l3<<sym.sh.l.l3<<sym.sh.l.l3;
                        gotoxy((board_x+21-(3*l2)),(board_y+(2*i)));
                        cout<<sym.sh.l.l3<<sym.sh.l.l3<<sym.sh.l.l3;
                        gotoxy((board_x+21-(3*l2)),(board_y+(2*i)+1));
                        cout<<sym.sh.l.l3<<sym.sh.l.l3<<sym.sh.l.l3;
                    }
                    if(l3>=0&&l3<8)
                    {
                        gotoxy((board_x+21+(3*l3)),(board_y+(2*i)));
                        cout<<sym.sh.l.l1<<sym.sh.l.l1<<sym.sh.l.l1;
                        gotoxy((board_x+21+(3*l3)),(board_y+(2*i)+1));
                        cout<<sym.sh.l.l1<<sym.sh.l.l1<<sym.sh.l.l1;
                        gotoxy((board_x+21-(3*l3)),(board_y+(2*i)));
                        cout<<sym.sh.l.l1<<sym.sh.l.l1<<sym.sh.l.l1;
                        gotoxy((board_x+21-(3*l3)),(board_y+(2*i)+1));
                        cout<<sym.sh.l.l1<<sym.sh.l.l1<<sym.sh.l.l1;
                    }
                    if(l4>=0&&l4<8)
                    {
                        gotoxy((board_x+21+(3*l4)),(board_y+(2*i)));
                        cout<<sym.sh.s.full<<sym.sh.s.full<<sym.sh.s.full;
                        gotoxy((board_x+21+(3*l4)),(board_y+(2*i)+1));
                        cout<<sym.sh.s.full<<sym.sh.s.full<<sym.sh.s.full;
                        gotoxy((board_x+21-(3*l4)),(board_y+(2*i)));
                        cout<<sym.sh.s.full<<sym.sh.s.full<<sym.sh.s.full;
                        gotoxy((board_x+21-(3*l4)),(board_y+(2*i)+1));
                        cout<<sym.sh.s.full<<sym.sh.s.full<<sym.sh.s.full;
                    }
                    if(l5>=0&&l5<8)
                    {
                        gotoxy((board_x+21+(3*l5)),(board_y+(2*i)));
                        cout<<"   ";
                        gotoxy((board_x+21+(3*l5)),(board_y+(2*i)+1));
                        cout<<"   ";
                        gotoxy((board_x+21-(3*l5)),(board_y+(2*i)));
                        cout<<"   ";
                        gotoxy((board_x+21-(3*l5)),(board_y+(2*i)+1));
                        cout<<"   ";
                    }
                    Sleep(32);
                }
                for(int k=0;k<15;k++)
                {
                    occupied[k][i]=false;
                }
                rows_cleared++;
                score+=level*10;
                for(int j=11;j>=0;j--)
                {
                    if(rows_cleared>level_index[j])
                        {
                            level=(j+1);
                            break;
                        }
                }
                info();
                for(int j=i;j>0;j--)
                {
                    to_be_cleared[j]=to_be_cleared[(j-1)];
                    for(int k=0;k<15;k++)
                    {
                        if(occupied[k][j-1])
                        {
                            gotoxy((board_x+(3*k)),(board_y+(2*(j-1))));
                            cout<<"   ";
                            gotoxy((board_x+(3*k)),(board_y+(2*(j-1))+1));
                            cout<<"   ";
                            gotoxy((board_x+(3*k)),(board_y+(2*j)));
                            cout<<sym.sh.s.full<<sym.sh.s.full<<sym.sh.s.full;
                            gotoxy((board_x+(3*k)),(board_y+(2*j)+1));
                            cout<<sym.sh.s.full<<sym.sh.s.full<<sym.sh.s.full;
                            occupied[k][j]=true;
                            occupied[k][(j-1)]=false;
                        }
                    }
                    for(int k=0;k<15;k++)
                    {
                        if(occupied[k][j])
                        {
                            Sleep(120);
                            break;
                        }
                    }
                }
                if(to_be_cleared[i])
                {
                    i++;
                }
                to_be_cleared[0]=false;
                for(int j=0;j<15;j++)
                {
                    occupied[j][0]=false;
                }
            }
        }
        while(_kbhit())
        {
            _getch();
        }
    }
    void info()
    {
        for(int i=0;i<14;i++)
        {
            gotoxy(57,6+i);
            for(int j=0;j<20;j++)
            {
                cout<<' ';
            }
            gotoxy(57,24+i);
            for(int j=0;j<20;j++)
            {
                cout<<' ';
            }
        }
        int x_range=0,y_range=0;
        int dummy_x[4],dummy_y[4];
        for(int i=0;i<4;i++)
        {
            dummy_x[i]=abs(current->d->inc_x[i]);
            dummy_y[i]=abs(current->d->inc_y[i]);
        }
        sort(&dummy_x[0],&dummy_x[3]);
        x_range=dummy_x[3]-dummy_x[0];
        x_range++;
        sort(&dummy_y[0],&dummy_y[3]);
        y_range=dummy_y[3]-dummy_y[0];
        y_range--;
        for(int i=0;i<current->d->index;i++)
        {
            x_range+=y_range;
            y_range=x_range-y_range;
            x_range-=y_range;
            y_range*=-1;
        }
        current->print_xy((66-((x_range*3)/2)),(12-(y_range)));
        for(int i=0;i<4;i++)
        {
            dummy_x[i]=upcoming->d->inc_x[i];
            dummy_y[i]=upcoming->d->inc_y[i];
        }
        sort(&dummy_x[0],&dummy_x[3]);
        x_range=dummy_x[3]-dummy_x[0];
        sort(&dummy_y[0],&dummy_y[3]);
        y_range=dummy_y[3]-dummy_y[0];
        for(int i=0;i<upcoming->d->index;i++)
        {
            x_range+=y_range;
            y_range=x_range-y_range;
            x_range-=y_range;
            y_range*=-1;
        }
        upcoming->print_xy((67-((abs(dummy_x[3])-(abs(x_range)/2))*3)),(29+((abs(dummy_y[0])-(abs(y_range)/2))*2)+((y_range+1)%2)));
        gotoxy(65,43);
        cout<<(int)score;
        gotoxy(65,47);
        for(int i=0;i<level&&i<12;i++)
        {
            cout<<sym.sh.s.centre;
        }
        for(int i=level;i<12;i++)
        {
            cout<<'+';
        }
    }
    void input(char *x)
    {
        *x=(int)'P';
        if(_kbhit())
        {
            *x=_getch();
            if(*x==0||*x==224)
            {
                *x=256+_getch();
            }
        }
        while(_kbhit())
        {
            _getch();
        }
        fflush(stdin);
        fflush(stdout);
    }
    void game_over()
    {
        confetti *t[24][5];
        for(int i=0;i<24;i++)
        {
            for(int j=0;j<5;j++)
            {
                t[i][j]=new confetti;
            }
        }
        for(int i=0;;i++)
        {
            for(int j=0;j<5;j++)
            {
                t[i%24][j]->visible=true;
                t[i%24][j]->erase_rain();
                t[i%24][j]->set_values(rand()%45,0);
                t[i%24][j]->print_rain();
            }
            Sleep(20);
            for(int j=0;j<24;j++)
            {
                for(int k=0;k<5;k++)
                {
                    if(t[j][k]->visible)
                    {
                        if(!t[j][k]->move_down())
                        {
                            t[j][k]->visible=false;
                        }
                    }
                }
            }
            if(_kbhit())
            {
                game_on=false;
                return;
            }
        }
    }
    void shadow_drop()
    {
        shadow->erase();
        shadow->x_index=current->x_index;
        shadow->y_index=current->y_index;
        shadow->d=current->d;
        while(shadow->move_down());
        shadow->print_ghost();
    }
public:
    void occupy(int x,int y)
    {
        if(x<0||y<0||x>14||y>24)
        {
            return;
        }
        occupied[x][y]=true;
    }
    void play()
    {
        info();
        int index_time=getTime();
        int flasher_time=index_time;
        bool flasher_on=false;
        level=0;
        score=0;
        rows_cleared=0;
        bool changed=true;
        float delay;
        char x;
        game_on=true;
        bool down=false;
        shadow_drop();
        do
        {
            if(is_highscore)
            {
                if(diffTime(flasher_time)>200)
                {
                    if(flasher_on)
                    {
                        gotoxy(5,0);
                        cout<<"!!!NEW HIGHSCORE!!!";
                        gotoxy(56,0);
                        cout<<"!!!NEW HIGHSCORE!!!";
                    }
                    else
                    {

                        gotoxy(5,0);
                        cout<<setfill(' ')<<setw(19)<<' ';
                        gotoxy(56,0);
                        cout<<setfill(' ')<<setw(19)<<' ';
                    }
                }
            }
            delay=level_time[level];
            input(&x);
            if(x!='P')
            {
                if(isalpha((char)x))
                {
                    x=int(toupper(char(x)));
                }
                switch(x)
                {
                    changed=true;
                case 'A':
                    {
                        current->erase();
                        if(current->move_left())
                        {
                            shadow_drop();
                        }
                        break;
                    }
                case 'D':
                    {
                        current->erase();
                        if(current->move_right())
                        {
                            shadow_drop();
                        }
                        break;
                    }
                case 'W':
                    {
                        current->erase();
                        if(current->rotate())
                        {
                            shadow_drop();
                        }
                        break;
                    }
                case 'S':
                    {
                        current->erase();
                        if(!current->move_down())
                        {
                            down=true;
                        }
                        break;
                    }
                case 'X':
                    {
                        current->erase();
                        while(current->move_down());
                        down=true;
                        break;
                    }
                case 'P':
                    {
                        for(int i=0;i<5;i++)
                        {
                            Sleep(40);
                            for(int j=((-1)*i);j<=i;j++)
                            {
                                for(int k=((-1)*(i+3));k<=(i+3);k++)
                                {
                                    if((abs(j)!=i)&&(abs(k)!=(i+3)))
                                    {
                                        continue;
                                    }
                                    gotoxy(board_x+21+(k*3),board_y+24+(j*2));
                                    cout<<sym.sh.l.l2<<sym.sh.l.l2<<sym.sh.l.l2;
                                    gotoxy(board_x+21+(k*3),board_y+24+1+(j*2));
                                    cout<<sym.sh.l.l2<<sym.sh.l.l2<<sym.sh.l.l2;
                                }
                            }
                            for(int j=((-1)*(i-1));j<=(i-1);j++)
                            {
                                for(int k=((-1)*(i+2));k<=(i+2);k++)
                                {
                                    if((abs(j)!=(i-1))&&(abs(k)!=(i+2)))
                                    {
                                        continue;
                                    }
                                    gotoxy(board_x+21+(k*3),board_y+24+(j*2));
                                    cout<<"   ";
                                    gotoxy(board_x+21+(k*3),board_y+24+1+(j*2));
                                    cout<<"   ";
                                    }
                            }
                        }
                        gotoxy((board_x+20),(board_y+25));
                        cout<<"PAUSED";
                        gotoxy((board_x+13),(board_y+26));
                        cout<<"Press P to continue!";
                        while(toupper(char(_getch()))!='P');
                        for(int j=((-1)*(5-1));j<=(5-1);j++)
                        {
                            for(int k=((-1)*(5+2));k<=(5+2);k++)
                            {
                                if((abs(j)!=(5-1))&&(abs(k)!=(5+2)))
                                {
                                    continue;
                                }
                                gotoxy(board_x+21+(k*3),board_y+24+(j*2));
                                cout<<"   ";
                                gotoxy(board_x+21+(k*3),board_y+24+1+(j*2));
                                cout<<"   ";
                                }
                        }
                        gotoxy((board_x+20),(board_y+25));
                        cout<<setfill(' ')<<setw(6)<<' ';
                        gotoxy((board_x+13),(board_y+26));
                        cout<<setw(20)<<' ';
                        for(int i=0;i<5;i++)
                        {
                            Sleep(40);
                            for(int j=((-1)*i);j<=i;j++)
                            {
                                for(int k=((-1)*(i+3));k<=(i+3);k++)
                                {
                                    if((abs(j)!=i)&&(abs(k)!=(i+3)))
                                    {
                                        continue;
                                    }
                                    if(b->is_occupied((7+k),(12+j)))
                                    {
                                        gotoxy(board_x+21+(k*3),board_y+24+(j*2));
                                        cout<<sym.sh.s.full<<sym.sh.s.full<<sym.sh.s.full;
                                        gotoxy(board_x+21+(k*3),board_y+24+1+(j*2));
                                        cout<<sym.sh.s.full<<sym.sh.s.full<<sym.sh.s.full;
                                    }
                                }
                            }
                        }
                        break;
                    }
                }
                current->print();
            }
            if((diffTime(index_time)>delay)||(down))
            {
                down=false;
                current->erase();
                index_time=getTime();
                if(!current->move_down())
                {
                    if(!current->settle())
                    {
                        //current->print_xy(board_x+current->x_index,board_x+current->y_index);
                        //getch();
                        current->settle();
                        game_over();
                    }
                    else
                    {
                        gotoxy(0,0);
                        for(int i=0;i<4;i++)
                        {
                            occupied[current->x_index+current->d->inc_x[i]][current->y_index+current->d->inc_y[i]]=true;
                        }
                        clear_rows();
                        index_time=getTime();
                        info();
                        delete shadow;
                        delete current;
                        current=upcoming;
                        upcoming=new block('A');
                        shadow=new block(current->d->shape);
                        shadow_drop();
                        info();
                    }
                }
                current->print();
            }
            if(x!='S')
            {
                Sleep(50);
            }
            else
            {
                Sleep(15);
            }
        }
        while(true);
    }
    board()
    {
        system("cls");
        fflush(stdin);
        fflush(stdout);
        framework();
        start_animation();
        init_pieces();
        current=new block('A');
        upcoming=new block('A');
        shadow=new block(current->d->shape);
        for(int i=0;i<12;i++)
        {
            level_index[i]=(4+(pow(i,1.9)*1.5));
            level_time[i+1]=(700-4*level_index[i]);
        }
        level_time[0]=720;
        for(int i=0;i<15;i++)
        {
            for(int j=0;j<25;j++)
            {
                occupied[i][j]=false;
            }
        }
        for(int i=0;i<57;i++)
        {
            for(int j=0;j<80;j++)
            {
                screen[i][j]=' ';
            }
            screen[i][80]='\0';
        }
    }
    bool is_occupied(int x,int y)
    {
        if(x<0||x>14||y<0||y>24)
        {
            return false;
        }
        return occupied[x][y];
    }
    class confetti
    {
    private:
        int x,y;
        char out;
    public:
        bool visible;
        confetti()
        {
            visible=false;
            out='*';
            x=0;
            y=0;
        }
        void set_values(int xx,int yy)
        {
            x=xx;
            y=yy;
        }
        void erase_rain()
        {
            if(!b->is_occupied((x/3),y))
            {
               gotoxy(board_x+x,board_y+(y*2));
               cout<<' ';
            }
        }
        void print_rain()
        {
            if(!b->is_occupied((x/3),y))
            {
               gotoxy(board_x+x,board_y+(y*2));
               cout<<'\'';
            }
        }
        bool move_down()
        {
            erase_rain();
            y++;
            if(b->is_occupied((x/3),y))
            {
                y--;
                return false;
            }
            else
            {
                print_rain();
                return true;
            }
        }
        bool progress()
        {
            return move_down();
        }
    };
};



void start_animation()
{
    for(int i=0;i<1000;i++)
    {
        for(int j=0;j<15;j++)
        {
            for(int k=0;k<25;k++)
            {
                if(((j+k)%2)!=0)
                {
                    continue;
                }
                gotoxy((board_x+(j*3)),(board_y+(k*2)));
                if(i%2==0)
                {
                    if((j%4)==(k%4))
                    {
                        cout<<sym.sh.s.full<<sym.sh.s.full<<sym.sh.s.full;
                    }
                    else
                    {
                        cout<<sym.sh.l.l3<<sym.sh.l.l3<<sym.sh.l.l3;
                    }
                }
                else
                {
                    if((j%4)==(k%4))
                    {
                        cout<<sym.sh.l.l3<<sym.sh.l.l3<<sym.sh.l.l3;
                    }
                    else
                    {
                        cout<<sym.sh.s.full<<sym.sh.s.full<<sym.sh.s.full;
                    }
                }
                gotoxy((board_x+(j*3)),(board_y+(k*2)+1));
                if(i%2==0)
                {
                    if((j%4)==(k%4))
                    {
                        cout<<sym.sh.s.full<<sym.sh.s.full<<sym.sh.s.full;
                    }
                    else
                    {
                        cout<<sym.sh.l.l3<<sym.sh.l.l3<<sym.sh.l.l3;
                    }
                }
                else
                {
                    if((j%4)==(k%4))
                    {
                        cout<<sym.sh.l.l3<<sym.sh.l.l3<<sym.sh.l.l3;
                    }
                    else
                    {
                        cout<<sym.sh.s.full<<sym.sh.s.full<<sym.sh.s.full;
                    }
                }
            }
        }
        if(_kbhit())
        {
            while(_kbhit())
            {
                _getch();
            }
            marquee();
            break;
        }
    }
    for(int i=0;i<25;i++)
    {
        gotoxy(board_x,(board_y+(i*2)));
        cout<<setfill(' ')<<right<<setw(45)<<' ';
        gotoxy(board_x,(board_y+(i*2)+1));
        cout<<setfill(' ')<<right<<setw(45)<<' ';
    }
}

void marquee()
{
    bool banner[5][40];
    for(int i=0;i<5;i++)
    {
        for(int j=0;j<40;j++)
        {
            banner[i][j]=false;
        }
    }
    for(int i=0;i<5;i++)
    {
        //T
        banner[i][2]=true;
        banner[0][i]=true;
        //E
        banner[i][6]=true;
        banner[0][min(6+i,9)]=true;
        banner[2][min(6+i,9)]=true;
        banner[4][min(6+i,9)]=true;
        //T
        banner[i][13]=true;
        banner[0][i+11]=true;
        //R
        banner[i][17]=true;
        banner[0][17+i]=true;
        banner[2][17+i]=true;
        banner[min(i+1,4)][17+i]=true;
        //I
        banner[0][23+i]=true;
        banner[i][25]=true;
        banner[4][23+i]=true;
        //S
        banner[0][29+i]=true;
        banner[2][29+i]=true;
        banner[4][29+i]=true;
    }
    banner[1][21]=true;
    banner[1][29]=true;
    banner[3][33]=true;
    char b[5][15][2][4];
    for(int i=0;i<1000;i++)
    {
        for(int j=0;j<5;j++)
        {
            for(int k=0;k<15;k++)
            {
                if(banner[j][(i+k)%40])
                {
                    for(int l=0;l<3;l++)
                    {
                        b[j][k][0][l]=sym.sh.s.full;
                        b[j][k][1][l]=sym.sh.s.full;
                    }
                    b[j][k][0][3]='\0';
                    b[j][k][1][3]='\0';
                }
                else
                {
                    strcpy(b[j][k][0],"   ");
                    strcpy(b[j][k][1],"   ");
                }
            }
        }
        for(int j=0;j<5;j++)
        {
            gotoxy(board_x,board_y+2*(10+j));
            for(int k=0;k<15;k++)
            {
                cout<<b[j][k][0];
            }
            gotoxy(board_x,board_y+2*(10+j)+1);
            for(int k=0;k<15;k++)
            {
                cout<<b[j][k][1];
            }
        }
        if(_kbhit())
        {
            while(_kbhit())
            {
                _getch();
            }
            break;
        }
        Sleep(40);
    }
}

void framework()
{
    gotoxy(30,0);
    cout<<sym.sh.s.b<<sym.sh.s.b<<sym.sh.s.b<<sym.sh.s.b<<"    "<<(char)196<<(char)210<<(char)196<<' '<<(char)222<<(char)254<<" "<<(char)229<<"  "<<(char)214<<(char)196;
    gotoxy(31,1);
    cout<<sym.sh.s.r<<sym.sh.s.l<<"  "<<(char)222<<(char)240<<"  "<<(char)186<<"  "<<(char)222<<"\\ "<<(char)186<<' '<<(char)196<<(char)189;
    gotoxy(0,2);
    cout<<setfill(sym.sh.s.centre)<<setw(80)<<sym.sh.s.centre;
    gotoxy(0,3);
    cout<<sym.s.c.t_l<<setfill(sym.s.h.u_b)<<setw(48)<<sym.s.c.t_r<<"  "<<sym.d.v.u_b<<sym.sh.l.l1<<sym.d.v.u_b;
    gotoxy(0,4);
    cout<<sym.s.v.u_b<<setfill('*')<<setw(47)<<'*'<<sym.s.v.u_b<<"  "<<sym.d.v.u_b<<sym.sh.l.l1<<sym.d.v.u_b;
    for(int i=0;i<50;i++)
    {
        gotoxy(0,5+i);
        cout<<sym.s.v.u_b<<'*';
        gotoxy(47,5+i);
        cout<<'*'<<sym.s.v.u_b<<"  "<<sym.d.v.u_b<<sym.sh.l.l1<<sym.d.v.u_b;
    }
    gotoxy(0,55);
    cout<<sym.s.v.u_b<<setfill('*')<<setw(47)<<'*'<<sym.s.v.u_b<<"  "<<sym.d.v.u_b<<sym.sh.l.l1<<sym.d.v.u_b;
    gotoxy(0,56);
    cout<<sym.s.c.b_l<<setfill(sym.s.h.u_b)<<setw(48)<<sym.s.c.b_r<<"  "<<sym.d.v.u_b<<sym.sh.l.l1<<sym.d.v.u_b;
    int col_x=56,col_y=4;
    gotoxy(col_x,col_y);
    cout<<setfill(' ')<<setw(15)<<"CURRENT";
    gotoxy(col_x,col_y+1);
    cout<<sym.sh.s.full<<setfill(sym.sh.s.t)<<setw(21)<<sym.sh.s.full;
    for(int i=0;i<14;i++)
    {
        gotoxy(col_x,col_y+2+i);
        cout<<sym.sh.s.l<<setfill(' ')<<setw(21)<<sym.sh.s.r;
    }
    gotoxy(col_x,col_y+16);
    cout<<sym.sh.s.full<<setfill(sym.sh.s.b)<<setw(21)<<sym.sh.s.full;
    col_y+=18;
    gotoxy(col_x,col_y);
    cout<<setfill(' ')<<setw(15)<<"UPCOMING";
    gotoxy(col_x,col_y+1);
    cout<<sym.sh.s.full<<setfill(sym.sh.s.b)<<setw(21)<<sym.sh.s.full;
    for(int i=0;i<14;i++)
    {
        gotoxy(col_x,col_y+2+i);
        cout<<sym.sh.s.r<<setfill(' ')<<setw(21)<<sym.sh.s.l;
    }
    gotoxy(col_x,col_y+16);
    cout<<sym.sh.s.full<<setfill(sym.sh.s.t)<<setw(21)<<sym.sh.s.full;
    col_y+=20;
    gotoxy(col_x,col_y);
    cout<<sym.s.centre<<setfill(sym.s.h.t_b)<<setw(8)<<sym.s.centre<<setw(13)<<sym.s.centre;
    gotoxy(col_x,col_y+1);
    cout<<sym.s.v.l_b<<setfill(' ')<<" SCORE "<<sym.s.v.u_b<<setw(13)<<sym.s.v.r_b;
    gotoxy(col_x,col_y+2);
    cout<<sym.s.centre<<setfill(sym.s.h.b_b)<<setw(8)<<sym.s.centre<<setw(13)<<sym.s.centre;
    col_y+=4;
    gotoxy(col_x,col_y);
    cout<<sym.s.centre<<setfill(sym.s.h.t_b)<<setw(8)<<sym.s.centre<<setw(13)<<sym.s.centre;
    gotoxy(col_x,col_y+1);
    cout<<sym.s.v.l_b<<setfill(' ')<<" LEVEL "<<sym.s.v.u_b<<setw(13)<<sym.s.v.r_b;
    gotoxy(col_x,col_y+2);
    cout<<sym.s.centre<<setfill(sym.s.h.b_b)<<setw(8)<<sym.s.centre<<setw(13)<<sym.s.centre;
    gotoxy(57,52);
    cout<<"Developed by-";
    gotoxy(57,53);
    cout<<"    Saksham Khanna";
}

void initialise_screen()
{
    system("cls");
    cout<<"1) For the best experience, maximise the console window"<<endl;
    cout<<"2) Use w/a/s/d for navigation\n\nPress any key to continue!";
    _getch();
}

bool login()
{
   return false;
}

int main()
{
    new highscore;
    for(int i=0;i<10;i++)
    {
        initialise_screen();
        srand(time(NULL));
        system("cls");
        b=new board;
        fflush(stdin);
        fflush(stdout);
        b->play();
        delete b;
        b=NULL;
        _getch();
    }
    return EXIT_SUCCESS;
}
