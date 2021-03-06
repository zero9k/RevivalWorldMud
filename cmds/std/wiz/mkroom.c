/* This program is a part of RW mudlib
 * See /doc/help/copyright for more info
 -----------------------------------------
 * File   : mkroom.c
 * Author : Clode@RevivalWorld
 * Date   : 2001-5-8
 * Note   : mkroom 指令
 * Update :
 *  o 2000-00-00  
 *
 -----------------------------------------
 */

#include <inherit.h>
#include <feature.h>
#include <message.h>

inherit COMMAND;

string help = @HELP
	標準 mkroom 指令
HELP;

private void command(object me, string arg)
{	
	object room;
	
	if( !is_command() ) return;
	
	if( !arg || arg == "")
		return tell(me, "請輸入你要建立房間之檔案名稱或路徑。\n", CMDMSG);

	
	arg = resolve_path(me, arg);
	
	if( file_size(arg[0..strsrch(arg,"/")]) != -2 ) 
		return tell(me, "請先建立目錄 "+arg[0..strsrch(arg,"/")]+" 再行建造房間。\n", CMDMSG);

	
	if( strsrch(arg, "/room/") == -1 )
		return tell(me, "房間只能建造在目錄 room 的子目錄下。\n", CMDMSG);

	
	tell(me, "建立基本房間 "+arg+" 中...。\n", CMDMSG);
	
	if( !(room = load_object(arg)))
		tell(me, "無法建立基本房間。\n", CMDMSG);
	else
	{
		tell(me, "房間 "+arg+" 建立存檔完畢。\n", CMDMSG);
		room->save();
	}
}