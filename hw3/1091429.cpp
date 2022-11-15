/*
use "g++ main.cpp -o main -pthread" to compile
and "./main data.txt" to run
*/
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <fstream>
#include <math.h>
#include <time.h>
#include <limits.h>
#include <unordered_set>
#include <unistd.h>
#include <random>
#include <algorithm>

using namespace std;

bool FINISH = false;
int PROD_NUM = 0; // how many producers this program has.
const int PARTS_SIZE = 3;
const int TOTAL_PROD_DRONE = 50; // I need _ drones.
vector<string> part_name = {"battery  ", "aircraft ", "propeller"}; 
int color[3] = {31, 32, 34}; // 31 red 32 green 34 blue
pthread_mutex_t global_lock;

class Producer { // child thread
public:
    Producer(bool* f): front(f), lock(true), num_drone(0) {
        PROD_NUM++;
        number = PROD_NUM;
        for (int i = 0; i < PARTS_SIZE; i++) {
            parts[i] = false;
        }

        if (number == 1)
            parts[1] = true; // aircraft
    }

    void create_thread() {
        if (pthread_create(&t, NULL, run, this) != 0) {
            cerr << "Error: pthread_create" << endl;
        }
    }

    void join_thread() {
        pthread_join(t, NULL);
    }

    int need(){
        int x = -1; // try to let all of the producers use the same instructions
        for(int i = 0; i < PARTS_SIZE; i++){
            if(front[i] && !parts[i]){
                x = i;
            }
            else{
                x = x; // useless
            }
        }
        return x; // and I don't know does it work or not
    }

    void getPart(int i){
        parts[i] = true;
        front[i] = false;
    }

    int getNumDrones(){
        return num_drone;
    }

    bool canGet(int i){
        return front[i];
    }

    void init(){
        for(int i = 0; i < PARTS_SIZE; i++){
            parts[i] = false;
        }
        if(number == 1)
            parts[1] = true;
        num_drone++;
        remain_drone--;
    }

    bool complete(){
        for(int i = 0; i < PARTS_SIZE; i++){
            if(!parts[i])
                return false;
        }
        return true;
    }

    void outputGetSomething(int i) {
        string preset;
        if(number == 1)
            preset = " (aircraft)";
        printf("\033[33mProducer %d%s\033[0m: get \033[%dm%s\033[0m ", number, preset.c_str(), color[i], part_name[i].c_str());
        outputArray();
        cout << endl;
    }

    void outputDroneNum(){
        string preset;
        if(number == 1)
            preset = " (aircraft)";
        printf("\033[46mProducer %d%s: OK, %d drone(s)\033[0m\n", number, preset.c_str(), num_drone);
    }

    void outputArray(){
        cout << "[";
        for(int i = 0; i < PARTS_SIZE; i++)
            cout << " " << (parts[i] ? "1" : "0");
        cout << " ]";
    }

    void outputFinalMsg(){
        string preset;
        if(number == 1)
            preset = " (aircraft)";
        printf("\033[33mProducer %d%s\033[0m: produced %d drone(s)\n", number, preset.c_str(), num_drone);
    }

    bool lock;
    static int remain_drone;
private:
    static void *run(void *arg) {
        while(!FINISH && remain_drone > 0){
            int n = ((Producer *)arg)->need();
            if(n != -1) {
                pthread_mutex_lock( &mutex1 ); //lock
                pthread_mutex_lock( &global_lock ); //lock
                if(FINISH) {
                    pthread_mutex_unlock( &global_lock ); //unlock
                    pthread_mutex_unlock( &mutex1 ); //unlock
                    break;
                }
                if(((Producer *)arg)->canGet(n)){
                    ((Producer *)arg)->getPart(n);
                    ((Producer *)arg)->outputGetSomething(n);
                    if(((Producer *)arg)->complete()){       // create a drone
                        ((Producer *)arg)->init();           // init the array (all false)
                        ((Producer *)arg)->outputDroneNum(); // now this producer has ? drone
                    }
                }
                pthread_mutex_unlock( &global_lock ); //unlock
                pthread_mutex_unlock( &mutex1 ); //unlock
            }
        }
        if(remain_drone == 0)
            FINISH = true;
        return NULL;
    }
    pthread_t t;            //thread
    int number;             // 1 ~ 3
    bool parts[PARTS_SIZE]; // battery aircraft propeller
    int num_drone;
    bool *front;
    
    static pthread_mutex_t mutex1;   //mutex
};

class Dispatcher { // main thread
public:
    Dispatcher() {
        front = new bool[PARTS_SIZE];
        for(int i = 0; i < PARTS_SIZE; i++)
            front[i] = false;

        for(int i = 0; i < PARTS_SIZE; i++)
            num_parts[i] = 0;

        for(int i = 0; i < PARTS_SIZE; i++)
            producers.push_back(Producer(front));
    }
    ~Dispatcher(){
        delete front;
    }

    void supply() { // which part dispatcher produced. (0, 1, 2)
        time_t t = time(0);
        srand(t);

        int n;
        do {
            n = rand() % PARTS_SIZE;
            srand(++t);
        } while(front[n]);

        if(front[n])
            return;
        pthread_mutex_lock( &global_lock ); //lock

        front[n] = true;
        num_parts[n]++;
        if(!FINISH)output(n);

        pthread_mutex_unlock( &global_lock ); //unlock
    }

    void run() {
        int size = producers.size();
        for (int i = 0; i < size; i++)
            producers[i].create_thread();
        mainThreadInfo();
        for(int i = 0; i < size; i++)
            producers[i].join_thread();
        outputPartTotal();
        outputProdDrones();
    }

    void mainThreadInfo() {
        while(producers[0].remain_drone > 0){
            supply();
        }
        FINISH = true;
    }

    static bool cmp(pair<int, int> a, pair<int, int> b){
        return a.second > b.second;
    }

    void output(int i) {
        printf("\033[37mDispatcher\033[0m: \033[%dm%s\033[0m ", color[i], part_name[i].c_str());
        outputArray();
        cout << endl;
    }
    void outputArray(){
        cout << "[";
        for(int i = 0; i < PARTS_SIZE; i++)
            cout << " " << (front[i] ? "1" : "0");
        cout << " ]";
    }
    void outputPartTotal(){
        printf("\n\033[37mDispatcher\033[0m produced:\n");
        for(int i = 0; i< PARTS_SIZE; i++){
            printf("\033[%dm%s\033[0m : %d\n", color[i], part_name[i].c_str(), num_parts[i]);
        }
    }
    void outputProdDrones(){
        cout << endl;
        vector<pair<int, int> > data(PARTS_SIZE, pair<int, int> (1, 0));
        for(int i = 0; i < data.size(); i++){
            data[i] = pair<int, int>(i, producers[i].getNumDrones());
        }

        sort(data.begin(), data.end(), cmp);

        for(int i = 0; i < PARTS_SIZE; i++){
            producers[data[i].first].outputFinalMsg();
        }
    }
private:
    vector<Producer> producers; //save the producers' data
    int num_parts[PARTS_SIZE];  //counting the number of parts
    bool *front;
};

pthread_mutex_t Producer::mutex1 = PTHREAD_MUTEX_INITIALIZER;
int Producer::remain_drone = TOTAL_PROD_DRONE;

int main(int argc, char *argv[]) {
    int mode = (int)(*argv[1] - '0');
    int seed = (int)(*argv[2] - '0');
    
    if(mode == 0){
        //normal mode
        Dispatcher process;
        process.run();
    }
    else{
        //advanced mode
    }
}