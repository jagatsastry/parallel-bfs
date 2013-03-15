//Author: Karthiek Chandrasekaran
#ifndef GRAPH_H
#define GRAPH_H

#include <vector>

using namespace std;

class Graph{
	private:
		long N; //no. of vertices
		vector<vector<long> > adjList;
	public:
		Graph(long n); //pass no. of vertices
		void setNumOfVertices(int);
		void addEdge(long u, long v); //add edge from u to v
		vector<long>& getNeighbors(long u); //get all neighbors of vertex u
		long numOfVertices();
};


Graph::Graph(long n): adjList(n+1){
	N = n;
}


void Graph::addEdge(long u, long v){
	adjList[u].push_back(v);
}

vector<long>& Graph::getNeighbors(long u){
	return adjList[u];
}

long Graph::numOfVertices(){
	return N;
}

#endif
