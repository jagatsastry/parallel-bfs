//Author: Karthiek Chandrasekaran
#include<iostream>
#include<cilk.h>
#include<cilk_mutex.h>
#include "Graph.h"
#include "parbfs.cpp"
#include "InputReader.h"

using namespace std;

int cilk_main(int argc, char *argv[]){
	if(argc < 3){
		cout<<"Provide input and output file names"<<endl;
		return 0;
	}
	char *inputFile = argv[1];
	char *outputFile = argv[2];
	InputReader reader(inputFile);
	ofstream out(outputFile);
	Graph g = reader.getGraph();
	vector<long> sources = reader.getSources();
	long long maxDistance, checksum;
    ParBfs bfs;
	for(int i=0;i<sources.size();i++){
        vector<long long> dist(g.numOfVertices()+1);
        vector<cilk::mutex> locks(cilk::current_worker_count());
                
		bfs.parBfs(g, sources[i], dist, locks);
		checksum = bfs.getChecksum(dist, maxDistance);
		cout<<"Max distance from source "<<sources[i]<<" : "<<maxDistance<<endl;
		cout<<"Checksum: "<<checksum<<endl<<endl;
		out<<maxDistance<<"  "<<checksum<<endl;
	}
	out.close();
	return 0;		
}
