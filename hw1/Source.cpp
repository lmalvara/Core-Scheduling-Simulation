//Imported
#include <iostream>
#include <vector>
#include <queue>
#include <cstring>
#include <fstream>
#include<string>
using namespace std;

struct Data {
	int line, time;
	string name;
};

struct process {
	int procnum, startline, endline, currentline, starttime;
	string state;
};

struct event {
	int procnum, corenum, starttime;
	string ename;
};

void inputData();
void insertEvent(event e);
void arrival(int a);
void print(int n, int c);
int getFreeCore(bool bt[]);
int numBusyCores(bool bt[]);
void showQueue(queue<int> q);
void showProcessTable(int n);
void showArrival(int a);

//Global Variables
string action; //Holds input data names
int num, nCores; // num is container for input data times
int line = -1; //line = -2 so we don't count NCORE and SLICE into our data
int pronum = 0; //a counter to establish process numbers when inserting into process tables
int globalTime = 0; // overall clock
bool SSDBusy = false; //checks to see if disk is currently busy
vector<Data> dataTable; //acts as table for input data
vector<process> processTable; //acts as table for processes
vector<event> eventList; //orignally used as a table but changed into a priority queue over time
queue<int> coreQueue;
queue<int> SSDQueue;

int main() {
	inputData();
	bool busyTimes[2];
	for (int x = 0; x < nCores; x++) {
		busyTimes[x] = false;
	}

	event tmp;
	process p;
	event next;

	//Start of Simulation
	while (eventList.size() != 0) {
		next = eventList.back();
		eventList.pop_back();
		globalTime = next.starttime;
		p = processTable[next.procnum];
		tmp.procnum = next.procnum;
		tmp.corenum = next.corenum;

		//Event Names
		if (next.ename == "NEW")
		{
			next.ename = "CORE";
			p.currentline++;
		}

		if (next.ename == "END") {
			if (dataTable[p.currentline].name == "CORE") {
				p.currentline++;
				if (coreQueue.empty())
				{
					busyTimes[next.corenum] = false;
				}
				else
				{
					tmp.procnum = coreQueue.front();
					coreQueue.pop();
					tmp.starttime = globalTime + dataTable[processTable[tmp.procnum].currentline].time;

					tmp.ename = "END";
					insertEvent(tmp);
					processTable[tmp.procnum].state = "RUNNING";
					tmp.procnum = next.procnum;
				}
				if (p.currentline>p.endline)
				{
					tmp.ename = "TERM";
					tmp.starttime = globalTime;
					insertEvent(tmp);
				}
				else if (dataTable[p.currentline].name == "CORE")
				{
					int a = getFreeCore(busyTimes);
					if (a != -1)
					{
						tmp.ename = "END";
						tmp.starttime = globalTime + dataTable[p.currentline].time;
						tmp.corenum = a;
						insertEvent(tmp);
						busyTimes[next.corenum] = true;
					}
					else
					{
						coreQueue.push(next.procnum);
						p.state = "READY";
					}
				}
				else
				{
					tmp.ename = dataTable[p.currentline].name;
					tmp.starttime = globalTime;
					insertEvent(tmp);
					p.state = "BlOCKED";
				}
			}
			else if (dataTable[p.currentline].name == "SSD")
			{
				p.currentline++;
				if (SSDQueue.empty())
				{
					SSDBusy = false;
				}
				else
				{
					tmp.procnum = SSDQueue.front();
					SSDQueue.pop();
					tmp.starttime = globalTime + dataTable[processTable[tmp.procnum].currentline].time;
					tmp.ename = "END";
					insertEvent(tmp);
					tmp.procnum = next.procnum;
				}
				if (p.currentline>p.endline) //Checks if current process needs to be terminated
				{
					tmp.ename = "TERM";
					tmp.starttime = globalTime;
					insertEvent(tmp);
				}
				else //if not we insert process's next step
				{
					tmp.ename = dataTable[p.currentline].name;
					tmp.starttime = globalTime;
					insertEvent(tmp);
				}
			}
			else if (dataTable[p.currentline].name == "INPUT")
			{
				p.currentline++;
				if (p.currentline>p.endline)
				{
					tmp.ename = "TERM";
					tmp.starttime = globalTime;
					insertEvent(tmp);
				}
				else
				{
					tmp.ename = dataTable[p.currentline].name;
					tmp.starttime = globalTime;
					insertEvent(tmp);
				}
			}
		}
		if (next.ename == "TERM") //If process gets terminated
		{
			p.state = "TERMINATED";
			processTable[next.procnum] = p;
		}

		if (next.ename == "CORE")
		{
			int c = getFreeCore(busyTimes);
			if (c != -1)
			{
				busyTimes[c] = true;
				tmp.ename = "END";
				tmp.starttime = globalTime + dataTable[p.currentline].time;
				tmp.corenum = c;
				insertEvent(tmp);
				p.state = "RUNNING";
			}
			else
			{
				coreQueue.push(next.procnum);
				p.state = "READY";
			}
		}
		if (next.ename == "SSD")
		{
			if (dataTable[p.currentline].time == 0)
			{
				p.currentline++;
				if (p.currentline>p.endline)
				{
					tmp.ename = "TERM";
					tmp.starttime = globalTime;
					insertEvent(tmp);
				}
				else
				{
					tmp.ename = dataTable[p.currentline].name;
					tmp.starttime = globalTime;
					insertEvent(tmp);
				}
			}
			else if (SSDBusy == false)
			{
				SSDBusy = true;
				tmp.ename = "END";
				tmp.starttime = globalTime + dataTable[p.currentline].time;
				insertEvent(tmp);
			}
			else
			{
				SSDQueue.push(next.procnum); //LINE 238 on Nathan's
			}
		}
		if (next.ename == "INPUT")
		{
			tmp.ename = "END";
			tmp.starttime = globalTime + dataTable[p.currentline].time;
			insertEvent(tmp);
		}
		processTable[next.procnum] = p;
	}
	system("pause");
	return 0;

}
void inputData()
{
	Data dtmp;
	process ptmp;
	event etmp;
	ifstream input("1.txt");
	if (input.is_open()) {

		while (input >> action)
		{
			if (action == "NCORES" || action == "CORE" || action == "SSD" || action == "INPUT" || action == "NEW")
			{
				input >> num;
				if (input.fail())
				{
					action = "ERROR";
				}
			}
			else
			{
				cout << action;
				action = "ERROR";
			}
			// Assign # of Cores
			if (action == "NCORES")
			{
				nCores = num;
			}
			else if (action == "CORE")
			{
				dtmp.name = action;
				dtmp.time = num;
				dtmp.line = line;
				dataTable.push_back(dtmp);
			}
			else if (action == "NEW" || action == "INPUT" || action == "SSD")
			{
				dtmp.name = action;
				dtmp.time = num;
				dtmp.line = line;
				dataTable.push_back(dtmp);
			}

			if (action == "NEW")
			{
				if (processTable.size() != 0)
				{
					processTable.back().endline = line - 1;
				}
				ptmp.procnum = pronum++;
				ptmp.startline = line;
				ptmp.currentline = line;
				ptmp.starttime = num;
				ptmp.state = "BLOCKED";
				processTable.push_back(ptmp);
				etmp.procnum = ptmp.procnum;
				etmp.starttime = ptmp.starttime;
				etmp.ename = "NEW";
				etmp.corenum = -1;
				insertEvent(etmp);
				arrival(pronum);
			}
			if (action != "ERROR")
			{
				line++;
			}

		}
		input.close();
	}
	processTable[processTable.size() - 1].endline = line - 1;
}
void insertEvent(event e)
{
	int cnt = 0;
	while (cnt <= eventList.size())
	{
		if (cnt == eventList.size())
		{
			eventList.push_back(e);
			cnt = eventList.size() + 1;
		}
		else if (e.starttime>eventList[cnt].starttime)
		{
			eventList.insert(eventList.begin() + cnt, e);
			cnt = eventList.size() + 1;
		}
		else if (e.starttime == eventList[cnt].starttime)
		{
			if (e.ename == "END")
			{
				while (cnt != eventList.size() && (eventList[cnt].ename != e.ename || eventList[cnt].ename == e.ename&&eventList[cnt].starttime>e.starttime) && e.starttime == eventList[cnt].starttime)
				{
					cnt++;
				}
				if (cnt == eventList.size() || e.starttime != eventList[cnt].starttime)
				{
					cnt--;
				}
				else
				{
					eventList.insert(eventList.begin() + cnt, e);
					cnt = eventList.size() + 1;
				}
			}
			else if (e.ename == "TERM")
			{
				eventList.insert(eventList.begin() + cnt, e);
				cnt = eventList.size() + 1;
			}
			else if ((e.procnum>eventList[cnt].procnum || eventList[cnt].ename == "END") && eventList[cnt].ename != "TERM")
			{
				eventList.insert(eventList.begin() + cnt, e);
				cnt = eventList.size() + 1;
			}
		}
		cnt++;
	}
}
void arrival(int a)
{
	cout << endl << "-- ARRIVAL event for process " << a << " at time "; showArrival(a);

}
//returns the position of the first core found to be free, if none are free then returns -1
int getFreeCore(bool bt[])
{
	for (int x = 0; x<nCores; x++)
	{
		if (bt[x] == false)
		{
			return x;
		}
	}
	return -1;
}
//displays the number of busy cores, only used for ease of print();
int numBusyCores(bool bt[])
{
	int cnt = 0;
	for (int x = 0; x<nCores; x++)
	{
		if (bt[x] == true)
		{
			cnt++;
		}
	}
	return cnt;
}
// shows the contents of a queue of integers, used for debugging as well as ease of print()
void showQueue(queue<int> q)
{
	queue<int> tmp = q;
	if (tmp.empty())
	{
		cout << "empty";
	}
	else
	{
		while (!tmp.empty())
		{
			cout << "Process " << tmp.front();
			tmp.pop();
			if (!tmp.empty())
			{
				cout << ", ";
			}
		}
	}
}
//displays the contents of processTable excluding the terminated processes that aren't process n
void showArrival(int a)
{
	for (int x = 0; x<processTable.size(); x++)
	{
		process p = processTable[x];
		if (p.state != "TERMINATED" || a == x)
		{
			cout << p.starttime << endl;
		}
	}
}