/* This program is a part of RW mudlib
 * See /doc/help/copyright for more info
 -----------------------------------------
 * File   : store.c
 * Author : Clode@RevivalWorld
 * Date   : 2002-4-13
 * Note   : 模組 - 一般商店
 * Update :
 *  o 2000-00-00  
 *
 -----------------------------------------
 */

#include <npc.h>
#include <ansi.h>
#include <map.h>
#include <feature.h>
#include <daemon.h>
#include <message.h>
#include <condition.h>

inherit ROOM_ACTION_MOD;

inherit __DIR__"inherit/_object_database_mod.c";
inherit __DIR__"inherit/_action_import.c";
inherit __DIR__"inherit/_action_export.c";
inherit __DIR__"inherit/_action_list.c";
inherit __DIR__"inherit/_action_buy.c";
inherit __DIR__"inherit/_action_sell.c";
inherit __DIR__"inherit/_action_gain.c";
inherit __DIR__"inherit/_action_setup.c";
inherit __DIR__"inherit/_action_transport";
inherit __DIR__"inherit/_heart_beat_store.c";
inherit __DIR__"inherit/_action_auction";

#define	POBJECT		0
#define PSHORT		1
#define PIDNAME		2
#define PAMOUNT		3
#define PPRICE		4

#define STORE_CAPACITY	100000

void do_gowork(object me, string arg)
{
	object env = environment(me);
	object master = env->query_master();
	
	if( !query_heart_beat(master) )
		CHANNEL_D->channel_broadcast("sys", me->query_idname()+"嘗試修復 "+base_name(master)+" 商店問題(原始心跳 "+query_heart_beat(master)+")");
		
	master->startup_heart_beat();

	msg("$ME拿著鞭子抽打員工：「給我用力工作！！」\n", me, 0, 1);
}

void do_confirm_sightsee(object me, object env, string moneyunit, mixed money, int directly, string arg)
{
	int floor = env->query_floor();

	if( !directly && lower_case(arg) != "y" && lower_case(arg) != "yes" )
	{
		tell(me, pnoun(2, me)+"取消付費遊覽的念頭。\n");
		me->finish_input();
		return;
	}

	if( !directly )
	{
		if( count(money, ">", 0) && !me->spend_money(moneyunit, money) )
			return tell(me, pnoun(2, me)+"身上的錢不夠支付遊覽費用。\n");
		else
		{
			msg("$ME支付了 $"+moneyunit+" "+NUMBER_D->number_symbol(money)+" 的遊覽費用。\n", me, 0, 1);
			set("money", copy(count(query("money", env),"+",money)), env);
			env->save();
		}
	}

	msg("$ME從摩天大樓的"HIW"觀"WHT"景台"NOR"往外瀏覽，只見"HIC"雲"NOR CYN"海"NOR"飄逸在四週，偉大的城市納入眼底，深深吸一口氣，感覺視野寬闊了許多。\n", me, 0, 1);

	if( floor == 100 )
       		me->start_condition(SIGHTSEEBLESS);
       	else if( floor == 120 )
       		me->start_condition(SIGHTSEEBLESS_ADV1);	
       	else if( floor == 140 )
       		me->start_condition(SIGHTSEEBLESS_ADV2);	
       	else if( floor == 160 )
       		me->start_condition(SIGHTSEEBLESS_ADV3);
}

void do_sightsee(object me, string arg)
{
	object env = environment(me);
	string city = env->query_city();
	string moneyunit = MONEY_D->city_to_money_unit(city);
	mixed money = query("sightseeing_price", env);
	
	if( count(money, ">", 0) && me->query_id(1) != query("owner", env) )
	{
		tell(me, pnoun(2, me)+"遊覽一次需支付 $"+moneyunit+" "+NUMBER_D->number_symbol(money)+" 的費用。\n是否確定要付費遊覽？(Yes/No)：\n");
		input_to((: do_confirm_sightsee, me, env, moneyunit, money, 0 :));
	}
	else
		do_confirm_sightsee(me, env, moneyunit, money, 1, "y");
}

