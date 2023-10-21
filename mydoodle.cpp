#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
using namespace std;

class World;

const int DIMENSION=20;
const int ANTS=100;
const int ANTBREED=3;
const int BUGS=5;
const int BUGBREED=8;
const int BUGSTARVE=3;

struct coord{ //defining all x,y cartesian coordinates as a coord
    coord():y(0),x(0){}; //note that when using a 2d vector as a grid the y indexing is first -> [y][x]
    coord(int row,int col):y(row),x(col){} //also y is read from the top down
    int x;
    int y;
};

int rgen(int max){return rand()%max;} //random number generator

class Organism{
public:
    Organism():breedspan(0),charof('-'),position(coord()){};
    virtual ~Organism(){} //This isn't really necessary since I delete the object in the die function and there isn't anything else to "clean up"
    //The destructor only does something if you define it, if you have an empty set of brackets {} it does absolutely nothing
    int breedspan;
    char charof;
    coord position;
    coord checksurroundingcells(World& w,char t) const;
    virtual void move(World& w)=0;
    virtual void breed(World& w)=0;
    void die(World& w);
};

class Doodlebug:public Organism{
public:
    int starvespan;
    Doodlebug(coord c){starvespan=1;breedspan=1;charof='X';position=c;}
    void move(World& w) override;
    void breed(World& w) override;
};

class Ant:public Organism{
public:
    Ant(coord c){breedspan=1;charof='O';position=c;}
    void move(World& w) override;
    void breed(World& w) override;
};

class World{
public:
    vector<Ant*> allants;
    vector<Doodlebug*> allbugs;
    Organism* grid[DIMENSION][DIMENSION];
    bool conflict(coord p){return grid[p.y][p.x]!=nullptr;}
    World(){
        for(int i=0;i<DIMENSION;i++){for(int j=0;j<DIMENSION;j++){grid[i][j]=nullptr;}}
        for(int a=0;a<ANTS;a++){
            coord potential;
            do{
                potential.x=rgen(DIMENSION);
                potential.y=rgen(DIMENSION);
            }while(conflict(potential));
            Ant* i=new Ant(potential);
            grid[i->position.y][i->position.x]=i;
        }
        for(int d=0;d<BUGS;d++){
            coord pot;
            do{
                pot.x=rgen(DIMENSION);
                pot.y=rgen(DIMENSION);
            }while(conflict(pot));
            Doodlebug* j=new Doodlebug(pot);
            grid[j->position.y][j->position.x]=j;
        }
    }
    void display(){
        for(int i=0;i<DIMENSION;i++){
            for(int j=0;j<DIMENSION;j++){
                ((grid[i][j]==nullptr)?cout<<'-':cout<<grid[i][j]->charof);
                cout<<" ";
            }
            cout<<endl;
        }
    }
    void update(){
        for(int y=0;y<DIMENSION;y++){
            for(int x=0;x<DIMENSION;x++){
                if(grid[y][x]==nullptr){continue;}
                if(grid[y][x]->charof=='X'){allbugs.push_back((Doodlebug*)grid[y][x]);}
            }
        }
        for(Doodlebug* d:allbugs){
            d->move(*this);
            if(d->breedspan>=BUGBREED){
                d->breedspan=1;
                d->breed(*this);
            }else{d->breedspan++;}
            if(d->starvespan>=BUGSTARVE){d->die(*this);}
            else{d->starvespan++;}
        }
        for(int y=0;y<DIMENSION;y++){
            for(int x=0;x<DIMENSION;x++){
                if(grid[y][x]==nullptr){continue;}
                if(grid[y][x]->charof=='O'){allants.push_back((Ant*)grid[y][x]);}
            }
        }
        for(Organism* a:allants){
            a->move(*this);
            if(a->breedspan>=ANTBREED){
                a->breedspan=1;
                a->breed(*this);
            }else{a->breedspan++;}
        }
        allants.clear();
        allbugs.clear();
    }
};

