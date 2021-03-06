/* This program is a part of RW mudlib
 * See /doc/help/copyright for more info
 -----------------------------------------
 * File   : golftdi.c
 * Author : Clode@RevivalWorld
 * Date   : 2006-09-12
 * Note   : GOLF TDI
 * Update :
 *  o 2000-00-00
 *
 -----------------------------------------
 */

#include <ansi.h>
#include <inherit.h>
#include <feature.h>
#include <equipment.h>
#include <buff.h>
#include <daemon.h>
#include <secure.h>
#include <map.h>
#include <location.h>

inherit STANDARD_OBJECT;
inherit EQUIPMENT;

mapping actions;

void fire_process(object me, object target, int combo, int sec)
{
	if( sec == 0 )
	{
		int damage, faint;
		for(int i=0;i<combo;i++)
		{
			damage = range_random(1, 125);
			
			if( !target->cost_health(damage) )
				faint = 1;
			
			msg("$YOU被$ME發射的 12 型光炮陣列擊中，導致 "HIR+damage+NOR" 的生命傷害("NOR GRN+target->query_health_cur()+NOR"/"HIG+target->query_health_max()+NOR")。\n", me, target, 1);

			if( faint )
				target->faint();
		}
		
		delete_temp("delay/fire");
	}
	else
	{
		msg("距離 12 型光炮陣列擊中$YOU還有 "+sec+" 秒鐘...\n", me, target, 1);
		call_out((: fire_process, me, target, combo, sec-1 :), 1);
	}
}

void fire(object me, string arg)
{
	int combo = random(11)+1;
	object target = find_player(arg) || present(arg);
	
	if( !me->is_equipping_object(this_object()) )
		return tell(me, pnoun(2, me)+"必須先裝備"+this_object()->query_idname()+"。\n");

	if( query_temp("delay/fire") > time() )
		return tell(me, "此武器無法連續發射，請等待武器系統冷卻。\n");

	if( !objectp(target) )
		return tell(me, "沒有 "+arg+" 這個人。\n");
	
	for(int i=0;i<combo;i++)
	msg("$ME操縱"+this_object()->query_idname()+"對著$YOU發射 12 型光炮陣列("HIY+combo+NOR YEL" 連發"NOR")。\n", me, target, 1);
	set_temp("delay/fire", time() + 5);
	call_out((: fire_process, me, target, combo, 5 :), 1);
}

void killest_confirm(object me, string arg, array loc, string yn)
{
	if( yn == "y" )
	{
		CHANNEL_D->channel_broadcast("news", me->query_idname()+"架駛"+this_object()->query_idname()+"對著 "+loc_short(loc)+" 發射 12 型光炮陣列，該地地表上的所有東西瞬間灰飛煙滅。\n");
		ESTATE_D->remove_estate(loc);
		ESTATE_D->remove_unknown_estate(loc);
	}
	else
		tell(me, pnoun(2, me)+"取消了攻擊。\n");

	me->finish_input();
}

void killest(object me, string arg)
{
	array loc = query_temp("location", me);		

	if( SECURE_D->level_num(me->query_id(1)) < ADMIN )
		return tell(me, "只有 Administrator 可以使用這項武器。\n");	
	
	if( !arrayp(loc) )
		return tell(me, pnoun(2, me)+"必須在地圖上開啟這個武器。\n");
		
	tell(me, HIY+pnoun(2, me)+"確定完全摧毀在這個座標上的 "+ESTATE_D->query_loc_estate(loc)["type"]+" 房地產"NOR"("HIR"注意！這是真的！"NOR")？(y/n)");
	input_to( (: killest_confirm, me, arg, loc :) );
}

/*
void killcity(object me, string arg)
{
	string city;
	int num;
	array loc;
	string owner;

	if( sscanf(arg, "'%s' %d", city, num) != 2 )
		sscanf(arg, "%s %d", city, num);
	
	num--;
	
	if( !CITY_D->city_exist(city, num) )
		return tell(me, "此城市不存在\n");
	
	for(int i=0;i<99;i++)
	for(int j=0;j<99;j++)
	{
		loc = arrange_city_location(i, j, city, num);
		
		owner = CITY_D->query_coor_data(loc, "owner");
		
		// 沒擁有者?? 什麼東西..全拆了
		if( !owner )
		{
			switch(CITY_D->query_coor_data(loc, TYPE))
			{
				case FARM:
				case PASTURE:
				case FISHFARM:
				case POOL:
				case LIGHT:
				case PAVILION:
				case TREE:
				case GRASS:
				case STATUE:
					killest_confirm(me, arg, loc, "y");
					break;
				default:
					break;
			}
		}
		// 政府的土地
		else if( belong_to_government(owner) )
		{
			switch(CITY_D->query_coor_data(loc, TYPE))
			{
				case FARM:
				case PASTURE:
				case FISHFARM:
				case POOL:
				case PAVILION:
				case TREE:
				case GRASS:
				case STATUE:
					killest_confirm(me, arg, loc, "y");
					break;
				default:
					break;
			}
		}
		// 企業的土地
		else if( belong_to_enterprise(owner) )
		{
			switch(CITY_D->query_coor_data(loc, TYPE))
			{
				case FARM:
				case PASTURE:
				case FISHFARM:
				case POOL:
				case PAVILION:
				case LIGHT:
				case TREE:
				case GRASS:
				case STATUE:
					killest_confirm(me, arg, loc, "y");
					break;
				default:
					break;
			}
		}
		// 不知道是誰的土地
		else {
			switch(CITY_D->query_coor_data(loc, TYPE))
			{
				case LIGHT:
					killest_confirm(me, arg, loc, "y");
					break;
				default:
					break;
			}
		}
	}
}
*/

