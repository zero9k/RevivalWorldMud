/* This program is a part of RW mudlib
 * See /doc/help/copyright for more info
 -----------------------------------------
 * File   : Barley.c
 * Author : 
 * Date   : 2003-4-23
 * Note   : 大麥
 * Update :
 *  o 2000-00-00
 *
 -----------------------------------------
 */

#include <inherit.h>

inherit STANDARD_OBJECT;

void create()
{
        set_idname("barley","大麥");
        
        if( this_object()->set_shadow_ob() ) return;
        
        set("long","可用來釀酒或作為粗纖維食品。\n");
        set("unit", "袋");
        set("mass", 500);
        set("value", 200);
}

