#include <iostream>
#include <map>
#include <string>
#include <string.h>
#include <algorithm>
#include <math.h>
#include <fstream>
#include <time.h>
#include <vector>
#include <queue>
#include <assert.h>
#include <ctime>
#include <set>
#include <unordered_set>
#include <omp.h>
#include <functional>

using namespace std;

#define ll long long
typedef pair<int, int> P;

const int MAXENERGY = 5 + 1;
const int MAXSHOP = 100 + 1;
const int MAXWIND = 100 + 1;
const int MAXINST = 1000 + 1;
const int MAXEDGE = 1000 + 1;
const int MAXAREA = MAXSHOP * 5 + 1;
const int MAXINSTTYPE = 3 + 1;
const int MAXCIR = 10 + 1;
const int MAXSTATE = (MAXWIND + 1) * (MAXCIR + 1) + 1;

const ll INF = 1e18;
const int iINF = 1e9;

clock_t begin_time;
string info;

ofstream *debug_file = nullptr;
ofstream *result_file = nullptr;

// utils
int StringToInt(string &str);
void Strip(string& str, const char& chr);
vector<string> Split(const string& str, const string& delim);

ll produce_k;
ll ener_time[MAXENERGY];
int shop_n;
int area_n;
int window_n;
int edge_n;
int inst_n;
int max_cir;
int first_cir;
int ener_n = 5;
int inst_type_n = 3;
int sta_n;

vector<int> inst_type_to_ener_type[MAXINSTTYPE];
int can_inst_ener[MAXINSTTYPE][MAXENERGY];

int heart_edge_n;
int heart_edge_ids[MAXEDGE];
int heart_inst_n;
int heart_inst_ids[MAXINST];

ll best_answer = INF;

int tp_sort_ids[MAXINST];

// 处于某个状态下，下一个得到某个能量类型的状态是谁
int next_ener_type[2][MAXINSTTYPE][MAXSTATE][MAXENERGY];
int next_inst_type[2][MAXINSTTYPE][MAXSTATE];
// 是否核心，处于某个状态下，上一个得到某个能量类型的状态是谁
int last_ener_type[2][MAXINSTTYPE][MAXSTATE][MAXENERGY];
int last_inst_type[2][MAXINSTTYPE][MAXSTATE];

struct Inst;
struct Window;
struct Area;
struct Edge;
struct Graph;
struct State;

// func
void ReadIn();
void Output();
bool Check(int inst_id, State &sta);
void InstallInst(int inst_id, State &sta, int update=1);
void CalcL(int inst_id, int ignore_heart, int use_sta_id);
void CalcR(int inst_id, int ignore_heart, int use_sta_id);

struct Shop {
    vector<int> area_ids;
    vector<int> window_ids;
    vector<int> ener_area_ids[MAXENERGY]; // 根据能源id找area id
    Shop() {
        area_ids.clear();
        window_ids.clear();
        for (int i = 0; i < MAXENERGY; ++i) {
            ener_area_ids[i].clear();
        }
    }
} shop[MAXSHOP];

struct Window {
    int cir_window_id; // 环回能到达的点
    int shop_id;
    int can_inst_type[MAXINSTTYPE];
    int use_ener_type[MAXENERGY];
    ll cost_k;
    ll enter_k;
    ll max_one_use_time;
    Window() {
        cir_window_id = -1; // 没有环回
        shop_id = 0;
        for (int i = 0; i < MAXINSTTYPE; i++) {
            can_inst_type[i] = 0;
            use_ener_type[i] = 0;
        }
        cost_k = 0;
        enter_k = 0;
        max_one_use_time = 0;
    }
    int GetEnerAreaId(int ener_type) {
        if (shop[shop_id].ener_area_ids[ener_type].size() > 0) {
            return shop[shop_id].ener_area_ids[ener_type][0];
        }
        return -1;
    }
    // 一次加工时间
    ll GetOneUseTime() {
        ll Max = 0;
        for (int i = 0; i < ener_n; ++i) {
            if (use_ener_type[i] == 0) {
                continue;
            }
            Max = max(Max, ener_time[i]);
        }
        return Max;
    }
    ll GetSumUseTime() {
        return GetOneUseTime() * enter_k;
    }
    ll GetCost() {
        ll answer = 0;
        answer += max_one_use_time * produce_k;
        answer += max_one_use_time * enter_k * cost_k;
        return answer;
    }
} window[MAXWIND];

