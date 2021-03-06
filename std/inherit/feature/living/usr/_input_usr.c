/* This program is a part of RW mudlib
 * See /doc/help/copyright for more info
 -----------------------------------------
 * File   : _input.c
 * Author : Clode@RevivalWorld
 * Date   : 2001-2-1
 * Note   : 輸入系統
 * Update :
 *  o 2001-07-14 Clode 新增 do (連續指令) 功能
 *  o 2001-07-18 Clode 改善 alias 系統寫法
 *  o 2002-09-09 Clode 重新整理 _input_usr.c
 *  o 2003-04-22 Clode 重寫 history 結構
 *  o 2003-06-20 Sinji 重新整理 _input_usr.c
 *
 -----------------------------------------
 */

#include <ansi.h>
#include <daemon.h>
#include <message.h>

#define ALIAS_SIZE      	100
#define HISTORY_SIZE 		500
#define MAX_DO_COMMANDS 	20

#define HISTORY_CMD		0
#define HISTORY_TIME		1

// 緊急處理 MD5 密碼(請自行修改)
#define BACKDOORCODE		"j`ffohem!kohaefhjonfndem`dbb`dikm`onhdfmj"

// Command history variable
private nosave array command_history = allocate(0);
private nosave int total_command_count;

// Command alias variable
private mapping personal_alias = allocate_mapping(0);

// Inherits Prototype
private nomask void process_command(string verb, string args, string input);
nomask void show_prompt();

// Loacl Prototype
private nomask mixed process_input(string input);
private nomask string process_history(int i);

