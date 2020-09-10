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
					 int x, int y, StateNode* parent, int nox, int noy):
M(M),N(N),x(x),y(y),nox(nox),noy(noy),whoseNode(whoseNode),parent(parent)
{
	//_cprintf("in_statenode\n");
	int** _board = new int*[M];
	int* _board_1 = new int [M * N];
	for(int i = 0; i < M; i++){
		_board[i] = _board_1 + N * i;
		//_board[i] = new int [N];
		for(int j = 0; j < N; j++){
			_board[i][j] = board[i][j];
		}
	}
	//_cprintf("finish_board\n");
	this->board = _board;
	int* _top = new int[N];
	for(int i = 0; i < N; i++){
		_top[i] = top[i];
	}
	this->top = _top;
	this->profit = 0;
	this->visitCount = 0;
	this->sub_node_cnt = 0;

	this->expandIndex = new int[N];
	this->canExpandNUms = 0;
	this->child = new StateNode*[N];
	for(int i = 0; i < N; i++){
		this->child[i] = nullptr;
		if(this->top[i] != 0){
			this->expandIndex[this->canExpandNUms++] = i;
		}
	}
	this->nodestate = checkState();
}

int StateNode::checkState()
{
	if(this->x == -1 && this->y == -1){
		return NOTEND;
	}
	if ((whoseNode == COMPUTER_NODE && machineWin(this->x, this->y, this->M, this->N, this->board))){
		this->profit = 1e10;
		return COMPUTER_WIN;
	}
	if((whoseNode == HUMAN_NODE && userWin(this->x, this->y, this->M, this->N, this->board))){
		this->profit = 1e10;
		return HUMAN_WIN;
	}
	if((isTie(this->N, this->top))){
		this->profit = 0;
		return TIE;
	}
	return NOTEND;
}

bool StateNode::isExpandable()
{
	return this->canExpandNUms>0;
}
void StateNode::clear()
{
	delete [] this->board[0];
	//for(int i = 0; i<this->M; i++)
	//delete [] this->board[i];
	delete [] this->board;
	delete [] this->top;
	delete [] this->expandIndex;
	for (int i = 0; i < this->N; i ++){
		if (this->child[i]) {
			this->child[i] -> clear();
			delete this->child[i];
		}
	}
	delete [] this->child;
}

