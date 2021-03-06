/* This program is a part of RW mudlib
 * See /doc/help/copyright for more info
 -----------------------------------------
 * File   : estate.c
 * Author : Clode@RevivalWorld
 * Date   : 2002-09-17
 * Note   : estate 指令
 * Update :
 *  o 2000-00-00
 *
 -----------------------------------------
 */

#include <ansi.h>
#include <daemon.h>
#include <feature.h>
#include <message.h>
#include <location.h>
#include <citydata.h>
#include <roommodule.h>
#include <map.h>

inherit COMMAND;

string help = @HELP
    房地產查詢指令。

estate			- 查詢自己的房地產資料
estate '玩家ID'		- 查詢其他市民的房地產資料(市長與官員指令)

HELP;

string showtime(int time)
{
	if(time > 86400)
		return time/86400+"D";
	else
		return time/3600+"H";
}

string time_description(string type, int regtime)
{
	if( type == "land" )
		return HIR+showtime( regtime + 3*24*60*60 - time() )+NOR;
	else if( time() - regtime > 365*24*60*60 )
		return GRN+"(古蹟)"+HIG+showtime(time() - regtime)+NOR;
	else
		return HIG+showtime(time() - regtime)+NOR;
}

varargs string show_estate(string owner, array myestates, string title, int enterprise)
{
	string level;
	string list, sloc;
	array loc;
	mapping estdata;
	mapping buildingtable = BUILDING_D->query_building_table();
	int floors, totalfloors, lands, totallands, flourish, totalflourish;
	string value, totalvalue;
	object room;

	list = HIW"房地產種類  "HIG"建"NOR"/"HIR"回收"NOR"時間 位置座標              佔地 總樓層 總繁榮 總價值\n";
	
	if( !enterprise )
		list += NOR WHT"───────────────────────────────────────\n"NOR;
	
	foreach(estdata in sort_array(myestates, (: strcmp($1["type"], $2["type"]) || ($1["regtime"] - $2["regtime"]) :)))
	{
		lands = 0;
		floors = 0;
		flourish = 0;
		value = 0;
		
		loc = restore_variable(estdata["walltable"][0]);
		value = ESTATE_D->query_estate_value(loc);
	
		lands = sizeof(estdata["walltable"]) + sizeof(estdata["roomtable"]);
		
		if( !sizeof(estdata["roomtable"]) )
			floors = sizeof(estdata["walltable"]);
		else
		{
			foreach(sloc in estdata["roomtable"])
			{
				loc = restore_variable(sloc);
				room = find_object(CITY_ROOM_MODULE(loc[CITY], loc[NUM], loc[X], loc[Y], estdata["type"]));
				
				if( objectp(room) )
					floors += query("floor", room)+1;
			}
		}
					
		flourish = ESTATE_D->query_estate_flourish(estdata);
		
		switch(estdata["type"])
		{
			case "farm":
				level = NOR GRN"(Lv"+HIG+CITY_D->query_coor_data(loc, "growth_level")+NOR GRN")"NOR;
				break;
			case "pasture":
				level = NOR YEL"(Lv"+HIY+CITY_D->query_coor_data(loc, "growth_level")+NOR YEL")"NOR;
				break;
			case "fishfarm":
				level = NOR BLU"(Lv"+HIB+CITY_D->query_coor_data(loc, "growth_level")+NOR BLU")"NOR;
				break;
			default:
				level = ""; break;
		}
		list += sprintf("%-12s%11s %-22s%-5d%-7d%-7d"HIY"%s\n"NOR,
				buildingtable[estdata["type"]][ROOMMODULE_SHORT] + level,
				time_description(estdata["type"], estdata["regtime"]),
				loc_short(loc), 
				lands,
				floors,
				flourish,
				NUMBER_D->number_symbol(value),
			    );
			    
		totalfloors += floors;
		totallands += lands;
		totalflourish += flourish;
		totalvalue = count(totalvalue, "+", value);
	}

	if( !enterprise )
		list += NOR WHT"───────────────────────────────────────\n"NOR;
	
	list += sprintf("%-46s%-5d%-7d%-7d"HIY"%s\n"NOR,"總數", totallands, totalfloors, totalflourish, NUMBER_D->number_symbol(totalvalue));
	
	if( !enterprise)
	{
		list += NOR WHT"───────────────────────────────────────\n"NOR;
		list += HIW+title+"目前共有 "+sizeof(myestates)+" 筆房地產\n"NOR;
	}
	
	return list;
}

private void do_command(object me, string arg)
{
	object ob;
	string list, title;
	array myestates;
	string city;
	
	city = query("city", me);
	
	if( !city ) 
		return tell(me, pnoun(2, me)+"必須先加入某個城市之後才能開始擁有房地產。\n");
	

	if( arg && wizardp(me) )
	{
		ob = find_player(arg);
		
		if( objectp(ob) )
			title = ob->query_idname();
		else
			title = capitalize(arg)+" ";
		
		myestates = ESTATE_D->query_player_estate(arg);
	}
	else if( arg && member_array(arg, CITY_D->query_citizens(city)) != -1 && (CITY_D->is_officer(city, me) ||  CITY_D->is_mayor(city, me)) )
	{
		myestates = ESTATE_D->query_player_estate(arg);
		title = "市民 "+capitalize(arg)+" ";
	}
	else
	{
		arg = me->query_id(1);
		myestates = ESTATE_D->query_player_estate(arg);
		title = pnoun(2, me);
	}
	

	if( !sizeof(myestates) )
		return tell(me, title+"目前沒有任何房地產。\n");

	
	list = show_estate(arg, myestates, title);

	me->more(list+"\n");
}