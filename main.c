#include <stdio.h>
#include <termios.h>
#include <unistd.h> 
#include <stdlib.h>
#include <time.h>
struct vser {
	char name;
	int xl;
	int xr;
	int y;
	int k;
	int opt;
	int blood;
};
/*0 12 1
* 6 ms 3
* 3 54 2
*/
#define up_left 0
#define up_right 1
#define down_right 2
#define down_left 3

#define mapa 15
/*   0
 * 3 c 1
 *   2
 */   
#define up 0
#define right 1
#define down 2
#define left 3

char map[mapa][mapa];

void delay_ms(unsigned int ms) {
	struct timespec ts;
	ts.tv_sec = ms / 1000;		  // 秒
	ts.tv_nsec = (ms % 1000) * 1000000; // 纳秒（1ms = 1,000,000 ns）
	nanosleep(&ts, NULL);
}

void setwall(){
	for(int i=0; i < mapa; i++){
		for(int j=0; j < mapa; j++){
			map[i][j] = (i*j == 0 || (mapa -1 - i)*(mapa -1 - j) == 0) ? 'W' : ' ';
		}
	}
}

void clearmap(){
	for(int i=0; i < mapa; i++){
		for(int j=0; j < mapa; j++){
			map[i][j] = ' ';
		}
	}
}
int getk(int n, int k, char where){
	if(where == 'y'){
		if(k == 6 || k == 3){
			return n;	
		}else if(k == 2 || k == 1){
			return n-1;
		}else if(k == 5 || k == 4){
			return n+1;
		}
	}else if(where == 'x'){
		if(k == 6){
			return n-1;
		}else if(k == 1 || k == 5){
			return n;
		}else if(k == 2 || k == 4){
			return n+1;
		}else if(k == 3){
			return n+2;
		}
	}	
	return 114514;
}
void getdxy(int *x, int *y, int opt){
	switch (opt) {
		case 0: (*x)=-1; (*y)=-1; break; 
		case 1: (*x)=+1; (*y)=-1; break; 
		case 2: (*x)=+1; (*y)=+1; break; 
		case 3: (*x)=-1; (*y)=+1; break; 
	}
}
void ddmovevser(struct vser *thisvser){
	int trydx,trydy;
	getdxy(&trydx, &trydy, thisvser->opt);
	if(map[thisvser->y + trydy][thisvser->xl] != ' ' &&
	   map[thisvser->y + trydy][thisvser->xr] != ' '){
	}else{
		thisvser->xl -= trydx;
		thisvser->xr -= trydx;
		thisvser->y  -= trydy;
		thisvser->opt = (thisvser->opt - 2) % 4;
	}
	thisvser->k = (thisvser->k + 1) % 6;
}

