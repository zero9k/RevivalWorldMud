/* This program is a part of RW mudlib
 * See /doc/help/copyright for more info
 -----------------------------------------
 * File   : herbage.c
 * Author : Clode@RevivalWorld
 * Date   : 2005-09-09
 * Note   : 牧草
 * Update :
 *  o 2000-00-00
 *
 -----------------------------------------
 */

#include <ansi.h>
#include <inherit.h>

inherit STANDARD_OBJECT;

void create()
{
        set_idname("herbage","牧草");
        
        if( this_object()->set_shadow_ob() ) return;
        
        set("long","作為飼養草食性動物的飼料。\n");
        set("unit", "把");
        set("mass", 400);
        set("value", 100);
}