#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <unordered_map>
#include <list>
#include <fstream>
#include <cstdlib>
#include <stack>
#include <ctime>

#define A 0
#define B 1

using namespace std;

int AREA;
int AREA_A, AREA_B;
int C_NUM, N_NUM;
int MAX_PIN;

struct Cell
{
    //string name;
    int size = 0;
    int gain = 0;
    int set = A;
    int pin = 0;
    bool isLocked = false;
    
    vector<string> this_net_list;
    list<string>::iterator  ptr;  // ptr to the bucket list
};

struct Net
{
    //string name;
    vector<string> this_cell_list;
};

unordered_map<string, Cell*> cellList;  // global cellList and netList
unordered_map<string, Net*> netList;

map<int, list<string>> bucket_A;
map<int, list<string>> bucket_B;

void netlist_in_cell(){
    // construct net_list in each Cell
    for(unordered_map<string, Net*>::iterator it=netList.begin(); it!=netList.end(); it++){
        for(int j=0; j<it->second->this_cell_list.size(); j++){
            string c = it->second->this_cell_list[j];  // Cell
            string n = it->first;               // Net
            cellList[c]->this_net_list.push_back(n);
        }
    }
    
    MAX_PIN = 0;
    for(unordered_map<string, Cell*>::iterator it=cellList.begin(); it!=cellList.end(); it++){
        it->second->pin = it->second->this_net_list.size();
        if(it->second->pin > MAX_PIN)
            MAX_PIN = it->second->pin;
    }
}

void init_set()
{
	AREA_A = 0; AREA_B = 0;
    int ct = 0;
    for(unordered_map<string, Cell*>::iterator it=cellList.begin(); it!=cellList.end(); it++){
        if(ct > C_NUM/2)
            it->second->set = B;
        else
            AREA_A += it->second->size;
        ct++;
    }
    AREA_B = AREA - AREA_A;
}

void init_gain()
{
    for(unordered_map<string, Cell*>::iterator it=cellList.begin(); it!=cellList.end(); it++){
        Cell* C = it->second;
        
        string nn, cn;
        int s1, s2;
        bool f1, f2;
        int gain = 0;
        
        for(int j=0; j<C->this_net_list.size(); j++){
            nn = C->this_net_list[j];	// Net
            
            s1 = C->set;  // set
            f1 = true; f2 = true;
            
            Net *N = netList[nn];
            for(int k=0; k<N->this_cell_list.size(); k++){
                cn = N->this_cell_list[k];  // cells in netList
                
                if((*it).first == cn)
                    continue;
                
                s2 = cellList[cn]->set;
                if(s1 == s2)
                    f1 = false;
                if(s1 != s2)
                    f2 = false;
            }
            if(f1)
                gain++;
            if(f2)
                gain--;
        }
        C->gain = gain;
    }
}

bool check_bc(int AA, int BB)
{
    int CC = abs(AA-BB);
    
    if(10*CC < AREA)
        return true;
    else
        return false;
}

void make_bucket_list()
{
    bucket_A.clear();
    bucket_B.clear();
    
    for(int i = -MAX_PIN; i<=MAX_PIN; i++){
        list<string> list_a, list_b;
        bucket_A[i] = list_a;
        bucket_B[i] = list_b;
    }
    
    for(unordered_map<string, Cell*>::iterator it=cellList.begin(); it!=cellList.end(); it++){
        int s = it->second->set;
        int g = it->second->gain;
        
        string* p = new string(it->first);
        
        if(s == A){
            bucket_A[g].push_back(*p);
            it->second->ptr = --bucket_A[g].end();
        }
        else{
            bucket_B[g].push_back(*p);
            it->second->ptr = --bucket_B[g].end();
        }
    }
    
}

string get_max_gain()
{
    int max_gain = 0;
    int mg_a = MAX_PIN, mg_b = MAX_PIN;
    string max_gain_cell = "";
    
    while(true){
       
		bool debug = true; 
        for(int i=mg_a; i>=-MAX_PIN; i--){
            if(bucket_A[i].empty())
                continue;
            else{
                mg_a = i;
				debug = false;
                break;
            }
        }
 
		debug = true;
        for(int i=mg_b; i>=-MAX_PIN; i--){
            if(bucket_B[i].empty())
                continue;
            else{
                mg_b = i;
				debug = false;
                break;
            }
        }
        
        if(mg_a > mg_b)
            max_gain = mg_a;
        else
            max_gain = mg_b;
        
        bool isFound = false;
        //if(!bucket_A[max_gain].empty()){
            for(list<string>::iterator it=bucket_A[max_gain].begin(); it!=bucket_A[max_gain].end(); it++){
                string mg_cell = (*it);
				max_gain_cell = mg_cell;
                int AA = AREA_A, BB = AREA_B;
                
                int mg_size = cellList[mg_cell]->size;
                AA -= mg_size;
                BB += mg_size;
                
                if(check_bc(AA, BB)){
                    isFound = true;
                    //max_gain_cell = mg_cell;
                    break;
                }
            }
        //}
        
        if(isFound)
            break;
        //if(!bucket_B[max_gain].empty()){
            for(list<string>::iterator it=bucket_B[max_gain].begin(); it!=bucket_B[max_gain].end(); it++){
                string mg_cell = (*it);
				max_gain_cell = mg_cell;
                int AA=AREA_A, BB=AREA_B;
                
                int mg_size = cellList[mg_cell]->size;
                AA += mg_size;
                BB -= mg_size;
                
                if(check_bc(AA, BB)){
                    isFound = true;
                    //max_gain_cell = mg_cell;
                    break;
                }
            }
        //}
       
		if(isFound) 
            break;
        else{
            mg_a--; mg_b--;
        }
        
        if(mg_a == -MAX_PIN-1){	// debug
            cout << "DIED" << endl;
            break;
        }
    }
    
	if(max_gain_cell == "")
		cout << "failed" << endl;
    return max_gain_cell;
}

