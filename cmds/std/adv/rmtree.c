/* This program is a part of RW mudlib
 * See /doc/help/copyright for more info
 -----------------------------------------
 * File   : rmtree.c
 * Author : Clode@RevivalWorld
 * Date   : 2002-06-03
 * Note   : rmtree 指令。
 * Update :
 *  o 2000-00-00
 *
 -----------------------------------------
 */

#include <feature.h>
#include <message.h>

inherit COMMAND;

string help = @HELP
        標準 rmtree 指令。
HELP;

private void confirm_rmtree(object me, string dir, string arg)
{
	string msg = "";
	array res;
	
	if( !arg || lower_case(arg) != "y" )
		return tell(me, "取消刪除 "+dir+" 目錄。\n", CMDMSG);
	
	res = rmtree(dir);
	
	foreach( string file in res[0] )
		msg += "rm "+file+" Done...。\n";
	
	foreach( string file in res[1] )
		msg += "rm "+file+" Failed...。\n";
		
	if( msg == "" )	
		msg = "rmtree "+dir+" Failed...。\n";

	tell(me, msg, CMDMSG);
	me->finish_input();	
}
private void command(object me, string arg)
{
	if( !is_command() ) return;
	
	if( !arg )
		return tell(me, "請輸入目錄路徑。\n", CMDMSG);

	arg = resolve_path(me, arg);
		
	if( file_size(arg) != -2 )
		return tell(me, arg+" 並不是一個目錄。\n", CMDMSG);

	tell(me, "確定刪除目錄："+arg+" (Yes/No)？", CMDMSG);
	input_to( (: confirm_rmtree, me, arg :) );
}
