/* This program is a part of RW mudlib
 * See /doc/help/copyright for more info
 -----------------------------------------
 * File   : zone_d.c
 * Author : Cookys@RevivalWorld
 * Date   : 2002-09-17
 * Note   : 
 * Update :
 *  o 2002-09-29 cookys 重新改寫以符合新的 map_d.c
 *
 -----------------------------------------
 */
 
/*           (place?)
** zone ── NUM ─ zonedbase
**           ├─
**           ├─
**           ├─ NUM ─┬─ Z ─ ZONE_ROOM
**                      ├─ Z
**                      └─ Z ─┬─ Y ─┬ X
**                               ├─     └
**
**
** zone[MAP]
** zone[RMAP][place][num][z][y][x]
**
** mapping database = allocate_mapping( ({ save_variable(({x,y,z})) }), ({ "特殊出口" }) );
== mapping database = ([ save_variable(({x,y,z})) : "特殊出口" ]);
*/

/*
 *          要利用 MAP_D 地圖系統之物件, 必須定義以下函式
 *          o string show_map(array loc, int cover_range)      // 顯示地圖
 *          o object query_maproom(array loc)                  // 回傳地圖物件
 *          o object *query_all_maproom()                      // 回傳所有地圖物件
 *          o int move(object ob, string dir)                  // 使用者移動
 *          o varargs array query_map(array loc, int realtime) // 回傳地圖陣列
 *          o void remove_player_icon(object me, array loc)    // 若想有人形圖像請設計此函式
 *          o void set_player_icon(object me, array loc)       // 若想有人形圖像請設計此函式
 *          並且必須在 map_system_table 中設定座標隸屬判斷函式
*/
#include <daemon.h>
#include <location.h>
#include <ansi.h>
#include <gender.h>
#include <message.h>
#include <nature.h>
#include <nature_data.h>

#define MAP_WIDTH       100     //城市寬
#define MAP_LENGTH      100     //城市長

/* 可視區範圍, 長與寬皆必須為奇數, 人才能剛好在地圖正中央 */
/* 新增遮避範圍設定, 現在的 25:9 比例剛好, 若無需要應避免更改, 否則對於遮避計算會有問題 */
#define VISION_WIDTH    25      //可視地圖大小-寬
#define VISION_LENGTH   9       //可視地圖大小-長


#define ZONE_GENERATOR_PATH "/zone/generator/"

#define MAP             "zonemap"
#define RMAP            "realtime_zonemap"
#define DATA            "data"

// ZONE_D 不用每一間都存 data
// 同一層(以 y 為單位)放一間
#define ZONE_ROOM       "zone_room"
#define SPECIAL_EXIT	"special_exit"

private mixed zone=([
                        RMAP:([]),
                        MAP:([]),
                        DATA:([]),
                   ]);
private mapping module_table=([]);                   



int is_zone_exist(string place,int num,int z,int y,int x)
{
	//return 1;
	if( !undefinedp(zone[MAP][place]) )
        {
        	if( num < sizeof(zone[MAP][place]) && !undefinedp(zone[MAP][place][num]) )
        	{
        		if( z < sizeof(zone[MAP][place][num]) && !undefinedp(zone[MAP][place][num][z]))
        		{
        			if(y<sizeof(zone[MAP][place][num][z]) && !undefinedp(zone[MAP][place][num][z][y]))
        			{
        				if( x< sizeof(zone[MAP][place][num][z][y]) && !undefinedp(zone[MAP][place][num][z][y][x]))
        					return 1;
        			}
        				
        		}
        	}
        	
        	//if( !undefinedp(zone[MAP][zone]) )
        	//	return 1;
        }
        
        return 0;
}

int is_zone_location(array loc)
{
	return is_zone_exist(loc[PLACE],loc[NUM],loc[Z],loc[Y],loc[X]);
}

int valid_coordinate(string place,int num,int z,int y,int x)
{
        return ( x >=0 && y>=0 && is_zone_exist(place,num,z,y,x)  );
}

// 當該格上面有 special_exit 的時候，就會直接傳送到 link_target
mixed query_special_exit(array loc)
{
	mapping a;
	if( !undefinedp( zone[DATA][save_variable(loc)] ) && sizeof(zone[DATA][save_variable(loc)][SPECIAL_EXIT]) )
		return zone[DATA][save_variable(loc)][SPECIAL_EXIT];
	else return a;
}

