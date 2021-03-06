/* This program is a part of RW mudlib
 * See /doc/help/copyright for more info
 -----------------------------------------
 * File   : give.c
 * Author : Clode@RevivalWorld
 * Date   : 2002-01-01
 * Note   : give 指令
 * Update :
 *  o 2000-00-00  
 *
 -----------------------------------------
 */

#include <ansi.h>
#include <object.h>
#include <daemon.h>
#include <feature.h>
#include <message.h>

inherit COMMAND;

string help = @HELP
將物品或金錢給予某人

give [某人] [數量|all] [物品] [排序]		- 將某數量或排序的某物品給予某人
give [數量|all] [物品] [排序] to [某人]		- 將某數量或排序的某物品給予某人
HELP;


private void do_give(object me, object target, string arg)
{
	int mass;
	object ob;
	string amount;

	if( target == me )
		return tell(me, "東西已經在"+pnoun(2, me)+"的身上了！\n");
	
	// Delay 中不能下指令
	if( me->is_delaying() )
		return tell(me,	me->query_delay_msg());

	if( sscanf(arg, "$%s %s", arg, amount) == 2 )
	{
		if( me->is_npc() )
			return me->do_command("say 我無法給予金錢。\n");

		if( !MONEY_D->money_unit_exist(arg) )
		{
			if( !arg || !arg[0] )
				return tell(me, pnoun(2, me)+"要給哪一種貨幣？\n");

			tell(me, "並沒有 "+arg+" 這種貨幣。\n");
			
			return;
		}
		
		amount = big_number_check(amount);
		if( !amount )
			return tell(me, "請輸入正確的數字。\n");
		
		if( count(amount, "<=", 0) )
			return tell(me, "金額必須要大於零。\n");
		
		if( !me->spend_money(arg, amount) )
			return tell(me, pnoun(2, me)+"身上沒有那麼多的 $"+upper_case(arg)+" 錢。\n");
			
		target->earn_money(arg, amount);
		
		if( !wizardp(target) )
		{
			log_file("command/give", me->query_idname()+"給"+target->query_idname()+" "+money(arg, amount));
			CHANNEL_D->channel_broadcast("nch", me->query_idname()+"給"+target->query_idname()+" "+money(arg, amount));
		}
		
		if( target->is_npc() )
			call_out((: call_other :), 0, target, "reply_get_money", me, arg, amount);

		return msg("$ME拿給$YOU "HIY+money(arg, amount)+NOR"。\n", me, target, 1);
	}
		
	if( sscanf(arg, "%s %s", amount, arg) == 2 )
	{
		if( amount == "all" ) ;
		// 如果 amount 並不是數字單位
		else if( !big_number_check(amount) )
		{
			arg = amount + " " + arg;
			amount = "1";	
		}	
		else if( count(amount, "<", 1) ) amount = "1";
	}
	else amount = "1";
	
	if( objectp(ob = present(arg, me)) )
	{
		object base_ob = load_object(base_name(ob));
		string ob_idname = ob->query_idname(), unit = query("unit", ob)||"個";
		
		if( amount != "all" && count(amount, ">", copy(query_temp("amount", ob))||1 ) )
			return tell(me, pnoun(2, me)+"身上沒有那麼多"+unit+ob_idname+"。\n");
		
		if( amount == "all" )
			amount = query_temp("amount", ob)||"1";
		
		if( ob->is_keeping() )
			return tell(me, pnoun(2, me)+"必須先解除"+ob->query_idname()+"的保留(keep)狀態。\n");

		if( query("flag/no_give", ob) )
			return tell(me, ob->query_idname()+"不允許交易。\n");

		if( target->get_object(ob, amount) )
			msg("$ME把"+QUANTITY_D->obj(ob || base_ob, amount)+"拿給$YOU。\n", me, target, 1);
		else
			msg("$ME想把"+QUANTITY_D->obj(ob || base_ob, amount)+"拿給$YOU，但對$YOU而言似乎太重了。\n", me, target, 0);
		
		return;
	}
	
	if( lower_case(arg) == "all" )
	{		
		object *all_ob = all_inventory(me);
		
		if( sizeof(all_ob) )
		{
			int longest, too_heavy, loading, max_loading;
			string msg="", str;
		
			max_loading = target->query_max_loading();
			
			foreach(ob in all_ob)
			{
				mass = query("mass", ob);
				
				if( mass == -1 || ob->is_keeping() || query("flag/no_give", ob) ) continue;
				
				amount = copy(query_temp("amount",ob)) || "1";
				
				loading = max_loading - target->query_loading();
				
				if( count(loading, "<", count(amount,"*",mass) ) )
				{
					if( loading < mass ) continue;
					
					str = (msg==""?"「 ":"   ")+QUANTITY_D->obj(ob, loading/mass, 1)+"\n";
					ob->move(target, loading/mass);
					too_heavy = 1;
				}
				else
				{
					str = (msg==""?"「 ":"   ")+QUANTITY_D->obj(ob, amount, 1)+"\n";
					ob->move(target, amount);
				}
				
				
				msg += str;	
				
				if( noansi_strlen(str) > longest ) longest = noansi_strlen(str);
			}
			
			if( msg == "" )
				return msg("$ME試著給$YOU所有的物品，但$YOU身上負重太重了，什麼都拿不動了。\n", me, target, 0);
			
			msg = msg[0..<2]+repeat_string(" ", longest-noansi_strlen(str)+1 );
			
			msg("$ME把身上能給的東西全給了$YOU。\n"+msg+"」\n\n", me, target, 1);
			
			if( too_heavy )
				msg("剩下的東西似乎太重$YOU拿不動了。\n", me, target, 0);
			
			return;
		}
		return tell(me, pnoun(2, me)+"身上沒有任何物品可以給"+target->query_idname()+"。\n");
	}
	return tell(me, pnoun(2, me)+"身上沒有 "+arg+" 這件物品。\n");
}

private void confirm_give(string input, object me, object target, string arg)
{
	if( input && lower_case(input)[0] == 'y' )
		do_give(me, target, arg);
	else
		tell(me, pnoun(2, me)+"取消將物品拿給"+target->query_idname()+"的動作。\n");
		
	me->finish_input();
}

private void do_command(object me, string arg)
{
	object target;
	string target_name;
	
	if( !arg )
		return tell(me, pnoun(2, me)+"想要給誰什麼東西？\n");

	if( sscanf(arg, "%s to %s", arg, target_name) != 2 )
		sscanf(arg, "%s %s", target_name, arg);
	
	if( !arg || !target_name )
		return tell(me, pnoun(2, me)+"想要給誰什麼東西？\n");

	if( !objectp(target = present(target_name, environment(me))) )
		return tell(me, "這裡沒有 "+target_name+" 這個人。\n");

	if( !target->is_living() )
		return tell(me, pnoun(2, me)+"不能給"+target->query_idname()+"任何東西。\n");
		
	if( userp(target) )
		do_give(me, target, arg);
	else
	{
		if( query("boss", target) == me->query_id(1) )
			tell(me, HIY"注意：若是給予員工金錢將無法再取回\n"NOR+pnoun(2, me)+"確定要將物品拿給"+target->query_idname()+"嗎(Yes/No)？\n");
		else
			tell(me, HIY"注意：給予的物品或金錢將無法再取回\n"NOR+pnoun(2, me)+"確定要將物品拿給"+target->query_idname()+"嗎(Yes/No)？\n");

		input_to((: confirm_give :), me, target, arg);
	}
}