void remove_cell(string cell)  // remove cell from bucket list
{
	cellList[cell]->isLocked = true;
    if(cellList[cell]->set == A){
        bucket_A[cellList[cell]->gain].erase(cellList[cell]->ptr);
    }
    else{
        bucket_B[cellList[cell]->gain].erase(cellList[cell]->ptr);
    }
}

void update_bucketList(string cell, int old_gain)
{
    Cell *C = cellList[cell];
    string *p = new string(cell);
    
    if(C->set == A){
        bucket_A[old_gain].erase(C->ptr);
        bucket_A[C->gain].push_back(*p);
        C->ptr = --bucket_A[C->gain].end();  // ???
    }
    else{
        bucket_B[old_gain].erase(C->ptr);
        bucket_B[C->gain].push_back(*p);
        C->ptr = --bucket_B[C->gain].end();
    }
}

void move_cell(string cell)
{
    Cell *C = cellList[cell];
    
    if(C->set == A){
        AREA_A -= C->size;
        AREA_B += C->size;
        C->set = B;
    }
    else{
        AREA_A += C->size;
        AREA_B -= C->size;
        C->set = A;
    }
}

void update_gain(string base_cell)
{
    Cell *C = cellList[base_cell];
    C->isLocked = true;  // Locked the cell!
    string nn, cn;
    int s1, s2;
    
    for(int i=0; i<C->this_net_list.size(); i++){
        nn = C->this_net_list[i];
        s1 = C->set;
        Net *N = netList[nn];
        
        bool cond_1_before = true, cond_2_before = true;
        string only_T_cell;
        int ct = 0;
        
        for(int j=0; j<N->this_cell_list.size(); j++){
            cn = N->this_cell_list[j];
            
            if(base_cell == cn)
                continue;
            
            s2 = cellList[cn]->set;
            
            if(s2 != s1){
                cond_1_before = false;
                only_T_cell = cn;
                ct++;
            }
        }
        cond_2_before = (ct == 1);
        if(cond_1_before){  // if T(n) == 0
            for(int j=0; j<N->this_cell_list.size(); j++){
                string ccc = N->this_cell_list[j];
                if(cellList[ccc]->isLocked == false){
                    int ggg = cellList[ccc]->gain;
                    cellList[ccc]->gain++;
                    update_bucketList(ccc, ggg);
                }
            }
        }
        else if(cond_2_before){  // elif T(n) == 1
            if(cellList[only_T_cell]->isLocked == false){
                int ggg = cellList[only_T_cell]->gain;
                cellList[only_T_cell]->gain--;
                update_bucketList(only_T_cell, ggg);
            }
        }
        // F(n) = F(n)-1; T(n)=T(n)-1;
        bool cond_1_after = true, cond_2_after = true;
        string only_F_cell;
        ct = 0;
        
        for(int j=0; j<N->this_cell_list.size(); j++){
            cn = N->this_cell_list[j];
            
            if(base_cell == cn)
                continue;
            
            s2 = cellList[cn]->set;
            
            if(s2 == s1){
                cond_1_after = false;
                only_F_cell = cn;
                ct++;
            }
        }
        cond_2_after = (ct == 1);
        if(cond_1_after){  // if F(n) == 0
            for(int j=0; j<N->this_cell_list.size(); j++){
                string ccc = N->this_cell_list[j];
                if(cellList[ccc]->isLocked == false){
                    int ggg = cellList[ccc]->gain;
                    cellList[ccc]->gain--;
                    update_bucketList(ccc, ggg);
                }
            }
        }
        else if(cond_2_after){  // elif F(n) == 1
            if(cellList[only_F_cell]->isLocked == false){
                int ggg = cellList[only_F_cell]->gain;
                cellList[only_F_cell]->gain++;
                update_bucketList(only_F_cell, ggg);
            }
        }
    }
    
    // Actually move cell
    move_cell(base_cell);
}


