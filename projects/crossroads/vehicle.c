
#include <stdio.h>

#include "threads/thread.h"
#include "devices/timer.h"
#include "threads/synch.h"
#include "projects/crossroads/vehicle.h"
#include "projects/crossroads/map.h"


// 차량의 갯수를 저장하는 전역변수
int car_cnt = 0;

/* path. A:0 B:1 C:2 D:3 */
const struct position vehicle_path[4][4][10] = {
	/* from A */ {
		/* to A */
		{{-1,-1},},
		/* to B */
		{{4,0},{4,1},{4,2},{5,2},{6,2},{-1,-1},},
		/* to C */
		{{4,0},{4,1},{4,2},{4,3},{4,4},{4,5},{4,6},{-1,-1},},
		/* to D */
		{{4,0},{4,1},{4,2},{4,3},{4,4},{3,4},{2,4},{1,4},{0,4},{-1,-1}}
	},
	/* from B */ {
		/* to A */
		{{6,4},{5,4},{4,4},{3,4},{2,4},{2,3},{2,2},{2,1},{2,0},{-1,-1}},
		/* to B */
		{{-1,-1},},
		/* to C */
		{{6,4},{5,4},{4,4},{4,5},{4,6},{-1,-1},},
		/* to D */
		{{6,4},{5,4},{4,4},{3,4},{2,4},{1,4},{0,4},{-1,-1},}
	},
	/* from C */ {
		/* to A */
		{{2,6},{2,5},{2,4},{2,3},{2,2},{2,1},{2,0},{-1,-1},},
		/* to B */
		{{2,6},{2,5},{2,4},{2,3},{2,2},{3,2},{4,2},{5,2},{6,2},{-1,-1}},
		/* to C */
		{{-1,-1},},
		/* to D */
		{{2,6},{2,5},{2,4},{1,4},{0,4},{-1,-1},}
	},
	/* from D */ {
		/* to A */
		{{0,2},{1,2},{2,2},{2,1},{2,0},{-1,-1},},
		/* to B */
		{{0,2},{1,2},{2,2},{3,2},{4,2},{5,2},{6,2},{-1,-1},},
		/* to C */
		{{0,2},{1,2},{2,2},{3,2},{4,2},{4,3},{4,4},{4,5},{4,6},{-1,-1}},
		/* to D */
		{{-1,-1},}
	}
};

static int is_position_outside(struct position pos)
{
	return (pos.row == -1 || pos.col == -1);
}

// 차량의 위치가 교차로 범위 내부에 있는지 검사하는 함수
static bool in_cross(struct position pos) {
	int x = pos.col, y = pos.row;
	int xx[] = {2, 2, 2, 3, 3, 3, 4, 4, 4};
	int yy[] = {2, 3, 4, 2, 3, 4, 2, 3, 4};

	for(int i = 0;i < 9;++i) if(y == yy[i] && x == xx[i]) return true;
	return false;
}

/* return 0:termination, 1:success, -1:fail */
static int try_move(int start, int dest, int step, struct vehicle_info *vi)
{
	// 교차로에 들어올 수 있는 최대 차량의 갯수를 저장하고 있는 변수
	static int limit_entry = 7;
	struct position pos_cur, pos_next;

	pos_next = vehicle_path[start][dest][step];
	pos_cur = vi->position;

	if (vi->state == VEHICLE_STATUS_RUNNING) {
		/* check termination */
		if (is_position_outside(pos_next)) {
			/* actual move */
			vi->position.row = vi->position.col = -1;
			/* release previous */
			lock_release(&vi->map_locks[pos_cur.row][pos_cur.col]);
			return 0;
		}
	}

	// 현재 위치가 사거리 내에 있지 않고, 다음 위치가 사거리 내에 있을 때
	if(in_cross(pos_next) && !in_cross(pos_cur)) {
		// 사거리 내에 접근이 가능할때
		if(limit_entry) {
			limit_entry--;
			lock_acquire(&vi->map_locks[pos_next.row][pos_next.col]);
		}
		// 사거리 내에 접근이 불가능 할때
		// -1 반환
		// 이렇게 하는 이유는 while문으로 busy_waiting을 하면 성능상 매우 안좋기 때문
		else{
			return -1;
		} 
	}
	// 그 외 모든 경로에 있을때에는 별다른 조건 없이 다음 위치에 대한 lock을 가져온다
	else
		lock_acquire(&vi->map_locks[pos_next.row][pos_next.col]);

	if (vi->state == VEHICLE_STATUS_READY) {
		/* start this vehicle */
		vi->state = VEHICLE_STATUS_RUNNING;
	} else {
		/* release current position */
		lock_release(&vi->map_locks[pos_cur.row][pos_cur.col]);
	}
	/* update position */
	// 현재 위치가 교차로 내에 있고 다음 위치가 교차로 외부이 있을 때
	// limit_entry++해준다
	if(in_cross(pos_cur) && !in_cross(pos_next)) limit_entry++;
	vi->position = pos_next;
	
	return 1;
}

void vehicle_loop(void *_vi)
{
	int res;
	int start, dest, step;
	// vehicle_loop를 실행한 thread의 갯수를 알아내기 위해  car_cnt 전역변수에 1씩 더해준다.
	car_cnt++;

	struct vehicle_info *vi = _vi;

	start = vi->start - 'A';
	dest = vi->dest - 'A';

	vi->position.row = vi->position.col = -1;
	vi->state = VEHICLE_STATUS_READY;

	step = 0;
	while (1) {
		/* vehicle main code */
		res = try_move(start, dest, step, vi);
		
		if (res == 1) {
			step++;
		}

		/* termination condition. */ 
		if (res == 0) {
			break;
		}

		/* sleep for 1 sec */
		timer_msleep(1000);
	}

	/* status transition must happen before sema_up */
	vi->state = VEHICLE_STATUS_FINISHED;
}
