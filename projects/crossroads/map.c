
#include "projects/crossroads/map.h"


#define ANSI_NONE "\033[0m"
#define ANSI_BLACK "\033[30m"
#define ANSI_RED "\033[31m"
#define ANSI_GREEN "\033[32m"
#define ANSI_YELLOW "\033[33m"
#define ANSI_BLUE "\033[34m"
#define ANSI_MAGENTA "\033[35m"
#define ANSI_CYAN "\033[36m"
#define ANSI_WHITE "\033[37m"

#define ON_ANSI_BLACK "\033[40m"
#define ON_ANSI_RED "\033[41m"
#define ON_ANSI_GREEN "\033[42m"
#define ON_ANSI_YELLOW "\033[43m"
#define ON_ANSI_BLUE "\033[44m"
#define ON_ANSI_MAGENTA "\033[45m"
#define ON_ANSI_CYAN "\033[46m"
#define ON_ANSI_WHITE "\033[47m"

#define clear() printf("\033[H\033[J")
#define gotoxy(y,x) printf("\033[%d;%dH", (y), (x))


const char map_draw_default[7][7] = {
	{'X', 'X', ' ', 'X', ' ', 'X', 'X'}, 
	{'X', 'X', ' ', 'X', ' ', 'X', 'X'}, 
	{' ', ' ', ' ', '-', ' ', ' ', ' '}, 
	{'-', '-', '-', '-', '-', '-', '-'}, 
	{' ', ' ', ' ', '-', ' ', ' ', ' '}, 
	{'X', 'X', ' ', '-', ' ', 'X', 'X'}, 
	{'X', 'X', ' ', '-', ' ', 'X', 'X'}, 
};


void map_draw(void)
{
	int i, j;
	clear();
	// car_temp에 car_cnt의 값을 넣어준다
	// 항상 map_draw 후에 map_draw_vehicle이 thread갯수만큼 실행된다
	// 따라서 이 부분에 car_temp = car_cnt를 해줘야 map_draw_vehicle에서 thread갯수만큼 map_draw_vehicle이 호출되었는지 감지할 수 있다.
	car_temp = car_cnt;

	for (i=0; i<7; i++) {
		for (j=0; j<7; j++) {
			printf("%c ", map_draw_default[i][j]);
		}
		printf("\n");
	}
	if(crossroads_step==0)
		printf("unit step: %d\n", crossroads_step);
	gotoxy(0, 0);
}

void map_draw_vehicle(char id, int row, int col)
{
	if (row >= 0 && col >= 0) {
		gotoxy(row + 1, col * 2 + 1);
		printf("%c ", id);
		gotoxy(0, 0);
	}
	// car_temp의 값을 1씩 빼준다
	car_temp--;
	// car_temp가 0이면 car_cnt만큼 map_draw_vehicle이 실행되었다는 의미이다
	// 이때 crossroads_step에 1을 더해준다.
	if(car_temp==0){
		crossroads_step++;
		gotoxy(8,0);
		printf("unit step: %d\n", crossroads_step);	
		gotoxy(0, 0);
	}
}

void map_draw_reset(void)
{
	clear();
}

//9466 0051 7199