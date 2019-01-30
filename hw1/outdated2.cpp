#include <iostream>	
#include <list>
#include <queue>
#include <string>
#include<fstream>
#include<vector>
#include<iomanip>
using namespace std;

int total = 0;	//# processes that completed
int ssdtotal = 0; //# ssd accesses
double ssdavg = 0;	//total ssd access time in ms
int elapsedtime = 0; //total elapsed time
double ssdutilization = 0; //ssd utilization percentage
double coreutil = 0; //total value of core time values
double ssdutil = 0;//total value of ssd time values

				   //make priority queue list from least to greatest
struct compare
{
	bool operator()(const int & a, const int & b)
	{
		return a>b;
	}
};
//processnum = process number, etime = end time, stime = start time, status = "IDLE", "READY", "TERMINATED", "RUNNING"
struct READ {
	int processnum;
	int etime;
	int stime;
};
//busy = false (not busy), pnum = process number, stime = start time, etime = end time, corenum = core number, status = "IDLE", "READY", "TERMINATED", "RUNNING"
struct coreInfo {
	bool busy;
	int pnum;
	int stime;
	int etime;
	int corenum;
	string status;
};
//cmd = command, value = time being spent, status = "IDLE", "READY", "TERMINATED", "RUNNING"

struct newInfo {
	int pnum;
	int etime;
};

void inserthdd(vector<READ> &hdd, queue<int> &HDDQueue, vector<queue<string>> &ptable, int index, int &time) {
	string cmd, value;
	value = ptable[index].front();

	cout << "--Process " << index << " requests input from user at time " << time << " ms for " << value << " ms" << endl;

	if (hdd[0].processnum != -1) {				//push to hdd queue
		HDDQueue.push(index);
		HDDQueue.push(stoi(value));
		cout << "--Process " << index << " must wait for user " << endl;
		cout << "--Input queue now contains " << HDDQueue.size() / 2 << " process(es) waiting for the user" << endl;
		//cout << "==HDD IN USE BY PROCESS " << hdd[0].processnum << " UNTIL T: " << hdd[0].etime << endl;
	}
	else {							//push to hdd if empty		
		ptable[index].pop();					//pop number value only if successfully enters hdd, if its in queue then dont pop!
		hdd[0].etime = time + stoi(value);
		hdd[0].processnum = index;
		hdd[0].stime = time;
		cout << "--Process " << index << " will complete input at time " << hdd[0].etime << " ms" << endl;
	}
}

void insertssd(vector<READ> &ssd, queue<int> &SSDQueue, vector<queue<string>> &ptable, int index, int &time) {
	string cmd, value;
	value = ptable[index].front();

	cout << "--Process " << index << " requests SSD access at time " << time << " ms for " << value << " ms" << endl;

	if (ssd[0].processnum != -1) {				//push to ssd queue
		SSDQueue.push(index);
		SSDQueue.push(stoi(value));
		cout << "--Process " << index << " must wait for a SSD " << endl;
		cout << "--SSD queue now contains " << SSDQueue.size() / 2 << " process(es) waiting for a SSD" << endl;
		//cout << "==SSD IN USE BY PROCESS " << ssd[0].processnum << " UNTIL T: " << ssd[0].etime << endl;
	}
	else {							//push to ssd if empty
		ssdtotal++;					//increment ssd total # accesses
		ssdavg = ssdavg + (time + stoi(value));		//ssdavg test, add etime on successful push to ssd
		ssdutil = ssdutil + stoi(value);
		ptable[index].pop();				//only pop number value from table if it successfully makes it to destination, if it makes it to queue then dont pop!
		ssd[0].etime = time + stoi(value);
		ssd[0].processnum = index;
		ssd[0].stime = time;
		cout << "--Process " << index << " will release the SSD at time " << ssd[0].etime << " ms" << endl;
	}
}