void do_scenery_setup(object me, string arg)
{
	int money;
	object env = environment(me);
	string moneyunit = MONEY_D->city_to_money_unit(env->query_city());

	if( !arg )
		return tell(me, "目前的遊覽費用為 $"+moneyunit+" "+NUMBER_D->number_symbol(query("sightseeing_price", env))+"。\n");
	
	if( me->query_id(1) != query("owner", env) )
		return tell(me, pnoun(2, me)+"不是此建築物的擁有者。\n");

	money = to_int(arg);
		
	if( !money )
	{
		set("sightseeing_price", 0, env);
		msg("$ME將觀景台的遊覽費用調整為免費。\n", me, 0, 1);
		env->save();
		return;
	}
	
	if( money <= 0 )
		return tell(me, "請輸入正確的數值。\n");

	if( money > 100000000 )
		return tell(me, "最大費用不得超過 $"+moneyunit+" "+NUMBER_D->number_symbol(100000000)+"。\n");

	set("sightseeing_price", money, env);
	
	msg("$ME將觀景台的遊覽費用調整為 $"+moneyunit+" "+NUMBER_D->number_symbol(arg)+"。\n", me, 0, 1);
	env->save();
}

// 設定建築物內房間型態種類
nosave mapping action_info =
([
	"front"	:
	([
		"short"	: HIC"店面"NOR,
		"help"	: 
			([ 
"topics":
@HELP
    店面是販售玩家商品的地方，同時亦可以收購特定商品。
HELP,

"import":
@HELP
將商品上架的指令，用法如下：[管理指令]
  import all		- 將所有身上的商品上架
  import all rice	- 將所有的 rice 商品上架
  import 3 rice		- 將 3 件 rice 商品上架
  import rice		- 將 1 件 rice 商品上架
HELP,

"export":
@HELP
將商品下架的指令，用法如下：[管理指令]
  export all		- 將所有的商品下架
  export all rice	- 將所有 rice 商品下架
  export 3 rice		- 將 3 件 rice 商品下架
  export rice		- 將 1 件 rice 商品下架
  export 3 1		- 將 3 件編號為 1 的商品下架
  export 2		- 將 1 件編號為 2 的商品下架
HELP,

"list":
@HELP
列出所有販賣中的商品的指令，用法如下：
  list			- 列出所有販賣中的商品
HELP,

"buy":
@HELP
購買商品的指令，用法如下：
  buy all rice			- 買下所有的 rice 商品
  buy 3 rice			- 買下 3 件 rice 商品
  buy rice			- 買下 1 件 rice 商品
  buy 3 2			- 買下 3 件編號為 2 的商品
  buy 2				- 買下 1 件編號為 2 的商品
  buy rice to center 1 51,22	- 買下 1 件 rice 商品並送達 center 城市第 1 衛星都市座標(51,22)處的建築物
HELP,

"gain":
@HELP
收取販賣商品所得的獲利，用法如下：[管理指令]
  gain			- 收取所有獲利
HELP,

"setup":
@HELP
設定商店功能的指令，用法如下：[管理指令]
  setup mode				- 切換「管理模式」或「營業模式」
  
  setup sell				- 顯示世界物品列表以及允許販賣商品的數量與價格比
  setup sell all with 50%		- 設定所有物品的收購價格百分比為 50%, 收購數量無限
  setup sell all with 50% for 100	- 設定所有物品的收購價格百分比為 50%, 限定只收購 100 單位
  setup sell all -d			- 取消收購所有物品
  setup sell 2 with 50%			- 設定世界物品列表上編號 2 物品的收購價格為 50%, 收購數量無限
  setup sell 2 with 50% for 100		- 設定世界物品列表上編號 2 物品的收購價格為 50%, 收購 100 單位
  setup sell 2 -d			- 取消收購世界物品列表上編號 2 的商品

  setup price 2 120 			- 設定編號為 2 的商品價格為 120
  setup price 2 150%			- 設定編號為 2 的商品價格為原價的 150%
  
  setup class 2 食品 			- 將編號為 2 的商品分類到食品類的架子上，可自訂分類
  setup class 2 促銷			- '促銷'是商店的特殊分類架，分類到此架子上的商品將可以提升銷售力
  setup class 2 展示			- '展示'是商店的特殊分類架，分類到此架子上的商品將純供展示，無法賣出
HELP,

"sell":
@HELP
販賣商品的指令，用法如下：
  sell			- 顯示目前允許販賣商品的種類與數量
  sell all		- 賣出所有身上可販賣的商品
  sell all rice		- 賣出所有的 rice 商品
  sell 3 rice		- 賣出 3 件 rice 商品
HELP,

"transport":
@HELP
運輸物品的指令，用法如下：
  transport rice to 1 51,22	- 將 1 件 rice 物品運輸至第 1 衛星都市座標(51,22)處的建築物
  transport 3 rice to 2 51,22	- 將 3 件 rice 物品運輸至第 2 衛星都市座標(51,22)處的建築物
  transport 2 3 to 1 51,22	- 將 2 件編號為 3 的物品運輸至 第 1 衛星都市座標(51,22)處的建築物
HELP,

"gowork":
@HELP
當商店長時間不販賣物品時，可以使用此指令嘗試修復問題
  gowork		- 修復商店莫名其妙停止販賣物品的問題
HELP,

"auction":
@HELP
拍賣物品的指令，用法如下：

auction '物品編號' '數量' $'底價' $'直接購買價' '拍賣時間'

例如:
	
auction 1 500 $20000 $1000000 5		- 拍賣編號為 1 的物品 500 個，底標價為 $20000，
					  直接購買價為 $1000000，拍賣時間為 5 小時

幣值單位一律為 $RW

HELP,

			]),
		"job"	: CLERK,
		"heartbeat":5,	// 實際時間 1 秒為單位
		"master":1,
		"action":
			([
				"import"	: (: do_import($1, $2, "products", STORE_CAPACITY) :),
				"export"	: (: do_export($1, $2, "products") :),
				"list"		: (: do_list($1, $2, "products", STORE_CAPACITY) :),
				"buy"		: (: do_buy($1, $2, "products", "ENVIRONMENT") :),
				"sell"		: (: do_sell($1, $2, "products", STORE_CAPACITY) :),
				"gain"		: (: do_gain($1, $2) :),
				"setup"		: (: do_setup($1, $2) :),
				"transport"	: (: do_transport($1, $2, "products") :),
				"auction"	: (: do_auction($1, $2, "products") :),
				"gowork"	: (: do_gowork($1, $2) :),
			]),
	]),

	"scenery":
	([
		"short"	: HIW"觀"WHT"景台"NOR,
		"help"	: 
			([ 
"topics":
@HELP
    這裡是高樓上的觀景台，有壯闊美麗的風景可以遊覽。
HELP,

"sightsee":
@HELP
遊覽美景指令
  sightsee		- 遊覽美景
HELP,

"setup":
@HELP
費用設定指令
  setup			- 查看目前費用
  setup 1000		- 設定遊覽費用為 1000 元
HELP,

"gain":
@HELP
收取獲利指令
  gain			- 收取所有獲利
HELP,

			]),
		"action":
			([
				"sightsee"	: (: do_sightsee :),
				"setup"		: (: do_scenery_setup :),
				"gain"		: (: do_gain :),
			]),
	]),
]);


