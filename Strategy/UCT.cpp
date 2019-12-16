#include "UCT.h"
#include "Judge.h"
#include <ctime>
#include <time.h>
#include <cstdlib>
#include <cmath>
#include <conio.h>
#include <atlstr.h>

UCT* UCT::pUctSingle = nullptr;

StateNode::StateNode(int **board, const int *top, 
		int M, int N, int whoseNode, 
		int x, int y, StateNode* parent, int nox, int noy)
{
	this->M = M;
	this->N = N;
	int** _board = new int*[M];
	for(int i = 0; i < M; i++){
		_board[i] = new int[N];
		for(int j = 0; j < N; j++){
			_board[i][j] = board[i][j];
		}
	}
	this->board = _board;
	int* _top = new int[N];
	for(int i = 0; i < N; i++){
		_top[i] = top[i];
	}
	this->top = _top;
	this->x = x;
	this->y = y;
	this->nox = nox;
	this->noy = noy;
	this->whoseNode = whoseNode;
	this->parent = parent;
	this->profit = 0;
	this->visitCount = 0;

	this->expandIndex = new int[N];
	this->canExpandNUms = 0;
	this->child = new StateNode*[N];
	for(int i = 0; i < N; i++){
		this->child[i] = nullptr;
		if(this->top[i] != 0){
			this->expandIndex[this->canExpandNUms++] = i;
		}
	}
	checkState();
}

int StateNode::checkState()
{
	if(x == -1 && y == -1){
		this->nodestate = NOTEND;
		return NOTEND;
	}
	if ((whoseNode == COMPUTER_NODE && machineWin(x, y, this->M, this->N, board))){
		this->nodestate = COMPUTER_WIN;
		return COMPUTER_WIN;
	}
	if((whoseNode == HUMAN_NODE && userWin(x, y, this->M, this->N, board))){
		this->nodestate = HUMAN_WIN;
		return HUMAN_WIN;
	}
	if((isTie(this->N, top))){
		this->nodestate = TIE;
		return TIE;
	}
	this->nodestate = NOTEND;
	return NOTEND;
}

bool StateNode::isExpandable()
{
	return this->canExpandNUms>0;
}
void StateNode::clear()
{
	for (int i = 0; i < this->M; i++)
			delete [] this->board[i];
	delete [] this->board;
	delete [] this->top;
	delete [] this->expandIndex;
	for (int i = 0; i < this->N; i ++)
		if (this->child[i]) {
			this->child[i] -> clear();
			delete this->child[i];
		}
	delete [] this->child;
}

StateNode* StateNode::deleteOtherBranches(int x, int y){
	//_cprintf("in deletebranches\n");
	StateNode* retNode = this->child[y];
	for (int i = 0; i < this->N; i ++){
		if (this->child[i]) {
			if( y != i){
				this->child[i] -> clear();
				delete this->child[i];
			}
		}
	}
	if(retNode == nullptr){
		this->x = x;
		this->y = y;
		this->whoseNode ^= 1;
		this->profit = 0;
		this->visitCount = 0;
		this->canExpandNUms = 0;
		//_cprintf("in changeboard\n");
		if(this->whoseNode == HUMAN_NODE){
			this->board[x][y] = HUMAN_PUT;
		}else{
			this->board[x][y] = COMPUTER_PUT;
		}
		//_cprintf("out changeboard\n");
		this->top[y]--;
		if((this->top[y]-1) == this->nox)
			this->top[y]--;
		for(int i = 0; i < N; i++){
			this->child[i] = nullptr;
			if(this->top[i] != 0){
				this->expandIndex[this->canExpandNUms++] = i;
			}
		}
		checkState();
		retNode = this;
	}else{
		for (int i = 0; i < this->M; i++)
			delete [] this->board[i];
		delete [] this->board;
		delete [] this->top;
		delete [] this->expandIndex;
		delete [] this->child;
	}
	//_cprintf("out delete branches\n");
	return retNode;
}



