#include <iostream>
#include <fstream>
#include <pthread.h>
#include <cstdio>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>

using namespace std;

vector<vector<char> > bufa, bufb;
int threadN = 4;
int n = 0, m = 0, it = 0, maxit = -1;
string data_source = "None";
bool running = false;
bool data_ready = false;
bool stop_requested = false;
bool stopping = false;
int cur_row;
vector<pthread_t> threads;
pthread_mutex_t cur_row_m;
pthread_attr_t default_attr;

pthread_mutex_t signaled_m;
pthread_cond_t signaled_c;


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

void* worker(void* num) {
    while (true) {
        int row_num;
        int last_it;
        pthread_mutex_lock(&cur_row_m);
        last_it = it;
        row_num = cur_row;
        cur_row++;
        pthread_mutex_unlock(&cur_row_m);
        if (row_num < n) {
            for (int i = 0; i < m; ++i)
                calc(row_num, i);
        }
        else if (row_num + 1 < n + threadN) {
            pthread_mutex_lock(&signaled_m);
            if (last_it == it) {
                pthread_cond_wait(&signaled_c, &signaled_m);
            }
            pthread_mutex_unlock(&signaled_m);

            if (stopping) {
                return NULL;
            }
        }
        else {
            swap(bufa, bufb);
            cur_row = 0;
            if (stop_requested || (maxit <= it + 1 && maxit != -1)) {
                stopping = true;
            }
            pthread_mutex_lock(&signaled_m);
            it++;
            pthread_cond_broadcast(&signaled_c);
            pthread_mutex_unlock(&signaled_m);
            
            if (stopping) {
                cout << "Task completed" << endl;
                running = false;
                return NULL;
            }
        }
    }
}

void run() {
    if (it >= maxit && maxit != -1) {
        running = false;
    }
    else {
        stopping = false;
        cur_row = 0;
        threads.resize(threadN);
        for (int i = 0; i < threadN; ++i) {
            int res = pthread_create(&threads[i], &default_attr, worker, NULL);
            if (res != 0) {
                cout << "Application panic: pthread_create error" << endl;
                exit(1);
            }
        }
    }
}

int main() {
    pthread_attr_init(&default_attr);
    pthread_attr_setdetachstate(&default_attr, PTHREAD_CREATE_DETACHED);
    pthread_mutex_init(&cur_row_m, NULL);
    pthread_mutex_init(&signaled_m, NULL);
    pthread_cond_init(&signaled_c, NULL);
    cout << "Welcome to ultimate program to calculate live game" << endl;
    cout << "by ryad0m.\nType 'help' for help" << endl;
    while (1) {
        string cmd;
        cin >> cmd;
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
                continue;
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
                continue;
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
                continue;
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
                continue;
            }
            int m = bufa[0].size();
            if (m == 0) {
                cout << "m is 0" << endl;
                data_ready = false;
                continue;
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
                continue;
            }
            int tc;
            cout << "write new number of threads" << endl;
            cin >> tc;
            threadN = tc;
            cout << "Threads number set to " << tc << endl;
        }
        else if (cmd == "stop") {
            if (!running) {
                cout << "task is'n running" << endl;
                continue;
            }
            stop_requested = true;
            cout << "stopping from next iteration" << endl;
        }
        else if (cmd == "start") {
            if (running) {
                cout << "task is already running" << endl;
                continue;
            }
            if (!data_ready) {
                cout << "data isn't loaded";
                continue;
            }
            cout << "write max iteration or -1 for infinite\n";
            cin >> maxit;
            running = true;
            stop_requested = false;
            run();
        }
        else if (cmd == "quit") {
            break;
        }
        else {
            cout << "unknown command\nType 'help' for help" << cmd << endl;
        }
    }
    pthread_attr_destroy(&default_attr);
    pthread_mutex_destroy(&cur_row_m);
    pthread_mutex_destroy(&signaled_m);
    pthread_cond_destroy(&signaled_c);

    return 0;
}

