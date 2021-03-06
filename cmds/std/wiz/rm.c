/* This program is a part of RW mudlib
 * See /doc/help/copyright for more info
 -----------------------------------------
 * File   : rm.c
 * Author : Clode@RevivalWorld
 * Date   : 2001-2-6
 * Note   : rm 指令
 * Update :
 *  o 2000-00-00  
 *
 -----------------------------------------
 */

#include <feature.h>
#include <message.h>

inherit COMMAND;

string help = @HELP
	標準 rm 指令
HELP;

private void command(object me, string arg)
{
	string *parse;
	
	if( !is_command() ) return;
	
	if( !arg )
		return tell(me, "請輸入欲刪除的檔名。\n", CMDMSG);
	
	parse = single_quote_parse(arg);
	
	arg = resolve_path(me, parse[0]);
	
	if( file_size(arg) < 0 )
		return tell(me, "沒有 "+arg+" 這個檔案。\n", CMDMSG);
	
	if( rm( arg ) )
		tell(me, "刪除 "+arg+" 完畢。\n", CMDMSG);
	else
		tell(me, "權限不足，無法刪除 "+arg+"。\n", CMDMSG);
}