UCT::UCT(int **board, const int *top, 
		int M, int N, int whoseNode, 
		int x, int y, int nox, int noy)
{
	this->root = new StateNode(board, top, M, N, whoseNode, x, y, nullptr, nox, noy);
	this->M = M;
	this->N = N;
	if(x == -1 && y == -1)
		this->depth = 0;
	else
		this->depth = 1;
	this->nox = nox; 
	this->noy = noy;
	srand(unsigned int(clock()));
	//this->c = COEFFICIENT;
}

UCT* UCT::getInstance(int **board, const int *top, 
		int M, int N, int whoseNode, 
		int x, int y, int nox, int noy, int newgameflag){
			if (nullptr == UCT::pUctSingle)
			{
				//AllocConsole();
				UCT::pUctSingle = new UCT(board, top, M, N, whoseNode, x, y, nox, noy);
				UCT::pUctSingle->starttime = long(clock());
				
			}else{
				UCT::pUctSingle->starttime = long(clock());
				if(newgameflag){
					UCT::pUctSingle->root->clear();
					delete UCT::pUctSingle->root;
					UCT::pUctSingle->root = new StateNode(board, top, M, N, whoseNode, x, y, nullptr, nox, noy);
					UCT::pUctSingle->M = M;
					UCT::pUctSingle->N = N;
					if(x == -1 && y == -1)
						UCT::pUctSingle->depth = 0;
					else
						UCT::pUctSingle->depth = 1;
					UCT::pUctSingle->nox = nox; 
					UCT::pUctSingle->noy = noy;
					//UCT::pUctSingle->c = COEFFICIENT;
					srand(unsigned int(clock()));
				}else{
					UCT::pUctSingle->chopBranches(x, y);
				}
			}
			return UCT::pUctSingle;
}


UCT::~UCT(void)
{
	this->root->clear();
	delete this->root;
	delete UCT::pUctSingle;
}

int UCT::checkNodeState(int**board, int*top, int whoseNode, int x, int y)
{
	if(x == -1 && y == -1){
		return NOTEND;
	}
	if ((whoseNode == COMPUTER_NODE && machineWin(x, y, this->M, this->N, board))){
		return COMPUTER_WIN;
	}
	if((whoseNode == HUMAN_NODE && userWin(x, y, this->M, this->N, board))){
		return HUMAN_WIN;
	}
	if((isTie(this->N, top))){
			return TIE;
	}
	return NOTEND;
}

void UCT::chopBranches(int x, int y){
	//_cprintf("in chopbranches\n");
	StateNode* nextroot = this->root->deleteOtherBranches(x, y);
	
	if(nextroot != root){
		delete root;
	}
	this->depth += 1;
	//if(this->depth%2){
		//this->c *= DECAY;
	//}
	//if( this->c < MINCOEF){
		//this->c = MINCOEF;
	//}
	nextroot->parent = nullptr;
	root = nextroot;
	//_cprintf("out chopbranches\n");
}

StateNode* UCT::expand(StateNode* node){
	//_cprintf("in expand\n");
	int whoseNode = node->whoseNode;
	int index = rand() % node->canExpandNUms;
	int expand_y = node->expandIndex[index];
	int expand_x = --node->top[expand_y];
	int nextwhoseNode = HUMAN_NODE;
	if(whoseNode == COMPUTER_NODE){
		node->board[expand_x][expand_y] = HUMAN_PUT;
		nextwhoseNode = HUMAN_NODE;
	}else{
		node->board[expand_x][expand_y] = COMPUTER_PUT;
		nextwhoseNode = COMPUTER_NODE;
	}
	if(expand_y == this->noy && (node->top[expand_y] - 1) == this->nox){
		node->top[expand_y]--;
	}
	node->child[expand_y] = new StateNode(node->board,node->top,this->M,this->N,nextwhoseNode,expand_x,expand_y,node,this->nox,this->noy);
	node->top[expand_y] = expand_x+1;
	node->board[expand_x][expand_y] = 0;
	node->expandIndex[index] = node->expandIndex[--node->canExpandNUms];
	//_cprintf("out expand\n");
	return node->child[expand_y];
}

