/* This program is a part of RW mudlib
 * See /doc/help/copyright for more info
 -----------------------------------------
 * File   : allowmulti.c
 * Author : Clode@RevivalWorld
 * Date   : 2006-03-09
 * Note   : allowmulti 指令。
 * Update :
 *  o 2000-00-00
 *
 -----------------------------------------
 */

#include <feature.h>
#include <message.h>

inherit COMMAND;

string help = @HELP
        標準 allowmulti 指令。
HELP;

private void command(object me, string arg)
{
	string id, *ids;
	object user;

	if( !is_command() ) return;

	if( !arg ) 
		return tell(me, pnoun(2, me)+"打算設定哪些玩家可以同 IP 連線？\n");
	
	ids = explode(arg, " ");
	
	if( sizeof(ids) < 2 )
		return tell(me, "請設定兩位以上的玩家。\n");
		
	foreach(id in ids)
		if( !user_exists(lower_case(id)) )
			return tell(me, "這個世界上沒有 "+id+" 這個玩家。\n");
	
	foreach(id in ids)
	{
		user = find_player(lower_case(id)) || load_user(lower_case(id));
		
		set("login/allowmulti", copy(ids - ({ id })), user);
		
		user->save();
	
		tell(me, "設定"+user->query_idname()+"可允許同 IP 登入的玩家為 "+implode(ids - ({ id }), ", ")+"\n");
		
		
		if( !interactive(user) )
			destruct(user);
	}
	
	log_file("command/allowmulti", "允許同 IP 登入："+implode(ids, ", "));
}
