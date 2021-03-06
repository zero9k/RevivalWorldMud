/* This program is a part of RW mudlib
 * See /doc/help/copyright for more info
 -----------------------------------------
 * File   : eat.c
 * Author : Clode@RevivalWorld
 * Date   : 2003-03-04
 * Note   : eat 指令
 * Update :
 *  o 2000-00-00  
 *
 -----------------------------------------
 */

#include <ansi.h>
#include <feature.h>

inherit COMMAND;

string help = @HELP
食用食物的指令
eat '物品'		- 將某個物品吃到肚子裡

HELP;

private void do_command(object me, string arg)
{
	int capacity;
	object ob;
	
	if( !arg )
	{
		if( me->is_npc() )
			return me->do_command("say 老闆想要我吃些什麼？\n");
		else
			return tell(me, pnoun(2, me)+"想要吃些什麼？\n");
	}
	
	ob = present(arg, me) || present(arg, environment(me));
	
	if( !objectp(ob) )
	{
		if( me->is_npc() )
			return me->do_command("say 附近並沒有 "+arg+" 這種食物。\n");
		else
			return tell(me, "附近並沒有 "+arg+" 這種食物。\n");
	}

	if( !ob->is_food() )
	{
		if( me->is_npc() )
			return me->do_command("say "+ob->query_idname()+"不是食物，無法食用。\n");
		else
			return tell(me, ob->query_idname()+"不是食物，無法食用。\n");
	}
	
	capacity = query("capacity", ob);

	if( capacity + me->query_food_cur() > me->query_food_max() )
	{
		if( me->is_npc() )
			return me->do_command("say 我吃不下"+ob->query_idname()+"了！\n");
		else
			return tell(me, pnoun(2, me)+"吃不下"+ob->query_idname()+"了！\n");
	}
	
	me->add_food(capacity);

	msg("$ME將$YOU吃進肚子裡。\n", me, ob, 1);

	ob->eat();
	destruct(ob, 1);
}