string look_room(object room)
{
	string msg = "";
	object master = room->query_master();

	if( query("function", room) == "front" )
	{
		msg += "本地買氣 "HIC+query("shopping/desire", room)+NOR;
		msg += " 平均買氣 "HIC+query("shopping/average_desire", master)+NOR;
		msg += " 銷售總數 "HIY+NUMBER_D->number_symbol(query("totalsell", master))+NOR;
		msg += " 巡邏保全 "HIG+query("shopping/security", master)+NOR;
		msg += " 商店管理 "HIY+(sizeof(master->query_slave())+1)+"/"+query("shopping/management", master)+NOR"\n";
	}
	return msg;
}

// 設定建築物資料
nosave array building_info = ({
	
	// 建築物之中文名稱
	HIC"商"NOR CYN"店"NOR

	// 開張此建築物之最少房間限制
	,1

	// 城市中最大相同建築數量限制(0 代表不限制)
	,0

	// 建築物建築種類, GOVERNMENT(政府), ENTERPRISE(企業集團), INDIVIDUAL(私人)
	,INDIVIDUAL

	// 建築物可建築區域, AGRICULTURE_REGION(農業), INDUSTRY_REGION(工業), COMMERCE_REGION(商業)
	,AGRICULTURE_REGION | INDUSTRY_REGION | COMMERCE_REGION

	// 開張儀式費用
	,"200000"
	
	// 建築物關閉測試標記
	,0

	// 繁榮貢獻度
	,5
	
	// 最高可加蓋樓層
	,300
	
	// 最大相同建築物數量(0 代表不限制)
	,10
	
	// 建築物時代
	,1
});
