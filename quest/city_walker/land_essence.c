/* This program is a part of RW mudlib
 * See /doc/help/copyright for more info
 -----------------------------------------
 * File   : land_essence.c
 * Author : 
 * Date   : 2005-01-05
 * Note   : 釣魚筆記
 * Update :
 *  o 2003-00-00  

 -----------------------------------------
 */

#include <ansi.h>
#include <inherit.h>

inherit STANDARD_OBJECT;

mapping actions;

void create()
{
	set_idname(HIY"land "NOR YEL"essence"NOR, HIY"大地"NOR YEL"精華本質"NOR);
	
	if( this_object()->set_shadow_database() ) return;
	
	set("long", "一塊可以吸納土地精華的媒介物質，但每塊只能吸納一次。");
	set("unit", "塊");
	set("mass", 3000);
	set("value", 100);
	set("badsell", 1);
	set("land_absorb", 1);
}