int set_special_exit(array loc,mixed target)
{
	if( undefinedp( zone[DATA][save_variable(loc)] ))
		zone[DATA][save_variable(loc)]=([]);

	//if( undefinedp(zone[DATA][save_variable(loc)][SPECIAL_EXIT]) )
	zone[DATA][save_variable(loc)][SPECIAL_EXIT]=target;
		
	//if( !undefinedp(zone[DATA][save_variable(loc)][SPECIAL_EXIT] ))
	//	return 0;
		
	//zone[DATA][save_variable(loc)][SPECIAL_EXIT]=target;
	
	return 1;
}

int del_special_exit(array loc)
{
	if( undefinedp( zone[DATA][save_variable(loc)]) || undefinedp(zone[DATA][save_variable(loc)][SPECIAL_EXIT]))
		return 0;
	
	//if(undefinedp(zone[DATA][save_variable(loc)][SPECIAL_EXIT][idx]))
	//	return 0;
		
	map_delete(zone[DATA][save_variable(loc)],SPECIAL_EXIT);
	return 1;
}

object find_module(string place)
{
	if( undefinedp(place) )
		return 0;
		
	if(!module_table[place] )
		module_table[place]=load_object(ZONE_GENERATOR_PATH+place+".c");
		
	return module_table[place];
}

mapping chinese =
([
    "north"             :"北",
    "south"             :"南",
    "east"              :"東",
    "west"              :"西",
    "northwest"         :"西北",
    "northeast"         :"東北",
    "southwest"         :"西南",
    "southeast"         :"東南",
    "down"              :"下",
    "up"                :"上",
]);


int valid_move(string place,int num,int z,int y,int x)
{
	object module;
	//mixed data;
	
	if( (module=find_module(place) )->special_move() )
	{
		//if( !undefinedp(data=zone[DATA][save_variable(place,num,z,y,x)]) )
		//	module->move(data);
		return module->move(zone[MAP][place][num][z][y][x]);
	}
	
	// not design yet
	return 1;	
}


//   1  2   4
//   8      16
//   32 64  128
int generate_move_dir(string place,int num,int z,int y,int x)
{
	int flag=0;
	
	if( valid_coordinate(place,num,z,y,x+1) && valid_move(place,num,z,y,x+1) )
		flag |= 16;
	if( valid_coordinate(place,num,z,y,x-1) && valid_move(place,num,z,y,x-1) )
		flag |= 8;
	if( valid_coordinate(place,num,z,y+1,x) && valid_move(place,num,z,y+1,x) )
		flag |= 64;
	if( valid_coordinate(place,num,z,y-1,x) && valid_move(place,num,z,y-1,x) )
		flag |= 2;
		
	if( valid_coordinate(place,num,z,y-1,x+1) && valid_move(place,num,z,y-1,x+1) )
		flag |= 4;	
	if( valid_coordinate(place,num,z,y+1,x+1) && valid_move(place,num,z,y+1,x+1) )
		flag |= 128;
	if( valid_coordinate(place,num,z,y-1,x-1) && valid_move(place,num,z,y-1,x-1) )
		flag |= 1;
	if( valid_coordinate(place,num,z,y+1,x-1) && valid_move(place,num,z,y+1,x-1) )
		flag |= 32;
	
	return flag;
}

