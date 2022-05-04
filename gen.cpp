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
const ll mod1 = 1e8;
ll rand32()
{
    return (ll)rand()*(ll)65536+(ll)rand();
}
ll rand48()
{
    return rand32()*(ll)65536+(ll)rand();
}
int p[10];
int v[10];
int mp[1010][1010];
int tp[1010][1010];
int he[1010];
int main()
{
    srand(time(NULL));
    freopen("./data/case1.in","w",stdout);
    cout<<100<<endl;
    for(int i=0;i<5;i++)
    {
        cout<<rand()%100<<" ";
    }
    cout<<endl;
    int shop_n=60;
    int area_n=shop_n*3;
    int window_n=100;
    int cir=8;
    cout<<shop_n<<endl;
    cout<<area_n<<endl;
    for(int i=0;i<area_n;++i)
    {
        cout<<rand()%shop_n<<" "<<rand()%5<<endl;
    }
    cout<<cir<<endl;
    int fcir=20;
    cout<<fcir<<endl;
    cout<<window_n<<endl;
    for(int i=0;i<window_n;++i)
    {
        int j=0;
        if(i>=fcir) j=rand()%2;
        cout<<j<<" "<<rand()%shop_n<<" "<<rand()%200<<" ";
        for(int j=0;j<3;j++) p[j]=j;
        random_shuffle(p,p+3);
        v[0]=v[1]=v[2]=0;
        if(rand()%2) v[p[0]]=v[p[1]]=1;
        else v[p[0]]=1;
        for(int j=0;j<3;j++) cout<<v[j]<<" ";
        cout<<endl;
    }   
    int inst=200;
    int k=80;
    cout<<inst<<endl;
    for(int i=0;i<inst;i++)
    {
        cout<<rand()%3<<" ";
        for(int j=0;j<5;j++)
            cout<<rand32()%mod1<<" ";
        cout<<endl;
    }
    for(int i=50;i<=99;i++) mp[i][i+1]=1;
    int e2 = 0;
    for(int i=0;i<10;i++)
    {
        int l=i*k;
        int r=min(inst-1,(i+1)*k-1);
        for(int u=l;u<=r;u++)
            for(int v=u+1;v<=r;v++)
            {
                if(e2==1000) continue;
                if(rand()%15==0)
                {
                    ++e2;
                    mp[u][v]=1;
                }
            }
        if(r==inst-1) break;
    }
    int e0=0,e1=0;
    for(int i=0;i<inst;i++)
        for(int j=0;j<inst;j++)
        {
            if(mp[i][j]==0) continue;
            if(rand()%20==0) tp[i][j]=1;
            e0++;
            if(50<=i&&i<=99&&i+1==j) e1++,he[e1]=e0;
        }
    cout<<e0<<endl;
    for(int i=0;i<inst;i++)
        for(int j=0;j<inst;j++)
        {
            if(mp[i][j]==0) continue;
            cout<<tp[i][j]<<" "<<i<<" "<<j<<endl;
        }
    cout<<e1<<endl;
    for(int i=1;i<=e1;i++) cout<<he[i]<<" ";
    return 0;
}