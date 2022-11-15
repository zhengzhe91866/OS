#ifndef MYTHREAD
#define MYTHREAD

#include "_myPack.h"

class _myThread {
public:
    //constructor
    _myThread();
    _myThread(string d_id, string line);

    //thread
    void create_thread(vector<_myThread>& data);
    void join_thread();

    //split and find
    vector<string> split(string line);
    bool find(const string& token);

    //set term frequency array
    void setArray(const vector<string>& ret);

    //get functions
    size_t get_tid();
    string get_docid();
    size_t get_word_size();
    vector<int>& get_word();
    double get_avgCos();

    //counting functions
    void counting(vector<_myThread>& data);
    double countCos(vector<_myThread>& data, int n);

    bool start_output;
private:
    static void *run(void *arg){
        clock_t begin = clock();
        (((_myPack *)arg)->_this)->tid = pthread_self(); //get tid
        ((_myPack *)arg)->_this->counting(((_myPack *)arg)->data);

        //output
        pthread_mutex_lock( &mutex1 ); //lock
        ((_myPack *)arg)->_this->start_output = true;
        while(((_myPack *)arg)->_this->start_output);
        ((_myPack *)arg)->_this->outputArray();    //cout [array]
        ((_myPack *)arg)->_this->outputCos();      //cout cos & avg cos

        clock_t end = clock();
        ((_myPack *)arg)->_this->cost_time = (double)(end - begin) / _SC_CLK_TCK;

        ((_myPack *)arg)->_this->outputCostTime(); //
        cout << endl;
        pthread_mutex_unlock( &mutex1 ); //unlock

        return NULL;
    }

    //output functions
    void outputArray();
    void outputCos();
    void outputCostTime();

private: //normal data
    size_t tid;        //TID
    pthread_t t;       //thread
    string doc_id;     //ex: 0001
    _myPack myPack;    //

private: //counting data
    double avg_cos; //highest cosine in this thread
    double cost_time;  //CPU time in one thread.
    vector<int> words; //all of the words in this doc(sorted and counted).
    vector< pair<string, double> > cosine; //<DocID, cosine>

private: //only one this in program
    static vector<string> data_name; //all of words name in this file
    static pthread_mutex_t mutex1;   //mutex
};

vector<string> _myThread::data_name = vector<string>();
pthread_mutex_t _myThread::mutex1 = PTHREAD_MUTEX_INITIALIZER;

#endif