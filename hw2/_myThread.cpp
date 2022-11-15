#include "_myThread.h"

_myThread::_myThread() : avg_cos(0), start_output(false), words(vector<int>()), cosine(vector<pair<string, double>>())
{
}
_myThread::_myThread(string d_id, string line) : doc_id(d_id), avg_cos(0), start_output(false), words(vector<int>()), cosine(vector<pair<string, double>>())
{
    vector<char> marks = {',', '.', ';', ':', '\'', '\"', '_', '-'};
    for (int i = 0; i < line.size(); i++)
        for (int k = 0; k < marks.size(); k++)
            if (line[i] == marks[k])
                line[i] = ' ';

    vector<string> ret = split(line);
    setArray(ret);
}

void _myThread::create_thread(vector<_myThread> &data) {
    myPack = {data, this};
    if (pthread_create(&t, NULL, run, &myPack) != 0) {
        cerr << "Error: pthread_create" << endl;
    }
}
void _myThread::join_thread()
{
    pthread_join(t, NULL);
}

// split and find
vector<string> _myThread::split(string line)
{
    vector<string> res;
    string token;
    size_t pos = 0;

    while ((pos = line.find(" ")) != string::npos)
    {
        token = line.substr(0, pos);
        res.emplace_back(token);
        if (!find(token))
            data_name.emplace_back(token); // unordered_set insert new element

        line.erase(0, pos + 1);
    }

    token = line.substr(0, pos);
    res.emplace_back(token);
    if (!find(token))
        data_name.emplace_back(token);

    return res;
}
bool _myThread::find(const string &token)
{
    for (int i = 0; i < data_name.size(); i++)
        if (data_name[i] == token)
            return true;
    return false;
}

// set term frequency array
void _myThread::setArray(const vector<string> &ret)
{
    words.resize(data_name.size(), 0);
    for (int i = 0; i < ret.size(); i++)
    {
        for (int k = 0; k < data_name.size(); k++)
            if (data_name[k] == ret[i])
            {
                words[k]++;
            }
    }
}

// get functions
size_t _myThread::get_tid()
{
    return tid;
}
string _myThread::get_docid()
{
    return doc_id;
}
size_t _myThread::get_word_size()
{
    return words.size();
}
vector<int> &_myThread::get_word()
{
    return words;
}
double _myThread::get_avgCos()
{
    return avg_cos;
}

// counting functions
void _myThread::counting(vector<_myThread> &data)
{
    for (int i = 0; i < data.size(); i++)
    {
        if (data[i].get_docid() != this->get_docid())
        {
            cosine.emplace_back(pair<string, double>(data[i].get_docid(), countCos(data, i)));
        }
    }
    avg_cos /= (data.size() - 1);
}
double _myThread::countCos(vector<_myThread> &data, int n)
{
    int size = max(words.size(), data[n].get_word_size());
    while (words.size() < size)
        words.emplace_back(0);
    while (data[n].get_word_size() < size)
        data[n].get_word().emplace_back(0);

    double sum = 0, vs = 0, vx = 0;
    for (int i = 0; i < size; i++)
    {
        sum += words[i] * data[n].get_word()[i];
        vs += pow(words[i], 2);
        vx += pow(data[n].get_word()[i], 2);
    }

    sum /= (double)((double)sqrt(vs) * (double)sqrt(vx));
    avg_cos += sum;
    return sum;
}

// output functions
void _myThread::outputArray()
{
    printf("[TID=%lu]DocID:\033[31m%s\033[0m [", tid, doc_id.c_str());
    for (int i = 1; i < words.size(); i++)
        cout << "," << words[i];
    for (int i = words.size(); i < data_name.size(); i++)
        cout << ",0";
    cout << "]" << endl;
}
void _myThread::outputCos()
{
    for (int i = 0; i < cosine.size(); i++)
    {
        cout << "[TID=" << tid << "] cosine(" << doc_id << "," << cosine[i].first << ")=" << cosine[i].second << endl;
    }
    cout << "[TID=" << tid << "] Avg_cosine: ";
    printf("\033[33m%.3f\033[0m\n", avg_cos);
}
void _myThread::outputCostTime()
{
    printf("[TID=%lu] CPU time: \033[36m%.1f\033[0mms\n", tid, cost_time);
}