struct State {
    int window_id;
    int area_id;
    int cir;
    State() {}
    State(int _window_id, int _area_id, int _cir) {
        window_id = _window_id;
        area_id = _area_id;
        cir = _cir;
    }
    bool cmp(State &p) {
        if (window_id < first_cir && p.window_id >= first_cir) return true;
        if (window_id >= first_cir && p.window_id < first_cir) return false;
        if (window_id < first_cir) {
            if (cir == p.cir) {
                return window_id < p.window_id;
            }
            return cir < p.cir;
        } else {
            if (window_id == p.window_id) {
                return cir < p.cir;
            }
            return window_id < p.window_id;
        }
        return true;
    }
    bool Next() {
        if (window[window_id].cir_window_id == -1 || cir == max_cir) {
            if (window_id + 1 == window_n) return false;
            ++window_id;
            if (window_id >= first_cir) {
                cir = 0;
            }
            return true;
        }
        ++cir;
        window_id = window[window_id].cir_window_id;
        return true;
    }
} sta[MAXSTATE];

struct Edge {
    int fr_inst_id;
    int to_inst_id;
    int edge_type;
    int is_heart;
    Edge() {
        fr_inst_id = 0;
        to_inst_id = 0;
        edge_type = 0;
        is_heart = 0;
    }
} edge[MAXEDGE];


struct Inst {
    int inst_type;
    ll ener_cost[MAXENERGY];
    
    vector<int> vec; // 邻接边id
    vector<int> r_vec; // 反图邻接边id

    int sta_id;

    State sta;
    int l_sta_id;
    int r_sta_id;
    State best_sta;

    int is_heart; // 核心点
    int last_heart_inst_id; // 核心点上个核心点
    int last_edge_type; // 核心点上一条边类型

    int length;

    Inst () {
        inst_type = 0;
        for (int i = 0; i < MAXENERGY; i++) {
            ener_cost[i] = 0;
        }
        is_heart = 0;
        last_edge_type = -1;
        sta_id = -1;
        vec.clear();
        r_vec.clear();
    }
} inst[MAXINST];

struct Area {
    int shop_id;
    int ener_type;
    Area() {
        shop_id = 0;
        ener_type = 0;
    }
} area[MAXAREA];

struct Graph {
    int in[MAXINST];
    int min_sta_id[MAXINST];
    priority_queue<P> que;
    Graph() {
    }
    void Init() {
        
    }
    void CheckInit() {
        for (int i = 0; i < inst_n; ++i) {
            min_sta_id[i] = max(0, inst[i].sta_id);
        }
    }
    void CalcIn() {
        for (int i = 0; i < inst_n; ++i) {
            in[i] = 0;
        }
        for (int i = 0; i < edge_n; ++i) {
            ++in[edge[i].to_inst_id];
        }
    }
    // 计算id
    bool TpSort(int cal_index) {
        CalcIn();
        while (que.size()) que.pop();
        for (int i = 0; i < inst_n; ++i) {
            if (in[i] == 0) {
                que.push(P(inst[i].is_heart, i));
            }
        }
        int u;
        State tmp;
        int area_id;
        int index = 0;
        while (que.size()) {
            u = que.top().second; 
            if (cal_index == 1) {
                tp_sort_ids[index] = u;
                ++index;
            }
            que.pop();
            for (auto &e_id : inst[u].vec) {
                Edge &e = edge[e_id];
                --in[e.to_inst_id];
                if (in[e.to_inst_id] == 0) {
                    que.push(P(inst[e.to_inst_id].is_heart, e.to_inst_id));
                }
            }
        }
        return true;
    }
    // bool Check(int index) {
    //     CheckInit();
    //     State tmp;
    //     int cur_sta_id;
    //     int u;
    //     for (int i = index; i < inst_n; ++i) {
    //         u = tp_sort_ids[i]; 
    //         for (auto &e_id : inst[u].r_vec) {
    //             Edge &e = ::edge[e_id];
    //             if (e.edge_type == 0) min_sta_id[u] = max(min_sta_id[e.fr_inst_id] + 1, min_sta_id[u]);
    //             else min_sta_id[u] = max(min_sta_id[e.fr_inst_id], min_sta_id[u]);
    //         }
    //         if (min_sta_id[u] >= sta_n) {
    //             return false;
    //         }
    //         cur_sta_id = iINF;
    //         if (inst[u].is_heart == 0) {
    //             for (auto &j : inst_type_to_ener_type[inst[u].inst_type]) {
    //                 if (cur_sta_id > next_ener_type[min_sta_id[u]][j]) {
    //                     cur_sta_id = next_ener_type[min_sta_id[u]][j];
    //                 }
    //             }
    //         } else {
    //             for (auto &j : inst_type_to_ener_type[inst[u].inst_type]) {
    //                 if (cur_sta_id > heart_next_ener_type[inst[u].inst_type][min_sta_id[u]][j]) {
    //                     cur_sta_id = heart_next_ener_type[inst[u].inst_type][min_sta_id[u]][j];
    //                 }
    //             }
    //         }
    //         if (cur_sta_id >= sta_n) return false;
    //         min_sta_id[u] = cur_sta_id;
    //     }
    //     return true;
    // }
} g;