mixed query_data()
{
	return zone[DATA];	
}
varargs int move(object me, string direction)
{
        //object *ppls;
        int x, y,z, n, ox, oy;
        //mapping data;
        string place;
        array loc, next_loc;

        if( !objectp(me) || !arrayp(loc = query_temp("location",me)) ) return 0;

        ox = x = loc[X];
        oy = y = loc[Y];
        z       =loc[Z];
        place = loc[CITY];
        n = loc[NUM];

        switch(direction)
        {
                case "north"    :       y--;            break;
                case "south"    :       y++;            break;
                case "west"     :       x--;            break;
                case "east"     :       x++;            break;
                case "northeast":       y--;x++;        break;
                case "southwest":       y++;x--;        break;
                case "northwest":       y--;x--;        break;
                case "southeast":       y++;x++;        break;
                default: return tell(me, "這個方向沒有出路。\n", CMDMSG);
        }

        if( !valid_coordinate(place,n,z,y,x) )
        	return tell(me, "這個方向沒有出路。\n", CMDMSG);
        	
	if( !valid_move(place,n,z,y,x)) 
		return tell(me, "這個方向沒有出路。\n", CMDMSG);
		
	
		
	next_loc = arrange_location(x,y,z,place,n,0,x+"/"+y+"/"+z+"/"+place+"/"+n+"/0");
	
	if( !undefinedp(query_special_exit(next_loc)) )
	{
		tell(me, "yes\n", CMDMSG);
		me->move(query_special_exit(next_loc));
		return 1;
	}
// icon change 改由 ppl 身上的 move 呼叫 map_d 上面的 icon 控制	
/*		
        next_loc = arrange_location(x,y,z,place,n,x+"/"+y+"/"+z+"/"+place+"/"+n+"/0");

	
		
        ppls = filter_array(present_objects(me)-({me}), (: userp($1) :));

        broadcast(loc, me->query_idname()+"往"+chinese[direction]+"離開。\n", 0, me);
      


        if( !sizeof( ppls ) )
                zone[RMAP][place][n][z][oy][ox] = copy(zone[MAP][place][n][z][oy][ox]);
        else if( sizeof(ppls) > 1 )
                zone[RMAP][place][n][z][oy][ox] = ansi_part(zone[MAP][place][n][z][oy][ox]) + HIC"※"NOR;
        else
                zone[RMAP][place][n][z][oy][ox] = ansi_part(zone[MAP][place][n][z][oy][ox]) + (query("gender", ppls[0]) == MALE_GENDER ? HIG + "♂" : HIR + "♀") + NOR;

        me->move(next_loc);

        if( sizeof(filter_array(present_objects(me), (: userp($1) :))) > 1 )
                zone[RMAP][place][n][z][y][x] = ansi_part(zone[MAP][place][n][z][y][x]) + HIC"※"NOR;
        else
                zone[RMAP][place][n][z][y][x] = ansi_part(zone[MAP][place][n][z][y][x]) + (query("gender", me) == MALE_GENDER ? HIG + "♂" : HIR + "♀") + NOR;
*/
	me->move(next_loc);
        return 1;
}

varargs string show_map(array loc, int cover_range)
{
        int i, j=1, dir, x, y,z, number, range,
        x_start, y_start,
        x_center, y_center,
        x_cover=1, y_cover=1;
        string return_map,place;
        mapping column = ([]);


        if( !loc ) return 0;

        x       =loc[X];
        y       =loc[Y];
        z       =loc[Z];
        place    =loc[CITY];
        number  =loc[NUM];
        
        x_center=(VISION_WIDTH+1)/2;
        y_center=(VISION_LENGTH+1)/2;

        if( cover_range > VISION_LENGTH/2 ) cover_range = VISION_LENGTH/2;
        x_center -= cover_range*3;
        y_center -= cover_range;

        // 顯示區塊計算公式
        x_start = (x>=x_center)*(x+1-x_center) - ((x+x_center)>MAP_WIDTH)*(x_center+x-MAP_WIDTH);
        y_start = (y>=y_center)*(y+1-y_center) - ((y+y_center)>MAP_LENGTH)*(y_center+y-MAP_LENGTH);

        range = y_start-cover_range;

        // 圖上面的橫線
        return_map =NOR WHT"\e[s\e["+j+++";1H   ┌"+sprintf(HIG" %|3d "NOR+WHT"－"+HIG" %|3d "NOR+WHT+"─"HIG" %|2d "NOR WHT,x+1,y+1,number+1)+"────────── "+HIC+TIME_D->gametime_digital_clock()+NOR+WHT" ┬─────┐";

        //dir = CITY_D->direction_check(x, y, place, number);
        //emblem = CITY_D->query_info(place, "emblem");
        
        dir = generate_move_dir(place,number,z,y,x);

        column[range+1] =       "│"+ WHT"          │";
        column[range+2] =       "│"+ WHT"          │";
        column[range+3] =       "│"+ WHT"          │";
        column[range+4] =       "│"HIW"人口："HIG+sprintf("幹！")+WHT"│";
        column[range+5] =	"│"HIW"天氣："+(NATURE_D->query_weather(MAP_D->query_maproom(loc)))[NATURE_WEATHER][WEATHER_CHINESE]+WHT+"│";
        column[range+6] =       "├─────┤";
        column[range+7] =       "│  "+(dir&1?HIW"↖":NOR WHT"╲")      +(dir&2?HIW"↑":NOR WHT"│")    +(dir&4?HIW"↗":NOR WHT"╱")    +NOR WHT"  │";
        column[range+8] =       "│  "+(dir&8?HIW"←":NOR WHT"─")      +HIW+"○"+NOR WHT               +(dir&16?HIW"→":NOR WHT"─")   +NOR WHT"  │";
        column[range+9] =       "│  "+(dir&32?HIW"↙":NOR WHT"╱")     +(dir&64?HIW"↓":NOR WHT"│")   +(dir&128?HIW"↘":NOR WHT"╲")  +NOR WHT"  │";

        x_cover += cover_range*6;
        y_cover += cover_range*2;

        for(i=0;i<cover_range;i++)
                return_map += "\e["+j+++";1H   │"NOR+"                                                  "+WHT+column[++range];

//shout(sprintf("%O\n",zone[RMAP][place][number]));
        // 以 Y 軸構置出圖形
        foreach( string *a in zone[RMAP][place][number][z][y_start..y_start+VISION_LENGTH-y_cover] )
        {

                if( range++ == y )
                        return_map += "\e["+j+++";1H   │"NOR+repeat_string("  ",cover_range*3)+implode(a[x_start..x-1]+({ansi_part(a[x])+HIC"╳"NOR})+a[x+1..x_start+VISION_WIDTH-x_cover],"")+NOR+repeat_string("　",cover_range*3)+WHT+column[range];
                else
                        return_map += "\e["+j+++";1H   │"NOR+repeat_string("  ",cover_range*3)+implode(a[x_start..x_start+VISION_WIDTH-x_cover],"")+NOR+repeat_string("　",cover_range*3)+WHT+column[range];
        }

        for(i=0;i<cover_range;i++)
                return_map += "\e["+j+++";1H   │"NOR+"                                                  "+WHT+column[++range];

        // 圖下的橫線
        return_map += "\e["+j+++";1H   └[ ";

        return_map += repeat_string(" ", 20);

        return_map+= NOR WHT" ]─────────────┴─────┘\e["+j+";1H"NOR MAG"________________________________________________________________\e[4m地圖系統_\e[u"NOR;

        return return_map;
}



