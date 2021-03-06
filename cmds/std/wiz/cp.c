/* This program is a part of RW mudlib
 * See /doc/help/copyright for more info
 -----------------------------------------
 * File   : cp.c
 * Author : Clode@RevivalWorld
 * Date   : 2001-2-6
 * Note   : cp 指令
 * Update :
 *  o 2000-00-00  
 *
 -----------------------------------------
 */

#include <feature.h>
#include <message.h>

inherit COMMAND;

#define FROM		0
#define TO		1

string help = @HELP
	標準 cp 指令
HELP;

private void command(object me, string arg)
{
	string from, to, *parse;
	int size;
	
	if( !is_command() ) return;
	
	if( !arg )
		return tell(me, "指令格式: cp <原檔名> <目標檔名> \n", CMDMSG);
	
	parse = single_quote_parse(arg);

	if( sizeof(parse) != 2 )
		return tell(me, "指令格式: cp <原檔名> <目標檔名> \n", CMDMSG);
		
	from = resolve_path(me, parse[FROM]);
	to = resolve_path(me, parse[TO]);

	size = file_size(from);
	if( size == -2 ) // it's a directory
	{
		string msg = "";
		array res = cptree(from, to);
		
		foreach( string file in res[0] )
			msg += "cp "+file+" Done...。\n";
		
		foreach( string file in res[1] )
			msg += "cp "+file+" Failed...。\n";
		
		if( msg == "" )
			msg = "cp "+from+" -> "+to+" Failed...。\n";
			
		return tell(me, msg, CMDMSG);
	}
	else if( size == -1 ) // file not found.
		return tell(me, "沒有 " + from + " 這個檔案或目錄。\n", CMDMSG);
	else
	{
		if( cp(from, to) )
			tell(me, "cp "+from+" -> "+to+" Done...。\n", CMDMSG);
		else
			tell(me, "cp "+from+" -> "+to+" Failed...。\n", CMDMSG);
	}
}