/* This program is a part of RW mudlib
 * See /doc/help/copyright for more info
 -----------------------------------------
 * File   : secure_d.c
 * Author : Clode@RevivalWorld
 * Date   : 2001-2-7
 * Note   : 讀寫權限與使用者等級精靈
 * Update :
 *  o 2001-02-08 Clode 用 trigger_object 可能會有一些意外狀況, 不過
 *  o 2002-09-11 Clode 重新整理 Secure_d
 *
 -----------------------------------------
 */

#include <daemon.h>
#include <secure.h>

#define DATA    "/system/kernel/data/secure"+__SAVE_EXTENSION__
#define LOG     "daemon/secure"

/* 巫師名單 */
private mapping wizards = 
([
    // 設定預設巫師, 設定完後刪除 DATA 檔, 重新啟動後便可更換新的巫師列表
    "npc"   :   "wizard",
    "mud"   :   "advisor",
    "ivy"   :   "admin",
]);

// 使用者等級數字與名稱對照
private nosave mapping wizlevel_contrast =
([
    PLAYER  :   "player",
    GUEST   :   "guest",
    WIZARD  :   "wizard",
    ADVISOR :   "advisor",
    ADMIN   :   "admin",
]);


/* 表單格式 : ({ WIZ_LEVEL, "ID 1", "ID 2", ... }).... */

/* 允許讀取目錄等級, 未受此表單限制者可讀取所有檔案目錄 */
private nosave mapping ALLOW_READ =
([
    "log"       :   ({ ADMIN | ADVISOR }),
    "system/kernel" :       ({ ADMIN | ADVISOR | WIZARD }),
    "doc/driver"    :       ({ ADMIN | ADVISOR | WIZARD }),
]);

/* 允許寫入目錄等級, 未受此表單限制者可寫入所有檔案目錄 */
private nosave mapping ALLOW_WRITE =
([
    "cmds"      :   ({ ADMIN | ADVISOR }),
    "doc"       :   ({ ADMIN | ADVISOR }),
    "include"       :   ({ ADMIN | ADVISOR }),
    "obj"       :   ({ ADMIN | ADVISOR }),
    "std"       :   ({ ADMIN | ADVISOR }),
    "system"        :   ({ ADMIN | ADVISOR }),
    "data/user"     :   ({ ADMIN | ADVISOR }),
]);

/* 允許編譯目錄等級, 未受此表單限制者可編譯載入所有檔案目錄 */
private nosave mapping ALLOW_COMPILE = 
([
    "system"        :   ({ ADMIN | ADVISOR }),
    "system/kernel" :   ({ ADMIN | ADVISOR }),
    "system/daemons"    :   ({ ADMIN | ADVISOR }),
]);

nomask int is_wizard(string id)
{
    return !undefinedp(wizards[id]);
}

/* 儲存檔案及備份 */
private nomask int save_data()
{
    return save_object(DATA) && cp(DATA, DATA+"_bak");
}

/* 回傳 level_num 相對應之 level_name */
nomask string level_num_to_level_name(int num)
{
    return wizlevel_contrast[num] || wizlevel_contrast[PLAYER];
}

/* 回傳 level_name 相對應之 level_num */
nomask int level_name_to_level_num(string name)
{
    foreach(int level_num, string level_name in wizlevel_contrast)
    if( name == level_name ) 
        return level_num;

    return 0;
}

/* 回傳使用者的巫師等級名稱 */
nomask string level_name(mixed userid)
{
    if( objectp(userid) )
        userid = userid->query_id(1);
    
    if( !stringp(userid) )
        return 0;

    return wizards[userid] || wizlevel_contrast[PLAYER];
}

/* 回傳使用者的巫師等級數字 */
nomask int level_num(mixed userid)
{
    string name;
    
    if( objectp(userid) )
        userid = userid->query_id(1);
    
    if( !stringp(userid) )
        return 0;
    
    name = level_name(userid);

    foreach(int level_num, string level_name in wizlevel_contrast)
    if( name == level_name ) 
        return level_num;
}

