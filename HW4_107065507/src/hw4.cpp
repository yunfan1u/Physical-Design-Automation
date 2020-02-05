#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <string>
#include <cstring>
#include <unordered_map>
#include <map>
#include <sstream>
#include <algorithm>
#include <utility>
#include <float.h>
#include <stack>
#include <cmath>
#include <assert.h>

using namespace std;

int maxDisplacement;
int NumNodes, NumTerminals, NumRows;
string filename = "";

struct Node
{
    string name;
    int index = -1;
    int width;
    int height;
    float x;
    float y;
    float g_x;  // global placement x
    int e = 1;  // weight
    
    int row = -1;  // in which row
    int site = -1;
    
    bool isCluster = false;
    int c1;  // index of pre
    int c2;  // index of now
    
    bool invalid = false;
    
};

struct Row
{
    int id;
    float x;
    float y;
    int siteWidth;
    int height;
    int numSite;
    float outline_x;  // max x
    
    vector<pair<int, int>> blockPos;
    vector<int> cell_list;  // index of sort_cellList
    
    int CW = 0;  // total cell width;
    
    vector<int> siteList;
    
};

unordered_map<string, Node> cellList;
unordered_map<string, Node> blockageList;
vector<Row> rowList;
vector<Row> subRowList;
unordered_map<int, Node> ps_cList;

void parse_nodes(string file)
{
    ifstream ifile(file.c_str());
    string line, token;
    getline(ifile, line);  // UCLA...
    
    while(getline(ifile, line)){
        if(line[0] == '#' || line.empty())
            continue;
        
        stringstream sStream(line);
        sStream >> token;  // read name
        
        if(token == "NumNodes"){
            sStream >> token;  // :
            sStream >> NumNodes;
        }
        else if(token == "NumTerminals"){
            sStream >> token;  // :
            sStream >> NumTerminals;
        }
        else{
            Node n;
            n.name = token;
            
            bool isBlockage = false;
            
            while(sStream >> token){
                if(token == "terminal"){
                    isBlockage = true;
                }
                else {
                    n.width = atoi(token.c_str());
                    sStream >> n.height;
                }
            }
            
            if(isBlockage)
                blockageList[n.name] = n;
            else
                cellList[n.name] = n;
            
        }
    }
    ifile.close();
}

void parse_pl(string file)
{
    ifstream ifile(file.c_str());
    string line, token;
    getline(ifile, line);  // UCLA...
    
    while(getline(ifile, line)){
        if(line.empty())
            continue;
        
        stringstream sStream(line);
        
        string name;
        float x, y;
        sStream >> name >> x >> y;
        
        bool isBlockage = false;
        while(sStream >> token){
            if (token == "/FIXED")
                isBlockage = true;
        }
        
        if(isBlockage){
            blockageList[name].x = x;
            blockageList[name].y = y;
        }
        else{
            cellList[name].g_x = x;
            cellList[name].y = y;
        }
    }
    ifile.close();
}

bool compare_row(Row lhs, Row rhs)
{
    return lhs.y < rhs.y;
}

void parse_scl(string file)
{
    ifstream ifile(file.c_str());
    string line, token;
    getline(ifile, line);  // UCLA...
    
    Row row;
    
    while(getline(ifile, line)){
        if(line[0] == '#' || line.empty())
            continue;
        
        stringstream sStream(line);
        sStream >> token;
        
        if(token == "NumRows"){
            sStream >> token;  // :
            sStream >> NumRows;
        }
        
        if(token == "Coordinate"){
            sStream >> token;
            sStream >> row.y;
        }
        if(token == "Height"){
            sStream >> token;
            sStream >> row.height;
        }
        if(token == "Sitewidth"){
            sStream >> token;
            sStream >> row.siteWidth;
        }
        if(token == "SubrowOrigin"){
            sStream >> token;  // :
            sStream >> row.x;
            sStream >> token;
            sStream >> token;
            sStream >> row.numSite;
        }
        if(token == "End"){
            row.outline_x = row.x + row.numSite * row.siteWidth;
            rowList.push_back(row);
        }
    }
    
    ifile.close();
    //sort(rowList.begin(), rowList.end(), compare_row);  // maybe no need
}

