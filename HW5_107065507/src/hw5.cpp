#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <string>
#include <queue>
#include <climits>
#include <cstring>
#include <algorithm>

using namespace std;

#define MODE1 1
#define MODE2 0

int Width, Height;
int capacity_v, capacity_h;
int netNum;

// queue node used in BFS
struct Node
{
    // (x, y) represents matrix cell coordinates
    // dist represent its minimum distance from the source
    int x, y, dist;
};

// Below arrays details all 4 possible movements from a cell
int row[] = { -1, 0, 0, 1 };
int col[] = { 0, -1, 1, 0 };

struct Tile
{
    //int x, y;
    int path = -1;
    bool isVisited = false;
    
    int cap_1;
    int cap_2;
    int cap_3;
    int cap_4;
};

struct Route
{
    int x, y;
    
};

struct Net
{
    int id;
    int pinNum;
    
    int x1, y1;  //s
    int x2, y2;  //t
    
    int wl;  // wirelength
    
    vector<Route> routeList;
    
};

vector<Net> netList;
vector<vector<Tile>> maze;

void input(string file)
{
    FILE *f;
    f = fopen(file.c_str(), "r");
    
    fscanf(f, "grid %d %d\n", &Width, &Height);
    fscanf(f, "vertical capacity %d\n", &capacity_v);
    fscanf(f, "horizontal capacity %d\n", &capacity_h);
    fscanf(f, "num net %d\n", &netNum);
    
    int id, pinNum, x1, x2, y1, y2;
    char netname[20];
    for(int i=0; i<netNum; i++){
        Net n;
        fscanf(f, "%s %d %d\n", netname, &n.id, &n.pinNum);
        fscanf(f, "  %d %d\n", &n.x1, &n.y1);
        fscanf(f, "  %d %d\n", &n.x2, &n.y2);
        
        n.wl = abs(n.x1-n.x2) + abs(n.y1-n.y2);
        
        netList.push_back(n);
    }
    
    fclose(f);
}

bool compare_wl(Net lhs, Net rhs)
{
    return lhs.wl > rhs.wl;
}

void sort_net_wl()
{
    sort(netList.begin(), netList.end(), compare_wl);
}

bool compare_id(Net lhs, Net rhs)
{
    return lhs.id < rhs.id;
}

void sort_net_id()
{
    sort(netList.begin(), netList.end(), compare_id);
}

void make_maze()
{
    //maze.resize(Height, vector<Tile>(Width));
    
    for(int i=0; i<Width; i++){
        vector<Tile> col;
        for(int j=0; j<Height; j++){
            Tile tile;
            tile.cap_1 = capacity_v;
            tile.cap_2 = capacity_h;
            tile.cap_3 = capacity_v;
            tile.cap_4 = capacity_h;
            
            col.push_back(tile);
        }
        maze.push_back(col);
    }
}

bool isValid(int row, int col, int mode)
{
    if(mode == MODE1){
        return (row >= 0) && (row < Width) && (col >= 0) && (col < Height) && !maze[row][col].isVisited;
    }
    else{
        return (row >= 0) && (row < Width) && (col >= 0) && (col < Height);
    }
}

void BFS(int i, int j, int x, int y)  // wave propagation
{
    queue<Node> q;
    
    maze[i][j].isVisited = true;
    q.push({i, j, 0});
    
    maze[i][j].path = 0;  // start point
    
    int min_dist = INT_MAX;
    
    while (!q.empty())
    {
        Node node = q.front();
        q.pop();
        
        // (i, j) represents current cell and dist stores its
        // minimum distance from the source
        int i = node.x, j = node.y, dist = node.dist;
        
        if (i == x && j == y)
        {
            min_dist = dist;
            break;
        }
        
        // check for all 4 possible movements from current cell
        // and enqueue each valid movement
        for (int k = 0; k < 4; k++)
        {
            // check if it is possible to go to position
            // (i + row[k], j + col[k]) from current position
            if (isValid(i + row[k], j + col[k], MODE1))
            {
                // mark next cell as visited and enqueue it
                maze[i + row[k]][j + col[k]].isVisited = true;
                q.push({ i + row[k], j + col[k], dist + 1 });
                
                maze[i+row[k]][j+col[k]].path = dist+1;  // wave propagate
            }
        }
    }
    
    //if (min_dist != INT_MAX)
        //cout << "The shortest path from source to destination has length " << min_dist << endl;
    //else
      //  cout << "Destination can't be reached from given source" << endl;
}

