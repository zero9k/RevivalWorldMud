/* This program is a part of RW mudlib
 * See /doc/help/copyright for more info
 -----------------------------------------
 * File   : rice.c
 * Author : Clode@RevivalWorld
 * Date   : 2002-2-20
 * Note   : 稻米
 * Update :
 *  o 2000-00-00
 *
 -----------------------------------------
 */

#include <inherit.h>

inherit STANDARD_OBJECT;

void create()
{
	set_idname("rice","稻米");
	
	if( this_object()->set_shadow_ob() ) return;
	
	set("long","可作為一般的主食。\n");
	set("unit", "袋");
	set("mass", 400);
	set("value", 100);
}