void CalcL(int inst_id, int ignore_heart, int use_sta_id) {
    inst[inst_id].l_sta_id = 0;
    for (auto &eid : inst[inst_id].r_vec) {
        if (ignore_heart == 1 && edge[eid].is_heart == 1) continue;
        if (use_sta_id == 1) {
            if (edge[eid].edge_type == 0)
                inst[inst_id].l_sta_id = max(inst[inst_id].l_sta_id, inst[edge[eid].fr_inst_id].sta_id + 1);
            if (edge[eid].edge_type == 1)
                inst[inst_id].l_sta_id = max(inst[inst_id].l_sta_id, inst[edge[eid].fr_inst_id].sta_id);
        } else {
            if (edge[eid].edge_type == 0)
                inst[inst_id].l_sta_id = max(inst[inst_id].l_sta_id, inst[edge[eid].fr_inst_id].l_sta_id + 1);
            if (edge[eid].edge_type == 1)
                inst[inst_id].l_sta_id = max(inst[inst_id].l_sta_id, inst[edge[eid].fr_inst_id].l_sta_id);    
        }
    }
    inst[inst_id].l_sta_id = next_inst_type[inst[inst_id].is_heart][inst[inst_id].inst_type][inst[inst_id].l_sta_id];
}

void CalcR(int inst_id, int ignore_heart, int use_sta_id) {
    inst[inst_id].r_sta_id = sta_n - 1;
    for (auto &eid : inst[inst_id].vec) {
        if (ignore_heart == 1 && edge[eid].is_heart == 1) continue;
        if (use_sta_id == 1) {
            if (edge[eid].edge_type == 0)
                inst[inst_id].r_sta_id = min(inst[inst_id].r_sta_id, inst[edge[eid].to_inst_id].sta_id - 1);
            if (edge[eid].edge_type == 1)
                inst[inst_id].r_sta_id = min(inst[inst_id].r_sta_id, inst[edge[eid].to_inst_id].sta_id);
        } else {
            if (edge[eid].edge_type == 0)
                inst[inst_id].r_sta_id = min(inst[inst_id].r_sta_id, inst[edge[eid].to_inst_id].r_sta_id - 1);
            if (edge[eid].edge_type == 1)
                inst[inst_id].r_sta_id = min(inst[inst_id].r_sta_id, inst[edge[eid].to_inst_id].r_sta_id);    
        }
    }
    inst[inst_id].r_sta_id = last_inst_type[inst[inst_id].is_heart][inst[inst_id].inst_type][inst[inst_id].r_sta_id];
}