coord Organism::checksurroundingcells(World& w,char t) const{ //check all valid moves that can be made, and return a random one for the organism to make
    coord j=this->position; //this same function can be used for all 3 cases: ant moving, doodlebug checking for ants, and doodlebug moving
    vector<coord> valid;
    if(t=='-'){
        if(j.y<DIMENSION-1&&w.grid[j.y+1][j.x]==nullptr){valid.emplace_back(coord(j.y+1,j.x));}
        if(j.y>0&&w.grid[j.y-1][j.x]==nullptr){valid.emplace_back(coord(j.y-1,j.x));}
        if(j.x<DIMENSION-1&&w.grid[j.y][j.x+1]==nullptr){valid.emplace_back(coord(j.y,j.x+1));}
        if(j.x>0&&w.grid[j.y][j.x-1]==nullptr){valid.emplace_back(coord(j.y,j.x-1));}
    }else{
        if(j.y<DIMENSION-1&&w.grid[j.y+1][j.x]!=nullptr&&w.grid[j.y+1][j.x]->charof==t){valid.emplace_back(j.y+1,j.x);}
        if(j.y>0&&w.grid[j.y-1][j.x]!=nullptr&&w.grid[j.y-1][j.x]->charof==t){valid.emplace_back(coord(j.y-1,j.x));}
        if(j.x<DIMENSION-1&&w.grid[j.y][j.x+1]!=nullptr&&w.grid[j.y][j.x+1]->charof==t){valid.emplace_back(coord(j.y,j.x+1));}
        if(j.x>0&&w.grid[j.y][j.x-1]!=nullptr&&w.grid[j.y][j.x-1]->charof==t){valid.emplace_back(coord(j.y,j.x-1));}
    }
    if(!valid.empty()){return valid[rgen(valid.size())];}
    else{return j;}
}

void Doodlebug::move(World& w){
    coord moveto=this->checksurroundingcells(w,'O');
    if(moveto.x==this->position.x&&moveto.y==this->position.y){
        moveto=this->checksurroundingcells(w,'-');
        if(moveto.x==this->position.x&&moveto.y==this->position.y){return;}
    }else{
        this->starvespan=1;
        w.grid[moveto.y][moveto.x]->die(w);
    }
    w.grid[moveto.y][moveto.x]=this;
    w.grid[this->position.y][this->position.x]=nullptr;
    this->position.y=moveto.y;
    this->position.x=moveto.x;
}

void Ant::move(World& w){
    coord moveto=this->checksurroundingcells(w,'-');
    if(moveto.x==this->position.x&&moveto.y==this->position.y){return;}
    w.grid[moveto.y][moveto.x]=this;
    w.grid[this->position.y][this->position.x]=nullptr;
    this->position.y=moveto.y;
    this->position.x=moveto.x;
}

void Doodlebug::breed(World& w){
    coord dropon=this->checksurroundingcells(w,'O');
    if(dropon.x==this->position.x&&dropon.y==this->position.y){
        dropon=this->checksurroundingcells(w,'-');
        if(dropon.x==this->position.x&&dropon.y==this->position.y){return;}
    }else{w.grid[dropon.y][dropon.x]->die(w);}
    Doodlebug* birth=new Doodlebug(dropon);
    w.grid[dropon.y][dropon.x]=birth;
}

void Ant::breed(World& w){
    coord dropon=this->checksurroundingcells(w,'-');
    if(dropon.x==this->position.x&&dropon.y==this->position.y){return;}
    Ant* birth=new Ant(dropon);
    w.grid[dropon.y][dropon.x]=birth;
}

void Organism::die(World& w){
    w.grid[this->position.y][this->position.x]=nullptr;
    delete this;
}

int main(){
    srand(time(0));
    int step=0;
    World gameboard;
    do{
        cout<<"World at time = "<<step<<endl;
        gameboard.display();
        gameboard.update();
        cin.get();
    }while(++step);
}