/* This program is a part of RW mudlib
 * See /doc/help/copyright for more info
 -----------------------------------------
 * File   : room_wizhall_8.c
 * Author : Clode@RevivalWorld
 * Date   : 2003-08-23
 * Note   : 標準房間
 * Update :
 *  o 2000-00-00
 *
 -----------------------------------------
 */

#include <ansi.h>
#include <inherit.h>

inherit STANDARD_ROOM;

string help = @HELP
在這個平凡無奇的房間內並沒有任何特殊的資訊。
HELP;

void create()
{
	set("short", HIW"巫師"NOR WHT"會議室"NOR);
	set("long",@TEXT
　　巫師會議廳的兩側平行地排滿著巨大石椅，許多巫師正坐在上面互相討論著重
大議題，同時也是眾巫師們聊天聚會的地方，這裡的四週圍繞著許多扇大門，通往
著各個巫師的休息處。
TEXT);

	set("exits", ([
		"down" : "/wiz/wizhall/room_wizhall_17",
		"sinji" : "/wiz/home/sinji/workroom",
                "cookys" : "/wiz/home/cookys/workroom",
                "abcd" : "/wiz/home/abcd/workroom",
		"ekac" : "/wiz/home/ekac/workroom",
	]));
	
	set("objects", ([
		"/obj/board/meeting" : 1,
	]));
	
	::reset_objects();
}