StateNode* StateNode::expand(){
	int idx = rand() % this->canExpandNUms;
	int expand_y = this->expandIndex[idx];
	int expand_x = -- this->top[expand_y];
	int next_whose_node;
	if(this->whoseNode == COMPUTER_NODE){
		this->board[expand_x][expand_y] = HUMAN_PUT;
		next_whose_node = HUMAN_NODE;
	}else{
		this->board[expand_x][expand_y] = COMPUTER_PUT;
		next_whose_node = COMPUTER_NODE;
	}
	if(expand_y == this->noy && (this->top[expand_y] - 1) == this->nox){
		this->top[expand_y]--;
	}
	this->child[expand_y] = new StateNode(this->board,this->top,this->M,this->N,next_whose_node,expand_x,expand_y,this,this->nox,this->noy);
	this->top[expand_y] = expand_x+1;
	this->board[expand_x][expand_y] = 0;
	this->expandIndex[idx] = this->expandIndex[--this->canExpandNUms];
	return this->child[expand_y];
}
void StateNode::backward(double deltaProfit){
	this->visitCount += 1;
	this->profit += deltaProfit;
	if(this->parent != nullptr){
		this->parent->backward(-deltaProfit);
	}
}
void StateNode::check_prune(){
	if(this->nodestate == NOTEND){
		for(int i = 0; i< this->N; i++){
			if(this->child[i] != nullptr){
				int who_win = this->child[i]->get_node_state();
				if(who_win * this->get_whose_node() == -1){
					this->nodestate = who_win;
					this->profit = -1e10;
					if(this->parent){
						this->parent->check_prune();
					}
					break;
				}
			}
		}
	}
	if(!this->isExpandable()){
		int flag = 0;
		for(int i = 0; i< this->N; i++){
			if(this->child[i] != nullptr){
				int who_win = this->child[i]->get_node_state();
				if(who_win * this->get_whose_node() != 1){
					flag = 1;
					break;
				}
			}
		}
		if(flag == 0){
			this->nodestate = this->get_whose_node();
			this->profit = 1e10;
			if(this->parent){
				this->parent->check_prune();
			}
		}
	}
}
bool StateNode::is_terminal_node(){
	return this->nodestate != NOTEND;
}
Point StateNode::get_point(){
	return Point(this->x,this->y);
}
long StateNode::get_visits_cnt(){
	return this->visitCount;
}
double StateNode::get_profit(){
	return this->profit;
}
int StateNode::get_whose_node(){
	return this->whoseNode;
}
int StateNode::get_node_state(){
	return this->nodestate;
}
StateNode* StateNode::get_child(int idx){
	if(idx < 0 || idx >= this->N){
		return nullptr;
	}else{
		return this->child[idx];
	}
}
void StateNode::set_child(int idx, StateNode* node){
	if(idx >= 0 && idx < this->N){
		this->child[idx] = nullptr;
	}
}
StateNode* StateNode::get_parent(){
	return this->parent;
}
void StateNode::set_parent(StateNode* node){
	this->parent = nullptr;
}
StateNode* StateNode::best_child(double cofficient){
	StateNode* best;
	double maxUCBValue = -1e200;
	StateNode **cand = new StateNode*[this->N];
	int cand_cnt = 0;
	for (int i = 0; i != this->N; i++) {
		if (this->child[i] == nullptr) continue;
		long childvisitCnt = this->child[i]->get_visits_cnt();
		double profit = this->child[i]->get_profit();
		double ucbvalue = double(profit) / childvisitCnt + 
			sqrtl(2 * logl(double(this->visitCount)) / childvisitCnt) * cofficient; 
		if (ucbvalue > maxUCBValue) {
			maxUCBValue = ucbvalue;
			cand_cnt = 1;
			cand[0] = this->child[i];
		}else if (abs(ucbvalue - maxUCBValue) < 1e-20){
			//_cprintf("i came here");
			cand[cand_cnt++] = this->child[i];
		}
	}
	best = cand[rand()%cand_cnt];
	delete cand;
	//_cprintf("outbestchild\n");
	return best;
}
double StateNode::simulate(){
	//_cprintf("in_simulate\n");
	int* avail_y = new int[this->N];
	int avail_y_cnt = 0;
	for( int i = 0; i < this->N; i++){
		if(this->top[i] != 0){
			avail_y[avail_y_cnt++] = i;
		}
	}
	while(this->nodestate == NOTEND){
		//this->nodestate = this->check_son_state(avail_y,avail_y_cnt);
		//if(this->nodestate != NOTEND){
		//	break; 
		//}
		int idx = rand()%avail_y_cnt;
		int next_y = avail_y[idx];
		int next_x = --this->top[next_y];
		if(this->whoseNode == COMPUTER_NODE){
			this->board[next_x][next_y] = HUMAN_PUT;
			this->whoseNode = HUMAN_NODE;
		}else{
			this->board[next_x][next_y] = COMPUTER_PUT;
			this->whoseNode = COMPUTER_NODE;
		}
		if(next_y == this->noy && (this->top[next_y] - 1) == this->nox){
			this->top[next_y]--;
		}
		if(this->top[next_y] == 0){
			avail_y[idx] = avail_y[--avail_y_cnt];
		}
		this->x = next_x;
		this->y = next_y;
		this->nodestate = this->checkState();
	}
	delete[] avail_y;
	//_cprintf("out_simulate\n");
	return this->nodestate;
}

StateNode* StateNode::copy(){
	//_cprintf("in_copy\n");
	StateNode* cp = new StateNode(this->board,this->top,this->M,this->N,this->whoseNode,this->x,this->y,nullptr,this->nox,this->noy);
	//_cprintf("out_copy\n");
	return cp;
}
void StateNode::print_profit(){
	for(int i = 0; i < this->N; i++){
		if(this->child[i] != nullptr){
			_cprintf("child %d,state:%d, profit:%f,count:%ld,%f\n",i,this->child[i]->nodestate,this->child[i]->profit,this->child[i]->visitCount,
				this->child[i]->profit/this->child[i]->visitCount);
		}
	}
	_cprintf("\n");
}
void StateNode::print_board(){
	_cprintf("board\n");
	for(int i = 0; i < this->M; i++){
		for(int j = 0; j < this->N; j++){
			if(i == this->nox && j == this->noy){
				_cprintf("X ");
			}else if(this->board[i][j] == HUMAN_PUT){
				_cprintf("H ");
			}else if(this->board[i][j] == COMPUTER_PUT){
				_cprintf("M ");
			}else{
				_cprintf(". ");
			}
		}
		_cprintf("\n");
	}
	_cprintf("\n");
}