void doNothing(){
	return;
}
int isblock(struct vser *thisvser, int opt){
	switch (opt) {
		case up:
			return map[thisvser->y -1][thisvser->xl] != ' ' ||
				   map[thisvser->y -1][thisvser->xr] != ' ';
		case down:
			return map[thisvser->y +1][thisvser->xl] != ' ' ||
				   map[thisvser->y +1][thisvser->xr] != ' ';
		case left:
			return map[thisvser->y][thisvser->xl -1] != ' ' ;
		case right:
			return map[thisvser->y][thisvser->xr +1] != ' ' ;
	}
	return 114514;
}
void trymove(struct vser *thisvser){
	switch (thisvser->opt) {
		case up_left:
			if(isblock(thisvser, up)||
			   isblock(thisvser,left)){
				return;
			}else{
				thisvser->xl -=1;
				thisvser->xr -=1;
				thisvser->y  -=1;
				return;
			}
		case up_right:
			if(isblock(thisvser, up) ||
			   isblock(thisvser, right)){
				return;
			}else{
				thisvser->xl +=1;
				thisvser->xr +=1;
				thisvser->y  -=1;
				return;
			}
		case down_right:
			if(isblock(thisvser, down) ||
			   isblock(thisvser, right)){
				return;		
			}else{
				thisvser->xl += 1;
				thisvser->xr += 1;
				thisvser->y  += 1;
				return;
			}
		case down_left:
			if(isblock(thisvser, down) ||
			   isblock(thisvser, left)){
				return;
			}else{
				thisvser->xl -= 1;
				thisvser->xr -= 1;
				thisvser->y  += 1;
				return;
			}
	}
}
void movevser(struct vser *thisvser){
	switch (thisvser->opt) {
		case up_left:
			if (isblock(thisvser, up) && isblock(thisvser, left)) {
				thisvser->opt = down_right;
			} else if (isblock(thisvser, up)) {
				thisvser->opt = down_left;
			} else if (isblock(thisvser, left)) {
				thisvser->opt = up_right;
			}
			break;

		case up_right:
			if (isblock(thisvser, up) && isblock(thisvser, right)) {
				thisvser->opt = down_left;
			} else if (isblock(thisvser, up)) {
				thisvser->opt = down_right;
			} else if (isblock(thisvser, right)) {
				thisvser->opt = up_left;
			}
			break;

		case down_right:
			if (isblock(thisvser, down) && isblock(thisvser, right)) {
				thisvser->opt = up_left;
			} else if (isblock(thisvser, down)) {
				thisvser->opt = up_right;
			} else if (isblock(thisvser, right)) {
				thisvser->opt = down_left;
			}
			break;

		case down_left:
			if (isblock(thisvser, down) && isblock(thisvser, left)) {
				thisvser->opt = up_right;
			} else if (isblock(thisvser, down)) {
				thisvser->opt = up_left;
			} else if (isblock(thisvser, left)) {
				thisvser->opt = down_right;
			}
			break;
	}
	trymove(thisvser);
}

int check_3x3(int y, int x, char target1, char target2) {
	for (int dy = -1; dy <= 1; dy++) {
		for (int dx = -1; dx <= 1; dx++) {
			int ny = y + dy;
			int nx = x + dx;
			// 边界检查
			if (ny < 0 || ny >= mapa || nx < 0 || nx >= mapa) {
				continue;
			}
			if (map[ny][nx] == target1 || map[ny][nx] == target2) {
				return 1;  // 命中
			}
		}
	}
	return 0;
}

