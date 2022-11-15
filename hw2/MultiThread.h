#ifndef MYMULTITHREAD
#define MYMULTITHREAD

#include "_myThread.cpp"

pthread_mutex_t main_mutex = PTHREAD_MUTEX_INITIALIZER;

class MultiThread {
public:
    //add a new thread
    void emplace_back(_myThread value){
        data.emplace_back(value);
    }

    //load this program
    void load(char filename[]){
        ifstream fin(filename);
        string id, line;
        while(getline(fin, id) && id != ""){
            getline(fin, line);
            data.emplace_back(_myThread(id, line));
        }
        fin.close();
    }

    void run(){
        //main thread
        clock_t begin = clock();

         //child thread
        int size = data.size();
        for(int i = 0; i < size; i++)
            create_thread(i);
        mainThreadInfo();
        for(int i = 0; i < size; i++)
            join_thread(i);

        //
        if(data.size() > 1){
            highest_avg_cos = data[0].get_avgCos();
            KeyDocID = data[0].get_docid();
            for(int i = 1; i < size; i++){
                double cur_cos = data[i].get_avgCos();
                if(highest_avg_cos < cur_cos){ //compare
                    highest_avg_cos = cur_cos;
                    KeyDocID = data[i].get_docid();
                }
            }
        }
        else{
            highest_avg_cos = data[0].get_avgCos();
            KeyDocID = data[0].get_docid();
        }
        

        clock_t end = clock();
        costTime(begin, end);

        outputResult(); //last two [main thread]
    }

    size_t size(){
        return data.size();
    }

    void create_thread(int n){
        data[n].create_thread(data);
    }
    void join_thread(int n){
        data[n].join_thread();
    }
    void mainThreadInfo(){
        int i = data.size();
        while(i > 0){
            for(int k = 0; k < data.size(); k++){
                if(data[k].start_output){
                    printf("[Main thread]: create TID:%lu, DocID:\033[31m%s\033[0m\n", data[k].get_tid(), data[k].get_docid().c_str());
                    data[k].start_output = false;
                    i--;
                }
            }
        }
    }

    //count CPU time
    void costTime(int begin, int end){
        cost_time = (double)(end - begin) / _SC_CLK_TCK; 
    }

    void outputResult(){
        printf("[Main thread] KeyDocID:\033[31m%s\033[0m Highest Average Cosine: \033[33m%.3f\033[0mms\n", KeyDocID.c_str(), highest_avg_cos);
        cout << "[Main thread] CPU time: " << cost_time << "ms" << endl;
    }

private:
    double cost_time;       //this program's CPU time.
    double highest_avg_cos; //highest cosine in all datas.
    string KeyDocID;        //the thread's docID which has highest average cosine.
    vector<_myThread> data; //initialize child thread in vector.
};

#endif