object query_maproom(array loc)
{
        string place;
        int num,z;
        
        place=loc[PLACE];
        num=loc[NUM];

        if( !is_zone_location(loc) ) return 0;

       
	//shout(sprintf("%O\n",zone[DATA][save_variable( ({place,num,z}) )][ZONE_ROOM]));
        return zone[DATA][save_variable( ({place,num,z}) )][ZONE_ROOM];
        
        //return load_object("/wiz/room/room_hall1");
}

object *query_all_maproom() 
{
	object* rooms=({});
	
	foreach(string t,mapping data in zone[DATA])
	{
		foreach(string id,object zoneroom in data)
			if( !undefinedp(zoneroom))
				rooms+=({zoneroom});
	}
	
	return rooms;
	//return children(ZONE_ROOM);
}

void set_player_icon(object me, array loc)
{
	
}

void remove_player_icon(object me, array loc)
{
	
}

void handle_zone()
{
        object module;

        foreach( string filename in get_dir(ZONE_GENERATOR_PATH) )
        {
                // 把 zone generator 產生的地圖弄進 zone_d,理論上會支援 xyz 三軸
                //printf("peer:%O\n",module->create_extra_info());
                //int
		int num_count = 0;
                if( !catch(module = load_object(ZONE_GENERATOR_PATH+filename)) )
                {
                        //shout(sprintf("%O\n",module->create_extra_info()));
                        zone[RMAP][filename[0..<3]]=fetch_variable("map",module);
                        //zone[RMAP][filename[0..<3]]=module->create_zone_map();
                        zone[MAP][filename[0..<3]]=copy(zone[RMAP][filename[0..<3]]);
                } else continue;
                // 製造每一層的 map_room (以 'y' 為單位)
                foreach(mixed num in zone[RMAP][filename[0..<3]])
                {
                        int z_count=0;
                        foreach(mixed z in num)
                        {
                                
                                //if( !undefinedp(z) )
                                //{
                                // 識別名稱 [zone][place][num][x][y][x]
                                zone[DATA][save_variable( ({filename[0..<3],num_count,z_count}) )]=([]);
                                zone[DATA][save_variable( ({filename[0..<3],num_count,z_count}) )][ZONE_ROOM]=load_zoneroom(filename[0..<3],num_count,z_count);
                                //}
                                z_count++;
                        }
                        num_count++;
                }
                module_table[filename[0..<3]]=module;
        }
}



void create()
{
        if( clonep() )
                destruct(this_object());

        handle_zone();
}

string query_name()
{
	return "特殊區域系統(ZONE_D)";
}