bool Check(int inst_id, int window_id, int area_id) {
    // 文档只需要预处理核心流
    if (inst[inst_id].is_heart == 1 && window[window_id].can_inst_type[inst[inst_id].inst_type] == 0) {
        return false;
    }
    int inst_type = inst[inst_id].inst_type;
    if (inst_type == 0) {
        if (area[area_id].ener_type != 0 && area[area_id].ener_type != 1)
            return false;
    }
    if (inst_type == 1) {
        if (area[area_id].ener_type != 0 && area[area_id].ener_type != 2)
            return false;
    }
    if (inst_type == 2) {
        if (area[area_id].ener_type != 3 && area[area_id].ener_type != 4)
            return false;
    }
    return true;
}

bool Check(int inst_id, State &sta) {
    return Check(inst_id, sta.window_id, sta.area_id);
}

ll CalcCost(int inst_id, int window_id, int area_id) {
    ll res = inst[inst_id].ener_cost[area[area_id].ener_type];
    if (inst[inst_id].is_heart == 0) return res;
    ll enter_k = window[window_id].enter_k;
    ++enter_k;
    if (inst[inst_id].last_edge_type == 1 && 
        inst[inst_id].last_heart_inst_id != -1 &&
        inst[inst[inst_id].last_heart_inst_id].sta.window_id == window_id) {
            --enter_k;
        }
    ll max_one_use_time = max(window[window_id].max_one_use_time, ener_time[area[area_id].ener_type]);
    res += max_one_use_time * produce_k;
    res += max_one_use_time * enter_k * window[window_id].cost_k;
    res -= window[window_id].GetCost();
    return res;
}

void InstallInst(int inst_id, int window_id, int area_id, int cir, int sta_id, int update=1) {
    // 非核心不需要计时
    if (inst[inst_id].is_heart == 0) return;
    // 和协同边位于同一窗口 暂时没判断cir和下一个节点
    if (update == 1 &&
        inst[inst_id].last_edge_type == 1 && 
        inst[inst_id].last_heart_inst_id != -1 &&
        inst[inst[inst_id].last_heart_inst_id].sta.window_id == window_id) {
            --window[window_id].enter_k;
        }
    inst[inst_id].sta_id = sta_id;
    inst[inst_id].sta = State(window_id, area_id, cir);
    if (update == 1) {
        ++window[window_id].enter_k;
        ++window[window_id].use_ener_type[area[area_id].ener_type];
        window[window_id].max_one_use_time = max(window[window_id].max_one_use_time, ener_time[area[area_id].ener_type]);
    }
}

void InstallInst(int inst_id, State &sta, int sta_id, int update=1) {
    InstallInst(inst_id, sta.window_id, sta.area_id, sta.cir, update);
}

void Clear(int clear_inst) {
    if (clear_inst == 1) {
        for (int i = 0; i < inst_n; ++i) {
            inst[i].sta_id = -1;
        }
    }
    for (int i = 0; i < window_n; ++i) {
        window[i].enter_k = 0;
        window[i].max_one_use_time = 0;
        for (int j = 0; j < ener_n; ++j) {
            window[i].use_ener_type[j] = 0;
        }
    }
}

void ReInstall() {
    for (int j = 0; j < inst_n; ++j) {
        int i = tp_sort_ids[j];
        InstallInst(i, inst[i].sta, inst[i].sta_id, 1);
    } 
}

void CheckVaild() {
    if (debug_file == nullptr) return ;
    int flag = 1;
    for (int i = 0; i < inst_n; ++i) {
        if (inst[i].sta_id == -1) {
            flag = 0;
        }
    }
    if (flag == 0) {
        //assert(false);
        (*result_file) << "check_fail sta_id = -1" << endl;
    }
    flag = 1;
    for (int i = 0; i < inst_n; ++i) {
        if (! Check(i, inst[i].sta)) {
            flag = 0;
        }
    }
    if (flag == 0) {
        //assert(false);
        (*result_file) << "check_fail sta install check" << endl;
    }
    flag = 1;
    int u, v;
    for (int i = 0; i < edge_n; ++i) {
        u = edge[i].fr_inst_id;
        v = edge[i].to_inst_id;
        if (edge[i].edge_type == 0) {
            if (inst[v].sta_id <= inst[u].sta_id) 
                flag = 0;
        } else {
            if (inst[v].sta_id < inst[u].sta_id) 
                flag = 0;
        }
    }
    if (flag == 0) {
        (*result_file) << "check_fail graph" << endl;
    }
}

