//#include<iostream>
#include "Graph.h"
#include <fstream>
#include<iostream>

using namespace std;

class InputReader{
	private:
		Graph g;
		vector<long> sources;
		//readInput(ifstream in);
	public:
		InputReader(char *filename);
		InputReader();
		Graph& getGraph();
		vector<long>& getSources();
};

InputReader::InputReader(): g(0){
	int i, n, e, r, u, v, s;     
    scanf("%d%d%d", &n, &e, &r);
    g = Graph(n);
    for(i=0;i<e;i++){
        scanf("%d%d", &u, &v);
        g.addEdge(u,v);
    }
    for(i=0;i<r;i++){
        cin>>s;
        sources.push_back(s);
    }
}

InputReader::InputReader(char *filename): g(0){
	ifstream in(filename);
	//readInput(in);
	int i, n, e, r, u, v, s;     
	in>>n>>e>>r;
    g = Graph(n);
        for(i=0;i<e;i++){
                in>>u>>v;
                g.addEdge(u,v);
        }
        for(i=0;i<r;i++){
                in>>s;
                sources.push_back(s);
        }

	in.close();
}
/*
void InputReader::readInput(ifstream input){
	int i, n, e, r, u, v, s;	
	input>>n>>e>>r;
	g.setNumOfVertices(n);
	for(i=0;i<e;i++){
		input>>u>>v;
		g.addEdge(u,v);
	}
	for(i=0;i<r;i++){
		input>>s;
		sources.push_back(s);
	}
}
*/
Graph& InputReader::getGraph(){
	return g;
}

vector<long>& InputReader::getSources(){
	return sources;
}
