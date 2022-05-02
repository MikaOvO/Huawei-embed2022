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
#include <unistd.h>
#include <ctime>
#include <set>
#include <unordered_set>
#include <omp.h>
#include <unistd.h>
#include <signal.h>
#include <functional>

using namespace std;

#define ll long long

const int MAXENERGY = 5 + 1;
const int MAXSHOP = 100 + 1;
const int MAXWIND = 100 + 1;
const int MAXINST = 1000 + 1;
const int MAXEDGE = 1000 + 1;
const int MAXAREA = MAXSHOP * 5 + 1;
const int MAXINSTTYPE = 3 + 1;

clock_t begin_time;
string info;

ofstream *debug_file = nullptr;
ofstream *result_file = nullptr;

// utils
int StringToInt(string &str);
void Strip(string& str, const char& chr);
vector<string> Split(const string& str, const string& delim);

// func
void ReadIn();

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

int heart_edge_n;
int heart_edge_ids[MAXEDGE];


struct Inst;
struct Window;
struct Area;
struct Edge;

struct Inst {
    int inst_type;
    ll ener_cost[MAXENERGY];
    int window_id;
    Inst () {
        inst_type = 0;
        for (int i = 0; i < MAXENERGY; i++) {
            ener_cost[i] = 0;
        }
        window_id = -1; // 未安装
    }
} inst[MAXINST];

struct Window {
    int cir_window_id; // 环回能到达的点
    int shop_id;
    int can_inst_type[MAXINSTTYPE];
    ll cost_k;
    Window() {
        cir_window_id = -1; // 没有环回
        shop_id = 0;
        for (int i = 0; i < MAXINSTTYPE; i++) {
            can_inst_type[i] = 0;
        }
        cost_k = 0;
    }
} window[MAXWIND];

struct Shop {
    vector<int> area_ids;
} shop[MAXSHOP];

struct Area {
    int shop_id;
    int ener_type;
    Area() {
        shop_id = 0;
        ener_type = 0;
    }
} area[MAXAREA];

struct Edge {
    int fr_inst_id;
    int to_inst_id;
    int edge_type;
    Edge() {
        fr_inst_id = 0;
        to_inst_id = 0;
        edge_type = 0;
    }
} edge[MAXEDGE];





int main(int argc, char *argv[]) {
    begin_time = clock();
    // local or oj
    ::info = "info";
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
    // end work

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
    window[first_cir - 1].cir_window_id = 0;
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