ll GetAnswer() {
    ll answer = 0;

    for (int inst_id = 0; inst_id < inst_n; ++inst_id) {
        int area_id = inst[inst_id].sta.area_id;
        answer += inst[inst_id].ener_cost[area[area_id].ener_type];
    }

    Clear(0);
    ReInstall();

    for (int window_id = 0; window_id < window_n; ++window_id) {
        answer += window[window_id].GetCost();
    }

    return answer;
}

int vis[MAXINST];
int DFS(int u) {
    if (vis[u]) return inst[u].length;
    inst[u].length = 1;
    vis[u] = 1;
    for (auto &e_id : inst[u].vec) {
        inst[u].length = max(inst[u].length, DFS(edge[e_id].to_inst_id) + 1);
    }
    return inst[u].length;
}

void Init() {
    for (int i = 0; i < heart_edge_n; ++i) {
        heart_inst_ids[i] = edge[heart_edge_ids[i]].fr_inst_id;    
    }
    heart_inst_ids[heart_edge_n] = edge[heart_edge_ids[heart_edge_n - 1]].to_inst_id;    
    heart_inst_n = heart_edge_n + 1;

    for (int i = 0; i < heart_edge_n; ++i) {
        edge[heart_edge_ids[i]].is_heart = 1;
        inst[edge[heart_edge_ids[i]].fr_inst_id].is_heart = 1;
        inst[edge[heart_edge_ids[i]].to_inst_id].is_heart = 1;
        inst[edge[heart_edge_ids[i]].to_inst_id].last_heart_inst_id = edge[heart_edge_ids[i]].fr_inst_id;
        inst[edge[heart_edge_ids[i]].to_inst_id].last_edge_type = edge[heart_edge_ids[i]].edge_type;
        if (i == 0) {
            inst[edge[heart_edge_ids[i]].fr_inst_id].last_heart_inst_id = -1; 
        }
    }

    for (int area_id = 0; area_id < area_n; ++area_id) {
        shop[area[area_id].shop_id].area_ids.emplace_back(area_id);
        shop[area[area_id].shop_id].ener_area_ids[area[area_id].ener_type].emplace_back(area_id);
    }

    for (int window_id = 0; window_id < window_n; ++window_id) {
        shop[window[window_id].shop_id].window_ids.emplace_back(window_id);
    }

    for (int edge_id = 0; edge_id < edge_n; ++edge_id) {
        inst[edge[edge_id].fr_inst_id].vec.emplace_back(edge_id);
        inst[edge[edge_id].to_inst_id].r_vec.emplace_back(edge_id);
    }

    sta[0] = State(0, -1, 0);
    sta_n = 1;
    while (true) {
        sta[sta_n] = sta[sta_n - 1];
        if (sta[sta_n].Next()) {
            ++sta_n;
        } else {
            break;
        }
    }

    if (debug_file != nullptr) {
        (*debug_file) << "sta_n: " << sta_n << endl;
    }

    inst_type_to_ener_type[0].emplace_back(0);
    inst_type_to_ener_type[0].emplace_back(1);
    inst_type_to_ener_type[1].emplace_back(0);
    inst_type_to_ener_type[1].emplace_back(2);
    inst_type_to_ener_type[2].emplace_back(3);
    inst_type_to_ener_type[2].emplace_back(4);
    
    for (int i = 0; i < inst_type_n; ++i) {
        for (auto &j : inst_type_to_ener_type[i]) {
            can_inst_ener[i][j] = 1;
        }
    }

    for (int heart = 0; heart < 2; ++heart) 
        for (int inst_type = 0; inst_type < inst_type_n; ++inst_type) 
            for (int i = 0; i < sta_n; ++i) {
                for (int j = 0; j < ener_n; ++j) {
                    next_ener_type[heart][inst_type][i][j] = iINF;
                    last_ener_type[heart][inst_type][i][j] = -iINF;
                    next_inst_type[heart][inst_type][i] = iINF;
                    last_inst_type[heart][inst_type][i] = -iINF;
                }
            }
    int area_id;
    for (int inst_type = 0; inst_type < inst_type_n; ++inst_type) {
        for (int i = sta_n - 1; i >= 0; --i) {
            for (int j = 0; j < ener_n; ++j) {
                if (i < sta_n - 1) {
                    next_ener_type[0][inst_type][i][j] = next_ener_type[0][inst_type][i + 1][j];      
                    next_inst_type[0][inst_type][i] =  min(next_inst_type[0][inst_type][i], next_inst_type[0][inst_type][i + 1]);
                    next_ener_type[1][inst_type][i][j] = next_ener_type[1][inst_type][i + 1][j];      
                    next_inst_type[1][inst_type][i] =  min(next_inst_type[1][inst_type][i], next_inst_type[1][inst_type][i + 1]);
                }
                area_id = window[sta[i].window_id].GetEnerAreaId(j);
                if (area_id == -1) continue;
                if (can_inst_ener[inst_type][j] == 0) continue;
                next_ener_type[0][inst_type][i][j] = i;
                next_inst_type[0][inst_type][i] = i;
                if (window[sta[i].window_id].can_inst_type[inst_type] == 0) continue;
                next_ener_type[1][inst_type][i][j] = i;
                next_inst_type[1][inst_type][i] = i;
            }
        }
    }
    for (int inst_type = 0; inst_type < inst_type_n; ++inst_type) {
        for (int i = 0; i < sta_n; ++i) {
            for (int j = 0; j < ener_n; ++j) {
                if (i > 0) {
                    last_ener_type[0][inst_type][i][j] = last_ener_type[0][inst_type][i - 1][j];      
                    last_inst_type[0][inst_type][i] =  max(last_inst_type[0][inst_type][i], last_inst_type[0][inst_type][i - 1]);
                    last_ener_type[1][inst_type][i][j] = last_ener_type[1][inst_type][i - 1][j];      
                    last_inst_type[1][inst_type][i] =  max(last_inst_type[1][inst_type][i], last_inst_type[1][inst_type][i - 1]);
                }
                area_id = window[sta[i].window_id].GetEnerAreaId(j);
                if (area_id == -1) continue;
                if (can_inst_ener[inst_type][j] == 0) continue;
                last_ener_type[0][inst_type][i][j] = i;
                last_inst_type[0][inst_type][i] = i;
                if (window[sta[i].window_id].can_inst_type[inst_type] == 0) continue;
                last_ener_type[1][inst_type][i][j] = i;
                last_inst_type[1][inst_type][i] = i;
            }
        }
    }

    for (int i = 0; i < inst_n; ++i) {
        DFS(i);
    }
    // if (debug_file != nullptr) {
    //     for (int shop_id = 0; shop_id < shop_n; ++shop_id) {
    //         (*debug_file) << "shop_id: " << shop_id
    //                       << endl;
    //         for (auto &area_id : shop[shop_id].area_ids)
    //             (*debug_file) << area_id << endl;
    //         (*debug_file) << "window_id: "  
    //                       << endl;
    //         for (auto &window_id : shop[shop_id].window_ids)
    //             (*debug_file) << window_id << endl;
    //         for (int ener_id = 0; ener_id < ener_n; ++ener_id) {
    //             (*debug_file) << "ener_id: " << ener_id
    //                       << endl;    
    //             for (auto &area_id : shop[shop_id].ener_area_ids[ener_id])
    //                (*debug_file) << area_id << endl;              
    //         }
    //     }
    // }
}

