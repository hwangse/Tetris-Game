#include "tetris.h"
int prev_num;	//rank 파일에 적혀진 전체 개수
int sub_num;	//수정된 rank파일 원소의 개수	
static struct sigaction act, oact;

Node* head=NULL;

RecNode* root;

int main(){
	int exit=0;

	initscr();
	noecho();
	keypad(stdscr, TRUE);	

	srand((unsigned int)time(NULL));

	createRankList();

	/*recommended Node생성*/
	root=(RecNode*)malloc(sizeof(RecNode));
	root->f=field;
	root->lv=0;
	root->score=0;

	while(!exit){
		clear();
		switch(menu()){
		case MENU_PLAY: play(); break;
		case MENU_RANK: rank(); break;
		case MENU_RPLAY: recommendedPlay(); break;
		case MENU_EXIT: exit=1; break;
		default: break;
		}
	}

	writeRankFile();

	endwin();
	system("clear");
	return 0;
}

void InitTetris(){
	int i,j;

	for(j=0;j<HEIGHT;j++)
		for(i=0;i<WIDTH;i++)
			field[j][i]=0;

	nextBlock[0]=rand()%7;
	nextBlock[1]=rand()%7;
	nextBlock[2]=rand()%7; //추가) 블럭 1개더 추가

	recommend(root);
	blockRotate=0;
	blockY=-1;
	blockX=WIDTH/2-2;
	score=0;	
	gameOver=0;
	timed_out=0;

	DrawOutline();
	DrawField();
	DrawBlockWithFeatures(blockY,blockX,nextBlock[0],blockRotate);
	DrawNextBlock(nextBlock);
	PrintScore(score);
}

void DrawOutline(){	
	int i,j;
	/* 블럭이 떨어지는 공간의 태두리를 그린다.*/
	DrawBox(0,0,HEIGHT,WIDTH);

	/* next block을 보여주는 공간의 태두리를 그린다.*/
	move(2,WIDTH+10);
	printw("NEXT BLOCK");
	DrawBox(3,WIDTH+10,4,8);

	/*다다음 블록을 보여주는 공간의 테두리를 그린다.*/
	move(9,WIDTH+10);
	DrawBox(10,WIDTH+10,4,8);

	/* score를 보여주는 공간의 태두리를 그린다.*/
	move(17,WIDTH+10);
	printw("SCORE");
	DrawBox(18,WIDTH+10,1,8);
}

int GetCommand(){
	int command;
	command = wgetch(stdscr);
	switch(command){
	case KEY_UP:
		break;
	case KEY_DOWN:
		break;
	case KEY_LEFT:
		break;
	case KEY_RIGHT:
		break;
	case ' ':	/* space key*/
		/*fall block*/
		break;
	case 'q':
	case 'Q':
		command = QUIT;
		break;
	default:
		command = NOTHING;
		break;
	}
	return command;
}

int ProcessCommand(int command){
	int ret=1;
	int drawFlag=0;
	switch(command){
	case QUIT:
		ret = QUIT;
		break;
	case KEY_UP:
		if((drawFlag = CheckToMove(field,nextBlock[0],(blockRotate+1)%4,blockY,blockX)))
			blockRotate=(blockRotate+1)%4;
		break;
	case KEY_DOWN:
		if((drawFlag = CheckToMove(field,nextBlock[0],blockRotate,blockY+1,blockX)))
			blockY++;
		break;
	case KEY_RIGHT:
		if((drawFlag = CheckToMove(field,nextBlock[0],blockRotate,blockY,blockX+1)))
			blockX++;
		break;
	case KEY_LEFT:
		if((drawFlag = CheckToMove(field,nextBlock[0],blockRotate,blockY,blockX-1)))
			blockX--;
		break;
	default:
		break;
	}
	if(drawFlag) DrawChange(field,command,nextBlock[0],blockRotate,blockY,blockX);
	return ret;	
}

