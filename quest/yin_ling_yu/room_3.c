/* This program is a part of RW mudlib
 * See /doc/help/copyright for more info
 -----------------------------------------
 * File   : room_3.c
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
	set("short", HIW"古"NOR WHT"厝"NOR);
	set("long",@TEXT
    這裡是一間廢棄的古厝，到處破舊不堪難以居住，但這裡的地上卻倒臥著一個
已經死亡的男子，手握著一把刀子，看刀身上的血跡與刀痕，似乎是在死前跟人拼
鬥，最後不敵被殺，男子的衣物明顯的都被翻掀過，看來很有可能是一樁搶奪殺人
案，不過男子的附近放著一包袋子，裡面似乎裝著一些草藥，或許可以搜尋看看袋
子裡放些什麼東西，此外眼尖的您似乎還看到了附近有些女子的足跡。
TEXT);

	set("exits", ([
		//原設定出口："north" 	: ({ 72, 63, 0, "lightforest", 0, 0, "72/63/0/lightforest/0/0" }),
		//原設定出口："west"	: ({ 71, 64, 0, "lightforest", 0, 0, "71/64/0/lightforest/0/0" }), 
		//該房間往西走後顯示座標是72,65，code設定是71,64。
		
		//以下為`06-09-07，由 tzj 遷位所變更的位置。
		"east" 		: ({ 16, 60, 0, "lightforest", 0, 0, "16/60/0/lightforest/0/0" }),
		"south" 	: ({ 15, 61, 0, "lightforest", 0, 0, "15/61/0/lightforest/0/0" }),
		"north"		: "/quest/yin_ling_yu/room_9",
	]));
	
	set("objects", ([
		"/quest/yin_ling_yu/yi_ke_wen" : 1 ,
	]));
	
	::reset_objects();
}

void do_search(object me, string arg)
{
	if( !arg || arg=="" )
		return tell(me, pnoun(2, me)+"想要搜尋什麼？\n");

	if( strsrch(arg, "袋") != -1 )
	{
 		if( me->query_quest_step(QUEST_YIN_LING_YU) == 0 )
		{
			tell(me, "袋子只有一些草藥，"+pnoun(2, me)+"不覺得這些草藥有啥特別。\n");
		}
		else if( me->query_quest_step(QUEST_YIN_LING_YU) == 1 || time() > me->query_quest_data(QUEST_YIN_LING_YU, "ginseng") + 7200 )
		{
			object ob = new("/quest/yin_ling_yu/wild_ginseng");
			
			msg("$ME打開了袋子，得到了$YOU。\n", me, ob, 1);

			me->set_quest_note(QUEST_YIN_LING_YU, QUEST_YIN_LING_YU_NAME, "從易柯文屍體旁的袋子中找到了一把野山人參。");
			me->set_quest_step(QUEST_YIN_LING_YU, 2);
			me->set_quest_data(QUEST_YIN_LING_YU, "ginseng", time());
			ob->move(me);
		}
		else
			tell(me, "袋子裡的物品似乎已經被取走了。\n");
	}
	else if( strsrch(arg, "女") != -1 || strsrch(arg, "足") != -1 )
		tell(me, "這個女子的足跡似乎是往南方的方向過去了。\n");
	else if( strsrch(arg, "刀") != -1 )
		tell(me, "這把生鏽的刀上到處都是砍劈後的裂痕，顯然不是什麼好刀。\n");
	else if( strsrch(arg, "厝") != -1 )
		tell(me, "一間荒廢已久的房子。\n");
	else if( strsrch(arg, "衣") != -1 )
		tell(me, "這個男子的衣物裡已經被搜括一空，什麼都不剩了。\n");
	else if( strsrch(arg, "草藥") != -1 )
		tell(me, "從這邊看不清楚袋子的東西是什麼，只看得出類似是草藥的模樣。\n");
	else
		tell(me, pnoun(2, me)+"想要搜尋什麼？\n");	
}

mapping actions = 
([
	"search" : (: do_search :),
]);