void Work() {

    g.Init();
    g.TpSort(1);

    for (int i = inst_n - 1; i >= 0; --i) {
        int inst_id = tp_sort_ids[i];
        CalcR(inst_id, 0, 0);
        if (debug_file != nullptr) {
            (*debug_file) << " inst_id: " << inst_id  
                          << " r_sta_id: " << inst[inst_id].r_sta_id
                          << endl;
        }
    }


    for (double qz = 0.5; qz < 10.0; ++qz) {
        Clear(1);
        for (int i = 0; i < inst_n; ++i) {
            int inst_id = tp_sort_ids[i];

            CalcL(inst_id, 0, 1);

            int l = inst[inst_id].l_sta_id;
            int r = inst[inst_id].r_sta_id;

            ll min_cost = INF;
            int min_sta_id = -1;
            State tmp;
            State min_sta;
            ll cost;
            
            int search_next =  (int)((double)(sta_n - l + 1) / (double)inst[inst_id].length * qz);

            for (int j = l; j <= r; ++j) {
                if (min_cost < INF && j > l + search_next) {
                    break ;
                }
                tmp = sta[j];
                if (inst[inst_id].is_heart == 1 && window[tmp.window_id].can_inst_type[inst[inst_id].inst_type] == 0) {
                    continue;
                }
                for (auto &ener_type : inst_type_to_ener_type[inst[inst_id].inst_type]) {
                    tmp.area_id = window[tmp.window_id].GetEnerAreaId(ener_type);
                    if (tmp.area_id == -1) continue;
                    cost = CalcCost(inst_id, tmp.window_id, tmp.area_id);
                    if (debug_file != nullptr) {
                        (*debug_file) << "CalcCost, inst_id: " << inst_id
                                    << " sta_id: " << j
                                    << " window_id: " << tmp.window_id
                                    << " area_id: " << tmp.area_id
                                    << " ener_type: " << ener_type 
                                    << " inst_type: " << inst[inst_id].inst_type 
                                    << " is_heart: " << inst[inst_id].is_heart
                                    << " cost: " << cost
                                    << endl;
                    }
                    if (cost < min_cost) {
                        min_cost = cost;
                        min_sta_id = j;
                        min_sta = tmp;
                    }
                }
            }
            if (debug_file != nullptr) {
                (*debug_file) << "TPInstall, inst_id: " << inst_id
                            << " sta_id: " << min_sta_id
                            << endl;
            }
            inst[inst_id].sta_id = min_sta_id;
            inst[inst_id].sta = min_sta;
        }

        CheckVaild();

        ll answer = GetAnswer();
        if (answer < best_answer) {
            best_answer = answer;
            for (int i = 0; i < inst_n; ++i) {
                inst[i].best_sta = inst[i].sta;
            }
        }
    }
}