int Gk()
{
    int lockedNum = 0;
    string Max_cell;
    stack<string> st;  // to restore cell
    vector<int> Gk;  // store partial Gk sum
    
    while(lockedNum != C_NUM){
        Max_cell = get_max_gain();
		Gk.push_back(cellList[Max_cell]->gain);
		st.push(Max_cell);
        remove_cell(Max_cell);
        update_gain(Max_cell);
        lockedNum++;
    }
    
    int turn = 0;
    int max_Gk = -9999999;
    int part_Gk_sum = 0;
    
    // get the max partial Gk in which turn
    for(int i=0; i<Gk.size(); i++){
        part_Gk_sum += Gk[i];
        if(part_Gk_sum > max_Gk){
            max_Gk = part_Gk_sum;
            turn = i;
        }
    }
    
    // restore the cell state
    int to_pop = C_NUM-1-turn;
    string cc;
    for(int i=0; i<to_pop; i++){
        cc = st.top();
        cellList[cc]->set = (cellList[cc]->set+1)%2;
        st.pop();
    }
    
    return max_Gk;
}

int cal_cut_size()
{
    int cut_size = 0;
    for(unordered_map<string, Net*>::iterator it=netList.begin(); it!=netList.end(); it++){
        Net* N = it->second;
        
        int s1 = cellList[N->this_cell_list[0]]->set;
        int s2;
        for(int j=1; j<N->this_cell_list.size(); j++){
            s2 = cellList[N->this_cell_list[j]]->set;
            if(s1 != s2){
                cut_size++;
                break;
            }
        }
    }
    return cut_size;
}

void output(string output)
{	
    vector<string> A_list, B_list;
	string p = "../output/" + output;
	cout << p << endl;
	const char *path = p.c_str();
    ofstream ofile;
    ofile.open(path);
    ofile << "cut_size " << cal_cut_size() << endl;
    
    for(unordered_map<string, Cell*>::iterator it=cellList.begin(); it!=cellList.end(); it++){
        if(it->second->set == A)
            A_list.push_back((*it).first);
        else
            B_list.push_back((*it).first);
    }
    
    ofile << "A " << A_list.size() << endl;
    for(int i=0; i< A_list.size(); i++){
        ofile << A_list[i] << endl;
    }
    ofile << "B " << B_list.size() << endl;
    for(int i=0; i<B_list.size(); i++){
        ofile << B_list[i] << endl;
    }
    ofile.close();
}

void init_all()
{
    AREA_A = 0;
    AREA_B = 0;
    for(unordered_map<string, Cell*>::iterator it=cellList.begin(); it!=cellList.end(); it++){
        it->second->isLocked = false;
        if(it->second->set == A)
            AREA_A += it->second->size;
        else
            AREA_B += it->second->size;
    }
}

void FM()
{
    time_t start, end;
    start = time(NULL);
    
    int _Gk = 999999;
	int pre_Gk = 999999;

    while(true){
        end = time(NULL);
        cout << "time = " << difftime(end, start) << endl;
        if(difftime(end, start) > 2500){
            cout << "TIME OUT" << endl;
            break;
        }
        
        _Gk = Gk();

		if(_Gk <= 0)
			break;		

//		cout << _Gk << endl;
		if(pre_Gk < _Gk)
			break;
		else
			pre_Gk = _Gk;

        init_all();
        init_gain();	
        make_bucket_list();
    }
}


int main(int argc, char *argv[])
{
    ifstream ifile(argv[1]);
    AREA = 0;
    int _size;
    string cellName;
    ifile >> cellName;
    while(ifile.good()){
        Cell* _Cell = new Cell();
        ifile >> _size;
        _Cell->size = _size;
        AREA += _size;        
        
        cellList[cellName] = _Cell;
        
        ifile >> cellName;
    }
    ifile.close();
    
    ifstream ifile2(argv[2]);
    string word;
    string netName;
    ifile2 >> word;         // read "NET"
    while(ifile2.good()){
        Net* _Net = new Net();
        //assert(word == "NET");
        ifile2 >> netName;
        ifile2 >> word;      // read "{"
        //assert(word == "{");
        
        while(true){
            ifile2 >> word;
            if(word == "}")
                break;
            _Net->this_cell_list.push_back(word);
        }
       	ifile2 >> word; // read "NET" 
        
        netList[netName] = _Net;
    }
    ifile2.close();
    
    netlist_in_cell(); // construct netlist in cell
    
    C_NUM = cellList.size();
    N_NUM = netList.size();
   
    init_set();
    init_gain();	
    make_bucket_list();
    
	cout << "start FM" << endl;
    FM();
    
    cout << "cut size " << cal_cut_size() << endl;
    
    string op = argv[3];
    output(op);
    
    return 0;
}



