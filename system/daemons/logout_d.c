/* This program is a part of RW mudlib
 * See /doc/help/copyright for more info
 -----------------------------------------
 * File   : logout_d.c
 * Author : Clode@RevivalWorld
 * Date   : 2001-7-29
 * Note   : 登出精靈
 * Update :
 *  o 2000-00-00  
 *
 -----------------------------------------
 */

#define LOG			"daemon/connect"

#include <ansi.h>
#include <daemon.h>
#include <message.h>

void net_dead(object ob)
{
	string network_short = NETWORK_D->query_network_short(ob);

	msg("$ME似乎與現實生活的連線出了差錯 ... 斷線了。\n", ob, 0, 1, 0, ({ ob }));

	CHANNEL_D->channel_broadcast("sys", "登出："+ob->query_idname()+WHT"由"+network_short+"斷線，倒數五分鐘離線。", 1);	

	log_file(LOG, ob->query_idname()+" disconnect "+network_short+NOR+"\n");
	// CHANNEL_D->remove_user(ob);
}

private void save_user_info(object ob)
{
	mapping current_login = copy(query_temp("login", ob));
	
	if( !mapp(current_login) )
		return;

	addn("total_online_time", time() - query("last_on/time", ob), ob );

	current_login["time"] = time();
	
	set("last_login", current_login, ob); 
	
	// 紀錄最後一次在線上的時間, 根據此資料由 CLEAN_D 處理帳號刪除
	set("last_on/time", time(), ob);

	if( environment(ob) )
		set("quit_place", copy(query_temp("location", ob)) || base_name(environment(ob)), ob);

	ob->save();
}

varargs void quit(object ob, string arg)
{
	object env = environment(ob);
	string network_short = NETWORK_D->query_network_short(ob);
	if( !userp(ob) && !query_temp("net_dead", ob) ) return;
	
	save_user_info(ob);

	/* 更新 TOP 資料 */
	TOP_D->calculate_top(ob);

	CHANNEL_D->remove_user(ob);

	if( query_temp("net_dead", ob) )
	{
		msg("$ME斷線超過五分鐘，被踢出遊戲了。\n", ob, 0, 1);
		CHANNEL_D->channel_broadcast("sys", "登出："+ob->query_idname()+WHT"由"+network_short+"斷線過久離開遊戲了。", 1);
	}
	else 
	{
		tell(ob, "\n"+pnoun(2, ob)+"本次共連線了"+CHINESE_D->chinese_period(time() - query_temp("login/time", ob))+"。\n");

		tell(ob, @POEM

----------

    不再流浪了，我不願做空間的歌者，
    寧願是時間的石人。
    然而，我又是宇宙的遊子，
    地球你不需留我。
    這土地我一方來，
    將八方離去。
				      ∼鄭愁予
			---------------------- 
			         [1;30mRevival World[0m

POEM);

		if(!arg) 
			msg("$ME離開遊戲了。\n", ob, 0, 1);
		else
			msg("$ME離開遊戲了，只聽到遠方細細傳來一聲：" + arg + "。\n", ob, 0, 1);
			
		CHANNEL_D->channel_broadcast("sys", "登出："+ob->query_idname()+WHT"由"+network_short+"離開遊戲。", 1);
	}
		
	MAP_D->remove_player_icon(ob, query_temp("location", ob));

	log_file(LOG, ob->query_idname(1)+" quit "+network_short+NOR+"\n");	
		
	flush_messages(ob);
	
	if( env )
		env->leave(ob);

	destruct(ob);
}
string query_name()
{
	return "登出系統(LOGOUT_D)";
}
