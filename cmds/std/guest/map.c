/* This program is a part of RW mudlib
 * See /doc/help/copyright for more info
 -----------------------------------------
 * File   : map.c
 * Author : Cookys@RevivalWorld
 * Date   : 2002-3-18
 * Note   : 顯示地圖概況指令。
 * Update :
 *  o 2000-00-00  
 *
 -----------------------------------------
 */

#include <feature.h>
#include <daemon.h>
#include <message.h>

inherit COMMAND;

string help = @HELP
	標準 map 指令。
HELP;

private void do_command(object me, string arg)
{
	array loc;
	string city;
	int num, ratio;
	
	if( !arg || sscanf(arg, "%s %d -%d", city, num, ratio) != 3 || !CITY_D->city_exist(city, --num) )
		return tell(me, "請輸入正確的城市名稱與都會區編號。(例: map center 1 -2)\n", CMDMSG);

	if( ratio > 10 ) ratio = 10;
	if( ratio < 2 ) ratio = 2;
		
	loc = arrange_city_location(1, 1, city, num);
		
	tell(me, CITY_D->query_city_idname(city, num)+"縮小 "+ratio+" 倍簡圖：\n"+MAP_D->query_map_reduction(CITY_D->query_map(loc, 1), ratio)+"\n\n", CMDMSG);
}