void DrawField(){
	int i,j;
	for(j=0;j<HEIGHT;j++){
		move(j+1,1);
		for(i=0;i<WIDTH;i++){
			if(field[j][i]==1){
				attron(A_REVERSE);
				printw(" ");
				attroff(A_REVERSE);
			}
			else printw(".");
		}
	}
}


void PrintScore(int score){
	move(19,WIDTH+11);
	printw("%8d",score);
}

void DrawNextBlock(int *nextBlock){
	int i, j;
	/*다음 블럭 그리기 (nextBlock[1])*/
	for( i = 0; i < 4; i++ ){ 
		move(4+i,WIDTH+13);
		for( j = 0; j < 4; j++ ){
			if( block[nextBlock[1]][0][i][j] == 1 ){
				attron(A_REVERSE);
				printw(" ");
				attroff(A_REVERSE);
			}

			else printw(" ");
		}
	}
	/*추가) 다다음 블럭 그리기 (nextBlock[2])*/
	for( i = 0; i < 4; i++ ){
		move(11+i,WIDTH+13);
		for( j = 0; j < 4; j++ ){
			if( block[nextBlock[2]][0][i][j] == 1 ){
				attron(A_REVERSE);
				printw(" ");
				attroff(A_REVERSE);
			}

			else printw(" ");
		}
	}
	

}

void DrawBlock(int y, int x, int blockID,int blockRotate,char tile){
	int i,j;
	for(i=0;i<4;i++)
		for(j=0;j<4;j++){
			if(block[blockID][blockRotate][i][j]==1 && i+y>=0){
				move(i+y+1,j+x+1);
				attron(A_REVERSE);
				printw("%c",tile);
				attroff(A_REVERSE);
			}
		}

	move(HEIGHT,WIDTH+10);
}

void DrawBox(int y,int x, int height, int width){
	int i,j;
	move(y,x);
	addch(ACS_ULCORNER);
	for(i=0;i<width;i++)
		addch(ACS_HLINE);
	addch(ACS_URCORNER);
	for(j=0;j<height;j++){
		move(y+j+1,x);
		addch(ACS_VLINE);
		move(y+j+1,x+width+1);
		addch(ACS_VLINE);
	}
	move(y+j+1,x);
	addch(ACS_LLCORNER);
	for(i=0;i<width;i++)
		addch(ACS_HLINE);
	addch(ACS_LRCORNER);
}

void play(){
	int command;
	clear();
	act.sa_handler = BlockDown;
	sigaction(SIGALRM,&act,&oact);
	InitTetris();
	do{
		if(timed_out==0){
			alarm(1);
			timed_out=1;
		}

		command = GetCommand();
		if(ProcessCommand(command)==QUIT){
			alarm(0);
			DrawBox(HEIGHT/2-1,WIDTH/2-5,1,10);
			move(HEIGHT/2,WIDTH/2-4);
			printw("Good-bye!!");
			refresh();
			getch();

			return;
		}
	}while(!gameOver);

	alarm(0);
	getch();
	DrawBox(HEIGHT/2-1,WIDTH/2-5,1,10);
	move(HEIGHT/2,WIDTH/2-4);
	printw("GameOver!!");
	refresh();
	getch();
	newRank(score);
}

char menu(){
	printw("1. play\n");
	printw("2. rank\n");
	printw("3. recommended play\n");
	printw("4. exit\n");
	return wgetch(stdscr);
}

