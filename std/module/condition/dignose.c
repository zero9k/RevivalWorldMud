/* This program is a part of RW mudlib
 * See /doc/help/copyright for more info
 -----------------------------------------
 * File   : dignose.c
 * Author : Clode@RevivalWorld
 * Date   : 2005-05-16
 * Note   : 
 * Update :
 *  o 2000-00-00  

 -----------------------------------------
 */
 
#include <ansi.h>
#include <buff.h>

#define CONDITION_ID		"dignose"
#define CONDITION_NAME		NOR WHT"鼻屎纏身"NOR
#define CONDITION_TYPE		"傷害"
#define DEFAULT_TIME		60
#define DEFAULT_HEART_BEAT	5

// 回傳狀態英文名稱
string query_condition_id()
{
	return CONDITION_ID;
}

// 回傳狀態中文名稱
string query_condition_name()
{
	return CONDITION_NAME;
}

// 回傳狀態型態名稱
string query_condition_type()
{
	return CONDITION_TYPE;
}

// 回傳狀態預設持續時間
int query_default_condition_time()
{
	return DEFAULT_TIME;
}

// 回傳狀態預設心跳時間
int query_default_condition_heart_beat()
{
	return DEFAULT_HEART_BEAT;
}

// 啟動狀態時的效果
void start_effect(object ob)
{
	msg("$ME受到「"CONDITION_NAME"」的"CONDITION_TYPE"影響。\n", ob, 0, 1);
}

// 結束狀態時的效果
void stop_effect(object ob)
{
	msg("$ME解除「"CONDITION_NAME"」的"CONDITION_TYPE"影響。\n", ob, 0, 1);
}

// 狀態進行中的效果
void heart_beat_effect(object ob)
{
	if( !ob->cost_health(20) )
		ob->faint();

	msg("$ME受到「"CONDITION_NAME"」的侵蝕傷害，損失 20 點的生命值("NOR RED+ob->query_health_cur()+"/"HIR+ob->query_health_max()+NOR")。\n", ob, 0, 1);	
}