void UCT::getnextPoint(int&x,int&y){
	StateNode* nextNode = this->search();
	x = nextNode->x;
	y = nextNode->y;
	this->chopBranches(x,y);
	//_cprintf("son:,time(ms):%ld\n",long(clock())-this->starttime);
	//for(int i = 0; i < this->N; i++){
	//	if(this->root->child[i] != nullptr){
	//		_cprintf("child %d,state:%d profit:%f,count:%ld,%f\n",i,this->root->child[i]->nodestate,this->root->child[i]->profit,this->root->child[i]->visitCount,
	//			this->root->child[i]->profit/this->root->child[i]->visitCount);
	//	}
	//}
	//_cprintf("\n");
}


StateNode* UCT::search(){
	int count = 0;
	while ((++count % 10 != 0) || long(clock()) - this->starttime <= TIMELIMIT) { //尚未耗尽计算时长 
		if(root->nodestate != NOTEND){
			break;
		}
		StateNode *selectedNode = this->TreePolicy(); //运用搜索树策略节点 
		int deltaProfit = this->DefaultPolicy(selectedNode); //运用模拟策略对选中节点进行一次随机模拟 
		this->backward(selectedNode, deltaProfit); //将模拟结果回溯反馈给各祖先 
	}
	//_cprintf("duringsearch,count:%d,time(ms):%ld\n",count,long(clock())-this->starttime);
	//for(int i = 0; i < this->N; i++){
	//	if(this->root->child[i] != nullptr){
	//		_cprintf("child %d,state:%d, profit:%f,count:%ld,%f\n",i,this->root->child[i]->nodestate,this->root->child[i]->profit,this->root->child[i]->visitCount,
	//			this->root->child[i]->profit/this->root->child[i]->visitCount);
	//	}
	//}
	//_cprintf("\n");
	return this->BestChild(this->root, 0.0);
}


void UCT::backward(StateNode* node, int deltaProfit){
		while (node) {
			node->visitCount++; //访问次数+1 
			node-> profit += deltaProfit; 
			//deltaProfit = deltaProfit * 0.999;
			node = node -> parent;
		}

}

//
StateNode* UCT::TreePolicy(){
	//_cprintf("intreepolicy\n");
	StateNode* currentNode = this->root;
	while(currentNode->nodestate == NOTEND){  
		if(currentNode->isExpandable())  
			return this->expand(currentNode); 
		else{
			currentNode = this->BestChild(currentNode, COEFFICIENT);
		}
	}
	//_cprintf("outpolicy\n");
	return currentNode;
}
StateNode* UCT::BestChild(StateNode* node, double cofficient){
	//_cprintf("inbestchild\n");
		StateNode* best;
		int flag = 1;
		if(node->whoseNode == COMPUTER_NODE){
			flag = -1;
		}
		double maxUCBValue = -1e200;
		int besti = -1;
		for (int i = 0; i != this->N; i++) {
			if (node->child[i] == nullptr) continue;
			long childvisitCnt = node->child[i]->visitCount;
			double profit = node->child[i]->profit * flag;
			double ucbvalue = double(profit) / childvisitCnt + 
				sqrtl(2 * logl(double(node->visitCount)) / childvisitCnt) * cofficient; 
			if (ucbvalue > maxUCBValue) {
				maxUCBValue = ucbvalue;
				best = node->child[i];
				besti = i;
			}else if (abs(ucbvalue - maxUCBValue) < 1e-10){
				//_cprintf("i came here");
				if(rand()%2){
					maxUCBValue = ucbvalue;
					best = node->child[i];
					besti = i;
				}
			}
		}
		//_cprintf("outbestchild\n");
		return best;

}