nomask varargs int permission(string file, string id, int option, string func)
{

    int i = strlen(file);
    array value;
    string *apart = explode(file, "/");
    mapping limit;

    switch(option)
    {
    case READ:  limit = ALLOW_READ;     break;
    case WRITE: limit = ALLOW_WRITE;    break;
    case COMPILE:   limit = ALLOW_COMPILE;  break;
    default:    return 0;
    }

    /* 解析目錄權限 */
    while(i--)
        if( !undefinedp(value = limit[implode(apart[0..i], "/")]) )
            return level_num(id) & value[0] || member_array(id, value[1..]) != -1;

        /* 完全沒找到符合者 return 1 */
    return 1;
}

nomask varargs int valid_check(string file, object ob, string func, int option)
{
    object tri_ob;
    string my_id;

    //printf("\nvalidcheck:\tfile:%s\t func:%s\n",file,func);
    //printf("origin: %s\n statck: %O\n",origin(),call_stack(1)[<1])

    if( !objectp(ob) || ob == this_object() || !intp(option) ) return 1;

    if( !file || !file[0] ) 
        file = "/";
    else if( file[0] != '/' ) 
        file = "/" + file;

    /* 修改 Driver 的 this_player(), 使 this_player(1) 可傳回 call_out 之 this_player() by Clode */
    tri_ob = this_player(1) || call_stack(1)[<1];

    if( func == "save_object")
    {
        // 只有 secure_d 可以改 secure.o
        if( ob != this_object() && file == DATA )
            return 0;
    }

    /* 若是觸動物件為互動物件 */
    if( objectp(tri_ob) && interactive(tri_ob) )
    {
        string your_id;

        /* 玩家一律 return 1 */
        if( !wizardp(tri_ob) ) return 1;

        /* 查詢不到 id 者 return 0 */
        if( !stringp(my_id = tri_ob->query_id(1)) ) return 0;

        /* 不能讀比自己等級高的巫師的檔案 */
        if( sscanf(file, "/wiz/home/%s/%*s", your_id) == 2 || sscanf(file, "/wiz/home/%s", your_id) == 1 )
            return level_num(my_id) >= ADVISOR || level_num(your_id) <= level_num(my_id);

        if( option == WRITE && (sscanf(file, "/data/user/%*s/%s/%*s", your_id) == 3 || sscanf(file, "/data/user/%*s/%s", your_id) == 2) )
                        return level_num(my_id) >= level_num(your_id) || your_id == my_id || !is_wizard(your_id);

        return permission(file, my_id, option, func);
    }

    /* 會由 /wiz/home 中檔案觸發的情形大概只有在 call_out 之類的情形吧? */
    if( tri_ob && sscanf(file_name(tri_ob), "/wiz/home/%s/%*s", my_id) )
        return permission(file, my_id, option, func);

    /* 若不是由 /wiz/home 中的檔案所觸發的 valid_read
       則一概 return 1, 也就是說對於其他目錄下的檔案存取
       必須交給值得信任的人, 這裡以後還可以補些東西.. */
    return 1;

}

nomask mapping query_wizards()
{
    return copy(wizards);
}

nomask int change_wizard_level(string id, string level_name)
{
    object this_player = this_player(1);

    /* 檢查是否為指令執行 && 避免 this_object 被繼承 */
    if( !level_name 
        || !id
        || !objectp(this_player)
        || level_num(this_player->query_id(1)) < level_name_to_level_num(level_name)
        || !is_command(previous_object()) 
        || this_object() != load_object(SECURE_D) 
        || !level_name_to_level_num(level_name) ) 
        return 0;

    /* 如果 level 為 player */
    if( level_name == "player" )
        map_delete(wizards, id);
    else 
        wizards[id] = level_name;

    log_file(LOG, sprintf("%s 修改 %s 權限為 %s。\n", this_player?this_player->query_idname():"", id, level_name));

    return save_data();
}

private void create()
{
    if( clonep() ) return;

    if( !restore_object(DATA, 1) )
    {
        save_data();
        log_file(LOG, "無法載入存檔。\n");
    }
}

string query_name()
{
    return "安全系統(SECURE_D)";
}
