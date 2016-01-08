#include <pthread.h>
#include <sstream>
#include <mpi.h>
#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>
#include <map>
#include <set>
#include <vector>
#include <algorithm>
#include <signal.h>

using namespace std;

set<pair<int, int> > bufa, bufb, world;
int world_size, rank, workers_cnt;
int n, m, maxit, status = 1;

void send_vector(vector<pair<int, int> > &v, int id) {
    int sz = v.size();
    MPI_Send(&sz, 1, MPI_INT, id, 0, MPI_COMM_WORLD);
    for (int i = 0; i < sz; ++i) {
        MPI_Send(&v[i].first, 1, MPI_INT, id, 0, MPI_COMM_WORLD);
        MPI_Send(&v[i].second, 1, MPI_INT, id, 0, MPI_COMM_WORLD);
    }
}

void send_int(int val, int id) {
    MPI_Send(&val, 1, MPI_INT, id, 0, MPI_COMM_WORLD);
}

int recv_int(int id) {
    int val;
    MPI_Recv(&val, 1, MPI_INT, id, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    return val;
}

vector<pair<int, int> > recieve_vector(int id) {
    int sz, q, w;
    MPI_Recv(&sz, 1, MPI_INT, id, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    vector<pair<int, int> > res(sz);
    for (int i = 0; i < sz; ++i) {
        MPI_Recv(&q, 1, MPI_INT, id, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&w, 1, MPI_INT, id, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        res[i] = make_pair(q, w);
    }
    return res;
}

int get_worker_id(int k) {
    k = (k % n + n) % n;
    int q = (n / workers_cnt + 1) * (n % workers_cnt);
    if (k < q) {
        return k / (n / workers_cnt + 1);
    } else {
        return (k - q) / (n / workers_cnt) + (n % workers_cnt);
    }
}

int get_border(int k) {
    k = (k % workers_cnt + workers_cnt) % workers_cnt;
    if (k < n % workers_cnt) {
       return k * (n / workers_cnt + 1);
    } else {
        int sz = n / workers_cnt;
        return (n % workers_cnt) * (sz + 1) + (k - n % workers_cnt) * sz;
    }
}

int dx[] = {1,  1,  0, -1, -1, -1,  0,  1, 0};
int dy[] = {0,  1,  1,  1,  0, -1, -1, -1, 0};


bool check(int x, int y) {
    return bufa.find(make_pair(x, y)) != bufa.end();
    
}

void calc(int x, int y) {
    int sum = 0;
    for (int i = 0; i < 8; ++i) {
        if (check((x + dx[i] + n) % n,(y + dy[i] + m) % m))
            sum++;
    }
    if (sum == 3 || (check(x, y) && sum == 2))
        bufb.insert(make_pair(x, y));
}

void process(int x, int y) {
    for (int i = 0; i < 9; ++i) {
        int nx = (x + dx[i] + n) % n;
        int ny = (y + dy[i] + m) % m;
        if (get_border(rank) >= nx && nx < get_border(rank + 1)) {
            calc(nx, ny);
        }
    }
}

void print(string s) {
    printf("%d: %s\n", rank, s.c_str());
}

void print_matrix(set<pair<int, int> > &s) {
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) {
            if (s.find(make_pair(i, j)) != s.end()) {
                printf("+");
            } else {
                printf("-");
            }
        }
        printf("\n");
    }
}

void* minishell(void* val) {
    while (1) {
        string s;
        cin >> s;
        if (s == "q") {
            status = 0;
            return NULL;
        } else {
            printf("unknown command. Type 'q' for stop.");
        }
    }
}

int main(int argc, char** argv) {
    if (argc != 4) {
        print("Args count doesn't equals 3");
        print("Exiting...");
        return 0;
    }
    n = atoi(argv[1]);
    m = atoi(argv[2]);
    maxit = atoi(argv[3]);

    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    workers_cnt = world_size - 1;   
    if (rank == world_size - 1) {
        // I'm boss
        // Generating data
        int alive = min(max(10, n * m / 3), 100000);
        for (int i = 0; i < alive; ++i) {
            int x = rand() % n;
            int y  =rand() % m;
            world.insert(make_pair(x, y));
        }
        print_matrix(world);
        // Move data to niggers
        
        vector<vector<pair<int, int> > > data(workers_cnt);
        for (set<pair<int, int> >::iterator it = world.begin(); it != world.end(); ++it) {
            int id = get_worker_id(it->first);
            data[id].push_back(*it);
            int lid = get_worker_id(it->first - 1);
            int rid = get_worker_id(it->first + 1);
            if (id != lid)
                data[lid].push_back(*it);
            if (id != rid)
                data[rid].push_back(*it);
        }
        for (int i = 0; i < workers_cnt; ++i) {
            send_vector(data[i], i);
        }

        pthread_t thread;

        int res = pthread_create(&thread, NULL, minishell, NULL);
        if (res != 0) {
            printf("pthread error!!!");
            return 1;
        }   

        for (int work_it = 0; work_it < maxit; ++work_it) {
            for (int i = 0; i < workers_cnt; ++i) {
                recv_int(i);
            }
            int sstatus = status;
            for (int i = 0; i < workers_cnt; ++i) {
                send_int(sstatus, i);
            }
            if (!sstatus)
                break;
        }

        pthread_kill(thread, 9);

        print("collecting data");
        // Collect data from niggers
        world.clear();
        for (int i = 0; i < workers_cnt; ++i) {
            vector<pair<int, int> > res = recieve_vector(i);
            for (int j = 0; j < (int)res.size(); ++j) {
                world.insert(res[j]);
            }
        }
        print_matrix(world);
        MPI_Finalize();
        return 0;
        print("all completed");
    } else {
        // I'm nigga
        // Recieving data
        vector<pair<int, int> > in_data = recieve_vector(world_size - 1);
        for (int i = 0; i < (int)in_data.size(); ++i) {
            bufa.insert(in_data[i]);
        }
        // Processing
        for (int work_it = 0; work_it < maxit && status; work_it++) {
            // Calculating
            send_int(1, workers_cnt);
            for (set<pair<int, int> >::iterator it = bufa.begin(); it != bufa.end(); it++) {
                process(it->first, it->second);
            }
            // Sending
            vector<pair<int, int> > lrow, rrow;
            set<pair<int, int> >::iterator lit = bufb.begin(), rit = bufb.end();
            while (lit != bufb.end() && lit->first == get_border(rank)) {
                lrow.push_back(*lit);
                lit++;
            }
            if (bufb.size()) {
                rit--;
                while (rit != bufb.begin() && rit->first == get_border(rank + 1) - 1) {
                    rrow.push_back(*rit);
                    rit--;
                }
                if (bufb.begin()->first == get_border(rank + 1) - 1) {
                    rrow.push_back(*bufb.begin());
                }
            }
            send_vector(lrow, (rank - 1 + workers_cnt) % workers_cnt);
            send_vector(rrow, (rank + 1) % workers_cnt);

            // Recieving

            lrow = recieve_vector((rank - 1 + workers_cnt) % workers_cnt);
            rrow = recieve_vector((rank + 1) % workers_cnt);
            for (int i = 0; i < (int)lrow.size(); ++i) {
                bufb.insert(lrow[i]);
            }
            for (int i = 0; i < (int)rrow.size(); ++i) {
                bufb.insert(rrow[i]);
            }
            status = recv_int(workers_cnt);
            bufa.clear();
            swap(bufa, bufb);
        }
        vector<pair<int, int> > res;
        for (set<pair<int, int> >::iterator it = bufa.begin(); it != bufa.end(); it++) {
            res.push_back(*it);
        }
        send_vector(res, world_size - 1);
        // Returning
    }
    MPI_Finalize();
    return 0;
}

