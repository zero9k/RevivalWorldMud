/* This program is a part of RW mudlib
 * See /doc/help/copyright for more info
 -----------------------------------------
 * File   : horse_stud.c
 * Author : Clode@RevivalWorld
 * Date   : 2005-09-09
 * Note   : 種馬
 * Update :
 *  o 2000-00-00
 *
 -----------------------------------------
 */

#include <ansi.h>
#include <inherit.h>
#include <nature.h>
#include <map.h>

inherit STANDARD_OBJECT;

void create()
{
        set_idname("horse stud","種馬");
        
        if( this_object()->set_shadow_ob() ) return;
        
        set("long", "用來交配生產的種畜。\n");
        set("unit", "隻");
        set("mass", 5000);
        set("value", 4000);
        set("badsell", 1);

	// 種子資訊
	set("growth",
	([
		//種類
		"type":		PASTURE,

		//名稱
		"idname":	this_object()->query_idname(),
		
		//開始生產時的訊息
		"start_msg":"健壯的種馬群四處跳躍奔跑，充滿活力與朝氣。\n" ,
			
	       	//每單位需要的原料種類與數量
	       	"material":
	       	([
	       		"/obj/materials/water":4,
	       		"/obj/farming/product/herbage":15,
	       	]),
			
	       	//產品的數量設定與產品路徑
	       	"harvest":
	       	([
	       		"/obj/pasture/product/horseflesh":7,
	       	]),

		//生成時的訊息
		"harvest_msg":"$ME歡喜地屠宰並取出新鮮的馬肉。\n",

		//種植此類產品所需要的技能與其等級
		"skill":
		([
			"pasture":50,
		]),

		//成長時的訊息
		"growing":
		([
			5:	"進入了濃情蜜意的交配期，種馬之間開始互相交配。\n",
			50:	"種馬們恢復到過去的生活，但可以注意到部份的母馬行為有些不同。\n",
			150:	"部份的母馬肚子漸漸大了起來，似乎是懷孕了。\n",
			300:	"母馬們分娩產下了許多健康的小馬，只見母馬們細心的圍繞在小馬身旁並舔拭牠們的身體。\n",
			500:	"小馬逐漸的成長，愈長愈壯。\n",
			700:	"小馬們全部健康的長大成為成馬，可以進行收成了。\n",
		]),
	
		// 天氣影響敘述
		"weather":
		([
			SNOWY:	"大雪連連嚴重使得馬群們陸續的凍死。\n",
			SHOWER:	"連日來的雨量使得部份的馬群們受涼病死。\n",
			TYPHOON:"颱風來襲使得部份的馬群們被強風吹來的木頭砸死。\n",
			FOEHN:	"焚風的吹襲讓部份的馬群們過熱中暑而死。\n",
		]),
	]));
}
