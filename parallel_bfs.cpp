//Implementation of parallel BFS using multiple queues and work-stealing approach
//Author: Jagat Sastry P.
//2013/3/15

#include<cilk.h>
#include <ctime>
#include<vector>
#include<cstdlib>
#include"Graph.h"
#include<iostream>
#include<deque>
#include<set>
#include<cilk_mutex.h>
#include"InputReader.h"
#include<reducer_max.h>
#include<reducer_opadd.h>
#include<cilkview.h>


using namespace std;

const int MAX_STEAL_ATTEMPTS = 47; //Empirically obtained
int MIN_STEAL_SIZE = 20; 

struct Checksumdist {
    long maxDistance;
    long long checksum;
};
    
    class DequeIdx {
        public:
            int start;
            int end;
            deque<long> * deqptr;
            deque<long> * origdeq;
            
            DequeIdx(): start(0), end(0){
            }

            void initDeque() {
                deqptr = new deque<long>;
                origdeq = deqptr;
            }

            void deleteDeque() {
                delete origdeq;
            }

            void set(deque<long> *deq, int sz, int e) {
                deqptr = deq;
                start = sz;
                end = e;
            }

            ~DequeIdx() {
            }

            void push_back(long a) {
                deqptr->push_back(a);
                end++;
            }

            long front() {
               return (*deqptr)[start];
            }

            void pop_front() {
                start++;
            }

            bool empty() {
                return end - start == 0;
            }

            int size() {
                return end - start;
            }
    };
    

class ParBFS {
    public:

    Checksumdist csd;

    Checksumdist getChecksum(vector<long long>& dist){

        long n = dist.size();

        cilk_for(long j=1; j<n; j++)
            if(dist[j] == -1)
                dist[j] = (n-1);

        cilk::reducer_opadd<long long> checksum;

        cilk_for(long k=1; k < n; k++)
            checksum += dist[k];

        csd.checksum = checksum.get_value();
        return csd;
    }
    
    template<class T>
    bool allEmpty(vector<T> arr) {
        int n = arr.size();
        for(int i =0; i < n; i++) {
            if(!arr[i].empty())
                return false;
        }
        return true;
    }
    
    

    void deleteDeques(vector<DequeIdx>* qin) {
        int n = qin->size();
        for(int i = 0; i < n; i++) {
            (*qin)[i].deleteDeque();
        }
    }

    void initDeques(vector<DequeIdx> *qin) {
        int num = qin->size();
        for(int i = 0; i < num; i++)
            (*qin)[i].initDeque();
    }

    void parBfs(Graph& graph, long src, vector<long long>& dist, vector<cilk::mutex>& locks) {
        int numVert = dist.size();
        cilk_for(long i = 0; i < numVert; i++)
            dist[i] = -1;
    
        dist[src] = 0;
        int p = locks.size();
        
        int levels = -1;
    
        vector<DequeIdx> * qin = new vector<DequeIdx>(p);
        vector<DequeIdx> * qout = new vector<DequeIdx>(p);
        initDeques(qin);
        initDeques(qout);
        
        (*qin)[0].push_back(src);
        while(!allEmpty(*qin)) {
            cilk_for(int k = 0; k < p-1; k++)
                cilk_spawn parBfsThread(k, graph, (*qout)[k], dist, qin, locks);
            parBfsThread(p-1, graph, (*qout)[p-1], dist, qin, locks);
            deleteDeques(qin);
            delete qin;
            qin = qout;
            qout = new vector<DequeIdx>(p);
            initDeques(qout);
            levels++;
        }
        csd.maxDistance = levels;

    }
    

    void parBfsThread(int curProc, Graph& graph, DequeIdx& qouti, vector<long long>& dist, 
                    vector<DequeIdx>* seg, vector<cilk::mutex>& locks) {
        DequeIdx& curdeque = (*seg)[curProc];
        while(!curdeque.empty()) {
            while(!curdeque.empty()) {
                long vert = curdeque.front();
                curdeque.pop_front();
                vector<long>& neighbors = graph.getNeighbors(vert);
                int nbsize = neighbors.size();
                if(nbsize <= 0) continue;
                vector<long>::const_iterator iter = neighbors.begin();
                while(iter != neighbors.end()) {
                	long nbidx = *iter;
                    if(dist[nbidx] == -1) {
                        dist[nbidx] = dist[vert] + 1;
                        qouti.push_back(nbidx);
                    }
                    ++iter;
                }
            }
            
            int attempt = 0; 
    
    
            locks[curProc].lock();
            while((*seg)[curProc].empty() && attempt < MAX_STEAL_ATTEMPTS) {
                int r = rand() % (*seg).size();
    			if(r != curProc && locks[r].try_lock()) {
                    long sz = 0;
                    if((sz = (*seg)[r].size()) > MIN_STEAL_SIZE) {
                            DequeIdx& rd = (*seg)[r];
                            DequeIdx& cur = (*seg)[curProc];
                            cur.set(rd.deqptr, (sz + 1)/2, rd.end);
                            rd.set(rd.deqptr, rd.start,  (sz+1)/2);
                    }
                    locks[r].unlock();
                }
                attempt++;
            }
            locks[curProc].unlock();
        }
    }
};
    
void printCurTime() {
    time_t t = time(0);   // get time now
    struct tm * now = localtime( & t );
    fflush(0);
    cerr << (now->tm_year + 1900) << '-' 
         << (now->tm_mon + 1) << '-'
         <<  now->tm_mday << ", "
         <<  now->tm_hour << ":"
         <<  now->tm_min << ":"
         <<  now->tm_sec
         << endl;
    fflush(0);
}

int cilk_main(int argc, char *argv[]){
    printCurTime();
    InputReader reader;
    Graph& g = reader.getGraph();
    vector<long>& sources = reader.getSources();
    vector<Checksumdist> results(sources.size());
    cerr<<"Done reading file"<<endl;
    printCurTime();
    int numSrc = sources.size();
    cilk::cilkview cv;
    cv.start();
    cilk_for(int i=0;i<numSrc;i++){
        ParBFS bfs;
        vector<long long> dist(g.numOfVertices()+1);
        vector<cilk::mutex> locks(cilk::current_worker_count());

        bfs.parBfs(g, sources[i], dist, locks);
        results[i] = bfs.getChecksum(dist);
    }
    cv.stop();
    printCurTime();
    for(int i=0;i<results.size();i++)
        cout<<results[i].maxDistance<<" " <<results[i].checksum<<endl;
    cerr<<cv.accumulated_milliseconds()<<endl;
    printCurTime();
    return 0;
}

