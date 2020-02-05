#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <string>
#include <unordered_map>
#include <map>
#include <cmath>
#include <stack>
#include <algorithm>
#include <ctime>

using namespace std;

#define K 10
int seed = (unsigned)time(NULL);

double ws_ratio;  // global ratio
int blockNum, terminalNum, netNum, pinNum;
int totalArea;
int globalArea;
int WIRELENGTH;
int OUTLINE;

struct Block
{
    string name;
    int width, height;
    int x=0, y=0;
	int cen_x, cen_y;
	int area;
    bool isRotate = false;
    
    bool isPseudo = false;
    string npe1, npe2;
    
};

struct Terminal
{
    int id;
    int x, y;
};

struct Net
{
    int degree;
    vector<string> in_block_list;
    vector<int> in_ter_list;
};

vector<Net> netList;
vector<Terminal> terList;

void input_block(FILE *in, map<string, Block>& blockList)
{
    fscanf(in, "NumHardRectilinearBlocks : %d\n", &blockNum);
    fscanf(in, "NumTerminals : %d\n\n", &terminalNum);
	totalArea = 0;    

    for(int i=0; i<blockNum; i++){
        Block b;
        int a1, a2, a3, a4, a5, a6, id;  // ignore
        fscanf(in, "sb%d hardrectilinear 4 (%d, %d) (%d, %d) (%d, %d) (%d, %d)\n", &id, &a1, &a2, &a3, &a4, &b.width, &b.height, &a5, &a6);
        int aa = b.width * b.height;
		totalArea += aa;
		b.area = aa;
        b.name = to_string(id);
        blockList[b.name] = b;

    }
    
}

void input_net(FILE *in)
{
    fscanf(in, "NumNets : %d\n", &netNum);
    fscanf(in, "NumPins : %d\n", &pinNum);

    for(int i=0; i<netNum; i++){
        Net n;
        int de;
        fscanf(in, "NetDegree : %d\n", &de);
        n.degree = de;
		
        for(int j=0; j<de; j++){
            char word[20];
            string idstr;
			int id;
            fscanf(in, "%s\n", word);
			            
			string tmp = word;
            if(tmp[0] == 's'){
                idstr = tmp.substr(2, tmp.size()-1);
                n.in_block_list.push_back(idstr);
            }
            else{  // p
                idstr = tmp.substr(1, tmp.size()-1);
                id = atoi(idstr.c_str());
                n.in_ter_list.push_back(id);
            }
        }
        netList.push_back(n);   
    } 
}

void input_terminal(FILE *in)
{
	for(int i=0; i<terminalNum; i++){
		Terminal t;
		fscanf(in, "p%d %d %d\n", &t.id, &t.x, &t.y);
		terList.push_back(t); 	   
	}
}

void cal_fixedoutline()
{
	OUTLINE = sqrt(totalArea*(1+ws_ratio));
}

/*void pre_rotate(map<string, Block>& blockList)
{
    for(map<string, Block>::iterator it=blockList.begin(); it!=blockList.end(); it++){
        if(it->second.width > it->second.height){
            it->second.isRotate = true;
		}
    }
}*/

bool compare1(Block lhs, Block rhs)
{
	/*int l = lhs.width;
	int r = rhs.width;
    if(lhs.isRotate == true)
        l = lhs.height;
    
    if(rhs.isRotate == true)
        r = rhs.height;
    
    return l > r;*/

	return lhs.width > rhs.width;
}

bool compare2(Block lhs, Block rhs)
{
    /*int l = lhs.width;
     int r = rhs.width;
     if(lhs.isRotate == true)
     l = lhs.height;
     
     if(rhs.isRotate == true)
     r = rhs.height;
     
     return l > r;*/
    
    return lhs.height > rhs.height;
}