void backtracking(int source_x, int source_y, int x, int y, int netID)
{
    int i = x;
    int j = y;
    
    while(true)
    {
        //cout << i << " " << j << endl;
        Route route;
        route.x = i; route.y = j;
        netList[netID].routeList.push_back(route);
        
        if(i == source_x && j == source_y){
            break;
        }
        
        int cur_i, cur_j;
        int direct = -1;
        int cap = INT_MIN;
        
        // up
        if(isValid(i, j+1, MODE2)){
            if(maze[i][j+1].path == maze[i][j].path-1){
                if(maze[i][j+1].cap_1 > cap){
                    cap = maze[i][j+1].cap_1;
                    cur_i = i;
                    cur_j = j+1;
                    direct = 1;
                }
            }
        }
        
        // right
        if(isValid(i+1, j, MODE2)){
            if(maze[i+1][j].path == maze[i][j].path-1){
                if(maze[i+1][j].cap_2 > cap){
                    cap = maze[i+1][j].cap_2;
                    cur_i = i+1;
                    cur_j = j;
                    direct = 2;
                }
            }
        }

        // down
        if(isValid(i, j-1, MODE2)){
            if(maze[i][j-1].path == maze[i][j].path-1){
                if(maze[i][j-1].cap_3 > cap){
                    cap = maze[i][j-1].cap_3;
                    cur_i = i;
                    cur_j = j-1;
                    direct = 3;
                }
            }
        }
        
        // left
        if(isValid(i-1, j, MODE2)){
            if(maze[i-1][j].path == maze[i][j].path-1){
                if(maze[i-1][j].cap_4 > cap){
                    cap = maze[i-1][j].cap_4;
                    cur_i = i-1;
                    cur_j = j;
                    direct = 4;
                }
            }
        }
        
        
        if(direct == 1){
            maze[cur_i][cur_j].cap_1--;
        }
        else if(direct == 2){
            maze[cur_i][cur_j].cap_2--;
        }
        else if(direct == 3){
            maze[cur_i][cur_j].cap_3--;
        }
        else{
            maze[cur_j][cur_j].cap_4--;
        }
        
        i = cur_i; j = cur_j;
    }
    
}

void init_maze()
{
    for(int i=0; i<Width; i++){
        for(int j=0; j<Height; j++){
            maze[i][j].isVisited = false;
            maze[i][j].path = -1;
        }
    }
    
}

void output(string file)
{
    string p = "../output/" + file;
    const char *path = p.c_str();
    
    ofstream ofile;
    ofile.open(path);
    
    for(int i=0; i<netList.size(); i++){
        ofile << "net" << netList[i].id << " " << netList[i].id << endl;
        
        for(int j=netList[i].routeList.size()-1; j>0; j--){
            ofile << "(" << netList[i].routeList[j].x << ", " << netList[i].routeList[j].y << ", 1)-(" << netList[i].routeList[j-1].x << ", " << netList[i].routeList[j-1].y << ", 1)" << endl;
        }
        ofile << "!" << endl;
        
    }
    
    ofile.close();
    
    cout << "=====DONE=====" << endl;
    
}



int main(int argc, char *argv[])
{
    input(argv[1]);
    sort_net_wl();
    
    make_maze();
    
    
    for(int i=0; i<netList.size(); i++){
        //cout << "net " << i << endl;
        
        BFS(netList[i].x1, netList[i].y1, netList[i].x2, netList[i].y2);
        backtracking(netList[i].x1, netList[i].y1, netList[i].x2, netList[i].y2, i);
        init_maze();
        
    }
    
    sort_net_id();
    
    string file = argv[2];
    output(file);
    
    return 0;
}