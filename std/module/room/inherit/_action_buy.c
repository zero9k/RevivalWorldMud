/* This program is a part of RW mudlib
 * See /doc/help/copyright for more info
 -----------------------------------------
 * File   : _buy_action.c
 * Author : Clode@RevivalWorld
 * Date   : 2004-06-23
 * Note   : 
 * Update :
 *  o 2002-00-00
 *
 -----------------------------------------
 */

#include <ansi.h>
#include <daemon.h>
#include <message.h>
#include <npc.h>

int over_capacity(object env, string database, string amount, mixed capacity);
void input_object(object env, string database, string basename, string amount);
void output_object(object env, string database, string basename, string amount);


#define STORE_CAPACITY		100000
#define WAREHOUSE_CAPACITY	-1

// 買入商品
void do_buy(object me, string arg, string database, string buyer)
{
	int price_skill_level;
	int i, which = 1, size, capacity;
	object env, product, newproduct, delivery, delivery_master;
	mapping products;
	string tmp, amount = "1", env_city, cost, earn, productname, delivery_city;
	int delivery_x, delivery_y, num;
	string *productlist, pamount, basename;
	array loc;
	
	env = environment(me)->query_master();

	if( !arg )
		return tell(me, pnoun(2, me)+"想要買什麼商品？\n");
	
	env_city = env->query_city();

	if( query("mode", env) )
		return tell(me, environment(me)->query_room_name()+"目前狀態為「管理模式」，停止販賣物品。\n");

	if( 
		sscanf(arg, "%s to '%s' %d %d,%d", arg, delivery_city, num, delivery_x, delivery_y) == 5 ||
		sscanf(arg, "%s to %s %d %d,%d", arg, delivery_city, num, delivery_x, delivery_y) == 5
	)
	{
		if( !CITY_D->city_exist(delivery_city||env_city, num-1) )
			return tell(me, "這座城市並沒有第 "+num+" 衛星都市。\n");
	
		loc = arrange_city_location(delivery_x-1, delivery_y-1, delivery_city||env_city, num-1);
		
		delivery = load_module(loc);
		
		if( !objectp(delivery) )
			return tell(me, "座標"+loc_short(loc)+"處並沒有任何建築物。\n");
			
		if( query("owner", delivery) != me->query_id(1) )
			return tell(me, delivery->query_room_name()+"並不是"+pnoun(2, me)+"的建築物。\n");
		
		delivery_master = delivery->query_master();

		switch( delivery->query_building_type() )
		{
			case "store":
				capacity = STORE_CAPACITY;
				break;
			case "warehouse":
				capacity = WAREHOUSE_CAPACITY;
				break;
			default:
				return tell(me, delivery->query_room_name()+"沒有辦法接收任何商品。\n");
		}
		
		switch( query("function", delivery) )
		{
			case "front":
			case "storeroom":
			case "warehouse":
				break;
			default:
				return tell(me, delivery->query_room_name()+"沒有辦法接收任何商品。\n");
		}
		
		if( delivery_master == env )
			return tell(me, pnoun(2, me)+"欲運送的商品地點不能與此地相同或有相同連鎖中心。\n");
	}
		
	sscanf(arg, "%s %s", amount, productname);
	
	if( !productname || (amount != "all" && !(amount = big_number_check(amount))) )
	{
		productname = arg;
		amount = "1";
	}
	
	if( sscanf( productname, "%s %d", tmp, which ) == 2 )
		productname = tmp;

	if( which < 1 ) which = 1;

	if( env->query_building_type() == "trading_post" )
		products = fetch_variable("allowable_trade");
	else
		products = query(database, env);
	
	if( !sizeof(products) )
		return tell(me, query("short", env)+"目前沒有販賣任何商品。\n");

	productlist = implode(values(products), (:$1+$2:))||allocate(0);
	size = sizeof(productlist);
	price_skill_level = me->query_skill_level("price");

	for(i=0;i<size;i+=2)
	{
		basename = productlist[i];
		pamount = productlist[i+1];

		if( catch(product = load_object(basename)) )
			continue;

		// 檔案已經消失
		if( !objectp(product) )
		{
			error(basename+" 商品資料錯誤。");
			continue;
		}

		if( (i+2)/2 == to_int(big_number_check(productname)) || (product->query_id(1) == lower_case(productname) && !(--which)) )
		{
			if( amount == "all" )
			{
				if( count(pamount, "<", 1) )
					return tell(me, pnoun(2, me)+"無法買下所有的"+product->query_idname()+"。\n");
				else
					amount = pamount;
			}
			else if( pamount != "-1" && count(amount,">",pamount) )
				return tell(me, "這裡並沒有販賣這麼多"+(query("unit", product)||"個")+product->query_idname()+"。\n");
			else if( count(amount, "<", 1) )
				return tell(me, "請輸入正確的購買數量。\n");
			
			foreach(string shelf, array data in products)
				if( member_array(basename, data) != -1 && !shelf )
					return tell(me, product->query_idname()+"目前僅供展示。\n");
			
			cost = count(amount,"*",copy(query("setup/price/"+replace_string(basename, "/", "#"), env)||query("value", product)));
			earn = cost;

			if( delivery_master )
			{
				// 直接運送的額外收費 1%
				if( query("function", env) == "lobby" )
				{
					if( count(cost, "/", 1000) == "0" )
						cost = count(cost, "+", 1*(100-price_skill_level));
					else
						cost = count(cost, "+", count((100-price_skill_level), "*", count(cost, "/", 1000)));
				}
				// 數量超過可上架數量
				if( over_capacity(delivery_master, database, amount, capacity) )
					return tell(me, delivery->query_room_name()+"無法再容納這麼多的物品了。\n");
			}
						
			if( !me->spend_money(MONEY_D->city_to_money_unit(env_city), cost) )
				return tell(me, pnoun(2,me)+"身上的 $"+MONEY_D->city_to_money_unit(env_city)+" 錢不夠了！！\n");
				
			if( delivery_master )
			{
				msg("$ME花了"HIY+QUANTITY_D->money_info(env_city, cost)+NOR" 購買了"+QUANTITY_D->obj(product, amount)+"，並支付 "+sprintf("%.1f", (100-price_skill_level)/10.)+"% 的運費直接送達"+query("short", delivery)+loc_short(loc)+"。\n",me,0,1);
				log_file("command/buy", me->query_idname()+"花了"HIY+QUANTITY_D->money_info(env_city, cost)+NOR" 購買了"+QUANTITY_D->obj(product, amount)+"，並支付 "+sprintf("%.1f", (100-price_skill_level)/10.)+"% 的運費直接送達"+query("short", delivery)+loc_short(loc)+"。");

				if( query("function", delivery_master) == "storeroom" )
					input_object(delivery_master, "storeroom", basename, amount);
				else
					input_object(delivery_master, database, basename, amount);
					
				delivery_master->save();
			}
			else
			{
				newproduct = new(basename);
				
				if( !query("flag/no_amount", newproduct) )
					set_temp("amount", amount, newproduct);
	
				if( !me->get_object(newproduct, amount) )
				{
					me->earn_money(MONEY_D->city_to_money_unit(env_city), cost);
					tell(me, pnoun(2, me)+"身上的物品太多或太重無法再拿更多東西了。\n");
					return;
				}
				
				msg("$ME花了"HIY+QUANTITY_D->money_info(env_city, cost)+NOR" 購買了"+QUANTITY_D->obj(product, amount)+"。\n",me,0,1);
				//log_file("command/buy", me->query_idname()+"花了"HIY+QUANTITY_D->money_info(env_city, cost)+NOR" 購買了"+QUANTITY_D->obj(product, amount)+"。");
			}

			switch(buyer)
			{
				case "GOVERNMENT":
					break;
				case "ENVIRONMENT":
					set("money", count(copy(query("money", env)), "+", earn), env);
					break;
			}

			if( pamount != "-1" )
				output_object(env, database, basename, amount);

			if( !userp(me) && product->is_module_object() && !SECURE_D->is_wizard(query("produce/designer", product)) )
			{
				set("sells", count(query("sells", product), "+", amount), product);
				
				//hmm
				product->save();
				TOP_D->update_top("product",
					query("produce/time", product) + product->query_id(1),
					query("sells", product),
					product->query_idname(),
					query("value", product),
					query("produce/designer", product),
					query("produce/factory", product));

				// TOP_D->set_top("product", ({ query("produce/time", product)+product->query_id(1), query("sells", product), product->query_idname(), query("value", product), query("produce/designer", product), query("produce/factory", product) }), 30);
			}
				
			return;
		}
	}
	
	return tell(me, "這裡並沒有販賣 "+productname+" 這種商品。\n");
}