void init_npe_opt1(vector<string>& npe, map<string, Block>& blockList)
{
    vector<Block> list;
    
    for(int i=0; i<blockList.size(); i++){
        string e = to_string(i);
        list.push_back(blockList[e]);
    }
    
    sort(list.begin(), list.end(), compare1);
    
    int h = list[0].height;
    npe.push_back(list[0].name);
    
    bool flag_1 = false;
    
    for(int i=1; i<list.size(); i++){
        if(h+list[i].height < OUTLINE){
            npe.push_back(list[i].name);
            npe.push_back("H");
            h += list[i].height;
        }
        else{
            if(flag_1)
                npe.push_back("V");
            else
                flag_1 = true;
            
            npe.push_back(list[i].name);
            h = list[i].height;
        }
    }
    
    npe.push_back("V");
}


void init_npe_opt2(vector<string>& npe, map<string, Block>& blockList)
{	
	//pre_rotate(blockList);
    vector<Block> list;
    
    for(int i=0; i<blockList.size(); i++){
		string e = to_string(i);
        list.push_back(blockList[e]);
    } 
    
    sort(list.begin(), list.end(), compare2);
    
    int w = list[0].width;
    npe.push_back(list[0].name);
    
    bool flag_1 = false;
    
    for(int i=1; i<list.size(); i++){
        if(w+list[i].width < OUTLINE){
            npe.push_back(list[i].name);
            npe.push_back("V");
            w += list[i].width;
        }
        else{
            if(flag_1)
                npe.push_back("H");
            else
                flag_1 = true;
            
            npe.push_back(list[i].name);
            w = list[i].width;
        }
    }
    
    npe.push_back("H");
}

unordered_map<string, Block> pseudoBL;

void height_dfs(Block ps_e, int height, map<string, Block>& blockList)
{
    if(pseudoBL[ps_e.npe1].isPseudo)
        height_dfs(pseudoBL[ps_e.npe1], height, blockList);
	else
		blockList[ps_e.npe1].y += height;
    if(pseudoBL[ps_e.npe2].isPseudo)
        height_dfs(pseudoBL[ps_e.npe2], height, blockList);
	else
		blockList[ps_e.npe2].y += height;
}

void width_dfs(Block ps_e, int width, map<string, Block>& blockList)
{
    if(pseudoBL[ps_e.npe1].isPseudo)
        width_dfs(pseudoBL[ps_e.npe1], width, blockList);
	else
		blockList[ps_e.npe1].x += width;
    if(pseudoBL[ps_e.npe2].isPseudo)
        width_dfs(pseudoBL[ps_e.npe2], width, blockList);
	else
		blockList[ps_e.npe2].x += width;
}

int cal_area(vector<string> npe, map<string, Block>& blockList)  // also calculate coordinate
{
	for(map<string, Block>::iterator it=blockList.begin(); it!=blockList.end(); it++){
		it->second.x = 0;
		it->second.y = 0;
	}

    stack<Block> _stack;
    pseudoBL.clear();
    int pseudoID = blockNum+100;
    
    int AREA = 0;
    
    for(int i=0; i<npe.size(); i++){
        if(!(npe[i]=="H" || npe[i]=="V")){
            _stack.push(blockList[npe[i]]);
            
        }
        else{
            Block e1 = _stack.top();
            _stack.pop();
            Block e2 = _stack.top();
            _stack.pop();
            
            string se = to_string(pseudoID);
			pseudoID++;
            Block b;
            b.name = se;
            b.isPseudo = true;
            b.npe1 = e1.name; b.npe2 = e2.name;
            
            if(!e1.isPseudo){
                if(e1.isRotate){
                    int tmp = e1.height;
                    e1.height = e1.width;
                    e1.width = tmp;
                }
            }
            if(!e2.isPseudo){
                if(e2.isRotate){
                    int tmp = e2.height;
                    e2.height = e2.width;
                    e2.width = tmp;
                }
            }
            
            if(npe[i] == "H"){
                
                b.height = e1.height + e2.height;
                b.width = max(e1.width, e2.width);
                b.x = e2.x;
                b.y = e2.y;
                
                if(!e1.isPseudo){
                    blockList[e1.name].x = e2.x;
                    blockList[e1.name].y = e2.y + e2.height;
                    
                }
                else{
                    pseudoBL[e1.name].x = e2.x;
                    pseudoBL[e1.name].y = e2.y + e2.height;
                    height_dfs(e1, e2.height, blockList);
                }
                
            }
            else{  // V
                b.height = max(e1.height, e2.height);
                b.width = e1.width + e2.width;
                b.x = e2.x;
                b.y = e2.y;
                
                if(!e1.isPseudo){
                    blockList[e1.name].x = e2.x + e2.width;
                    blockList[e1.name].y = e2.y;
                }
                else{
                    pseudoBL[e1.name].x = e2.x + e2.width;
                    pseudoBL[e1.name].y = e2.y;
					width_dfs(e1, e2.width, blockList);
                }
                
            }
            pseudoBL[se] = b;
            _stack.push(b);
            
        }
        
        
    }
    Block last_b = _stack.top();
    AREA = last_b.width*last_b.height;

	pseudoBL.clear();
    
	globalArea = AREA;
    return AREA;
}