int main(int argc, char *argv[]) {
    begin_time = clock();
    // local or oj
    ::info = "info";
    string data_file;
    for (int i = 1; i < argc; ++i) {
        int len = strlen(argv[i]);
        string tmp = "";
        for (int j = 0; j < len; ++j) tmp += argv[i][j];
        Strip(tmp, '\r');
        vector<string> str_vec = Split(tmp, "=");
        if (str_vec[0] == "info") {
            info = str_vec[1];
        }
        if (str_vec[0] == "data_file") {
            data_file = str_vec[1];
            freopen((char *)str_vec[1].data(), "r", stdin);
        }
        if (str_vec[0] == "output_file") {
            freopen((char *)str_vec[1].data(), "w", stdout);
        }
        if (str_vec[0] == "result_file") {
            result_file = new ofstream();
            result_file->open(str_vec[1], ofstream::app);
        }
        if (str_vec[0] == "debug_file") {
            debug_file = new ofstream();
            debug_file->open(str_vec[1], ios::out);
        }
    }

    // work here
    ReadIn();
    Init();
    Work();
    Output();
    // end work

    if (result_file != nullptr && info[0] != '!') {
        (*result_file) << "info: " << endl;
        (*result_file) << "data_file: " << data_file <<endl;
        (*result_file) << "sum_cost: " << GetAnswer() 
                       << " best_answer: " << best_answer 
                       << " cost_time: " << (double)(clock() - begin_time) / CLOCKS_PER_SEC << "s" 
                       << endl;
        (*result_file) << endl;
    }

    if (debug_file != nullptr) {
        for (int i = 0; i < inst_n; ++i) {
            (*debug_file) << "inst_id: " << i
                          << " sta: " << inst[i].sta.window_id << " " << inst[i].sta.area_id << " " << inst[i].sta.cir 
                          << " install_cost: " << inst[i].ener_cost[area[inst[i].sta.area_id].ener_type]
                          << endl;
            sort(inst[i].ener_cost, inst[i].ener_cost + ener_n);
            (*debug_file) << "Max 2 cost: " << inst[i].ener_cost[ener_n - 2] << " " << inst[i].ener_cost[ener_n - 1] << endl;
        }
        for (int i = 0; i < window_n; ++i) {
            (*debug_file) << "window_id: " << i
                          << " one_time: " << window[i].GetOneUseTime()
                          << " enter_k: " << window[i].enter_k
                          << " cost_k: " << window[i].cost_k
                          << " sum_time: " << window[i].GetSumUseTime()
                          << endl;
        }
    }

    if (debug_file != nullptr) {
        (*debug_file).close();
    }
    if (result_file != nullptr) {
        (*result_file).close();
    }
    return 0;
}

