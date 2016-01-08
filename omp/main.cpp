#include <iostream>
#include <fstream>
#include <omp.h>
#include <cstdio>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>

using namespace std;

vector<vector<char> > bufa, bufb;
int n = 0, m = 0, it = 0, maxit = -1;
string data_source = "None";
int threadN = 4;
bool running = false;
bool data_ready = false;
bool stop_requested = false;
bool stopping = false;
int cur_row;


int dx[] = {1,  1,  0, -1, -1, -1,  0,  1};
int dy[] = {0,  1,  1,  1,  0, -1, -1, -1};

void calc(int x, int y) {
    int sum = 0;
    for (int i = 0; i < 8; ++i) {
        sum += bufa[(x + dx[i] + n) % n][(y + dy[i] + m) % m];
    }
    bufb[x][y] = 0;
    if (sum == 3 || (bufa[x][y] && sum == 2))
        bufb[x][y] = 1;
}

void worker() {
    while (true) {
        int row_num;
        #pragma omp critical(row_cnt)
            row_num = cur_row++;
        if (row_num < n) {
            for (int i = 0; i < m; ++i)
                calc(row_num, i);
        } else {
            #pragma omp barrier
            #pragma omp single
            {
                swap(bufa, bufb);
                cur_row = 0;
                if (stop_requested || (maxit <= it + 1 && maxit != -1)) {
                    stopping = true;
                }
                it++;
                if (stopping) {
                    cout << "Task completed" << endl;
                    running = false;
                }
            }
            if (stopping)
                return;
            
        }
    }
}

void shell(string cmd) {
        if (cmd == "help") {
            cout << "    help\t - print this message\n\
    status\t - print status info\n\
    loadrnd\t - load random field\n\
    loadfile\t - load field from file\n\
    stop\t - stop calculating from next iteration\n\
    threadcnt\t - set threads number\n\
    start\t - start calculations\n\
    quit\t - exit application\n";
        }
        else if (cmd == "status") {
            cout << "***********   STATUS INFO   ****************" << endl;
            cout << "State: " << (running ? "running" : "waiting") << endl;
            cout << "Threads cnt: " << threadN << endl;
            cout << "Data source: " << data_source << endl;
            cout << "n: " << n << endl;
            cout << "m: " << m << endl;
            cout << "iterations: " << it << endl;
            cout << "max iteration: " << maxit << endl;
            cout << "data: " << (data_ready ? "ready to run" : "No data") << endl;
            if (n * m > 50) {
                cout << "Too many data" << endl;
            }
            else if (running) {
                cout << "Data is unavaliable while running" << endl;
            }
            else {
                for (int i = 0; i < n; ++i) {
                    for (int j = 0; j < m; ++j) {
                        cout << (bufa[i][j] ? 1 : 0) << " ";
                    }
                    cout << endl;
                }
            }
        }
        else if (cmd == "loadrnd") {
            if (running) {
                cout << "previous task is still running" << endl;
                return;
            }
            it = 0;
            cout << "print n and m numbers:" << endl;
            cin >> n >> m;
            it = 0;
            bufa.assign(n, vector<char>(m, 0));
            bufb.assign(n, vector<char>(m, 0));
            data_source = "random data";
            for (int i = 0; i < n; ++i) {
                for (int j = 0; j < m; ++j) {
                    bufa[i][j] = rand() % 2;
                }
            }
            data_ready = true;
            cout << "completed" << endl;
        }
        else if (cmd == "loadfile") {
            if (running) {
                cout << "previous task is still running" << endl;
                return;
            }
            it = 0;
            bufa.clear();
            bufb.clear();
            cout << "print filename" << endl;
            string filename, sbuf;
            cin >> filename;
            ifstream fin(filename.c_str());
            if (!fin.is_open()) {
                cout << "Some file open error" << endl;
                data_ready = false;
                return;
            }
            while (getline(fin, sbuf)) {
                vector<char> v;
                replace(sbuf.begin(), sbuf.end(), ';', ' ');
                stringstream ssbuf;
                ssbuf << sbuf;
                int x;
                while (ssbuf >> x)
                    v.push_back(x);
                bufa.push_back(v);
            }
            int n = bufa.size();
            if (n == 0) {
                cout << "n is 0" << endl;
                data_ready = false;
                return;
            }
            int m = bufa[0].size();
            if (m == 0) {
                cout << "m is 0" << endl;
                data_ready = false;
                return;
            }
            bufb.assign(n, vector<char>(m, 0));
            data_ready = true;
            data_source = "File: " + filename;
            for (int i = 0; i < n; ++i)
                if ((int)bufa[i].size() != m) {
                    cout << "data file is corrupted" << endl;
                    data_ready = false;
                }
            cout << "loaded" << endl;
        }
        else if (cmd == "threadcnt") {
            if (running) {
                cout << "task is already running\n";
                return;
            }
            int tc;
            cout << "write new number of threads" << endl;
            cin >> tc;
            threadN = tc;
            omp_set_num_threads(threadN);
            cout << "Threads number set to " << tc << endl;
        }
        else if (cmd == "stop") {
            cout << "task is'n running" << endl;
        }
        else if (cmd == "start") {
            cout << "task is already running" << endl;
        }
        else if (cmd == "quit") {
            exit(0);
        }
        else if (cmd == "") {
            
        }
        else {
            cout << "unknown command\nType 'help' for help" << cmd << endl;
        }
    

}

string run() {
    string cmd = "";
    #pragma omp parallel num_threads(2)
    {
        #pragma omp master
        {
            while (cin >> cmd) {
                if (!running) {
                    break;
                }
                if (cmd == "stop") {
                    stop_requested = true;
                    cout << "stopping from next iteration" << endl;
                    break;
                } else {
                   shell(cmd);
                }
            }
        }
        #pragma omp single nowait
        {
            #pragma omp parallel num_threads(threadN)
            worker();
        }
    }
    return cmd;
}

void main_shell() {
    string cmd;
    string next_cmd = "";

    while (cin >> cmd) {
        next_cmd = cmd;
        while (next_cmd != "") {
            if (next_cmd == "start") {
                if (!data_ready) {
                    cout << "data isn't loaded";
                    continue;
                }
                cout << "write max iteration or -1 for infinite\n";
                cin >> maxit;
                if (it >= maxit && maxit != -1) {
                    cout << "Bad maxit" << endl;
                    next_cmd = "";
                } else {
                    running = true;
                    stop_requested = false;
                    stopping = false;
                    cur_row = 0;
                    next_cmd = run();
                }

            } else {
                shell(next_cmd);
                next_cmd = "";
            }
        }
    }
}

int main() {
    omp_set_nested(1);
    omp_set_num_threads(threadN);
    cout << "Welcome to ultimate program to calculate live game" << endl;
    cout << "by ryad0m.\nType 'help' for help" << endl;
    main_shell();
    return 0;
}

