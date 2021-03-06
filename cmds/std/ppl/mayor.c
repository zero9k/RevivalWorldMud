/* This program is a part of RW mudlib
 * See /doc/help/copyright for more info
 -----------------------------------------
 * File   : mayor.c
 * Author : Clode@RevivalWorld
 * Date   : 2003-08-14
 * Note   : 城市資訊
 * Update :
 *  o 2003-00-00  
 *
 -----------------------------------------
 */

#include <ansi.h>
#include <feature.h>
#include <daemon.h>

inherit COMMAND;

string help = @HELP
	顯示城市資訊。
HELP;

private void do_command(object me, string arg)
{
	int i, j, totalresident, totalflourish;
	string str;
	string *cities, city, mayor, *emblem;
	float rate;
	string GNP, NNP;

	if(!arg)
	{
		cities = sort_array(CITY_D->query_cities() - ({ "wizard" }), (: CITY_D->query_city_info($1, "totalflourish") < CITY_D->query_city_info($2, "totalflourish") ? 1 : -1 :));
	
		str = HIW"城市             衛星都市 佔領 市長        總繁榮度 市民 文明時代 開放註冊 經濟成長\n";
		str +=NOR WHT"─────────────────────────────────────────\n"NOR;
		
		foreach(city in cities)
		{
			mayor = CITY_D->query_city_info(city, "government/mayor");

			GNP = CITY_D->query_city_info(city, "GNP");
			NNP = CITY_D->query_city_info(city, "NNP");
			
			if( count(GNP, "<=", 0) )
				rate = 0.;
			else
				rate = to_float(count(count(NNP, "*", 10000), "/", GNP))/100.;

			for(i=0;CITY_D->city_exist(city, i);i++);
			
			if( CITY_D->query_city_info(city, "fallen") ) rate = 0.;
			
			str += sprintf("%-24s"HIM"%d %3s  "HIW"%-12s"HIG"%-9s"HIC"%-5d"HIY"%-9s%-6s"+(rate>0?HIG:HIR)+"%9.2f%%\n"NOR,
				CITY_D->query_city_idname(city),
				i,
				CITY_D->query_city_info(city, "fallen") ? HIR"╳"NOR : HIG"○"NOR,
				mayor ? capitalize(mayor) : "",
				NUMBER_D->number_symbol(CITY_D->query_city_info(city, "totalflourish")),
				sizeof(CITY_D->query_city_info(city, "citizens")),
				CITY_D->query_age_name(city),
				CITY_D->query_city_info(city, "fallen") ? "":CITY_D->query_city_info(city, "register_open")?HIG"開放"NOR:HIR"封閉"NOR,
				rate,
			);
		}
		str +=NOR WHT"─────────────────────────────────────────\n"NOR;
		str +=NOR WHT"mayor '城市ID' 查詢細節內容\n"NOR;
	}
	else
	{
		string city_idname, moneyunit, area, city_scale_mayor;
		mapping officer;
		
		city = arg;

		if( !CITY_D->city_exist(city) )
			return tell(me, "沒有 "+city+" 這個城市。\n");
		
		//if( CITY_D->query_city_info(city, "fallen") )
		//	return tell(me, "無法查詢"+CITY_D->query_city_idname(city)+"的資料。\n");

		emblem = CITY_D->query_city_info(city, "emblem");
		mayor = CITY_D->query_city_info(city, "government/mayor");
		officer = CITY_D->query_city_info(city, "government/officer");
		
		totalresident = 0;
		totalflourish = CITY_D->query_city_info(city, "totalflourish");

		for(i=0;CITY_D->city_exist(city, i);i++)
			totalresident += CITY_D->query_section_info(city, i, "resident");
		
		city_idname = CITY_D->query_city_idname(city);
		str =  "                                                                "+emblem[0]+"\n";
		str += sprintf("%-24s"NOR WHT"───────────────────  "NOR"%10s\n", city_idname, emblem[1]);
		str +=  "                                                                "+emblem[2]+"\n";


		str += "時代 "+CITY_D->query_age_name(city)+" "HIY+repeat_string("*", CITY_D->query_city_info(city, "level"))+NOR"\n";
		str += "市長 "HIW+(mayor ? capitalize(mayor) : "無")+"\n"NOR;
		str += "官員 \n"NOR;

		if( !sizeof(officer) )
			str += "     無\n";
		else
			foreach( string officerid, string title in officer )
				str += sprintf("     %-12s%s\n", HIW+capitalize(officerid)+NOR, title);
		
		str += "\n總市民數      "HIG+sizeof(CITY_D->query_city_info(city, "citizens"))+NOR"\n";
		str += "總繁榮度      "HIG+NUMBER_D->number_symbol(totalflourish)+NOR"\n";
		str += "總居民數      "HIG+NUMBER_D->number_symbol(totalresident)+NOR"\n\n";

		moneyunit = MONEY_D->city_to_money_unit(city);
		GNP = CITY_D->query_city_info(city, "GNP");
		NNP = CITY_D->query_city_info(city, "NNP");
		
		if( count(GNP, "<=", 0) )
			rate = 0.;
		else
			rate = to_float(count(count(NNP, "*", 10000), "/", GNP))/100.;

		if( CITY_D->query_city_info(city, "fallen") ) rate = 0.;
		str += sprintf("%-38s%-32s\n", "幣制單位      "+(moneyunit ? "$"+moneyunit:"未定"), "政府資產   "+NUMBER_D->number_symbol(CITY_D->query_city_info(city, "assets")));
		str += sprintf("%-38s%-32s\n", "國際匯率      "+EXCHANGE_D->query_exchange_data(moneyunit||"0"), "地產稅     "+CITY_D->query_city_info(city, "estatetax"));
		str += sprintf("%-38s%-32s\n\n", "市民總資產    "+money(MONEY_D->query_default_money_unit(), GNP), "經濟成長率 "+(rate>0?HIG:HIR)+sprintf("%-.2f%%", rate))+NOR;
		
		for(j=0;j<i;j++)
		{
			switch(CITY_D->query_section_info(city, j, "level"))
			{
				case 0: city_scale_mayor = "酋長"; break;
				case 1: city_scale_mayor = "鄉長"; break;
				case 2: city_scale_mayor = "副市長"; break;
				case 3: city_scale_mayor = "副首長"; break;
				case 4: city_scale_mayor = "副首長"; break;
			}

			str += (CITY_D->query_section_info(city, j, "name")||"第 "+(j+1)+" 衛星都市")+"\n";
			str += sprintf("%-14s%-22s%-13s%s\n", city_scale_mayor, 	HIC+(capitalize(CITY_D->query_section_info(city, j, "vicemayor"))||"")+NOR, 			HIY"  金屬資源   "NOR, NUMBER_D->number_symbol(CITY_D->query_section_info(city, j, "resource/metal")));
			str += sprintf("%-14s%-22s%-13s%s\n", "繁榮度", 	NUMBER_D->number_symbol(CITY_D->query_section_info(city, j, "flourish")), 			WHT"  原石資源   "NOR, NUMBER_D->number_symbol(CITY_D->query_section_info(city, j, "resource/stone")));
			str += sprintf("%-14s%-22s%-13s%s\n", "居民數", 	NUMBER_D->number_symbol(CITY_D->query_section_info(city, j, "resident")), 			HIC"  清水資源   "NOR, NUMBER_D->number_symbol(CITY_D->query_section_info(city, j, "resource/water")));
			str += sprintf("%-14s%-22s%-13s%s\n", "規模", 		CITY_D->query_city_scale(city, j), 							YEL"  原木資源   "NOR, NUMBER_D->number_symbol(CITY_D->query_section_info(city, j, "resource/wood")));
			str += sprintf("%-14s%-22s%-13s%s\n\n", "消費能力/月", money(moneyunit, count(24,"*",SHOPPING_D->query_shopping_info(city, j, "totalmoney"))),	HIM"  燃料資源   "NOR, NUMBER_D->number_symbol(CITY_D->query_section_info(city, j, "resource/fuel")));
		}
		str += sprintf(HIW"\n%-14s%-22s%-13s%-14s\n"NOR, "郊區名稱", "軍隊駐守", "資源掠奪", "特殊科技探索" );
		
		foreach(area in AREA_D->query_areas())
		{
			str += sprintf("%-14s%-22s%-13s%-14s\n", AREA_D->query_area_name(area), WHT+NUMBER_D->number_symbol(0)+NOR, HIG+0+"%"NOR, HIR"無任何發現"NOR);
		}
		str +=NOR WHT"\n─────────────────────────────────────\n"NOR;
	}
	me->more(str);
}