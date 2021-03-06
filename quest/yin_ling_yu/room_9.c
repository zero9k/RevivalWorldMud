/* This program is a part of RW mudlib
 * See /doc/help/copyright for more info
 -----------------------------------------
 * File   : room_9.c
 * Author : Clode@RevivalWorld
 * Date   : 2005-12-19
 * Note   : 標準房間
 * Update :
 *  2006-09-07 遷位 by Tzj@RevivalWorld
 *  
 -----------------------------------------
 */

#include <ansi.h>
#include <inherit.h>
#include <quest.h>

inherit STANDARD_ROOM;

string help = @HELP
在這個平凡無奇的房間內並沒有任何特殊的資訊。
HELP;



void create()
{
	set("short", YEL"雜物"NOR"間"NOR);
	set("long",@TEXT
    這裡是一間堆滿各式雜物的房間，雖然東西很多，但所有的物品都還算井然有
序，這裡的主人應該是個細心的人，角落也都還算打掃的十分乾淨，並不像是一個
大男人居住的地方。
TEXT);

	set("exits", ([
		"south"		: "/quest/yin_ling_yu/room_3",
	]));
	
	::reset_objects();
}