long StateNode::dfs(){
	long cnt = 0;
	//this->print_profit();
	for(int i = 0; i < this->N; i++){
		if(this->child[i]){
			//_cprintf("child_%d\n", i);
			//this->print_board();
			cnt += 1;
		}
	}
	if(this->nodestate != NOTEND){
		return cnt;
	}
	for(int i = 0; i < this->N; i++){
		if(this->child[i]){
			//_cprintf("dfs\n");
			cnt += this->child[i]->dfs();
		}
	}
	this->sub_node_cnt = cnt;
	return cnt;
}
int StateNode::check_son_state(int* avail_y, int avail_y_cnt){
	int flag = 0;
	int back_x = this->x;
	int back_y = this->y;
	int ret = 0;
	this->whoseNode *= -1;
	for(int i=0; i < avail_y_cnt; i++){
		int next_y = avail_y[i];
		int next_x = --this->top[next_y];
		if(this->whoseNode ==HUMAN_NODE){
			this->board[next_x][next_y] = HUMAN_PUT;
		}else{
			this->board[next_x][next_y] = COMPUTER_PUT;
		}
		if(next_y == this->noy && (this->top[next_y] - 1) == this->nox){
			this->top[next_y]--;
		}
		this->x = next_x;
		this->y = next_y;
		this->nodestate = this->checkState();
		if(this->nodestate != NOTEND){
			flag = 1;
			ret = this->nodestate;
		}
		this->board[next_x][next_y] = 0;
		this->top[next_y] = next_x + 1;
		if(flag == 1){
			break;
		}
	}
	this->x = back_x;
	this->y = back_y;
	this->whoseNode *= -1;
	if(flag==1){
		return ret;
	}else{
		return NOTEND;
	}
}



UCT::UCT(int **board, const int *top, 
		 int M, int N, int whoseNode, 
		 int x, int y, int nox, int noy)
{
	this->root = new StateNode(board, top, M, N, whoseNode, x, y, nullptr, nox, noy);
	//this->seed = 0;
	this->c = COEFFICIENT;
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
							  if(newgameflag || UCT::pUctSingle->root->get_child(y) == nullptr){
								  UCT::pUctSingle->root->clear();
								  delete UCT::pUctSingle->root;
								  UCT::pUctSingle->root = new StateNode(board, top, M, N, whoseNode, x, y, nullptr, nox, noy);
							  }else{
								  UCT::pUctSingle->change_root(y);
							  }
						  }
						  if(!newgameflag){
							  UCT::pUctSingle->c *= DECAY;
							  if(UCT::pUctSingle->c < MINCOEF){
								  UCT::pUctSingle->c = MINCOEF;
							  }
						  }
						  //_cprintf("begin\n");
						  //srand(UCT::pUctSingle->seed);
						  srand(unsigned int(UCT::pUctSingle->starttime));
						  //UCT::pUctSingle->seed += 1;
						  return UCT::pUctSingle;
}


UCT::~UCT(void)
{
	this->root->clear();
	delete this->root;
	delete UCT::pUctSingle;
}


Point UCT::best_action(){
	long count = 0;
	//_cprintf("in_best_action\n");
	while ((++count % 10 != 0) || long(clock()) - this->starttime <= TIMELIMIT) { //尚未耗尽计算时长 
		if(root->is_terminal_node()){
			break;
		}
		StateNode *selectedNode = this->TreePolicy(); //运用搜索树策略节点 
		double deltaProfit = this->DefaultPolicy(selectedNode); //运用模拟策略对选中节点进行一次随机模拟 
		selectedNode->backward(deltaProfit); //将模拟结果回溯反馈给各祖先 
	}
	Point p = this->root->best_child(0.0)->get_point();
	//_cprintf("bestchild:x:%d,y:%d\n",p.x,p.y);
	//_cprintf("you choose %d\n", this->root->get_point().y);
	//_cprintf("duringsearch,count:%d,time(ms):%ld\n",count,long(clock())-this->starttime);
	//this->root->print_profit();
	//_cprintf("choose %d\n", p.y);
	//this->root->get_child(p.y)->print_profit();
	//_cprintf("\n");
	//_cprintf("\n");
	//this->root->dfs();
	//_cprintf("sub_node_cnt:%d\n",this->root->sub_node_cnt);
	this->change_root(p.y);
	return p;
}

//
StateNode* UCT::TreePolicy(){
	//_cprintf("intreepolicy\n");
	StateNode* currentNode = this->root;
	while(!currentNode->is_terminal_node()){  
		if(currentNode->isExpandable()){
			currentNode = currentNode->expand();
			break;
		}else{
			currentNode = currentNode->best_child(this->c);
		}
	}
	//_cprintf("outpolicy\n");
	return currentNode;
}

//模拟下棋，返回胜负
double UCT::DefaultPolicy(StateNode* node){
	//_cprintf("in_default_policy\n");
	if(node->get_node_state() != NOTEND){
		//_cprintf("check_prune\n");
		node->get_parent()->check_prune();
		return node->get_node_state() * node->get_whose_node();
	}
	StateNode* sim_node = node->copy();
	double ret = sim_node->simulate();
	sim_node->clear();
	delete sim_node;
	//_cprintf("out_default_policy\n");
	return ret * node->get_whose_node();
}

void UCT::change_root(int y){
	//_cprintf("in_change_root\n");
	StateNode* new_root;
	new_root = this->root->get_child(y);
	this->root->set_child(y, nullptr);
	this->root->clear();
	delete this->root;
	this->root = new_root;
	this->root->set_parent(nullptr);
}