void pre_WL(map<string, Block>& blockList)
{
	for(map<string, Block>::iterator it=blockList.begin(); it!=blockList.end(); it++){
        if(it->second.isRotate==false){
            it->second.cen_x = it->second.x + (it->second.width/2);
            it->second.cen_y = it->second.y + (it->second.height/2);
        }
        else{
            it->second.cen_x = it->second.x + (it->second.height/2);
            it->second.cen_y = it->second.y + (it->second.width/2);
        }
    }
}


int cal_wirelength(map<string, Block>& blockList)
{
    pre_WL(blockList);
    int WL = 0;
    
    for(int i=0; i<netList.size(); i++){
		int x1=99999, x2=-99, y1=99999, y2=-99;

        for(int j=0; j<netList[i].in_block_list.size(); j++){
            if(blockList[netList[i].in_block_list[j]].cen_x < x1)
                x1 = blockList[netList[i].in_block_list[j]].cen_x;
            if(blockList[netList[i].in_block_list[j]].cen_x > x2)
                x2 = blockList[netList[i].in_block_list[j]].cen_x;
            
            if(blockList[netList[i].in_block_list[j]].cen_y < y1)
                y1 = blockList[netList[i].in_block_list[j]].cen_y;
            if(blockList[netList[i].in_block_list[j]].cen_y > y2)
                y2 = blockList[netList[i].in_block_list[j]].cen_y;
        }
        for(int j=0; j<netList[i].in_ter_list.size(); j++){
            int index = netList[i].in_ter_list[j] - 1;
            if(terList[index].x < x1)
                x1 = terList[index].x;
            if(terList[index].x > x2)
                x2 = terList[index].x;
            if(terList[index].y < y1)
                y1 = terList[index].y;
            if(terList[index].y > y2)
                y2 = terList[index].y;
        }
        
        WL = WL + (x2-x1) + (y2-y1);
        //assert(WL > 0);
    }
    
	WIRELENGTH = WL;
    return WL;
}

int cal_cost(vector<string> npe, map<string, Block>& blockList)
{
    return cal_area(npe, blockList) + cal_wirelength(blockList);
}

void M1(vector<string>& npe)  // swap 2 adjacent operands
{
    vector<int> A;
    
    for(int i=0; i<npe.size(); i++){
        if(!(npe[i]=="H" || npe[i]=="V")){
            A.push_back(i);
        }
    }
    
    int p = rand()%(A.size()-1);
    iter_swap(npe.begin()+A[p], npe.begin()+A[p+1]);
    
}

