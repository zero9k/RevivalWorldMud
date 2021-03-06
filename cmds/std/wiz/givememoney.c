/* This program is a part of RW mudlib
 * See /doc/help/copyright for more info
 -----------------------------------------
 * File   : givememoney.c
 * Author : Clode@RevivalWorld
 * Date   : 2003-04-13
 * Note   : givememoney 指令
 * Update :
 *  o 2000-00-00  
 *
 -----------------------------------------
 */
 
#include <feature.h>
#include <ansi.h>
#include <message.h>
#include <daemon.h>

inherit COMMAND;

string help = @HELP
	標準 givememoney 指令
HELP;

private void command(object me, string arg)
{
	string unit;

	if( !is_command() ) return;
	
	if( !arg || sscanf(arg, "$%s %s", unit, arg) != 2 )
		return tell(me, "請輸入金額，例：$CT 100。\n", CMDMSG);

	if( !MONEY_D->money_unit_exist(unit) )
		return tell(me, "沒有 "+unit+" 這種貨幣。\n", CMDMSG);
		
	arg = big_number_check(arg);
	
	if( !arg || count(arg, "<=", 0) )
		return tell(me, "請輸入正確的金額數目。\n", CMDMSG);

	me->earn_money(unit, arg);
	
	msg("$ME的頭頂上突然飄下來了"+HIY+QUANTITY_D->money_info(unit, arg)+NOR" 的鈔票，$ME馬上舉起雙手全數抓進口袋裡。\n", me,0,1);
}
