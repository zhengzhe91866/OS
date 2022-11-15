/*
use "g++ hw.cpp -o out -lrt" to compile
and "./out x y 0" to run
(x, y is random seed of parent and child)
*/
#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>

using namespace std;

const int MAP_SIZE = 4;

typedef struct region
{
	bool endGame;
	int winner;
	int _pid;
	int _cid;
	int pbombs;
	int cbombs;
	int guessX;
	int guessY;
	int statue;
}REGION;

class Process{
public:
	Process(int s, int p, int st) : seed(s), pid(p), status(st), hit(0), bombs(0), power(false){
		for(int i = 0; i < MAP_SIZE; i++)
			for(int k = 0; k < MAP_SIZE; k++)
				map[i][k] = 0, enemy[i][k] = false;
	}

	////////////////////////////////////////////////////////////////////////////////////

	void setSeed(int s){
		seed = s;
	}

	void setPid(int p){
		pid = p;
	}

	void setStat(int st){
		status = st;
	}

	void setBoat(int x, int y){
		map[x][y] = 1;
	}

	////////////////////////////////////////////////////////////////////////////////////

	void outputMsg(){
		cout << "[" << pid << (status == 2 ? " Parent]: " :  " Child]: ");
	}

	void outputSeed(){
		outputMsg();
		cout << "Random Seed " << seed << endl;
	}
	void outputBoat(){
		outputMsg();
		cout << "The gunboat: ";
		for(int i = 0; i < MAP_SIZE; i++)
			for(int k = 0; k < MAP_SIZE; k++)
				if(map[i][k] == 1)
					cout << "(" << i << "," << k << ")";
		cout << endl;
	}
	void outputBomb(int x, int y){
		outputMsg();
		cout << "bombing " << "(" << x << "," << y << ")" << endl;
		bombs++;
	}
	void outputMiss(){
		outputMsg();
		cout << "missed" << endl;
	}
	void outputHit(){
		outputMsg();	
		cout << "hit" << endl;
	}
	void outputSink(){
		outputMsg();	
		cout << "hit and sinking" << endl;
		outputMsg();
	}
	////////////////////////////////////////////////////////////////////////////////////

	bool isBoat(int x, int y){
		return map[x][y] == 1 ? true : false;
	}
	bool canSetBoat(int x, int y){
		return map[x][y] == 0 ? true : false;
	}
	void bomb(int x, int y){
		enemy[x][y] = true;
		outputBomb(x, y);
	}
	bool beenBomb(int x, int y){
		hit++;
		map[x][y] = -1;
		(hit > 1) ? outputSink() : outputHit();
		return (hit > 1) ? true : false;
	}
	bool alreadyBomb(int x, int y){
		return enemy[x][y] ? true : false;
	}
	int findSeed(){
		return seed;
	}
private:
	int seed;
	int hit;
	int pid;
	int status;
	int bombs;
	int map[MAP_SIZE][MAP_SIZE]; // 0:nothing, 1:boat, -1:bomb
	bool enemy[MAP_SIZE][MAP_SIZE]; // 0:nothing, 1:bomb
	bool power;
};

int main(int argc, char* argv[])
{
    int parentSeed = atoi(argv[1]);
    int childSeed = atoi(argv[2]);
	//int mode = (int)*argv[3];

	int shm_fd = shm_open("myshare", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);

	REGION* ptr = (REGION*)mmap(0, sizeof(REGION), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
	ptr->statue = 1;
	ptr->endGame = false;

	pid_t pid = fork();
	Process *data;
	int x, y, size = 2;
	ptr->pbombs = 0, ptr->cbombs = 0;

	if (pid == 0) {                     // child
		ptr->_cid = getpid();
		data = new Process(childSeed, ptr->_cid, 1);

		while (ptr->statue == 1);
		data->outputSeed();

		while(size > 0){
			srand(++childSeed);
			x = rand() % MAP_SIZE;
			y = rand() % MAP_SIZE;
			if(data->canSetBoat(x, y)) data->setBoat(x, y), size--;
		}

		ptr->statue = 1;
		while (ptr->statue == 1);

		data->outputBoat();

		ptr->statue = 1;
		while (ptr->statue == 1);

		///////////////////////// now game start /////////////////////////

		for (int i = 0; i < 16; ++i){
			while (ptr->statue == 1);
			if(ptr->endGame) break;

			if (data->isBoat(ptr->guessX, ptr->guessY)){
				ptr->endGame = data->beenBomb(x, y);
				if(ptr->endGame) {
					cout << ptr->_pid << " wins with " << ptr->pbombs << " bombs" << endl;
					ptr->winner = 1;
					ptr->statue = 3;
					break;
				}
			}
			else if (i < 15){
				data->outputMiss();
			}

			while(true){
				x = rand() % MAP_SIZE;
				y = rand() % MAP_SIZE;
				if(!data->alreadyBomb(x, y)) break;
			}
			data->bomb(x, y);
			ptr->cbombs++;
			ptr->guessX = x;
			ptr->guessY = y;

			ptr->statue = 1;
		}
	}
	else if (pid > 0){                        // parent
		ptr->_pid = getpid();
		data = new Process(parentSeed, ptr->_pid, 2);

		data->outputSeed();
		while(size > 0){
			x = rand() % MAP_SIZE;
			y = rand() % MAP_SIZE;
			if(data->canSetBoat(x, y)) data->setBoat(x, y), size--;
		}

		ptr->statue = 2;
		while (ptr->statue == 2);

		data->outputBoat();

		ptr->statue = 2;
		while (ptr->statue == 2);

		for (int i = 0; i < 16; ++i){
			if(ptr->endGame) break;
			while(true){
				srand(++parentSeed);
				x = rand() % MAP_SIZE;
				y = rand() % MAP_SIZE;
				if(!data->alreadyBomb(x, y)) break;
			}
			data->bomb(x, y);
			ptr->pbombs++;
			ptr->guessX = x;
			ptr->guessY = y;

			ptr->statue = 2;
			while (ptr->statue == 2);
			if(ptr->endGame) break;

			if (data->isBoat(ptr->guessX, ptr->guessY)){
				ptr->endGame = data->beenBomb(x, y);
				if(ptr->endGame) {
					cout << ptr->_cid << " wins with " << ptr->cbombs << " bombs" << endl;
					ptr->winner = 0;
					ptr->statue = 3;
					break;
				}
			}
			else if (i < 15){
				data->outputMiss();
			}
		}
	}
	
	/*if(ptr->statue == 3)
		if(ptr->winner == 0 && ptr->_cid != 0) {
			cout << ptr->_cid << " wins with " << ptr->cbombs << " bombs" << endl;
		}
		else if(ptr->winner == 1 && ptr->_pid != 0){
			cout << ptr->_pid << " wins with " << ptr->pbombs << " bombs" << endl;
		}*/
	/*if(ptr->winner == 1 && ppid != 0){
		cout << ppid << " wins with " << pbombs << " bombs" << endl;
	}
	else if(ptr->winner == 0 && cpid != 0) {
		cout << cpid << " wins with " << cbombs << " bombs" << endl;
	}*/
	return 0;
}
