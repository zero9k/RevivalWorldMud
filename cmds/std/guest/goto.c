/* This program is a part of RW mudlib
 * See /doc/help/copyright for more info
 -----------------------------------------
 * File   : goto.c
 * Author : Clode@RevivalWorld
 * Date   : 2001-2-5
 * Note   : goto 指令
 * Update :
 *  o 2000-00-00  
 *
 -----------------------------------------
 */

#include <ansi.h>
#include <daemon.h>
#include <feature.h>
#include <message.h>

inherit COMMAND;

string help = @HELP
	標準 goto 指令。
HELP;

private void command(object me, string arg)
{
	object ppl, newenv;
	
	if( !is_command() ) return;
	
	if( !arg )
		return tell(me, pnoun(2, me)+"想要去哪裡？\n", CMDMSG);
	if( ppl = find_player(arg) )
		newenv = environment(ppl);
	else
	{
		arg = resolve_path(me, arg);
		intact_file_name(ref arg);
		
		if( find_object(arg) || file_exist(arg) )
			newenv = load_object(arg);
	}
	
	if( !newenv ) return tell(me, "沒有這個地方。\n", CMDMSG);
	
	msg("$ME走進一陣"HIW"白霧"NOR"後消失了。\n", me, 0, 1, ENVMSG);

	if( me == ppl )
		return tell(me, "前往"+pnoun(2, me)+"目前所在的地方？\n", CMDMSG);

	if( same_environment(me, ppl) )
		return tell(me, pnoun(2, me)+"與"+ppl->query_idname()+"已經在同一個地方。\n", CMDMSG);

	if( objectp(ppl) && userp(ppl) && newenv->is_maproom() )
		me->move(query_temp("location", ppl));
	else
		me->move(newenv);
	
	msg(HIW"漫漫白霧"NOR"之中突然出現$ME的身影。\n", me, 0, 1, ENVMSG);
}