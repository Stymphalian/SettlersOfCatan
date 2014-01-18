#pragma once
#include <list>

template<class T>
class GraphVertex{
public:
	int visited;

	//constructor and destructor
	GraphVertex(unsigned long long key, T data){
		//Logger::getLog("jordan.log").log(Logger::DEBUG,"Graph Vertex Constructor(key=%llu,data=%x)", key, data);
		this->key = key;
		this->data = data;
		this->num_edges = 0;
	}
	virtual ~GraphVertex(){
		//Logger::getLog("jordan.log").log(Logger::DEBUG, "Graph Vertex Destructor key=%llu,data=%x,num_edge=%d",key,data,num_edges);
		key = -1;
		data = 0;
		num_edges = 0;
	}
	//methods
	unsigned long long getKey(){ return key; }
	T getData(){ return data; }
	int getNumEdges(){ return num_edges; }
	void incrementEdges(int v){ num_edges += v; }
private:
	unsigned long long key;
	int num_edges;
	T data;	
};

template<class T>
class GraphEdge{
public:	
	//variables
	int visited;
	GraphVertex<T>* start;
	GraphVertex<T>* end;
	// constructor and destructor
	GraphEdge(unsigned long long key, T data, GraphVertex<T>* start, GraphVertex<T>* end){
		//Logger::getLog("jordan.log").log(Logger::DEBUG,"Graph Edge Constructor(key=%llu,data=%x,start=%x,end=%x)", key, data, start, end);
		this->key = key;
		this->data = data;
		this->start = start;
		this->end = end;
	}
	virtual ~GraphEdge(){
		//Logger::getLog("jordan.log").log(Logger::DEBUG, "Graph Edge Destructor key=%llu,data=%x,start=%x,end=%x",key,data,start,end);
		key = -1;
		data = 0;
		start = nullptr;
		end = nullptr;
	}
	// methods
	unsigned long long getKey(){return key;}
	T getData(){return data;}
private:
	unsigned long long key;
	T data;	
};



template<class T>
class Graph{
public:
	Graph();
	~Graph();	
	int numVertices();
	int numEdges();
	int degree(GraphVertex<T>& v);
	void clear();
	bool areAdjacent(GraphVertex<T>& v, GraphVertex<T>& w);
	bool removeVertex(GraphVertex<T>& v);
	bool removeEdge(GraphEdge<T>& e);	
	GraphEdge<T>* insertEdge(GraphVertex<T>& v, GraphVertex<T>& w, T data);
	GraphVertex<T>* opposite(GraphEdge<T>& e, GraphVertex<T>& v);	
	GraphVertex<T>* insertVertex(T data);
	std::list<GraphEdge<T>*> edges();
	std::list<GraphEdge<T>*> incidentEdges(GraphVertex<T>& v);
	std::list<GraphVertex<T>*> vertices();	
	std::list<GraphVertex<T>*> adjacentVertices(GraphVertex<T>& v);	
	std::list<GraphVertex<T>*> endVertices(GraphEdge<T>& e);
	std::list<GraphVertex<T>*> dfs(GraphVertex<T>* root);
	std::list<GraphVertex<T>*> bfs(GraphVertex<T>* root);
private:
	struct enode_t{
		GraphEdge<T>* edge;
		unsigned long long key;
	};

	struct vnode_t{
		GraphVertex<T>* vertex;
		unsigned long long key;
	};
	//Logger& logger;
	unsigned long long edge_keys;
	unsigned long long vertex_keys;
	std::list<vnode_t> vertex_list;
	std::list<enode_t> edge_list;
};

template<class T>
Graph<T>::Graph() {//: logger(Logger::getLog("jordan.log")){
	//logger.log(Logger::DEBUG, "Graph::Graph() Construtcor");
	edge_keys = 1;
	vertex_keys = 1;
}

template<class T>
Graph<T>::~Graph(){
	clear();	
}
template<class T>
void Graph<T>::clear(){
	//logger.log(Logger::DEBUG, "Graph::~Graph() Destructor");
	std::list<vnode_t>::iterator v_it;
	for(v_it = vertex_list.begin(); v_it != vertex_list.end(); ++v_it){
		//logger.log(Logger::DEBUG, "Graph::~Graph() vertex %x,%llu", v_it->vertex, v_it->key);
		delete v_it->vertex;
		v_it->vertex = nullptr;
	}
	std::list<enode_t>::iterator e_it;
	for(e_it = edge_list.begin(); e_it != edge_list.end(); ++e_it){
		//logger.log(Logger::DEBUG, "Graph::~Graph() edge %x,%llu", e_it->edge, e_it->key);
		delete e_it->edge;
		e_it->edge = nullptr;
	}
	vertex_list.clear();
	edge_list.clear();
}

template<class T>
int Graph<T>::numVertices(){
//	//logger.log(Logger::DEBUG, "Graph::numVertices %d", vertex_list.size());
	return vertex_list.size();
}

