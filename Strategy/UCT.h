#pragma once

#include "Point.h"

#define COMPUTER_NODE 1
#define HUMAN_NODE 0
#define COMPUTER_WIN	1
#define HUMAN_WIN	-1
#define TIE		0
#define NOTEND	2
#define COMPUTER_PUT	2
#define HUMAN_PUT	1

#define TIMELIMIT	2300
#define COEFFICIENT 0.80
//#define DECAY		0.98
//#define MINCOEF		0.75



class StateNode{
public:
	int **board;
	int *top;
	int M, N;
	double profit;
	long visitCount;
	int whoseNode;
	int x, y;
	int nox, noy;
	int nodestate;
	StateNode* parent;
	StateNode** child;
	int* expandIndex;
	int canExpandNUms;
public:
	StateNode(int **board, const int *top, 
		int M, int N, int whoseNode, 
		int x, int y, StateNode* parent, int nox, int noy);
	
	bool isExpandable();
	void clear();
	int checkState();
	StateNode* deleteOtherBranches(int x, int y);
};

class UCT
{
private:
	UCT(int **board, const int *top, 
		int M, int N, int whoseNode, 
		int x, int y, int nox, int noy);
	StateNode* root;
	int nox, noy;
	int depth;
	int M, N;
	//double c;
	long starttime;
	static UCT *pUctSingle;
public:
	static UCT * getInstance(int **board, const int *top, 
		int M, int N, int whoseNode, 
		int x, int y, int nox, int noy, int newgameflag);
	void chopBranches(int x, int y);
	StateNode* expand(StateNode* node);
	StateNode* select();
	void simulate();
	void backward(StateNode* node, int deltaProfit);
	int checkNodeState(int**board, int* top, int whoseNode, int x, int y);
	StateNode* search();
	StateNode* TreePolicy();
	StateNode* BestChild(StateNode* node,double cofficient);
	int DefaultPolicy(StateNode* node);
	void getnextPoint(int&x,int&y);
	~UCT(void);
};