void shield(object me, string arg)
{
	if( !me->is_equipping_object(this_object()) )
		return tell(me, pnoun(2, me)+"必須先裝備"+this_object()->query_idname()+"。\n");

	if( !query_temp("shield/on") )
	{
		msg("$ME啟動"+this_object()->query_idname()+"自動調頻防護罩系統。\n", me, 0, 1);
		set_temp("status", HIR"(防護罩)"NOR);
		set_temp("shield/on", 1);
	}
	else
	{
		msg("$ME關閉"+this_object()->query_idname()+"自動調頻防護罩系統。\n", me, 0, 1);
		delete_temp("status");
		delete_temp("shield/on");
	}
}

void antimatter_process(object me, object target, int sec)
{
	if( sec == 0 )
	{
		msg("$YOU被$ME所啟動的反物質散布口徹底毀滅，導致 "HIR+1000000+NOR" 的生命傷害("NOR RED+"-1000000"+NOR"/"HIG+target->query_health_max()+NOR")。\n", me, target, 1);

		target->faint();
		
		if( userp(target) )
		LOGOUT_D->quit(target);
		
		delete_temp("delay/antimatter");
	}
	else
	{
		msg("距離反物質散布口聚集能量完成還有 "+sec+" 秒鐘...\n", me, target, 1);
		call_out((: antimatter_process, me, target, sec-10 :), 10);
	}
}

// 反物質散布口
void antimatter(object me, string arg)
{
	object target = find_player(arg) || present(arg);
	
	if( !me->is_equipping_object(this_object()) )
		return tell(me, pnoun(2, me)+"必須先裝備"+this_object()->query_idname()+"。\n");

	if( query_temp("delay/antimatter") > time() )
		return tell(me, "此武器無法連續發射，請等待武器系統冷卻。\n");

	if( !objectp(target) )
		return tell(me, "沒有 "+arg+" 這個人。\n");
		
	msg("$ME操縱"+this_object()->query_idname()+"對著$YOU開啟反物質散布口。\n", me, target, 1);
	set_temp("delay/antimatter", time() + 60);
	call_out((: antimatter_process, me, target, 60 :), 1);
}

void create()
{
	set_idname(HIW"golf"WHT" tdi"NOR, HIW"GOLF"NOR"-"WHT"TDI");
	set_temp("status", HIY"小鋼"NOR YEL"砲");

	actions = ([
		"fire" : (: fire :),
		"shield" : (: shield :),
		"antimatter": (: antimatter :),
		"killest": (: killest :),
		//"killcity":(:killcity:),
	]);

	if( this_object()->set_shadow_ob() ) return;
	
	set("long", @LONG
　　銀色的小鋼砲，有個可愛的圓圓大屁股...
LONG
	);

	set("unit", "輛");
	set("mass", 0);
	set("value", 0);
	set("flag/no_amount", 1);

	set(BUFF_STR, 200);
	set(BUFF_PHY, 200);
	set(BUFF_INT, 200);
	set(BUFF_AGI, 200);
	set(BUFF_CHA, 200);

	set(BUFF_STAMINA_MAX, 200);
	set(BUFF_HEALTH_MAX, 200);
	set(BUFF_ENERGY_MAX, 200);
	
	set(BUFF_STAMINA_REGEN, 200);
	set(BUFF_HEALTH_REGEN, 200);
	set(BUFF_ENERGY_REGEN, 200);

	set(BUFF_LOADING_MAX, 200);
	set(BUFF_FOOD_MAX, 200);
	set(BUFF_DRINK_MAX, 200);
	
	set(BUFF_SOCIAL_EXP_BONUS, 200);

	set("buff/status", HIW"GOLF"NOR"-"WHT"TDI");
	
	::setup_equipment(EQ_MOUNT, HIG"駕"NOR GRN"駛"NOR);
}