void ReadIn() {
    scanf("%lld", &produce_k);
    for (int i = 0; i < ener_n; ++i) {
        scanf("%lld", &ener_time[i]);
    }
    scanf("%d", &shop_n);
    scanf("%d", &area_n);
    for (int i = 0; i < area_n; ++i) {
        scanf("%d", &area[i].shop_id);
        scanf("%d", &area[i].ener_type);
    }
    scanf("%d", &max_cir);
    scanf("%d", &first_cir);
    scanf("%d", &window_n);
    int can_self_cir;
    for (int i = 0; i < window_n; ++i) {
        scanf("%d%d%lld", &can_self_cir, &window[i].shop_id, &window[i].cost_k);
        if (can_self_cir) window[i].cir_window_id = i;
        else window[i].cir_window_id = -1;
        for (int j = 0; j < 3; ++j) {
            scanf("%d", &window[i].can_inst_type[j]);
        }
    }
    window[first_cir - 1].cir_window_id = 0;
    scanf("%d", &inst_n);
    for (int i = 0; i < inst_n; ++i) {
        scanf("%d", &inst[i].inst_type);
        for (int j = 0; j < ener_n; ++j) {
            scanf("%lld", &inst[i].ener_cost[j]);
        }
    }
    scanf("%d", &edge_n);
    for (int i = 0; i < edge_n; ++i) {
        scanf("%d%d%d", &edge[i].edge_type, &edge[i].fr_inst_id, &edge[i].to_inst_id);
    }
    scanf("%d",&heart_edge_n);
    for (int i = 0; i < heart_edge_n; ++i) {
        scanf("%d", &heart_edge_ids[i]);
    }
    if (debug_file != nullptr) {
        (*debug_file) << "inst_n: " << inst_n
                      << " edge_n: " << edge_n
                      << " max_cir: " << max_cir
                      << endl;
        for (int i = 0; i < window_n; ++i) {
            (*debug_file) << "window_id: " << i
                          << " cir: " << window[i].cir_window_id
                          << endl; 
        }
    }
}

void Output() {
    printf("%d\n%d", inst_n, inst[0].best_sta.area_id);
    for (int i = 1; i < inst_n; ++i) {
        printf(" %d", inst[i].best_sta.area_id);
    }
    printf("\n");
    printf("%d\n%d", heart_edge_n + 1, inst[edge[heart_edge_ids[0]].fr_inst_id].best_sta.window_id);
    for (int i = 0; i < heart_edge_n; ++i) {
        printf(" %d", inst[edge[heart_edge_ids[i]].to_inst_id].best_sta.window_id);
    }
}


int StringToInt(string &str) {
    int ret = 0;
    for (int i = 0; i < str.length(); ++i) {
        ret = ret * 10 + str[i] - '0';
    }
    return ret;
}

void Strip(string& str, const char& chr) {
    while (str.length() && str[str.length() - 1] == chr) {
        str.pop_back();
    }
}

vector<string> Split(const string& str, const string& delim) {
    vector<string> ret;
    string tmp = "";
    for (int i = 0; i < str.length(); ++i) {
        int is_split = 1;
        for (int j = 0; j < delim.length(); ++j) {
            if (str[i] != delim[j]) {
                is_split = 0;
                break;
            }
        }
        if (is_split) {
            ret.emplace_back(tmp);
            tmp = "";
            i += delim.length() - 1;
        } else {
            tmp += str[i];
        }
    }
    if (tmp != "") {
        ret.emplace_back(tmp);
    }
    return ret;    
}