int CheckToMove(char f[HEIGHT][WIDTH],int currentBlock,int blockRotate, int blockY, int blockX){
	/*블록이 해당 위치로 이동하거나 회전할 수 있다면 1 아니면 0 반환*/
	int i,j;

	for(i=0;i<4;i++)
	{
		for(j=0;j<4;j++)
		{
			if(block[currentBlock][blockRotate][i][j]==1)
			{
				if(blockX+j>=WIDTH || blockX+j<0)
					return 0;
				if(blockY+i>=HEIGHT || blockY+i<0)
					return 0;
				if(f[i+blockY][j+blockX]==1)
					return 0;

			}
		}
	}
	return 1;
}
void DrawChange(char f[HEIGHT][WIDTH],int command,int currentBlock,int blockRotate, int blockY, int blockX){
	int i,j;
	int blk = currentBlock,rot = blockRotate, y = blockY, x = blockX;
	int shadow;

	/*command를 통해 이전 (x,y)찾기*/
	switch(command){
	case KEY_UP:
		rot=(rot+3)%4;
		break;
	case KEY_DOWN:
		y--;
		break;
	case KEY_LEFT:
		x++;
		break;
	case KEY_RIGHT:
		x--;
		break;
	}
	/*추가) 이전 그림자 y위치 찾기*/
	shadow=y;
	while(CheckToMove(f,blk,rot,shadow,x)==1)
		shadow++;
	shadow -= 1;

	/*이전 블록의 흔적 모두 없애기*/
	for(i=0;i<4;i++)
		for(j=0;j<4;j++){
			if(block[blk][rot][i][j]==1){
				if(i+y>=0){ 
					move(i+y+1,j+x+1);
					printw(".");
				}
				/*추가) 이전 그림자 흔적 없애기*/
				if(i+shadow>=0)
				{
					move(shadow+i+1,j+x+1);
					printw(".");
				}
			}
		}
	/*추가)DrawBlockWithFeatures 함수를 통해 DrawBlock 과 DrawShadow를 모두 호출*/
	DrawBlockWithFeatures(blockY,blockX,currentBlock,blockRotate);
	move(HEIGHT,WIDTH+10);
}
void BlockDown(int sig){

	if(CheckToMove(field,nextBlock[0],blockRotate,blockY+1,blockX)) //블록이 내려갈수 있는 상태라면 
	{
		blockY++; //블록을 내리고
		DrawChange(field,KEY_DOWN,nextBlock[0],blockRotate,blockY,blockX); //변화된 모습을 화면상에 그려준다.
	}
	else //블록이 내려갈 수 없는 상태라면
	{
		if(blockY==-1) //블럭이 쌓일 수 있는 공간이 존재하지 않을때 
			gameOver=1; //게임 종료
		else //블록이 필드에 쌓이는 경우에 
		{
			score += AddBlockToField(field,nextBlock[0],blockRotate,blockY,blockX); //touced*10만큼의 점수가 올라감 
		    	score += DeleteLine(field); //지워진라인^2*100 만큼 점수 증가
			
			/*다음 블록 랜덤 할당*/
			nextBlock[0]=nextBlock[1];
			nextBlock[1]=nextBlock[2];
			/*추가) 다다음 블록 랜덤 할당*/
			nextBlock[2]=rand()%7;
			DrawNextBlock(nextBlock);
			
			recommend(root);
			PrintScore(score);

			blockY=-1;
			blockX=(WIDTH/2)-2;
			blockRotate=0;

			DrawField();
			DrawBlockWithFeatures(blockY,blockX,nextBlock[0],blockRotate);
		}
	}

	timed_out=0;
}
/*BlockDown에서 호출되는 함수로 블럭이 필드상에서 쌓이는 경우*/
int AddBlockToField(char f[HEIGHT][WIDTH],int currentBlock,int blockRotate, int blockY, int blockX){
	int i,j;
	int touched=0;

	for(i=0;i<4;i++)
	{
		for(j=0;j<4;j++)
		{
			if(block[currentBlock][blockRotate][i][j]==1) //블럭의 모양대로
			{
				f[blockY+i][blockX+j]=1; //필드를 채워넣음
				
				if(f[blockY+i+1][blockX+j]==1 || blockY+i+1==HEIGHT) //맨 아래 놓이는 블럭이거나 이미 놓인 블럭과 접촉시에
					touched++; 
			}

		}
	}

	return touched*10;
}

int DeleteLine(char f[HEIGHT][WIDTH]){
	int DLine=0,flag=0;
	int i,j,k;
	/*지워야할 라인 수 세기*/
	for(i=0;i<HEIGHT;i++)
	{	
		flag=1;
		for(j=0;j<WIDTH;j++)
		{
			if(f[i][j]==0)
			{
				flag=0; //지워질수 있는 줄이 아니다
				break; 
			}
		}
		if(flag==1) //지워질수 있는 줄이라면
		{
			/*지워야 할 라인 위의 원소 옮기기*/
			DLine++;
			for(k=i-1;k>=0;k--)
				for(j=0;j<WIDTH;j++)
					f[k+1][j]=f[k][j];
		}
	}

	return DLine*DLine*100;
}