void parse_aux(string file)
{
    FILE *f;
    f = fopen(file.c_str(), "r");
    char n[30], p[30], s[30];
    fscanf(f, "RowBasedPlacement : %s %s %s\n", n, p, s);
    fscanf(f, "MaxDisplacement : %d\n", &maxDisplacement);
    fclose(f);
    
    size_t slash = file.rfind('/');
    string path = file.substr(0, slash+1);
    
    string pp = file.substr(0, slash);
    size_t slash2 = pp.rfind('/');
    size_t len = pp.size() - slash2 - 1;
    filename = path.substr(slash2+1, len);
    
    string n_str(n);
    n_str = path + n_str;
    parse_nodes(n_str);
    
    string p_str(p);
    p_str = path + p_str;
    parse_pl(p_str);
    
    string s_str(s);
    s_str = path + s_str;
    parse_scl(s_str);
}


bool isInRow(Node block, Row row)
{
    return (row.x <= block.x && block.x < row.outline_x) && (block.y <= row.y && block.y+block.height >= row.y+row.height);
}

bool compare_block(pair<float, float> lhs, pair<float, float> rhs)
{
    return lhs.first < rhs.first;
}

void make_subrow()
{
    if(!blockageList.empty()){
        for(int i=0; i<rowList.size(); i++){
            for(unordered_map<string, Node>::iterator it=blockageList.begin(); it!=blockageList.end(); it++){
                if(isInRow(it->second, rowList[i])){
                    pair<float, float> p (it->second.x, it->second.x + it->second.width);
                    rowList[i].blockPos.push_back(p);
                }
            }
            sort(rowList[i].blockPos.begin(), rowList[i].blockPos.end(), compare_block);
        }
    }
    
    for(int i=0; i<rowList.size(); i++){
        int sWidth = rowList[i].siteWidth;
        int siteNum;
        
        int id = 0;
        if(!rowList[i].blockPos.empty()){
            for(int j=0; j<=rowList[i].blockPos.size(); j++){
                Row sr;
                sr.y = rowList[i].y; sr.height = rowList[i].height;
                
                if(j == 0){
                    sr.x = rowList[i].x;
                    sr.outline_x = rowList[i].blockPos[j].first;
                }
                else if(j == rowList[i].blockPos.size()){
                    sr.x = rowList[i].blockPos[j-1].second;
                    sr.outline_x = rowList[i].outline_x;
                }
                else{
                    sr.x = rowList[i].blockPos[j-1].second;
                    sr.outline_x = rowList[i].blockPos[j].first;
                }
                
                siteNum = (sr.outline_x - sr.x)/sWidth;
                for(int k=0; k<siteNum; k++){
                    int s = sr.x + k*sWidth;
                    sr.siteList.push_back(s);
                }
                
                sr.id = id;
                subRowList.push_back(sr);
                id++;
            }
            
        }
        else{
            //siteNum = (rowList[i].outline_x-rowList[i].x)/sWidth;
            
            for(int k=0; k<rowList[i].numSite; k++){
                int s = rowList[i].x + k*sWidth;
                rowList[i].siteList.push_back(s);
            }
            
            rowList[i].id = id;
            subRowList.push_back(rowList[i]);
            id++;
        }
    }
}

float cal_cost(vector<Node> nowList, vector<Node> preList)
{
    float cost = 0;
    for(int i=0; i<nowList.size(); i++){
        cost += abs(nowList[i].g_x - preList[i].g_x) + abs(nowList[i].y - preList[i].y);
    }
    return cost;
}

bool compare_x(Node lhs, Node rhs)
{
    if(lhs.g_x < rhs.g_x)
        return true;
    else if (lhs.g_x == rhs.g_x)
        return lhs.width < rhs.width;
    else
        return false;
}

void sort_cell(vector<Node>& cList)
{
    for(unordered_map<string, Node>::iterator it=cellList.begin(); it!=cellList.end(); it++)
        cList.push_back(it->second);
    
    sort(cList.begin(), cList.end(), compare_x);
    cellList.clear();
    
    for(int i=0; i<cList.size(); i++){
        cList[i].index = i;
    }
}


