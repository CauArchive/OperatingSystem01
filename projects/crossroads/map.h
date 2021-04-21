#ifndef __PROJECTS_PROJECT1_MAPDATA_H__
#define __PROJECTS_PROJECT1_MAPDATA_H__

#include <stdio.h>
#include "projects/crossroads/position.h"

// car_cnt의 값을 임시로 저장할 변수
// crossroads_step 계산을 위해 생성한 변수
int car_temp;
// crossroads.c에서 가져온 crossroads_step 외부 변수
extern int crossroads_step;
// vehicle.c에서 가져온 car_cnt 외부 변수
extern int car_cnt;

void map_draw(void);
void map_draw_vehicle(char id, int row, int col);
void map_draw_reset(void);

#endif /* __PROJECTS_PROJECT1_MAPDATA_H__ */