void updatecore(vector<coreInfo> &cores, vector<queue<string>> &ptable, queue<int> &readyQueue, int pnum, int &time) {
	int index = 0;
	bool free = false;
	string cmd, value;
	for (int i = 0; i < cores.size(); i++) {			//find a free core, update free as true if so
		if (cores[i].busy == false) {
			index = i;
			free = true;
			break;
		}
	}

	value = ptable[pnum].front();

	cout << "--Process " << pnum << " requests a core at time " << time << " ms for " << value << " ms" << endl;

	if (free == true) {
		coreutil = coreutil + stoi(value);
		cores[index].etime = time + stoi(value);
		cores[index].busy = true;
		cores[index].pnum = pnum;
		cores[index].stime = time;
		cout << "--Process " << pnum << " will release a core at time " << time + stoi(value) << " ms" << endl;
		ptable[pnum].pop();			//make sure to remove from ptable since its being used, if it goes to queue leave it in ptable for when it can be processed
	}
	else {
		cout << "--Process " << pnum << " must wait for a core" << endl;
		readyQueue.push(pnum);					//insert process #
		readyQueue.push(stoi(value));				//insert CORE time

		cout << "--Ready queue now contains " << readyQueue.size() / 2 << " process(es) waiting for a core" << endl;
	}

}
//0 if pq is empty, 1 if new is first, 2 if ssd is first, 3 if input is first, 4 if core is first
int priority(priority_queue <int, vector<int>, compare> &pq, vector<coreInfo> cores, vector<READ> ssd, vector<READ> hdd, queue<int> &table) {
	bool SSD = false;
	bool HDD = false;
	bool CORE = false;
	bool NEW = false;

	while (!pq.empty()) {	//make sure pq is empty
		pq.pop();
	}

	if (!table.empty()) {
		pq.push(table.front());
		NEW = true;
	}

	if (ssd[0].processnum != -1) {
		pq.push(ssd[0].etime);
		SSD = true;
	}
	if (hdd[0].processnum != -1) {
		pq.push(hdd[0].etime);
		HDD = true;
	}
	for (int i = 0; i < cores.size(); i++) {
		if (cores[i].busy == true) {
			pq.push(cores[i].etime);
			CORE = true;
		}
	}

	if (pq.empty()) {
		cout << "PRIORITY QUEUE WAS EMPTY!" << endl;
		return 0;
	}
	int x;
	x = pq.top();
	if (CORE == true) {
		for (int i = 0; i < cores.size(); i++) {
			if (x == cores[i].etime) {
				return 4;
			}
		}
	}
	if (x = ssd[0].etime && SSD == true)
		return 2;
	if (x = hdd[0].etime && HDD == true)
		return 3;
	if (x = table.front() && NEW == true) {
		return 1;
	}
	return 0;
}

void loop(vector<coreInfo> &cores, queue<int> &readyQueue, queue<int> &SSDQueue, queue<int> &HDDQueue, priority_queue <int, vector<int>, compare> &pq, vector<READ> &ssd, vector<READ> &hdd, queue<int> &table, int index, vector<queue<string>> &ptable, int presult, int &time) {
	string a;
	string b, c;
	if (presult == 1) {		//update to core
		if (ptable[index].empty()) {																		//when it terminates, print out the status of processes **************************************************************************
			cout << "==Process " << index << " terminates at time " << time << " ms" << endl;
			total++;
			return;
		}
		updatecore(cores, ptable, readyQueue, index, time);
		table.pop();
	}
	else if (presult == 2) {	//update to core or terminate	
		if (ptable[index].empty()) {																		//when it terminates, print out the status of processes **************************************************************************
			cout << "==Process " << index << " terminates at time " << time << " ms" << endl;
			total++;
			return;
		}

		b = ptable[index].front();			//keep track

		ptable[index].pop();
		a = ptable[index].front();
		updatecore(cores, ptable, readyQueue, index, time);
	}
	else if (presult == 3) {	//update to core or terminate								
		if (ptable[index].empty()) {
			cout << "==Process " << index << " terminates at time " << time << " ms" << endl;
			total++;
			return;
		}

		b = ptable[index].front();			//keep track

		ptable[index].pop();
		a = ptable[index].front();
		updatecore(cores, ptable, readyQueue, index, time);
	}
	else if (presult == 4) {	//update to ssd or hdd
		if (ptable[index].empty()) {
			cout << "==Process " << index << " terminates at time " << time << " ms" << endl;
			total++;
			return;
		}
		a = ptable[index].front();
		ptable[index].pop();

		if (a == "INPUT")
			inserthdd(hdd, HDDQueue, ptable, index, time);
		if (a == "SSD") {
			ssdavg = ssdavg - time;				//ssdavg test subtract on arrival
			insertssd(ssd, SSDQueue, ptable, index, time);
		}
	}

}
//return the core # that will end the fastest, if empty return -1
int pcore(priority_queue <int, vector<int>, compare> &pq, vector<coreInfo> &cores) {
	while (!pq.empty()) {	//make sure pq is empty
		pq.pop();
	}
	for (int i = 0; i < cores.size(); i++) {
		if (cores[i].pnum != -1) {
			pq.push(cores[i].etime);
		}
	}
	int x = 0;
	if (pq.empty()) {
		cout << "priority queue is empty! -pcore, returning -1" << endl;
		return -1;
	}
	x = pq.top();

	for (int i = 0; i < cores.size(); i++) {
		if (cores[i].busy == true) {
			if (cores[i].etime == x) {
				//	cout << i << endl;						//to test if its grabbing correct core
				return i;
			}
		}
	}
	return -1;
}