template<class T>
int Graph<T>::numEdges(){
	//logger.log(Logger::DEBUG, "Graph::numEdges %d", edge_list.size());
	return edge_list.size();
}

template<class T>
int Graph<T>::degree(GraphVertex<T>& v){
	//logger.log(Logger::DEBUG, "Graph::degree key=%llu,degree=%d", v.getKey(), v.getNumEdges());
	return v.getNumEdges();
}

template<class T>
bool Graph<T>::areAdjacent(GraphVertex<T>& v, GraphVertex<T>& w){
	std::list<enode_t>::iterator it;
	for(it = edge_list.begin(); it != edge_list.end(); ++it){
		if((it->edge->start == &v && it->edge->end == &w) ||
			(it->edge->start == &w && it->edge->end == &v))
		{
			//logger.log(Logger::DEBUG, "Graph::areAdjacent() v=%llu w=%llu edge=%llu", v.getKey(), w.getKey(), it->edge->getKey());
			return true;
		}
	}
	return false;
}

template<class T>
GraphVertex<T>* Graph<T>::opposite(GraphEdge<T>& e, GraphVertex<T>& v){
	if(e.start == nullptr || e.end == nullptr){ return nullptr; }
	if(e.start == &v){ return e.end; }
	return e.start;
}

template<class T>
std::list<GraphEdge<T>*> Graph<T>::edges(){
	std::list<GraphEdge<T>*> list;
	std::list<enode_t>::iterator it;
	for(it = edge_list.begin(); it != edge_list.end(); ++it){
		//logger.log(Logger::DEBUG, "Graph::edge() edge=%x,edge key=%llu", it->edge, it->key);
		list.push_back(it->edge);
	}
	return list;
}

template<class T>
std::list<GraphEdge<T>*> Graph<T>::incidentEdges(GraphVertex<T>& v){
	std::list<GraphEdge<T>*> list;
	std::list<enode_t>::iterator it;
	for(it = edge_list.begin(); it != edge_list.end(); ++it){
		if(it->edge->start == &v || it->edge->end == &v){
			//logger.log(Logger::DEBUG, "Graph::incidentEdges() vertex=%llu,edge=%d,edge key=%llu", v.getKey(), it->edge, it->key);
			list.push_back(it->edge);
		}
	}
	return list;
}

template<class T>
std::list<GraphVertex<T>*> Graph<T>::vertices(){
	std::list<GraphVertex<T>*> list;
	std::list<vnode_t>::iterator it;
	for(it = vertex_list.begin(); it != vertex_list.end(); ++it){
		//logger.log(Logger::DEBUG, "Graph::vertices() vertex=%x, vertex key =%llu", it->vertex, it->key);
		list.push_back(it->vertex);
	}
	return list;
}

template<class T>
std::list<GraphVertex<T>*> Graph<T>::adjacentVertices(GraphVertex<T>& v){
	std::list<GraphVertex<T>*> list;
	std::list<enode_t>::iterator it;
	for(it = edge_list.begin(); it != edge_list.end(); ++it){
		if(it->edge->start == &v  && it->edge->end != nullptr){
			//logger.log(Logger::DEBUG, "Graph::adjacentVertices original=%llu, neighbour=%llu", v.getKey(), it->edge->end->getKey());
			list.push_back(it->edge->end);
		} else if(it->edge->end == &v  && it->edge->start != nullptr){
			//logger.log(Logger::DEBUG, "Graph::adjacentVertices original=%llu, neighbour=%llu", v.getKey(), it->edge->start->getKey());
			list.push_back(it->edge->start);
		} else{
			continue;
		}
	}
	return list;
}

template<class T>
std::list<GraphVertex<T>*> Graph<T>::endVertices(GraphEdge<T>& e){
	std::list<GraphVertex<T>*> list;
	list.push_back(e.start);
	list.push_back(e.end);
	return list;
}

template<class T>
GraphEdge<T>* Graph<T>::insertEdge(GraphVertex<T>& v, GraphVertex<T>& w, T data){
	if(&v == &w){ return nullptr; }
	if(areAdjacent(v, w)){ return nullptr; }
	v.incrementEdges(1);
	w.incrementEdges(1);

	enode_t enode;
	enode.edge = new GraphEdge<T>(edge_keys++, data, &v, &w);;
	enode.key = enode.edge->getKey();
	edge_list.push_back(enode);
	//logger.log(Logger::DEBUG, "Graph::insertEdge edge=%x,edge_key=%llu", enode.edge, enode.key);
	return enode.edge;
}

template<class T>
GraphVertex<T>* Graph<T>::insertVertex(T data){
	vnode_t vnode;
	vnode.vertex = new GraphVertex<T>(vertex_keys++, data);
	vnode.key = vnode.vertex->getKey();
	vertex_list.push_back(vnode);
	//logger.log(Logger::DEBUG, "Graph::insertVertex vertex=%x,vertex_key=%llu", vnode.vertex, vnode.key);
	return vnode.vertex;
}