float displace(Node cell, Row row)
{
    //if(cell.width > row.outline_x-row.x)
      //  return FLT_MAX;
    
    /*if(cell.g_x < row.x){
        return (abs(row.x - cell.g_x) + abs(row.y - cell.y));
    }
    else{
        return abs(row.y - cell.y);
    }*/
    
    return (abs(row.x - cell.g_x) + abs(row.y - cell.y));
    //return abs(row.y - cell.y);
}

float displace_2(Node cell, Row row)
{
    return abs(row.y - cell.y);
}

void init_row(vector<Node>& cList)
{
    sort_cell(cList);
    
    for(int i=0; i<cList.size(); i++){
        float dis = FLT_MAX;
        int index = -1;
        
        
        for(int j=0; j<subRowList.size(); j++){
            
            if(!(subRowList[j].x <= cList[i].g_x && subRowList[j].outline_x >= cList[i].g_x))
                continue;
            
            float _dis = displace(cList[i], subRowList[j]);
            
            if(_dis > dis)
                break;
            
            if(_dis < dis){
                dis = _dis;
                index = j;
            }
        }
        cList[i].row = index;
        cList[i].y = subRowList[index].y;  // !!
        
        //  align site
        int site = 0;
        for(int j=0; j<subRowList[index].siteList.size(); j++){
            
            if(cList[i].g_x < subRowList[index].siteList[j]){
                if(j==0){
                    site = 0;
                    break;
                }
                site = j-1;
                break;
            }
        }
        cList[i].g_x = subRowList[index].siteList[site];
        cList[i].site = site;
    }  
}


void init_row2(vector<Node>& cList)
{
    sort_cell(cList);
    
    for(int i=0; i<cList.size(); i++){
        float dis = FLT_MAX;
        int index = -1;
        
        for(int j=0; j<subRowList.size(); j++){
            float _dis = displace(cList[i], subRowList[j]);
            
            if(_dis > dis)
                break;
            
            if(_dis < dis){
                dis = _dis;
                index = j;
            }
        }
        cList[i].row = index;
        cList[i].y = subRowList[index].y;  //!!
        
        //  align site
        int site = 0;
        for(int j=0; j<subRowList[index].siteList.size(); j++){
            
            if(cList[i].g_x < subRowList[index].siteList[j]){
                if(j==0){
                    site = 0;
                    break;
                }
                site = j-1;
                break;
            }
        }
        cList[i].g_x = subRowList[index].siteList[site];
        cList[i].site = site;
    }
}

void abacus(vector<Node>& cList)
{
    init_row(cList);
}

void insert_row(Node &cell, Row row)
{
    cell.y = row.y;
    cell.row = row.id;
}

void dfs(Node &cell, int _si, vector<Node>& cList, Row row)
{
    //assert(cell.site + _si >=0);
    if(cell.index > NumNodes){
        
        if(cell.c1 > NumNodes){
            dfs(ps_cList[cell.c1], _si, cList, row);
        }
        else{
            dfs(cList[cell.c1], _si, cList, row);
        }
        
        if(cell.c2 > NumNodes){
            dfs(ps_cList[cell.c2], _si, cList, row);
        }
        else{
            dfs(cList[cell.c2], _si, cList, row);
        }
    }
    else{
        cList[cell.index].site += _si;
        int ss = cList[cell.index].site;
        if(ss >= row.siteList.size()){
            ss = row.siteList.size()-1;
            cList[cell.index].site -= _si;
        }
        else if(ss < 0){
            ss = 0;
            cList[cell.index].site -= _si;
        }
        cList[cell.index].g_x = row.siteList[ss];
    }
}

Node clustering(Node &pre, Node &now, vector<Node> &cList, Row row)
{
    Node cluster;
    cluster.isCluster = true;
    cluster.g_x = (pre.e*pre.g_x + now.e*(now.g_x-pre.width))/(pre.e+now.e);
    cluster.width = pre.width + now.width;
    cluster.e = pre.e + now.e;
    
    float d1 = cluster.g_x - row.siteList[pre.site];
    int _si = floor(d1/row.siteWidth);
    
    if(pre.site+_si < 0){
        cluster.site = 0;
        cluster.g_x = row.siteList[0];
        _si = 0 - pre.site;
    }
    else if(pre.site+_si > row.siteList.size()-1){
        cluster.site = row.siteList.size()-1;
        cluster.g_x = row.siteList[row.siteList.size()-1];
        _si = row.siteList.size()- 1 - pre.site;
    }
    else{
        cluster.site = pre.site + _si;
        cluster.g_x = row.siteList[pre.site+_si];
    }
    
    dfs(pre, _si, cList, row);
    
    int _si2 = cluster.site + (pre.width/row.siteWidth) - now.site;
    dfs(now, _si2, cList, row);
    
    cluster.c1 = pre.index;
    cluster.c2 = now.index;
    
    return cluster;
}

