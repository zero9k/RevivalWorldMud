/* This program is a part of RW mudlib
 * See /doc/help/copyright for more info
 -----------------------------------------
 * File   : client.c
 * Author : Clode@RevivalWorld
 * Date   : 2002-3-15
 * Note   : 客戶端軟體板
 * Update :
 *  o 2000-00-00
 *
 -----------------------------------------
 */

#include <inherit.h>

inherit BULLETIN_BOARD;

void create()
{
	set_idname("progress board", "客戶端軟體討論板");
	
	// 利用 shadow_ob 技術來做分散式公佈欄, 即時資料皆儲存於 shadow_ob 上
	if( clonep() ) 
		set("shadow_ob", find_object(base_name(this_object())));
	
	else
	{
		set("boardid", "client");
		set("capacity", 2000);
		
		initialize_board();
	}
	startup_action();
}
