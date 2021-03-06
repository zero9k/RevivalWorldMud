/* This program is a part of RW mudlib
 * See /doc/help/copyright for more info
 -----------------------------------------
 * File   : room_wizhall_5.c
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
#include <time.h>
#include <daemon.h>

inherit STANDARD_ROOM;

string help = @HELP
在這個平凡無奇的房間內並沒有任何特殊的資訊。
HELP;

#define HOR 	24
#define VER	14

void online_user_record();

void create()
{
	set("short", HIW"巫師"NOR WHT"神殿 "NOR YEL"西北殿"NOR);
	set("long",@TEXT
    這裡是巫師神殿西北邊的房間，這裡空無一物，與其他的房間相較起來，似乎
詭譎許多。
TEXT);

	set("exits", ([
		"east" : "/wiz/wizhall/room_wizhall_4",
	    ]));
	set("objects", ([
		"/obj/etc/mj/mj" : 1,
	    ]));
	online_user_record();
	set_heart_beat(600);
	::reset_objects();
}

//雙極界時代 code 直接引用
void online_user_record()
{
	int *realtime = TIME_D->query_realtime_array();
	int i, j, k, hour = realtime[HOUR];
	string str,last_str="",b="█",*sections=({});
	string s="                                                                         ";
	string *list=({s,s,s,s,s,s,s,s,s,s,s,s,s,s});
	mapping record;	
	float w, max_num=0., min_num=10000.;
	string list_name = HIW"          重 "NOR WHT"生 的 世 界 當 天 上 線 人 數 統 計 表";
	string time = "       0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23\n";
	int *num= allocate(HOR);

	record = query("login/maxusers/hour/", SYSDATABASE_D->query_ob());

	if( !mapp(record) ) return;

	foreach( string hourstr, int sizeofusers in record  )
	{
		num[to_int(hourstr)] = sizeofusers;
		if( sizeofusers > max_num ) max_num = sizeofusers;
		if( sizeofusers < min_num ) min_num = sizeofusers;
	}

	w = ((max_num-min_num)/(VER-2));
	
	if( w < 0.001 )
		w = 0.001;
		
	str = NOR WHT"   ↑"+list_name+"("+(realtime[MON]+1)+" 月 "+(realtime[MDAY]+1)+" 日)"+NOR+"\n";
	for(i=0;i<VER;i++)
	{
		for(j=0;j<HOR;j++)
		{
			if((num[j]-min_num)/w > VER-3-i)
			{
				if(list[i][j*3+1..j*3+2] != b)
					list[i][j*3..j*3+2] = sprintf("%3d",num[j]);
				if( i < VER-1 )
					list[i+1][j*3+1..j*3+2]=b;
			}
			//if(j==hour)
			//	list[i][j*3+3..j*3+3] = "|";
		}
		k = w*(VER-2-i)+min_num;
		str +=HIC+(i%2==0 ? sprintf("%3d", k) : "   ")+NOR WHT"∣"NOR+list[i]+"\n";
	}

	str += HIC"  0"NOR WHT"└───────────────────────────────────→\n"NOR;
	str += HIC+time[0..(hour*3+4)]+HIM+"["+HIR+sprintf("%2d",hour)+HIM+"]"+HIC+time[(hour*3+9)..<0]+NOR;

	// 替 | 符號後之資料加上 WHY 顏色
	sections = explode( str, "\n" );
	for(i=0;i<sizeof(sections);i++)
	{
		if(i>1 && i<16)
		{
			j = strsrch(sections[i],"|");
			sections[i]=sections[i][0..j]+NOR WHT+sections[i][j+1..<0];
		}
		last_str += sections[i]+"\n";
	}	

	set("long", last_str);
}

void heart_beat()
{
	online_user_record();
}