void M2(vector<string>& npe)  // chain invert
{
    vector<int> A;
    
    for(int i=1; i<npe.size(); i++){
        if((npe[i]=="H" || npe[i]=="V") && !(npe[i-1]=="H" || npe[i-1]=="V")){
            A.push_back(i);
        }
    }
    
    int p = rand()%(A.size()-1);
    int i = A[p];
    
    while(true){
             
        if(npe[i] == "H"){
            npe[i] = "V";
        }
        else{
            npe[i] = "H";
        }
        
        if(!(npe[i+1]=="H" || npe[i+1]=="V")){
            break;
        }
		if(i == A.size()-1)
			break;
        else
            i++;
     
    }
    
}

bool balloting(vector<string> npe)
{
    int a=0, b=0;
    
    for(int i=0; i<npe.size(); i++){
        if(npe[i]=="H" || npe[i]=="V"){
            a++;
        }
        else{
            b++;
        }
        
        if(a>=b)
            return false;
    }
    
    return true;
}

bool skewed(vector<string> npe)
{
    for(int i=0; i<npe.size()-1; i++){
        if(npe[i]=="H"){
            if(npe[i+1]=="H")
                return false;
        }
       else if(npe[i]=="V"){
            if(npe[i+1]=="V")
                return false;
        }
    }
    
    return true;
}

void M3(vector<string>& npe)  // swap 2 adjacent operand and operator
{
    vector<int> A;
    
    for(int i=0; i<npe.size()-1; i++){
        if(npe[i]!="H" && npe[i]!="V"){
            if(npe[i+1]=="H" || npe[i+1]=="V"){
                A.push_back(i);
            }
        }
        else if(npe[i]=="H" || npe[i]=="V"){
            if(npe[i+1]!="H" && npe[i+1]!="V"){
                A.push_back(i);
            }
        }
    }

	vector<string> _npe;
    
    while(true){
		if(A.empty()){
			break;
		}

		_npe = npe;
		int p = 0;
		if(A.size()==1)
			p = 0;
		else		
        	p = rand()%(A.size()-1);
        int r = A[p];
			
        iter_swap(_npe.begin()+r, _npe.begin()+r+1);
        
        if(balloting(_npe) && skewed(_npe))
            break;
		else{
			A.erase(A.begin()+p);
		}
	}

	if(!A.empty()){
		npe = _npe;	
	}
}

void M4(vector<string>& npe, map<string, Block>& blockList)  // rotate
{
    vector<int> A;
    
    for(int i=0; i<npe.size(); i++){
        if(!(npe[i]=="H" || npe[i]=="V")){
            A.push_back(i);
        }
    }
    
    int p = rand()%(A.size()-1);
    string e = npe[A[p]];
    blockList[e].isRotate = !blockList[e].isRotate;
}

bool isFixed(map<string, Block> blockList)
{
    for(map<string, Block>::iterator it=blockList.begin(); it!=blockList.end(); it++){
        int x, y;
        if(!it->second.isRotate){
            x = it->second.x + it->second.width;
            y = it->second.y + it->second.height;
        }
        else{
            x = it->second.x + it->second.height;
            y = it->second.y + it->second.width;
        }
        
        if(x > OUTLINE || y > OUTLINE){
            return false;
        }
    }
    return true;
    
}


