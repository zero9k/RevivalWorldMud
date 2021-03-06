/* This program is a part of RW mudlib
 * See /doc/help/copyright for more info
 -----------------------------------------
 * File   : resource.c
 * Author : Clode@RevivalWorld
 * Date   : 2005-04-06
 * Note   : 資源集中場
 * Update :
 *  o 2000-00-00  
 *
 -----------------------------------------
 */

#include <ansi.h>
#include <map.h>
#include <feature.h>
#include <message.h>
#include <daemon.h>
#include <npc.h>

inherit ROOM_ACTION_MOD;

inherit __DIR__"inherit/_mod_factory.c";

// 設定建築物內房間型態種類
nosave mapping action_info =
([
	"line"	:
	([
		"short"	: NOR WHT"生產線"NOR,
		"help"	:
			([
"topics":
@HELP
    生產線提供產品的製造與科技研發。
HELP,

"import":
@HELP
輸入原料的指令，用法如下：[管理指令]
  import			- 盤點目前生產線內的原料數量
  import all			- 將身上所有原料類物品輸入生產線
  import all stone		- 輸入所有的 stone 原料
  import 3 stone		- 輸入 3 件 stone 原料
  import stone			- 輸入 1 件 stone 原料
HELP,

"export":
@HELP
輸出原料的指令，用法如下：[管理指令]
  export all			- 將所有生產線上的原料輸出
  export all stone		- 輸出所有的 stone 原料
  export 3 stone		- 輸出 3 件 stone 原料
  export stone			- 輸出 1 件 stone 原料
HELP,

"plist":
@HELP
列出此生產線可以生產的產品種類，用法如下：
  plist				- 列出此生產線可以生產的產品種類
HELP,

"design":
@HELP
設定全新產品的指令，用法如下：[管理指令]
  design			- 顯示目前的產品設計資料
  design type chair		- 設計新產品的種類為 chair 種類
  design id big chair		- 設計新產品的英文名稱叫做 'big chair'
  design name 大椅子		- 設計新產品的中文名稱叫做 '大椅子'
  design sample			- 嘗試生產樣品，倘若樣品生產成功，即可立刻進行產品的量產
HELP,

"line":
@HELP
生產線控制指令，用法如下：[管理指令]
  line				- 顯示目前生產線的生產情形
  line start			- 啟動生產線的運作
  line stop			- 停止生產線的運作
  line cancel			- 永遠刪除並停止生產正在生產的產品
HELP,

"setup":
@HELP
設定生產線資料的指令，用法如下：[管理指令]
  setup reserach 21,33 		- 設定此條生產線的對應研發中心房間座標在 21,33
HELP,
			]),
		"heartbeat":0,	// 實際時間 1 秒為單位
		"job": WORKER,
		"action":
			([
				//"import"	: (: ::do_import_action($1, $2) :),
				//"plist"		: (: ::do_plist_action($1, $2) :),
				//"design"	: (: ::do_design_action($1, $2) :),
				//"line"		: (: ::do_line_action($1, $2) :),
				//"setup"		: (: ::do_setup_action($1, $2) :),
			]),
	]),
	
	"research":
	([
		"short"	: NOR HIW"研發中心"NOR,
		"help"	:
			([
"topics":
@HELP
   研發中心負責進行科技的研發，但需要大量工程師。
HELP,

"info":
@HELP
查看研發中心的研發成果，用法如下：
  info			- 顯示研發中心的研發成果
HELP,

"research":
@HELP
研發某種類型產品，用法如下：
  research		- 針對某類型的產品進行研發
HELP,
			]),
		"heartbeat":0,	// 實際時間 1 秒為單位
		"job"	: ENGINEER,
		"action":
			([
				//"research"	: (: ::do_research_action($1, $2) :),
				//"plist"		: (: ::do_plist_action($1, $2) :),
				//"info"		: (: ::do_research_info_action($1, $2) :),
			]),
	]),
]);

// 設定建築物資料
nosave array building_info = ({
	
	// 建築物之中文名稱
	HIY"資源"NOR YEL"採集場"NOR

	// 開張此建築物之最少房間限制
	,1

	// 城市中最大相同建築數量限制(0 代表不限制)
	,0

	// 建築物建築種類, GOVERNMENT(政府), ENTERPRISE(企業集團), INDIVIDUAL(私人)
	,INDIVIDUAL

	// 建築物可建築區域, AGRICULTURE_REGION(農業), INDUSTRY_REGION(工業), COMMERCE_REGION(商業)
	,INDUSTRY_REGION

	// 開張儀式費用
	,"5000000"

	// 建築物關閉測試標記
	,1

	// 繁榮貢獻度
	,5
	
	// 最高可加蓋樓層
	,1
	
	// 最大相同建築物數量(0 代表不限制)
	,1
	
	// 建築物時代
	,2
});
