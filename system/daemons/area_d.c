/* This program is a part of RW mudlib
 * See /doc/help/copyright for more info
 -----------------------------------------
 * File   : area_d.c
 * Author : Clode@RevivalWorld
 * Date   : 2003-08-18
 * Note   : 區域精靈, 處理所有區域資料庫
 * Update :
 *
 -----------------------------------------
 */

inherit __DIR__"area_d_main.c";

private void create()
{
	if( clonep() )
		destruct(this_object());

	// 避免發生慘劇
	if( base_name(this_object()) != "/system/daemons/area_d" )
		destruct(this_object());

	this_object()->restore_all_data();
}

int remove()
{
	return this_object()->save_all();
}

string query_name()
{
	return "郊區系統(AREA_D)";
}