void DrawShadow(int y, int x, int blockID,int blockRotate){
	/*CheckToMove를 이용하여 더 이상 움직일 수 없는 y값 찾기*/

	while(CheckToMove(field,blockID,blockRotate,y,x)==1) //더이상 갈 수 없을때까지 블럭 내리기
		y++;
	DrawBlock(y-1,x,blockID,blockRotate,'/'); //그 때의 블럭을 '/'로 그려준다.

}

void DrawBlockWithFeatures(int y,int x, int blockID,int blockRotate){

	DrawRecommend();
	DrawBlock(y,x,blockID,blockRotate,' ');
	DrawShadow(y,x,blockID,blockRotate);
}
/*score와 Name을 받아서 새로운 노드를 점수 내림차순으로 생성해주는 함수*/
void InsertNode(int tempScore,char tempName[]){

	Node* pNew,*temp=head,*prev=head;

	 		pNew=(Node*)malloc(sizeof(Node));
			pNew->score=tempScore;
			strcpy(pNew->name,tempName);
			pNew->link=NULL;

			if(head==NULL)//linkedlist가 비어있었을 경우
				head=pNew;
			else if(head->link==NULL) //linkedlist에 하나의 원소만 존재할 때
			{
				if(head->score>tempScore)
					head->link=pNew;
				else{
					pNew->link=head;
					head=pNew;
				}
			}
			else{ 	//linkedlist의 중간에 노드 삽입할 경우
				temp=head->link;
				while(temp->link!=NULL && temp->score>tempScore){
					temp=temp->link;
					prev=prev->link;
				}

				if(temp->link==NULL) //새 노드가 맨 끝에 놓일지를 판단
				{
					if(temp->score>tempScore)
						temp->link=pNew;
					else{
						prev->link=pNew;
						pNew->link=temp;
					}

				}
				else	//새 노드가 linkedlist의 중간에 놓일때
				{
					prev->link=pNew;
					pNew->link=temp;
				}
			}

}
/*프로그램 처음 부분에서 호출되어 랭킹을 생성*/
void createRankList(){
	FILE *fp=fopen("rank.txt","r");
	Node* pNew,*temp=head,*prev=head;
	int i,tempScore;
	char tempName[NAMELEN];

	if(fp!=NULL) //파일이 정상적으로 열렸다면
	{
		fscanf(fp,"%d",&prev_num);
		sub_num=prev_num;
		
		for(i=0;i<prev_num;i++){
			fscanf(fp,"%s %d",tempName,&tempScore);

			InsertNode(tempScore,tempName);
		}

		fclose(fp);
		return;
	}
	else{	//rank.txt파일이 정상적으로 열리지 않았을 때
		printw("file open error\n");
		return;
	}
}
void PrintNode(int x,int y)
{
	Node* temp=head;
	int i=0;

	if(head!=NULL){
		if(x!=0){
			for(i=0;i<x;i++)
				temp=temp->link;
		}

		for(;i<=y;i++){
			printw(" %s		| %d        \n",temp->name,temp->score);
			temp=temp->link;
		}
	}
	else
		printw("Empty List\n");

}

