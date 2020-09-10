#pragma once

#include "Point.h"

#define COMPUTER_NODE 1
#define HUMAN_NODE -1
#define COMPUTER_WIN	1
#define HUMAN_WIN	-1
#define TIE		0
#define NOTEND	2
#define COMPUTER_PUT	2
#define HUMAN_PUT	1

#define TIMELIMIT	2300
#define COEFFICIENT 1
#define DECAY		0.95
#define MINCOEF		0.8



class StateNode{
private:
	int **board;
	int *top;
	int M, N;
	int whoseNode;
	int x, y;
	int nox, noy;
	double profit;
	long visitCount;
	int nodestate;
	StateNode* parent;
	StateNode** child;
	
	int* expandIndex;
	int canExpandNUms;
	int checkState();
public:
	long sub_node_cnt;
	StateNode(int **board, const int *top, 
		int M, int N, int whoseNode, 
		int x, int y, StateNode* parent, int nox, int noy);

	inline bool isExpandable();
	void clear();
	inline int get_node_state();
	StateNode* expand();
	void backward(double deltaProfit);
	inline bool is_terminal_node();
	inline Point get_point();
	inline long get_visits_cnt();
	inline double get_profit();
	inline int get_whose_node();
	inline StateNode* get_child(int idx);
	inline void set_child(int idx, StateNode* node);
	inline StateNode* get_parent();
	inline void set_parent(StateNode* node);
	StateNode* best_child(double cofficient);
	double simulate();
	StateNode* copy();
	void check_prune();
	inline int check_son_state(int* avail_y, int avail_y_cnt);
	void print_profit();
	void print_board();
	long dfs();
};

class UCT
{
private:
	UCT(int **board, const int *top, 
		int M, int N, int whoseNode, 
		int x, int y, int nox, int noy);
	StateNode* root;
	//unsigned int seed;
	double c;
	long starttime;
	static UCT *pUctSingle;
public:
	static UCT * getInstance(int **board, const int *top, 
		int M, int N, int whoseNode, 
		int x, int y, int nox, int noy, int newgameflag);
	void change_root(int y);
	StateNode* TreePolicy();
	double DefaultPolicy(StateNode* node);
	Point best_action();
	~UCT(void);
};