void simulated_anealing(vector<string>& npe, map<string, Block>& blockList){
    
    float T=200;
    
    int MT, uphill, reject;
    int N = K*blockNum;
    
    vector<string> tempNPE = npe;
    vector<string> bestNPE = npe;
    map<string, Block> tempBL = blockList;
    map<string, Block> bestBL = blockList;
    
    
    do{
        MT=0; uphill=0; reject=0;
        
        do{
            int old_cost = cal_cost(tempNPE, tempBL);
            int a = rand()%6+1;

            switch(a){
                case 1:{
                    M1(tempNPE);
                    break;
                }
                case 2:{
                    M2(tempNPE);
                    break;
                }
                case 3:{
                    M3(tempNPE);
                    break;
                }
                case 4:{
                    M4(tempNPE, tempBL);
                    break;
                }
				case 5:{
					M1(tempNPE);
					break;
				}
				case 6:{
					M4(tempNPE, tempBL);
					break;
				}
            }
            
            MT++;
            int new_cost = cal_cost(tempNPE, tempBL);
            int deltaCost = old_cost - new_cost;
            
            double r=((double)rand()/(RAND_MAX)); //randomly picks a number between 0 and 1
            
            if(deltaCost < 0 || (r < exp(-1*(deltaCost/T)))){
                if(deltaCost > 0)
                    uphill++;
                
                npe = tempNPE;
                blockList = tempBL;
                
                if(cal_cost(npe, blockList) < cal_cost(bestNPE, bestBL) && isFixed(blockList)){
                    bestNPE = npe;
                    bestBL = blockList;
					cout << "accept " << a <<endl;
				}
                else{
					reject++;
				}
            }
            
            T = 0.85*T;
        }while(uphill < N && MT <= 2*N);
        
        
    }while((reject/MT)<=0.95 && T>1);
   
	npe = bestNPE;
    blockList = bestBL;
	//cal_cost(npe, blockList);
	//cout << "A: " << globalArea << " WL: " << WIRELENGTH << endl;
}



void output(string output, map<string, Block> blockList)
{
    string p = "../output/" + output;
    const char *path = p.c_str();
    
    ofstream ofile;
    ofile.open(path);
    
    ofile << "Wirelength" << " " << WIRELENGTH << endl;
    ofile << "Blocks" << endl;
    

	for(int i=0; i<blockNum; i++){
		string e = to_string(i);
		ofile << "sb" << i << " " << blockList[e].x << " " << blockList[e].y << " ";
		if(blockList[e].isRotate)
			ofile << "1" << endl;
		else
			ofile << "0" << endl;
	}
    ofile.close();
}


void plot(map<string, Block> blockList)
{
    ofstream ofile;
    
    //ofile.open("n100.img");
    ofile.open("n200.img");
    //ofile.open("n300.img");
    
	for(int i=0; i<blockNum; i++){
		string e = to_string(i);
		ofile << blockList[e].x << " " << blockList[e].y << " ";
		if(blockList[e].isRotate)
			ofile << blockList[e].height << " " << blockList[e].width << endl;
		else
			ofile << blockList[e].width << " " << blockList[e].height << endl; 
	}
	ofile.close();
}


int main(int argc, char *argv[])
{
	time_t start, end;
	start = time(NULL);
    
    map<string, Block> blockList;

	FILE *fp_block;
	fp_block = fopen(argv[1], "r");
    input_block(fp_block, blockList);
    fclose(fp_block);
    
    FILE *fp_net;
    fp_net = fopen(argv[2], "r");
	input_net(fp_net);
    fclose(fp_net);

	FILE *fp_pl;
	fp_pl = fopen(argv[3], "r");
	input_terminal(fp_pl);
	fclose(fp_pl);

	ws_ratio = atof(argv[5]);  // input white space ratio
    
	srand(seed);
    
	cal_fixedoutline();
    cout << "outline: " << OUTLINE << endl;
 
    vector<string> NPE;
    
	init_npe_opt2(NPE, blockList);
    
    //for(int i=0; i<NPE.size(); i++)
    //    cout << NPE[i] << "-";
    //cout << endl << endl;
 
	/*cout << cal_area(NPE, blockList) << endl;
 
	for(int i=0; i<blockNum; i++){
		string e = to_string(i);
		cout << "sb" << i << " " << blockList[e].width << " " << blockList[e].height << " R: " << blockList[e].isRotate << endl;
	}*/
   
    while(true){
        simulated_anealing(NPE, blockList);
		end = time(NULL);
	
		if(difftime(end, start) > 100){
			cout << "TIME OUT" << endl;
			break;
		}
    }
   
	cal_cost(NPE, blockList); 
	string op = argv[4];
 	output(op, blockList); 
	cout << WIRELENGTH << endl;
	//plot(blockList);
	
    return 0;
}