void rank(){
	int x=0,y=0,searchFlag=0;
	int i=0;
	char tempName[NAMELEN];
	Node *temp;
	clear();

	printw("1. list ranks from X to Y\n");
	printw("2. list ranks by a specific name\n");
	printw("3. delete a specific rank\n");
	switch(wgetch(stdscr)){
		case '1' :
		echo();
		printw("X : ");
		scanw("%d",&x);
		printw("Y : ");
		scanw("%d",&y);
		noecho();
		if(x<=y || (x==0 || y==0)){
			printw("	name	|	score	\n");
			printw("--------------------------------\n");
		
			if(y>sub_num){		//over the boundary
				return;
			}



			if(x==0 && y==0){
				PrintNode(0,sub_num-1);	
			}
			else if(x==0 && y!=0){
				PrintNode(0,y-1);
			}
			else if(x!=0 && y==0){
				PrintNode(x-1,sub_num-1);
				
			}
			else{
				PrintNode(x-1,y-1);
			}
		}
		else{
			printw("Search failure : no rank in the list\n");
		}

		refresh();
		getch();
		break;

		case '2' :

		echo();
		printw("Input the name: ");
		scanw("%s",tempName);
		noecho();

		printw("	name	|	score	\n");
		printw("--------------------------------\n");

		temp=head;
		while(temp!=NULL){
			if(!strcmp(temp->name,tempName)){
				printw(" %s		| %d        \n",temp->name,temp->score);
			searchFlag=1;	
			}
			temp=temp->link;
		}
		if(searchFlag==0)
			printw("search failure : no name in the list\n");

		refresh();
		getch();
		break;

		case '3' : 
		echo();
		printw("Input the rank : ");
		scanw("%d",&x);
		noecho();
		
		if(x<=sub_num	&& x>=1){		//입력된 수의 랭킹이 존재할때
			DeleteNode(x);
			printw("\nresult : the rank deleted\n");
		}
		else
			printw("\nsearch failure : the rank not in the list\n");

		refresh();
		getch();
		break;
		default : break;
	}

		

}
void DeleteNode(int n)
{
	Node* delete=head->link;
	Node* prev=head;
	int i;

	
	if(!head)
		return;
	else{
		if(n==1){
			head=head->link;
			free(prev);
		}
		else{
			for(i=2;i<n;i++)
			{
				delete=delete->link;
				prev=prev->link;
			}
			prev->link=delete->link;
			free(delete);
		}

	}
	sub_num--;

}

void writeRankFile(){
	Node* temp=head;
	FILE* fp;

		fp=fopen("rank.txt","w");
		fprintf(fp,"%d\n",sub_num);
		while(temp!=NULL){
			fprintf(fp,"%s ",temp->name);
			fprintf(fp,"%d\n",temp->score);

			temp=temp->link;
		}

		fclose(fp);
}

void newRank(int score){
	clear();
	char str[NAMELEN];

	echo();
	printw("your name : ");
	scanw("%s",str);
	noecho();

	InsertNode(score,str);
	sub_num++;
	return;
}

void DrawRecommend(){
	if(CheckToMove(field,nextBlock[0],recommendR,recommendY,recommendX)==1)
		DrawBlock(recommendY,recommendX,nextBlock[0],recommendR,'R');
}