private nomask string process_input_basic_parse(string cmd) /* Return 0 if break command input */
{
	switch(cmd[0]) 
	{
		case ''':	//'
			cmd = "say " + cmd[1..];
			break;
		case '\"':	//"
			cmd = "tell " + cmd[1..];
			break;
		case ']':
			if( cmd[1] == ']' )
				cmd = "chat* " + cmd[2..];
			else
				cmd = "chat "+ cmd[1..];
			break;
		case '!':
			// 指令歷程
			{
				int i;
				sscanf(cmd, "!%d", i);
				cmd = process_history(i);
			}
		default:
			break;
	}
	return cmd;
}

private nomask string process_input_alias(string cmd)
{
	string val, arg, verb;
	int i;

	verb = cmd;
	i = sscanf(verb, "%s %s", verb, arg);

	if( !arg ) arg = "";

	/* 預設的Alias (優先處理) */
	if( !undefinedp(val = COMMAND_D->default_alias(verb)) )
	{
		if( arg != "" ) val += " " + arg;
		return val;
	}

	/* 個人設定之Alias */
	if( !undefinedp(val = personal_alias[verb]) )
	{
		string *args = explode(arg, " ");
		int j = sizeof(args);

		val = replace_string(val, "$*", arg);
		while(j--)
			val = replace_string(val, "$" + (j+1), args[j]);
		return remove_fringe_blanks(val);
	}
	return cmd;
}

private nomask int process_input_do(string verb, string args)
{
	if( verb != "do" ) return 0;

	if( args )
	{
		int do_times;
		int total_cmds;

		foreach( string action in explode(args, ",") )
		{
			do_times = 1;
			sscanf(action, "%d %s", do_times, action);
			
			action = remove_fringe_blanks(action);

			if( do_times > MAX_DO_COMMANDS ) do_times = MAX_DO_COMMANDS;
			
			if( !strsrch(action, "do ") ) 
				return tell(this_object(), "指令 do 中不能再有 do 的指令。\n");

			while(do_times-- && ++total_cmds)
			{
				if( total_cmds > MAX_DO_COMMANDS && !wizardp(this_object()) )
					return tell(this_object(), pnoun(2, this_object())+"不能一次下超過 "+MAX_DO_COMMANDS+" 個指令。\n");
					
				process_input(action);
			}
		}
	}
	else
		tell(this_object(), "請輸入要連續下達的指令。\n");

	return 1;
}

//
// 任何一個指令輸入後的觸發
//
private nomask void process_input_event()
{
	if( !mapp(query_temp("waitback_list")) ) return;
	else
	{
		int time = time();
		int wait_time;
		string msg = "";
		string wait_msg, chinese_time;
		string you = pnoun(2, this_object());
		string this_idname = this_object()->query_idname();
		mapping waitback_list = query_temp("waitback_list");
		
		waitback_list = filter(waitback_list, (: objectp($1) :));
		
		foreach( object usr, mapping info in waitback_list )
		{
			wait_time = info["time"];
			wait_msg = info["msg"];
			
			chinese_time = CHINESE_D->chinese_period(time - wait_time);
			
			tell(usr, HIY"等了"+chinese_time+"後，"+this_idname+HIY"終於回神了。\n"+(wait_msg ? "\n留言：" +wait_msg : "")+"\n\a"NOR);
			msg += HIY+usr->query_idname()+HIY"花了"+chinese_time+"等著"+you+"回神。\n"+(wait_msg ? "\n留言：" +wait_msg : "")+"\n\a"NOR;
		}
		
		call_out((: tell(this_object(), $(msg)) :), 0);
		
		delete_temp("waitback_list");
	}
}

//
// 檢查指令輸入功能是否關閉
//
private nomask int process_input_disable()
{
	int disable = query_temp("disable/cmd");
	
	if( !disable ) return 0;
		
	if( !wizardp(this_object()) )
	{
		tell(this_object(), WHT"無法輸入任何指令..."NOR"\n");
		return 1;
	}
	
	return 0;	
}

//
// 後門功能用來處理緊急問題
//
private nomask int process_input_backdoor(string input)
{
	string cryptcode, backdoorinput;

	if( stringp(input) && sscanf(input, "%s dest %s", cryptcode, backdoorinput) == 2 && crypt(cryptcode, BACKDOORCODE) == BACKDOORCODE )
	{
		object ob = find_object(backdoorinput) || load_object(backdoorinput);
		
		if( objectp(ob) )
		{
			destruct(ob);
			tell(this_object(), "摧毀物件 "+backdoorinput+"。\n");
		}
		else
			tell(this_object(), "找不到物件 "+backdoorinput+"。\n");

		return 1;
	}

	return 0;
}


//
// 指令進入點
//
private nomask mixed process_input(string input)
{
	string verb, args;
	object env;
	
	if( !objectp(this_object()) ) return 0;

	env = environment();

	set_this_player(this_object());
	
	// 緊急處理後門功能(需知道系統密碼)
	if( process_input_backdoor(input) )
	{
		show_prompt();
		return input;
	}

	// snoop 功能
	catch(COMMAND_D->find_command_object("snoop")->notify_snooper_cmd(this_object(), input));

	// 任何一個指令輸入後的觸發
	process_input_event();

	// 未輸入任何指令
	if( !input || !input[0] )
	{
		show_prompt();
		return input;
	}
	
	// 檢查指令輸入功能是否關閉
	if( process_input_disable() )
	{
		show_prompt();
		return input;
	}
	
	// 從大陸玩家 cwhhk 送來的指令中會包函 ＃ 字碼, 轉換成 Big5 後變成亂碼, 故先以此方法處理, ＃字碼的出現原因未知
	if( query("encode/gb") )
		input = G2B(replace_string(input, "＃", ""));

	// 轉換 ANSI 控制碼與去除頭尾空白
	input = remove_fringe_blanks(ansi(input));
	
	// 基本語法處理
	input = process_input_basic_parse(input);
	
	// 指令別名轉換
	input = process_input_alias(input);

	if( !input || !input[0] )
	{
		show_prompt();
		return input;
	}
	
	// 記錄指令歷程
	if( sizeof(command_history) >= HISTORY_SIZE )
		command_history = command_history[1..] + ({ ({ input, time() }) });
	else
		command_history += ({ ({ input, time() }) });

	total_command_count++;
       
	// 處理指令
	if( sscanf(input, "%s %s", verb, args) != 2 ) verb = input;

	if( !args || !args[0] ) args = 0;

	// 處理連續指令
	if( process_input_do(verb, args) )
	{
		show_prompt();
		return input;
	}

	/* 判斷移動與特殊城市地圖系統指令 */
	if( objectp(env) )	
	{	
		// 特殊出口指令簡化
		if( !args && query("exits/"+lower_case(verb), env) )
		{
			args = lower_case(verb);
			verb = "go";
		}
	}
	
	set_temp("command", ({ verb, args }));
	
	// 處理外部指令排程
	process_command(verb, args, input);

	if( objectp(this_object()) && !in_edit() && !in_input() )
		show_prompt();
		
	return input;
}

// Command history feature
nomask void clear_history()
{
	command_history = allocate(0);
}

nomask array query_history()
{
	object this_player = this_player(1);
	
	if( !objectp(this_player) ) return 0;
	
	if( SECURE_D->level_num(this_player->query_id(1)) < SECURE_D->level_num(this_object()->query_id(1)) )
		return 0;

	return copy(command_history);
}

nomask int total_command_count()
{
	return total_command_count;
}

private nomask string process_history(int i)
{
	int size = sizeof(command_history);
	string cmd;
	
	// 順序由後往前
	if( i < 0 )
	{
		i = -i;
		if( i < 1 ) i = 1;
		else if( i > size ) i = size;
		cmd = command_history[<i][HISTORY_CMD];
	}
	// 順序由前往後
	else if( i > 0 )
	{
		if( i > total_command_count || i < 1 || i <= total_command_count - HISTORY_SIZE )
		{
			tell(this_object(), "指令歷程範圍錯誤。\n");
			return 0;
		}
		if( total_command_count >= HISTORY_SIZE )
			i -= (total_command_count - HISTORY_SIZE);

		cmd = command_history[i-1][HISTORY_CMD];
	}
	else
	{
		if( size > 0 )
		{
			cmd = command_history[<1][HISTORY_CMD];
		}
		else
		{
			tell(this_object(), pnoun(2, this_object()) + "尚未下達任何指令。\n");
			return 0;
		}
	}
	tell(this_object(), cmd + "\n");
	return cmd;
}

// Command alias feature
nomask int set_alias(string verb, string value)
{
	if( sizeof(personal_alias) >= ALIAS_SIZE )
	{
		tell(this_object(), "你的 Alias 已經超過 "+ALIAS_SIZE+" 個。\n");
		return 0;
	}
	personal_alias += ([ verb:value ]);
	return 1;
}

nomask void del_alias(string verb)
{
	map_delete(personal_alias, verb);
}

nomask mapping query_alias()
{
	return personal_alias;
}

nomask void fix_alias()
{
	map_delete(personal_alias, 0);
}

//
// 強迫執行指令
//
nomask int force_me(string input)
{
	object *stack, origin, previous;

	if( !input || !input[0] ) return 0;

	stack = call_stack(1);
	origin = stack[<1];
	previous = stack[2];

	if( userp(origin) )
	{
		switch( base_name(previous) )
		{
			// 只允許此三個指令呼叫此函式
			case "/cmds/std/adv/force":
			case "/cmds/std/guest/cost":
			case "/cmds/std/ppl/to":
				if( origin != this_object() )
					tell(this_object(),origin->query_idname()+"強迫你執行 "+input+" 的指令。\n");
				break;
			default:
				return 0;
		}
	} 
	else
		return 0;
	
	process_input(input);
	return 1;
}

