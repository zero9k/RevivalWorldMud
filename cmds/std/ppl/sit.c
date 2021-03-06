/* This program is a part of RW mudlib
 * See /doc/help/copyright for more info
 -----------------------------------------
 * File   : sit.c
 * Author : Clode@RevivalWorld
 * Date   : 2002-02-20
 * Note   : 城市指令
 * Update :
 *  o 2000-00-00
 *
 -----------------------------------------
 */

#include <ansi.h>
#include <map.h>
#include <delay.h>
#include <feature.h>
#include <daemon.h>
#include <message.h>

inherit COMMAND;

string help = @HELP
    你可以使用這個指令在有草叢、大樹、涼亭的地方休息，體力的恢復會比
普通什麼也不做的時候還要快一點。而在這三個地方休息的效果也略有不同，這點就
留待各位比較了。如果說在你附近沒有這些景觀的話，你也可以考慮買塊在附近的地
建造(build)一個。

另外 sit 可以由休息設施的擁有者來設定允許使用此設施的玩家名單
     sit -l          查看目前允許使用名單
     sit -onlyme     只允許擁有者自己使用
     sit -open       開放所有人使用
     sit 'ID'        設定開放給某些特定玩家使用(再輸入一次取消)

HELP;

#define REGEN	90

private void do_command(object me, string arg)
{
	int cost, time, regen, type;
	array loc = query_temp("location", me);
	string owner, myid;
	string *allow;
	string map_system;

	if( !arrayp(loc) || (!CITY_D->is_city_location(loc) && !AREA_D->is_area_location(loc)) )
		return tell(me, pnoun(2, me)+"無法在這個地方休息。\n");

	map_system = MAP_D->query_map_system(loc);

	myid = me->query_id(1);
	owner = map_system->query_coor_data(loc, "owner");
	allow = map_system->query_coor_data(loc, "sit_allow");
	type = map_system->query_coor_data(loc, TYPE);

	if( type != PAVILION && type != TREE && type != GRASS )
		return tell(me, "這個地方沒辦法坐下休息。\n");

	if( map_system == CITY_D )
	{
		if( arg  )
		{
			if( arg == "-l" )
			{
				if( sizeof(allow) )
					tell(me, "此休息設施目前被允許使用的玩家有 "+implode(allow, " ")+"。\n");
				else if( arrayp(allow) )
					tell(me, "此休息設施目前只允許擁有者自己使用。\n");
				else
					tell(me, "此休息設施目前開放所有人使用。\n");

				return;
			}

			if( owner != me->query_id(1) )
				return tell(me, pnoun(2, me)+"不是此地的擁有人，不能對這個休息地點做任何設定。\n");

			if( arg == "-onlyme" )
			{
				CITY_D->set_coor_data(loc, "sit_allow", ({ }));
				return tell(me, pnoun(2, me)+"將此地設定為只有"+pnoun(2, me)+"自己可以使用。\n");
			}
			else if( arg == "-open" )
			{
				CITY_D->delete_coor_data(loc, "sit_allow");
				return tell(me, pnoun(2, me)+"將此地設定為開放所有人使用。\n");
			}
			else
			{
				allow = allow || allocate(0);
				arg = remove_ansi(lower_case(arg));

				if( strlen(arg) > 12 )
					return tell(me, "請輸入正確的玩家 ID。\n");

				if( member_array(arg, allow) != -1 )
				{
					allow -= ({ arg });
					tell(me, pnoun(2, me)+"取消 "+capitalize(arg)+" 使用此設施的資格。\n");
				}
				else
				{
					if( sizeof(allow) > 20 )
						return tell(me, "允許名單無法超過 20 人。\n");

					allow |= ({ arg });
					tell(me, pnoun(2, me)+"允許 "+capitalize(arg)+" 使用此設施。\n");
				}

				CITY_D->set_coor_data(loc, "sit_allow", allow);
				return;
			}
		}

		if( owner != myid && arrayp(allow) && member_array(myid, allow) == -1 )
			return tell(me, "這是 "+capitalize(owner)+" 私人擁有的設施，"+pnoun(2, me)+"目前不被允許使用。\n");
	}

	if( me->is_delaying() )
		return tell(me, me->query_delay_msg());

	switch( type )
	{
		case PAVILION:
			cost = me->query_stamina_max() - me->query_stamina_cur();
			msg("$ME倚著"+(map_system->query_coor_data(loc, "short")||"涼亭")+"內的石柱旁，坐在石椅上納涼休息(恢復 "+cost+" 體力)。\n", me, 0, 1);
			regen = me->stamina_regen()+REGEN;
			time = cost/regen + 1;

			if( time <= 0 )
				tell(me, "休息完畢。\n");
			else
			{
				set_temp("rest_regen/stamina", REGEN, me);
				me->start_delay(REST_DELAY_KEY, time, pnoun(2, me)+"正在"+(map_system->query_coor_data(loc, "short")||"涼亭")+"內休息。\n", "休息完畢。\n", bind((: delete_temp("rest_regen", $(me)), $(me)->set_stamina_full() :), me));
			}
			break;

		case TREE:
			cost = me->query_health_max() - me->query_health_cur();
			msg("$ME倚著"+(map_system->query_coor_data(loc, "short")||"樹幹")+"，在濃密的樹蔭下享受自然的氣息(恢復 "+cost+" 生命)。\n", me, 0, 1);

			regen = me->health_regen()+REGEN;
			time = 2*cost/regen + 1;

			if( time <= 0 )
				tell(me, "休息完畢。\n");
			else
			{
				set_temp("rest_regen/health", REGEN, me);
				me->start_delay(REST_DELAY_KEY, time, pnoun(2, me)+"正坐在樹下休息。\n", "休息完畢。\n", bind((: delete_temp("rest_regen", $(me)), $(me)->set_health_full() :), me));
			}
			break;

		case GRASS:
			cost = me->query_energy_max() - me->query_energy_cur();
			msg("$ME斜躺在"+(map_system->query_coor_data(loc, "short")||"草皮")+"上，讓微風輕輕拂拭(恢復 "+cost+" 精神)。\n", me, 0, 1);

			regen = me->energy_regen()+REGEN;
			time = 2*cost/regen + 1;

			if( time <= 0 )
				tell(me, "休息完畢。\n");
			else
			{
				set_temp("rest_regen/energy", REGEN, me);
				me->start_delay(REST_DELAY_KEY, time, pnoun(2, me)+"正躺在"+(map_system->query_coor_data(loc, "short")||"草皮")+"上休息。\n", "休息完畢。\n", bind((: delete_temp("rest_regen", $(me)), $(me)->set_energy_full() :), me));
			}
			break;
		default:
			error("sit error.");
	}
}