int recommend(RecNode *root){
	int max=0,extra=0,ymax=0;	//extra는 효율적인 recommend를 위한 추가첨수 	
	int i,j,r,PMove=0,PRotate,temp=0;
	int h,w;
	int x,y;
	int res=0;
	int n=root->lv;		//nextBlock배열에서의 현재블럭 인덱스를 저장
	RecNode* t;

	int flag=0;
	int tempR,tempX,tempY;

	PRotate=4;		//r은 회전가능한 수(0,4,5,6 번 블럭을 제외한 다른 블럭은 4번 회전가능)
	switch(nextBlock[n]){
		case 0 : PRotate=2; break;
		case 4 : PRotate=1; break;
		case 5 : PRotate=2; break;
		case 6 : PRotate=2; break;
	}
	/*가능한 회전수 만큼 루프를 돌린다*/
	for(r=0;r<PRotate;r++){

		/*회전에 따른 각 블럭마다 x축으로 움직일 수 있는 횟수를 구한다 O(1)*/
 		   PMove=0;
		for(i=0;i<4;i++){
			for(j=0;j<4;j++){
				if(block[nextBlock[n]][r][j][i]){
					PMove++;
					break;
				}
			}
		}
		PMove=WIDTH-PMove+1;//보정된 PMove 값

		/*PRotate loop 안의 각각의 x좌표에 대한 PMove loop*/
		for(i=temp;i<temp+PMove;i++){
			(root->c)[i]=(RecNode*)malloc(sizeof(RecNode));  //child node 동적할당
			t=(root->c)[i];

			/*child node에 대한 정보 저장. 레벨&필드&스코어*/
			t->lv=n+1;//child node의 레벨은 root노드보다 하나 더 크게
			t->score=root->score;

			/*이전 필드 정보를 현재의 c노드에 일단 저장*/
			t->f=(char (*)[WIDTH])malloc(sizeof(char)*HEIGHT*WIDTH);

			for(h=0;h<HEIGHT;++h)
				for(w=0;w<WIDTH;++w)
					t->f[h][w]=root->f[h][w];

			x=i-temp;
			y=0;
			if(CheckToMove(t->f,nextBlock[n],r,y,x)){	//최대로 내려갈 수 있는 y값 구하기
				while(CheckToMove(t->f,nextBlock[n],r,++y,x));
			y -= 1;
			}
			else
				continue;
			
			/*만약 블럭이 필드에 놓일 수 있을 때 점수를 구함*/
			extra = CheckTouched(t->f,nextBlock[n],r,y,x);
			if(y>ymax){
			    ymax=y;
			    extra ++;
			}

			t->score += AddBlockToField(t->f,nextBlock[n],r,y,x);
			t->score += DeleteLine(t->f);

			if(n<VISIBLE_BLOCKS-1)
				res=recommend(t);
			else //재귀 종료조건
			    res=t->score;
			if(res+extra>max) //현재 구해진 점수가 이전까지 구해진 점수보다 더 클 때
			{
			    max=res;
			    tempR=r;
			    tempY=y;
			    tempX=x;
			    flag=1;
			}
			

				}
		temp += PMove;
	}	
	if(n==0 && flag){
		recommendR=tempR;
		recommendY=tempY;
		recommendX=tempX;
	}
	return max;
		}
int CheckTouched(char f[HEIGHT][WIDTH],int BlockID,int r,int y,int x)
{
    int touched=0;
    int i,j;

    for(i=0;i<4;i++)
	for(j=0;j<4;j++)
	{
		if(block[BlockID][r][i][j]==1)
		{
		    if(f[y+i][x+j+1]==1 || f[y+i][x+j-1]==1)
			touched++;
		    if(blockX+j+1==WIDTH || blockX+j==0)
			touched++;
		}

	}
    return touched;
}
void recommendedPlay(){
	int command;
	clear();
	act.sa_handler = autoPlay;
	sigaction(SIGALRM,&act,&oact);
	InitTetris();
	do{
		if(timed_out==0){
			alarm(1);
			timed_out=1;
		}

		command = GetCommand();
		if(ProcessCommand(command)==QUIT){
			alarm(0);
			DrawBox(HEIGHT/2-1,WIDTH/2-5,1,10);
			move(HEIGHT/2,WIDTH/2-4);
			printw("Good-bye!!");
			refresh();
			getch();

			return;
		}
	}while(!gameOver);

	alarm(0);
	getch();
	DrawBox(HEIGHT/2-1,WIDTH/2-5,1,10);
	move(HEIGHT/2,WIDTH/2-4);
	printw("GameOver!!");
	refresh();
	getch();

}
void autoPlay(int sig){
    int i;
    if(!CheckToMove(field,nextBlock[0],blockRotate,blockY,blockX)) gameOver=1;
	else{
		score+=AddBlockToField(field,nextBlock[0],recommendR,recommendY,recommendX);
		score+=DeleteLine(field);
		blockY=-1;blockX=(WIDTH/2)-2;blockRotate=0;
		for(i=0;i<VISIBLE_BLOCKS-1;++i){
			nextBlock[i] = nextBlock[i+1];
		}
		nextBlock[VISIBLE_BLOCKS-1] = rand()%7;
		recommend(root);
		DrawNextBlock(nextBlock);
		PrintScore(score);
		DrawField();
		DrawBlockWithFeatures(blockY,blockX,nextBlock[0],blockRotate);
		timed_out=0;
	}

}