int main(int argc, char *argv[]){
	struct termios oldt, newt;
	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);  // 关缓冲+回显
	newt.c_cc[VMIN] = 0;			   // 非阻塞
	newt.c_cc[VTIME] = 0;
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	
	char c;

	printf("starting Microsoft vs Code");
	for(int i = 0; i<6; i++){
		delay_ms(100 + i*80);
		printf(".");fflush(stdout);
	}
	printf("\n\033[?1049h");
	srand(time(NULL));

	int r1 = rand() % (mapa -3) + 1;
	int r2 = rand() % (mapa -3) + 1;
	struct vser ms   = {'m', r1, r1 + 1, r2, 5, 3, 10};

	int r3 = rand() % (mapa -3) + 1;
	int r4 = rand() % (mapa -3) + 1;
	struct vser code = {'c', r3, r3 + 1, r4, 6, 1, 10};

	for(int round=0; code.blood > 0 && ms.blood > 0; round++){
		clearmap();
		setwall();
		/*set vser*/
		map[code.y][code.xl] = 'd';
		map[code.y][code.xr] = 'm';
		
		map[ms.y][ms.xl] = 'w';
		map[ms.y][ms.xr] = 'r';

		/*move vser*/
		if(round%2){
			movevser(&ms);
			movevser(&code);
		}else{
			movevser(&code);
			movevser(&ms);
		}
		
		clearmap();

		setwall();
		/*set vser*/
		map[code.y][code.xl] = 'd';
		map[code.y][code.xr] = 'm';
		
		map[ms.y][ms.xl] = 'w';
		map[ms.y][ms.xr] = 'r';

		/*if(map[getk(code.y, code.k, 'y')][getk(code.xl, code.k, 'x')] == 'w' ||
		   map[getk(code.y, code.k, 'y')][getk(code.xl, code.k, 'x')] == 'r'){
			ms.blood --;
		}
		if(map[getk(ms.y,   ms.k,   'y')][getk(ms.xl,   ms.k,   'x')] == 'd' || 
		   map[getk(ms.y,   ms.k,   'y')][getk(ms.xl,   ms.k,   'x')] == 'm'){
			code.blood --;
		}*/

		int code_bullet_y = getk(code.y, code.k, 'y');
		int code_bullet_x = getk(code.xl, code.k, 'x');
		if (check_3x3(code_bullet_y, code_bullet_x, 'w', 'r')) {
			ms.blood--;
		}

		int ms_bullet_y = getk(ms.y, ms.k, 'y');
		int ms_bullet_x = getk(ms.xl, ms.k, 'x');
		if (check_3x3(ms_bullet_y, ms_bullet_x, 'd', 'm')) {
			code.blood--;
		}

		map[getk(code.y, code.k, 'y')][getk(code.xl, code.k, 'x')] = (code.k % 3) ? '|' : '-' ;
		map[getk(ms.y,   ms.k,   'y')][getk(ms.xl,   ms.k,   'x')] = (ms.k   % 3) ? '|' : '-' ;
		/*set vser over*/
		
		ms.k   = (ms.k  ) % 6 +1;
		code.k = (code.k) % 6 +1;

		printf("Microsoft vs Code\n");
		printf("微软大战代码\n\n");
		for(int i=0; i<mapa; i++){
			for(int j=0; j<mapa; j++){
				char tmp = map[i][j];
				if(tmp == 'W') printf("墙");
				else if(tmp == 'w') printf("微");
				else if(tmp == 'r') printf("软");
				else if(tmp == 'd') printf("代");
				else if(tmp == 'm') printf("码");
				else if(tmp == ' ') printf("  ");
				else if(tmp == '-') printf("==");
				else if(tmp == '|') printf("||");
				else printf("%c",tmp);
			}
			printf("\n");
		}
		if (read(STDIN_FILENO, &c, 1) > 0) {
			if (c == 'q') {ms.blood = 0; code.blood = 0;}
		}
		printf("ms: (%d,%d)-(%d,%d) opt=%d\n", ms.xl, ms.y, ms.xr, ms.y, ms.opt);
		printf("code: (%d,%d)-(%d,%d) opt=%d\n", code.xl, code.y, code.xr, code.y, code.opt);
		printf("microsoft lifeleft: %2d\ncode	  lifeleft: %2d\n", ms.blood, code.blood);
		printf("get bored?press q\n\033[2J\033[H");
		delay_ms(100);
	}
	printf("\033[?1049l");
	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	delay_ms(1000);
	printf("\n");
	delay_ms(1000);
	printf("%s vs%s: command not found and can't be installed with\n",
	ms.blood?"Microsoft" : "dead", code.blood? "code" : "dead");
	delay_ms(2000);

	char *filename = argv[1];

	const char *editor = getenv("EDITOR");
	if (!editor || !*editor) {
		const char *fallbacks[] = {"nano", "vim", "vi", "nvim", "emacs", "ed", "sh", "genshin"};
		for (int i = 0; fallbacks[i]; i++) {
			char cmd[256];
			snprintf(cmd, sizeof(cmd), "which %s 2>/dev/null", fallbacks[i]);
			
			FILE *fp = popen(cmd, "r");
			if (!fp) continue;
			
			char path[256];
			if (fgets(path, sizeof(path), fp) && path[0] != '\n') {
			editor = fallbacks[i];
			pclose(fp);
			break;
			}
			pclose(fp);
		}
	}
	printf("but luckily we can use %s instead :)\n(could be changed by setting $EDITOR)\n",editor);
	for(int i=4; i>0; i--){
		for(int j=0; j<i; j++){
			printf(".");
		}
		printf("        ");
		printf("\r");
		fflush(stdout);
		delay_ms(1000);
	}
	if (argc < 2) {
		execlp(editor, editor, (char *)NULL);	
		return 0;
	}else{
		execlp(editor, editor, filename, (char *)NULL);  // 有文件
	}

	return 0;
}