int main() {
	int process = -1;
	int index = 0;
	vector<queue<string>> ptable;
	queue<int> table;

	queue<int> readyQueue;
	queue<int> SSDQueue;
	queue<int> HDDQueue;
	vector<int> timez;
	vector<coreInfo> cores;
	priority_queue <int, vector<int>, compare> pq;
	newInfo temp;
	string cmd;
	string value;
	string thing;
	vector<READ> ssd(1);
	vector<READ> hdd(1);
	int coreNum = 2;
	int x = 0;		//processing number
	int y = 0;		//value
	int presult;
	int time = 0;
	bool trick = false;
	bool step1 = false;
	bool status = false;
	bool exit = false;

	//initialize ssd and hdd
	ssd[0].processnum = -1;
	hdd[0].processnum = -1;

	
	//organize data from file into ptable, and creates our cores
							//loop used to collect all the data in file and organize it in a vector of queues where each vector is a process			
					// Set up sizes. (HEIGHT x WIDTH), first value will be process number, second value is process time
				
				cores.resize(coreNum);

				for (int i = 0; i < coreNum; i++) {	//set default to false, to represent its not busy
					cores[i].busy = false;
					cores[i].corenum = i;
					cores[i].pnum = -1;
					cores[i].stime = 0;
					cores[i].etime = 0;
				}
				process = 1;
				ptable.push_back(queue<string>());
				ptable.push_back(queue<string>());

				ptable[0].push("NEW");
				ptable[0].push("5");
				ptable[0].push("CORE"); 
				ptable[0].push("100");
				ptable[0].push("INPUT");
				ptable[0].push("5000");
				ptable[0].push("CORE");
				ptable[0].push("80");
				ptable[0].push("SSD");
				ptable[0].push("1");
				ptable[0].push("CORE");
				ptable[0].push("30");
				ptable[1].push("NEW");
				ptable[1].push("100");
				ptable[1].push("CORE"); 
				ptable[1].push("20");
				ptable[1].push("SSD");
				ptable[1].push("0");
				ptable[1].push("CORE");
				ptable[1].push("20");
		

	/*
	//prints out ptable
	for (int i = 0; i < ptable.size(); i++) {
	cout << "Process " << i << " table, status: " << stats[i].status << endl;
	cout << "-------------------" << endl;
	while (!ptable[i].empty()) {
	cout << ptable[i].front() << "\t";
	ptable[i].pop();
	cout << ptable[i].front() << endl;
	ptable[i].pop();
	}
	cout << endl;
	}
	*/

	if (process == -1)
		exit = true;

	while (!exit) {
		while (index <= process) {
			cmd = ptable[index].front();
			ptable[index].pop();
			value = ptable[index].front();
			ptable[index].pop();
			if (cmd == "NEW") {
				table.push(stoi(value));			//time it finishes
				table.push(index);					//process #
				cout << "--ARRIVAL event for process " << index << " at time " << value << " ms" << endl;
				temp.etime = stoi(value);
				temp.pnum = index;
				cout << "Process " << index << " starts at time " << value << " ms" << endl;
				ptable[index].pop();
			}
			index++;
		}

		presult = priority(pq, cores, ssd, hdd, table);

		if (presult == 1) {
			if (index >= process + 1) {
				index = process;
				trick = true;
			}

			x = table.front();		// NEW value
			table.pop();
			y = table.front();		//process #
			table.pop();
			time = x;
			table.push(x);			//will be popped out in the loop function below
			if (table.size() > 1) {
				for (int i = 0; i < table.size() - 1; i++) {	//function to move everything bellow value x that we just pushed into queue since we want it at the front
					x = table.front();
					table.pop();
					table.push(x);
				}
			}
			loop(cores, readyQueue, SSDQueue, HDDQueue, pq, ssd, hdd, table, y, ptable, presult, time);
			if (trick == true)
				index = process + 1;
		}
		else if (presult == 2) {				//at end of loop update, update ssd to not busy
			time = ssd[0].etime;
			loop(cores, readyQueue, SSDQueue, HDDQueue, pq, ssd, hdd, table, ssd[0].processnum, ptable, presult, time);
			ssd[0].processnum = -1;
			if (!SSDQueue.empty()) {
				x = SSDQueue.front();
				SSDQueue.pop();
				y = SSDQueue.front();
				SSDQueue.pop();
				insertssd(ssd, SSDQueue, ptable, x, time);
			}
			//else 		
			//cout << "ssd queue was empty, ssd is now idle" << endl;						
		}
		else if (presult == 3) {
			time = hdd[0].etime;
			loop(cores, readyQueue, SSDQueue, HDDQueue, pq, ssd, hdd, table, hdd[0].processnum, ptable, presult, time);
			hdd[0].processnum = -1;
			if (!HDDQueue.empty()) {
				x = HDDQueue.front();		//process # from hdd queue
				HDDQueue.pop();
				y = HDDQueue.front();		//value from hdd queue make sure to pop both numbers
				HDDQueue.pop();
				inserthdd(hdd, HDDQueue, ptable, x, time);
			}
			//else 		
			//cout << "hdd queue was empty, hdd is now idle" << endl;	
		}
		else if (presult == 4) {

			x = pcore(pq, cores);	//what core number is the one ready to exit
			time = cores[x].etime;

			loop(cores, readyQueue, SSDQueue, HDDQueue, pq, ssd, hdd, table, cores[x].pnum, ptable, presult, time);		//move process from core to ssd/hdd
			cores[x].busy = false;			//since core is now free
			if (!readyQueue.empty()) {
				x = readyQueue.front();		//process number from readyqueue
				readyQueue.pop();
				y = readyQueue.front();		//value from readyqueue, make sure to pop both in this case since a core is now free
				readyQueue.pop();
				updatecore(cores, ptable, readyQueue, x, time);
			}
			else {
				cores[x].busy = false;
				cores[x].pnum = -1;
				cores[x].stime = time;
				//cout << "ready queue was empty, core # " << x << " is idle" << endl;
			}
		}

		else {
			//cout << "completed" << endl;
			//system("pause");
			break;
		}
		if (index <= process) {
			index++;
		}
		cout << endl;
	}
	cout << "\nSUMMARY:" << endl;
	cout << "Number of processes that completed: " << total << endl;
	cout << "Total number of SSD accesses: " << ssdtotal << endl;
	cout << "Average SSD acess time: " << ssdavg / ssdtotal << " ms" << endl;
	cout << "Total elapsed time: " << time << " ms" << endl;
	//cout << "Core utilization: " << (coreutil / time) * 100 << " percent" << endl;
	//cout << "SSD utilization: " << (ssdutil / time) * 100 << " percent" << endl;
	printf("Core utilization: %.2f percent\n", (coreutil / time) * 100);
	printf("SSD utilization: %.2f percent\n", (ssdutil / time) * 100);

	system("pause");
	return 0;
}