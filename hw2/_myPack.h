#ifndef MYPACK
#define MYPACK

class _myThread;

class _myPack{
public:
    _myPack(){
    }
    _myPack(vector<_myThread> d, _myThread* t) : data(d), _this(t){
    }
    vector<_myThread> data;
    _myThread* _this;
    _myPack& operator=(const _myPack& right){
        data = right.data;
        _this = right._this;
        return *this;
    }
};

#endif