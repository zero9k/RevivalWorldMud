/* This program is a part of RW mudlib
 * See /doc/help/copyright for more info
 -----------------------------------------
 * File   : purewater.c
 * Author : Clode@RevivalWorld
 * Date   : 2003-03-04
 * Note   : 清水
 * Update :
 *  o 2003-00-00  

 -----------------------------------------
 */

#include <ansi.h>
#include <inherit.h>

inherit STANDARD_OBJECT;

void create()
{
	set_idname("pure water",HIC"清"NOR CYN"水"NOR);
	
	if( this_object()->set_shadow_database() ) return;
	
	set("unit", "杯");
	set("capacity", 10);
	set("mass", 100);
	set("value", 10);
	set("badsell", 1);
}

int do_drink()
{
	object me = this_player();
	
	addn("abi/str/cur", 10, me);
	
	if( query("abi/str/cur", me) > query("abi/str/max", me) )
		set("abi/str/cur", copy(query("abi/str/max", me)), me);
	
	msg("$ME咕嚕咕嚕地喝下$YOU，恢復了些許的精力。\n", me, this_object(), 1);
	
	destruct(this_object(), 1);
	
	return 1;
}