bool isOverlap(Node pre, Node now)  // (pre, now)
{
    return (pre.g_x + pre.width > now.g_x);
}

void placeRow(Row row, vector<Node>& cList)
{
    ps_cList.clear();
    stack<Node> _stack;
    
    if(row.cell_list.empty())
        cout << "Error" << endl;
    
    _stack.push(cList[row.cell_list[0]]);
    
    int ci = NumNodes + 1000;
    
    for(int i=1; i<row.cell_list.size(); i++)
    {
        Node now = cList[row.cell_list[i]];
        
        Node pre = _stack.top();
        
        if(isOverlap(pre, now))
        {
            Node cluster = clustering(pre, now, cList, row);
            
            cluster.index = ci;  ci++;
            ps_cList[ci] = cluster;
            _stack.pop();
            
            while(true)
            {
                if(_stack.empty())
                    break;
                
                Node p2 = _stack.top();
                if(isOverlap(p2, cluster)){
                    cluster = clustering(p2, cluster, cList, row);
    
                    
                    cluster.index = ci;  ci++;
                    ps_cList[ci] = cluster;
                    _stack.pop();
                    
                }
                else{
                    break;
                }
            }
            
            _stack.push(cluster);
            
        }
        else{
            _stack.push(now);
        }
        
    }
    
    ps_cList.clear();
}



bool isLegal(Node cell, Row row)
{
    return (row.outline_x-row.x) >= (row.CW+cell.width);
}

void Abacus(vector<Node>& cList)
{
    init_row(cList);
    
    vector<Node> nowList = cList;
    vector<Node> preList = cList;

    for(int i=0; i<cList.size(); i++)
    {
        cout << i << endl;
        
        float c_best = FLT_MAX;
        int r_best = -1;
    
        for(int j=0; j<subRowList.size(); j++)
        {
            if(isLegal(nowList[i], subRowList[j])){
                insert_row(nowList[i], subRowList[j]);
                subRowList[j].cell_list.push_back(i);
                subRowList[j].CW += nowList[i].width;
            }
            else{
                continue;
            }
            
            placeRow(subRowList[j], nowList);
            
            float c = cal_cost(nowList, preList);
            
            subRowList[j].cell_list.pop_back();
            subRowList[j].CW -= nowList[i].width;
            nowList = preList;
            
            if(c > c_best)
                break;
            
            if(c_best > c){
                c_best = c;
                r_best = j;
            }
            
        }
        //???
        insert_row(preList[i], subRowList[r_best]);  // insert cell i to r_best
        subRowList[r_best].cell_list.push_back(i);
        subRowList[r_best].CW += preList[i].width;
        
        placeRow(subRowList[r_best], preList);
        nowList = preList;
    
    }
    
    cList = nowList;
    
}



void output(vector<Node> cList)
{
    string o = filename + ".result";
    string p = "../output/" + o;
    const char *path = p.c_str();
    
    ofstream ofile;
    ofile.open(path);
    
    ofile << "UCLA pl 1.0" << endl << endl;
    
    for(int i=0; i<cList.size(); i++){
        ofile << cList[i].name << " " << cList[i].g_x << " " <<  cList[i].y << " : N" << endl;
    }
    
    for(unordered_map<string, Node>::iterator it=blockageList.begin(); it!=blockageList.end(); it++){
        ofile << it->first << " " << it->second.x << " " << it->second.y << " : N /FIXED" << endl;
    }
    
}


int main(int argc, char *argv[])
{
    parse_aux(argv[1]);
    
    make_subrow();
    
    vector<Node> cList;
    
    abacus(cList);
    //Abacus(cList);
    
    //cout << endl;
    //for(int i=0; i<cList.size(); i++){
      //  cout << cList[i].name << " " << cList[i].g_x << " " << cList[i].y << endl;
    //}
    //output(cList);
    
    return 0;
}