template<class T>
bool Graph<T>::removeVertex(GraphVertex<T>& v){
	// erase all the edges which are attached to the vertex v
	std::list<enode_t>::iterator e_it;
	for(e_it = edge_list.begin(); e_it != edge_list.end();){
		if(e_it->edge->start == &v || e_it->edge->end == &v){
			// we have found an edge which has a refrence to vertex v
			//logger.log(Logger::DEBUG, "Graph::removeVertex vertex key=%llu, edge key=%llu", v.getKey(), e_it->edge->getKey());
			if(e_it->edge->start != nullptr){
				e_it->edge->start->incrementEdges(-1);
			}
			if(e_it->edge->end != nullptr){
				e_it->edge->end->incrementEdges(-1);
			}

			delete e_it->edge;
			e_it->edge = nullptr;
			edge_list.erase(e_it++);
		} else{
			e_it++;
		}
	}

	// erase the vertex from the vertex list
	std::list<vnode_t>::iterator v_it;
	bool found = false;
	for(v_it = vertex_list.begin(); v_it != vertex_list.end();){
		if(v_it->key == v.getKey()){
			// we have found the vertex
			//logger.log(Logger::DEBUG, "Graph::removeVertex vertex=%x vertex key=%llu", v_it->vertex, v.getKey());
			found = true;
			delete v_it->vertex;
			v_it->vertex = nullptr;
			vertex_list.erase(v_it++);
			break;
		} else{
			v_it++;
		}
	}
	return found;
}

template<class T>
bool Graph<T>::removeEdge(GraphEdge<T>& e){
	if(e.start != nullptr){ e.start->incrementEdges(-1); }
	if(e.end != nullptr){ e.end->incrementEdges(-1); }

	// erase the edge from the edge list	
	bool found = false;
	std::list<enode_t>::iterator e_it;
	for(e_it = edge_list.begin(); e_it != edge_list.end();){
		if(e_it->key == e.getKey()){
			//we have found the edge, therefore delete it
			//logger.log(Logger::DEBUG, "Graph::removeEdge() edge=%x,edge key=%llu", e_it->edge, e_it->key);
			found = true;
			delete e_it->edge;
			e_it->edge = nullptr;
			edge_list.erase(e_it++);
			break;
		}
	}
	return found;
}

template< class T>
std::list<GraphVertex<T>*> Graph<T>::dfs(GraphVertex<T>* root){	
	std::list<GraphVertex<T>*> list;
	if(root == nullptr){ return list; }

	GraphVertex<T>* current;
	std::list<GraphVertex<T>*> stack;
	std::list<GraphVertex<T>*> children;
	std::list<GraphVertex<T>*>::iterator child_it;
	
	// set everything as NOT visted
	std::list<vnode_t>::iterator it;
	for(it = vertex_list.begin(); it != vertex_list.end(); ++it){
		it->vertex->visited = 0;		
	}

	stack.push_front(root);
	while(stack.empty() != true){
		// pop-off the firsts element from the stack
		current = stack.front();
		stack.erase(stack.begin());
		if(current == nullptr){ continue; }
		if(current->visited == 1){ continue; }		

		// set the current node as vivisted, and do any processing
		current->visited = 1;
		list.push_back(current);
		
		// add every child  of current to the stack
		children = this->adjacentVertices(*current);
		for(child_it = children.begin(); child_it != children.end(); ++child_it){			
			if((*child_it)->visited == 0){
				// discovery edge ,only push unvivisted children onto the stack
				stack.push_front(*child_it);
			} else if((*child_it)->visited == 1){
				// do nothing ( back-edge)
			} else{
				continue;
			}
		}
	}

	return list;
}

template<class T>
std::list<GraphVertex<T>*> Graph<T>::bfs(GraphVertex<T>* root){
	std::list<GraphVertex<T>*> list;
	if(root == nullptr){ return list; }

	GraphVertex<T>* current;
	std::list<GraphVertex<T>*> queue;
	std::list<GraphVertex<T>*> children;
	std::list<GraphVertex<T>*>::iterator child_it;

	// set everything as NOT visted
	std::list<vnode_t>::iterator it;
	for(it = vertex_list.begin(); it != vertex_list.end(); ++it){
		it->vertex->visited = 0;
	}

	queue.push_front(root);
	while(queue.empty() != true){
		// dequeue the firsts element from the queue
		current = queue.front();
		queue.erase(queue.begin());
		if(current == nullptr){ continue; }
		if(current->visited == 1){ continue; }

		// set the current node as vivisted, and do any processing
		current->visited = 1;
		list.push_back(current);

		// add every child  of current to the stack
		children = this->adjacentVertices(*current);
		for(child_it = children.begin(); child_it != children.end(); ++child_it){
			// only enqueue unvivisted children onto the stack
			if((*child_it)->visited == 0){
				// discovery edge
				queue.push_back(*child_it);
			} else if((*child_it)->visited == 1){
				// do nothing ( back-edge)
			} else{
				continue;
			}
		}
	}

	return list;
}