//模拟下棋，返回胜负
int UCT::DefaultPolicy(StateNode* node){
	//_cprintf("indefaultpolicy\n");
	if(node->nodestate != NOTEND){
		if(node->parent == nullptr){
			//_cprintf("outdefaultpolicy0\n");
			return node->nodestate;
		}
		if(node->whoseNode == COMPUTER_NODE){
			if(node->nodestate == COMPUTER_WIN){
				node->profit = 1e10;
				node->parent->nodestate = COMPUTER_WIN;
				node->parent->profit = 1e10;
			}else if(node->nodestate == HUMAN_WIN){
				StateNode* parent = node->parent;
				int flag = 1;
				if(parent->canExpandNUms == 0){
					for( int i = 0; i < this->N; i++){
						if((parent->child[i]!=nullptr)&&(parent->child[i]->nodestate != HUMAN_WIN)){
							flag = 0;
							break;
						}
					}
				}else{
					flag = 0;
				}
				if(flag == 1){
					//_cprintf("the second,backward\n");
					node->parent->nodestate = HUMAN_WIN;
					node->parent->profit = -1e10;
				}
			}
		}else if(node->whoseNode == HUMAN_NODE){
			if(node->nodestate == HUMAN_WIN){
				node->profit = -1e10;
				node->parent->nodestate = HUMAN_WIN;
				node->parent->profit = -1e10;
			}else if(node->nodestate == COMPUTER_WIN){
				StateNode* parent = node->parent;
				int flag = 1;
				if(parent->canExpandNUms == 0){
					for( int i = 0; i < this->N; i++){
						if((parent->child[i]!=nullptr)&&(parent->child[i]->nodestate != COMPUTER_WIN)){
							flag = 0;
							break;
						}
					}
				}else{
					flag = 0;
				}
				if(flag == 1){
					//_cprintf("the second,backward,computerwin\n");
					node->parent->nodestate = COMPUTER_WIN;
					node->parent->profit = 1e10;
				}
			}
		}
		//_cprintf("outdefaultpolicy1\n");
		return node->nodestate;
	}
	int** _board = new int*[this->M];
	for(int i = 0; i < this->M; i++){
		_board[i] = new int[this->N];
		for(int j = 0; j < this->N; j++){
			_board[i][j] = node->board[i][j];
		}
	}
	int* _top = new int[this->N];
	for(int i = 0; i < this->N; i++){
		_top[i] = node->top[i];
	}
	int whoseNode = node->whoseNode;
	int x = node->x;
	int y = node->y;
	int nodestate = node->nodestate;
	int* availy = new int[this->N];
	while(nodestate == NOTEND){
		int availynums = 0;
		//int earlyendflag = 0;
		for( int i = 0; i < this->N; i++){
			if(_top[i] != 0){
				availy[availynums++] = i;
			}
		}
		//for( int i = 0; i < availynums; i++){
		//	int nexty = availy[i];
		//	int nextx = --_top[nexty];
		//	if(whoseNode == COMPUTER_NODE){
		//		_board[nextx][nexty] = HUMAN_PUT;
		//		whoseNode = HUMAN_NODE;
		//	}else{
		//		_board[nextx][nexty] = COMPUTER_PUT;
		//		whoseNode = COMPUTER_NODE;
		//	}
		//	if(nexty == this->noy && (_top[nexty] - 1) == this->nox){
		//		_top[nexty]--;
		//	}
		//	nodestate = this->checkNodeState(_board,_top,whoseNode,nextx,nexty);
		//	if(nodestate != NOTEND){
		//		earlyendflag = 1;
		//		break;
		//	}
		//	_board[nextx][nexty] = 0;
		//	_top[nexty] = nextx + 1;
		//}
		//if(earlyendflag == 1){
		//	break;
		//}
		int nexty = availy[rand()%availynums];
		int nextx = --_top[nexty];
		if(whoseNode == COMPUTER_NODE){
			_board[nextx][nexty] = HUMAN_PUT;
			whoseNode = HUMAN_NODE;
		}else{
			_board[nextx][nexty] = COMPUTER_PUT;
			whoseNode = COMPUTER_NODE;
		}
		if(nexty == this->noy && (_top[nexty] - 1) == this->nox){
			_top[nexty]--;
		}
		nodestate = this->checkNodeState(_board,_top,whoseNode,nextx,nexty);
		//nodestate = NOTEND;
	}
	for (int i = 0; i < this->M; i++)
		delete[] _board[i];
	delete[] _board;
	delete[] availy;
	delete[] _top;
	//_cprintf("outdefaultpolicy2\n");
	return nodestate;
}