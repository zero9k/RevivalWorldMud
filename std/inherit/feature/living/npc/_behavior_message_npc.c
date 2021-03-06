/* This program is a part of RW mudlib
 * See /doc/help/copyright for more info
 -----------------------------------------
 * File   : _behavior_message_npc.c
 * Author : Clode@RevivalWorld
 * Date   : 2001-2-6
 * Note   : NPC 訊息行為
 * Update :
 *  o 2000-00-00  
 *
 -----------------------------------------
 */

#include <ansi.h>
#include <daemon.h>
#include <function.h>

/* 由 catch_tell 傳入訊息與訊息分類 */
varargs void receive_msg(string msg, string classes)
{
	object me;
	string *command, verb, args;
	
	if( !objectp(me = this_player()) || !userp(me) ) return;
	
	command = query_temp("command", me);
	
	if( sizeof(command) != 2 ) return;
	
	verb = command[0];
	args = command[1];
	
	// 表情指令
	if( args && EMOTE_D->is_emote(verb) )
	{
		string id, arg;

		if( sscanf(args, "%s %s", id, arg) == 2 && this_object()->id(id) )
			call_out((: call_other :), 1, this_object(), "reply_emote", me, verb, arg);
		else if( this_object()->id(args) )
			call_out((: call_other :), 1, this_object(), "reply_emote", me, verb);
	}
	// Say 指令
	else if( verb == "say" )
		call_out((: call_other :), 1, this_object(), "reply_say", me, args);
		
	// 任意 catch_tell 所收到的訊息
	//else
	//	call_out((: call_other :), 1, this_object(), "reply_catch_tell", me, verb, args);
}
