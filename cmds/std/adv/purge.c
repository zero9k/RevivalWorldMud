/* This program is a part of RW mudlib
 * See /doc/help/copyright for more info
 -----------------------------------------
 * File   : purge.c
 * Author : Clode@RevivalWorld
 * Date   : 2002-09-26
 * Note   : purge 指令
 * Update :
 *  o 2000-00-00
 *
 -----------------------------------------
 */

#include <ansi.h>
#include <feature.h>
#include <daemon.h>
#include <secure.h>
#include <message.h>

inherit COMMAND;

#define LOG	"command/purge"

string help = @HELP
	標準 purge 指令。
HELP;

private void command(object me, string arg)
{
	int target_level_num, my_level_num;
	object target;
	string target_idname;
	
	if( !is_command() ) return;
	
	if( !arg ) 
		return tell(me, pnoun(2,me)+"想要刪除哪位使用者的資料？\n", CMDMSG);
	
	if( file_size(user_path(arg)) != -2 )
		return tell(me, "沒有 "+arg+" 這位使用者資料。\n", CMDMSG);
	
	target_level_num = SECURE_D->level_num(arg);
	my_level_num = SECURE_D->level_num(me->query_id(1));
	
	if( my_level_num < target_level_num )
		return tell(me, pnoun(2,me)+"的權限不足以刪除 "+arg+" 的使用者資料。\n", CMDMSG);
	
	
	target = find_player(arg) || load_user(arg);
	
	set("password", PASSWORD_D->query_password(arg), target );
	
	target->save();
	
	target_idname = target->query_idname();

	if( CHAR_D->destruct_char(arg) )
	{
		CHANNEL_D->channel_broadcast("news", me->query_idname()+"在震怒之下將"+target_idname+"的檔案執行了反安裝程序，並儲存到磁片上隨手丟到檔案室的角落。\n");
		log_file(LOG, me->query_idname()+"刪除"+target_idname+"檔案。");
	}
	else
		return tell(me, "刪除"+target->query_idname()+"資料失敗。\n", CMDMSG);
		
	if( objectp(target) ) 
	{
		tell(target, HIR+pnoun(2, target)+"被系統管理員"+me->query_idname()+"刪除玩家檔案了，若有任何意見可再建立新角色與巫師們討論。\n", CMDMSG);
		destruct(target);
	}	
}	