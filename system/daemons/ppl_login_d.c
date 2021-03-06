/* This program is a part of RW mudlib
 * See /doc/help/copyright for more info
 -----------------------------------------
 * File   : ppl_login_d.c
 * Author : Clode@RevivalWorld
 * Date   : 2000-12-29
 * Note   : 玩家登錄精靈
 * Update :
 *  o 2001-02-09 Clode 刪除 port 分法, 改進一些程式寫法
 *  o 2001-07-20 Clode 重寫 repeat login id 的檢查
 *
 -----------------------------------------
 */

#include <ip.h>
#include <ansi.h>
#include <login.h>
#include <daemon.h>
#include <object.h>
#include <secure.h>
#include <gender.h>
#include <mudlib.h>
#include <message.h>
#include <ip.h>

#define INITIALIZE		1
#define INPUT_ID		2
#define INPUT_PASSWORD		3
#define INPUT_NEW_ID		4
#define INPUT_NEW_NAME		5
#define INPUT_NEW_PASSWORD	6
#define CONFIRM_PASSWORD	7
#define INPUT_EMAIL		8
#define INPUT_GENDER		9
#define RECONNECT		10
#define NEW_PLAYER		11
#define OLD_PLAYER		12
#define ENTER_GAME		13
#define FPASS_ID		14
#define FPASS_EMAIL		15

mapping newchar = allocate_mapping(0);
private nosave mapping ip_time = allocate_mapping(0);
private mapping login_ip_num = allocate_mapping(0);
string *reserved_id = ({  });

nomask void reset_newchar()
{
	newchar = allocate_mapping(0);
}

nomask void reset_cmdlimit()
{
	login_ip_num = allocate_mapping(0);
}

mapping query_loginip()
{
	return login_ip_num;
}

// 到達上限 30 就設定 banip 24 小時
private void set_user_cmdlimit( string ip )
{
	// 再 check 一次
	if( login_ip_num[ip] >= 30 )
	{
		if( !(IP_D->query_ip(ip)) )
		{
			IP_D->set_ip( this_object(), ip, BAN, 24*3600);
			IP_D->set_ip_note( this_object(), ip, "同 ip 一天 Login 超過 30 次");
		}
	}
}

private nomask varargs void logon_handle(int state, object ob, string arg)
{
	ob->reset_time_out(); // 重設Time_out時間

	if( arg && query("encode/gb", ob) ) arg = G2B(arg);

	switch(state)
	{
		/* 啟始 Login */
	case INITIALIZE:
		{
			string ip = query_ip_number(ob), ban;

			foreach(string limitedip, mapping data in IP_D->query_ipdata())
			if( regexp(ip, limitedip) && data["status"] == BAN )
			{
				ob->directly_receive(HIR"\n警告，您所使用的 IP("+limitedip+") 位置目前被系統列為禁止登入。\n"NOR);
				CHANNEL_D->channel_broadcast("nch", ip+" 的使用者嘗試使用被禁止的 IP 登入");
				destruct(ob);
				return;
			}

			if(sscanf( query_ip_name(ob), "%*s.%s.edu.tw", ban )==2)
			{
				if( ban == "cyut" )
				{
					tell( ob, sprintf( HIR"\n警告，您所使用的 IP(%s) 位置目前被系統列為禁止登入。\n"NOR, query_ip_number(ob)));
					destruct(ob);
					return;
				}
			}

			if(sscanf( query_ip_name(ob), "%*s.%*s.%s.edu.tw", ban )==3)
			{
				if( ban == "cyut" )
				{
					tell( ob, HIR"\n警告，您所使用的 IP(%s) 位置目前被系統列為禁止登入。\n"NOR, query_ip_number(ob));
					destruct(ob);
					return;
				}
			}


			if( time() - ip_time[ip] < 60 )
			{
				tell(ob, sprintf( HIY"此連線 IP 距離前次上線時間僅隔 %d 秒, 請 %d 秒後再試。\n"NOR, time() - ip_time[ip], 60 + ip_time[ip] - time()));
				CHANNEL_D->channel_broadcast("nch", ip +" 的使用者離上次 login 時間過短, 拒絕登入 !");
				destruct(ob);
				return;
			}

			ob->directly_receive("Current charset is " + (query("encode/gb", ob) ? "Simplified Chinese (GB)":"Traditional Chinese (BIG5)") + "\n");
			ob->directly_receive("Please input GB/BIG5 to change charset or directly login user.\n");

			tell(ob, sprintf(
				"創造新的遊戲角色請輸入 'new'。%s\n"
				"您的英文名字：",
				(LOGIN_D->query_wiz_lock() < GUEST ? HIC"(允許玩家登入)"NOR:HIR"(禁止玩家登入)"NOR)));

			input_to( (: logon_handle, INPUT_ID, ob :) );
			return;
		}
		/* 輸入 ID */
	case INPUT_ID:
		{
			int level_num;

			if( !arg || arg == "" )
				return logon_handle(INITIALIZE, ob);

			arg = remove_ansi(remove_fringe_blanks(lower_case(arg)));

			if( arg == "gb" )
			{
				delete("encode/big5", ob);
				set("encode/gb", 1, ob);
				return this_object()->logon(ob);
			}
			else if( arg == "big5" )
			{
				delete("encode/gb", ob);
				set("encode/big5", 1, ob);
				return this_object()->logon(ob);
			}

			if( arg == "new" && LOGIN_D->query_wiz_lock() < GUEST )
			{
				string ip = query_ip_number(ob);

				if( ip == "75.84.17.242" )
				{
					tell(ob, "抱歉您所使用的 ip ，無法再註冊新角色。\n");
					return logon_handle(INITIALIZE, ob);
				}

				if( query_ip_name(ob)[<13..] == "csupomona.edu" )
				{
					tell(ob, "抱歉您所使用的 ip ，無法再註冊新角色。\n");
					return logon_handle(INITIALIZE, ob);
				}

				if( ++newchar[ip] >= 5 )
				{
					CHANNEL_D->channel_broadcast("sys", "登入："+ip+" 的使用者本日新角色已註冊超過 "+5+" 位，無法再註冊新角色。");
					tell(ob, "抱歉您本日新角色已註冊超過 "+5+" 位，無法再註冊新角色。\n");
					return logon_handle(INITIALIZE, ob);
				}

				tell(ob, "請輸入您想使用的英文ID：");
				input_to( (: logon_handle, INPUT_NEW_ID, ob :) ); return;
			}
			if( arg == "pass" )
			{
				tell(ob, "請輸入您的英文ID：");
				input_to( (: logon_handle, FPASS_ID, ob :) ); return;
			}

			level_num = SECURE_D->level_num(arg);

			if( sizeof(users()) > MAX_USERS && level_num < GUEST )
			{
				tell(ob, "抱歉本遊戲玩家上線人數之最大上限為 "+MAX_USERS+" 位，請耐心等候。\n");
				return logon_handle(INITIALIZE, ob);
			}

			if( level_num < LOGIN_D->query_wiz_lock() )
			{
				tell(ob, HIY"系統更新維護中，目前僅允許權限在 "+SECURE_D->level_num_to_level_name(LOGIN_D->query_wiz_lock())+" 以上的使用者連線。\n"NOR);
				return logon_handle(INITIALIZE, ob);
			}

			if( file_size(user_data(arg)) <= 0 )
			{
				tell(ob, "本遊戲沒有 "+arg+" 這位玩家。\n");
				return logon_handle(INITIALIZE, ob);
			}

			if( LOGIN_D->is_repeat_login(arg) )
			{
				tell(ob, "抱歉您輸入的 "+capitalize(arg)+" 正在被登錄中，請勿重覆登錄。\n");
				return logon_handle(INITIALIZE, ob);
			}
			else
				set("id", arg, ob);

			if( level_num > PLAYER )
				tell(ob, "您的使用者權限為 "+SECURE_D->level_num_to_level_name(level_num)+"，提醒您必須由連接埠 "+WIZ_PORT+" 上線才能擁有巫師權限。\n");

			tell(ob, "請輸入密碼：");

			input_to( (: logon_handle, INPUT_PASSWORD, ob :), 1 );
			return;
		}
	case FPASS_ID:
		{
			tell(ob, "該服務尚未啟用。");
			destruct(ob);
			return;

			if( file_size(user_data(arg)) <= 0 )
			{
				tell(ob, "本遊戲沒有 "+arg+" 這位玩家。\n");
				return logon_handle(INITIALIZE, ob);
			}
			set("id", arg, ob);
			tell(ob, "請輸入該角色的信箱位置：");
			input_to( (: logon_handle, FPASS_EMAIL, ob :) ); return;
		}
	case FPASS_EMAIL:
		{
			object user_ob = load_user(query("id", ob));
			if( query("email", user_ob) == arg )
			{
				tell(ob, "系統已將新密碼 Email 至您的信箱。\n");
				SMTP_D->mail(arg, "該服務尚未啟用", "重生的世界 - "+query("id", ob)+" 角色密碼");
			}
			else tell(ob, "您輸入的資料該角色不符。\n");
			destruct(user_ob);
			destruct(ob);
			return;
		}
		/* 輸入密碼 */
	case INPUT_PASSWORD:
		{
			string password = PASSWORD_D->query_password(query("id", ob));

			if( !arg || crypt(arg, password) != password )
			{
				tell(ob, "密碼輸入錯誤。\n");

				if( LOGIN_D->input_wrong_password(ob) >= 3 )
				{
					tell(ob, "你已經輸入錯誤密碼三次，請重新登入再試。\n");
					destruct(ob); return;
				}

				tell(ob, "請輸入正確密碼：");
				input_to( (: logon_handle, INPUT_PASSWORD, ob :), 1 ); return;
			}

			// 若在遊戲中找到相同 ID 使用者物件
			if( find_player(query("id", ob)) )
			{
				object player = find_player(query("id", ob));
				set("player", player , ob);

				// 若為互動物件, 則詢問是否取代
				if( interactive(player) )
				{
					tell(ob, "你想取代目前在遊戲中相同的人物嗎(Y/N)？\n");
					input_to( (: logon_handle, RECONNECT, ob :) );
					return;
				}

				logon_handle(RECONNECT, ob, "y");return;
			}

			/* 密碼檢查正確, 進入遊戲 */
			return logon_handle(OLD_PLAYER, ob);
		}

		/* 輸入新的 ID */
	case INPUT_NEW_ID:
		{
			int len;
			string ban;

			if( sscanf( query_ip_name(ob), "%*s-%*s-%*s-%*s.dynamic.%s.net", ban ) ==5 )
			{
				if( ban == "hinet" )
				{
					tell( ob, sprintf(HIR"\n警告，您所使用的 IP(%s) 位置目前被系統列為禁止登錄新角色。\n"NOR, query_ip_number(ob)));
					destruct(ob);
					return;
				}
			}

			if( sscanf( query_ip_name(ob), "%*s-%*s-%*s-%*s..HINET-IP.%s.net", ban ) ==5 )
			{
				if( ban == "hinet" )
				{
					tell( ob, sprintf(HIR"\n警告，您所使用的 IP(%s) 位置目前被系統列為禁止登錄新角色。\n"NOR, query_ip_number(ob)));
					destruct(ob);
					return;
				}
			}

			if(sscanf( query_ip_name(ob), "%*s.%s.edu.tw", ban )==2)
			{
				if( ban = "cyut" )
				{
					tell( ob, HIR"\n警告，您所使用的 IP(%s) 位置目前被系統列為禁止登錄新角色。\n"NOR, query_ip_number(ob));
					destruct(ob);
					return;
				}
			}

			if(sscanf( query_ip_name(ob), "%*s.%*s.%s.edu", ban )== 3)
			{
				if( ban = "csupomona" )
				{
					tell( ob, HIR"\n警告，您所使用的 IP(%s) 位置目前被系統列為禁止登錄新角色。\n"NOR, query_ip_number(ob));
					destruct(ob);
					return;
				}
			}

			if( !arg || arg == "" )
			{
				tell(ob, "請輸入您想使用的英文代號(ID)：");
				input_to( (: logon_handle, INPUT_NEW_ID, ob :) ); return;
			}

			len = strlen((arg = lower_case(arg)));

			if( (len = strlen(arg)) < 3 || len > 12 )
			{
				tell(ob, "對不起，你的英文名字只介於 3 到 12 個字。\n請輸入您想使用的英文代號(ID)：");
				input_to( (: logon_handle, INPUT_NEW_ID, ob :) );return;
			}

			foreach( string a in arg )
			if( (int) a<'a' || (int) a>'z' )
			{
				tell(ob, "對不起，你的英文名字只能用英文字母。\n請輸入您想使用的英文代號(ID)：");
				input_to( (: logon_handle, INPUT_NEW_ID, ob :) );return;
			}

			if( member_array(arg, reserved_id) != -1 )
			{
				tell(ob, "本 ID 為系統保留字，請換另一個名字。\n請輸入您想使用的英文代號(ID)：");
				input_to( (: logon_handle, INPUT_NEW_ID, ob :) );return;
			}

			if( file_size(user_data(arg)) > 0 )
			{
				tell(ob, "本遊戲已有玩家使用 "+capitalize(arg)+"，請換另一個名字。\n請輸入您想使用的英文代號(ID)：");
				input_to( (: logon_handle, INPUT_NEW_ID, ob :) );return;
			}

			if( LOGIN_D->is_repeat_login(arg) )
			{
				tell(ob, "抱歉您的ID:"+capitalize(arg)+"正在登入中，請勿重覆登入。\n");
				return logon_handle(INITIALIZE, ob);
			}
			else
				set("id", arg, ob);


			tell(ob, "請輸入您的名稱(Name)：");
			input_to( (: logon_handle, INPUT_NEW_NAME, ob :) );
			return;
		}

		/* 輸入新名稱 */
	case INPUT_NEW_NAME:
		{
			int len;
			if( !arg || (len=strlen(arg)) < 2 || len > 12 )
			{
				tell(ob, "您必須輸入 1 到 12 個字母：");
				input_to( (: logon_handle, INPUT_NEW_NAME, ob :) ); return;
			}

			if( strsrch(arg, "　") != -1 || strsrch(arg, " ") != -1 )
			{
				tell(ob, "請勿用空格來當做名稱。\n請輸入名稱：");
				input_to( (: logon_handle, INPUT_NEW_NAME, ob :) ); return;
			}

			foreach( int a in arg )
			{
				if( a <= ' ' )
				{
					tell(ob, "您的名稱不能用控制字元。\n請輸入名稱(Name)：");
					input_to( (: logon_handle, INPUT_NEW_NAME, ob :) );return;
				}
			}

			set("name", arg, ob);

			tell(ob, "歡迎"+arg+"("+capitalize(query("id", ob))+")進入"MUD_FULL_NAME"。\n");
			tell(ob, "請輸入您想使用的密碼：");
			input_to( (: logon_handle, INPUT_NEW_PASSWORD, ob :), 1 );
			return;
		}

		/* 輸入新的密碼 */
	case INPUT_NEW_PASSWORD:
		{
			if( !arg || strlen(arg) < 4 )
			{
				tell(ob, "請輸入多於四個字母的密碼：");
				input_to( (: logon_handle, INPUT_NEW_PASSWORD, ob :), 1 );return;
			}

			set("password", crypt(arg, 0), ob);
			tell(ob, "請再輸入一次您的密碼﹕");
			input_to( (: logon_handle, CONFIRM_PASSWORD, ob :), 1 );
			return;
		}

		/* 再次確認密碼 */
	case CONFIRM_PASSWORD:
		{
			if( !arg || crypt( arg, query("password", ob) ) != query("password", ob) )
			{
				tell(ob, "確認密碼失敗。\n請重新輸入您想使用的密碼：");
				input_to( (: logon_handle, INPUT_NEW_PASSWORD, ob :), 1 );return;
			}

			PASSWORD_D->set_password(query("id", ob), query("password", ob));

			tell(ob, "您的信箱位址將作為日後索取角色備份檔案的依據，或直接按 Enter 放棄輸入。\n您的信箱位址：");
			input_to( (: logon_handle, INPUT_EMAIL, ob :) ); return;
		}

		/* 設定EMAIL */
	case INPUT_EMAIL:
		{
			if( !arg || arg=="" )
				tell(ob, "您自願放棄意外喪失角色時，向巫師申請備份檔案的權利。\n");
			else if( sscanf(arg,"%*s@%*s.%*s") != 3 )
			{
				tell(ob, "此信箱位址格式有問題。\n請重新輸入您的信箱位址：");
				input_to( (: logon_handle, INPUT_EMAIL, ob :) ); return;
			}
			else set("email", arg, ob);

			tell(ob, "請選擇您的性別，男(M)、女(F)﹕");
			input_to( (: logon_handle, INPUT_GENDER, ob :) ); return;
		}
		/* 設定性別 */
	case INPUT_GENDER:
		{
			if( arg == "m" || arg == "M" )
				set("gender", MALE_GENDER, ob);
			else if( arg == "f" || arg == "F" )
				set("gender", FEMALE_GENDER, ob);
			else
			{
				tell(ob, "請輸入 M 或是 F。\n請選擇您的性別，男(M)、女(F)﹕");
				input_to( (: logon_handle, INPUT_GENDER, ob :) );return;
			}

			return logon_handle(NEW_PLAYER, ob);
		}

		/* 新玩家登錄 */
	case NEW_PLAYER:
		{
			object user_ob = load_object(user_file_name(query("id", ob)));

			/*轉換資料並儲存*/
			user_ob->set_idname(query("id", ob), query("name", ob));

			set("email", query("email", ob), user_ob);
			set_temp("login/ident", query("ident", ob), user_ob);
			set_temp("login/remoteport", query("remote_port", ob), user_ob);
			set_temp("login/terminaltype", query("terminaltype", ob), user_ob);
			set_temp("windowsize", query("windowsize", ob) || DEFAULT_WINDOW_SIZE, user_ob);

			if( query("encode/gb", ob) )
				set("encode/gb", 1, user_ob);
			else
				delete("encode/gb", user_ob);

			set_temp("new_player", 1, user_ob);

			exec( user_ob, ob );

			CHAR_D->create_char(user_ob, query("gender", ob));

			user_ob->save();

			destruct(ob);

			return logon_handle(ENTER_GAME, user_ob);
		}

		/* 舊玩家登錄 */
	case OLD_PLAYER:
		{
			object user_ob = load_user(query("id", ob));

			if( !objectp(user_ob) )
			{
				object login_ob = new(LOGIN_OB);

				tell(ob, "無法載入 "+capitalize(query("id", ob))+" 的儲存檔，請聯絡巫師處理。\n");
				exec(login_ob, ob);
				destruct(ob);
				logon_handle(INITIALIZE, login_ob);return;
			}

			set_temp("login/ident", query("ident", ob), user_ob);
			set_temp("login/remoteport", query("remote_port", ob), user_ob);
			set_temp("login/terminaltype", query("terminaltype", ob), user_ob);
			set_temp("windowsize", query("windowsize", ob) || DEFAULT_WINDOW_SIZE, user_ob);

			if( query("encode/gb", ob) )
				set("encode/gb", 1, user_ob);
			else
				delete("encode/gb", user_ob);

			// 由 LOGIN_OB 切換至 USER_OB
			exec( user_ob , ob );
			destruct(ob);

			return logon_handle(ENTER_GAME, user_ob);
		}

		/* 重新連線 */
	case RECONNECT:
		{
			object reconnect_ob;

			if( !arg || remove_ansi(remove_fringe_blanks(lower_case(arg))) != "y" )
			{
				tell(ob, "您決定不取代遊戲中相同的人物。\n");
				ob->reset_database();

				logon_handle(INITIALIZE, ob);return;
			}

			if( !objectp(reconnect_ob = query("player", ob)) )
			{
				tell(ob, "原本遊戲中的人物已經消失，請重新登入。\n");
				ob->reset_database();
				logon_handle(INITIALIZE, ob);return;
			}

			if( !interactive(reconnect_ob) )
				reconnect_ob->reconnect();
			else
			{
				object temp = new(VOID_OB);

				tell(reconnect_ob, "另外一位使用者自 "+query_ip_name(ob)+" 連線取代了你。\n");

				/* 先將原本的玩家轉換至一個暫時 USER_OB 再將之摧毀 */
				exec(temp, reconnect_ob);
				destruct(temp);
			}

			set_temp("reconnect", 1, reconnect_ob);
			set_temp("login/ident", query("ident", ob), reconnect_ob);
			set_temp("login/remoteport", query("remote_port", ob), reconnect_ob);
			set_temp("windowsize", query("windowsize", ob) || DEFAULT_WINDOW_SIZE, reconnect_ob);
			set_temp("login/terminaltype", query("terminaltype", ob), reconnect_ob);

			if( query("encode/gb", ob) )
				set("encode/gb", 1, reconnect_ob);
			else
				delete("encode/gb", reconnect_ob);

			/* 取代舊 user_ob */
			exec( reconnect_ob , ob );
			destruct(ob);

			delete_temp("new_player", reconnect_ob);

			return logon_handle(ENTER_GAME, reconnect_ob);
		}

		/* 進入遊戲 */
	case ENTER_GAME:
		{
			// 記錄該 ip 上線時間
			string ip = query_ip_number(ob);
			ip_time[ip] = time();

			// 記錄該 ip 上線次數
			if( !login_ip_num[ip] )
				login_ip_num[ip] = 1;
			else
			if( login_ip_num[ip] >= 30 )
			{
				login_ip_num[ip] = 30;
				set_user_cmdlimit(ip);
			} else
				login_ip_num[ip] = ( login_ip_num[ip] + 1 );

			// 進入遊戲
			LOGIN_D->enter_game(ob);
			return;

		}/* ENTER_GAME */

	default: return logon_handle(INITIALIZE, ob);

	}/* END SWITCH */

}

void wait_for_login(object login_ob, string arg)
{
	tell(login_ob, "載入中，請稍後 ...\n");
	input_to( (: wait_for_login, login_ob :) );
}

nomask void logon(object login_ob)
{
	set("encode/gb", 1, login_ob);
	if (query("encode/big5", login_ob)) {
		delete("encode/gb", login_ob);
	}

	/* 載入系統 */
	if( !SYSTEM_D->valid_login() )
	{
		tell(login_ob, HIY + MUD_FULL_NAME + "載入中，請稍後 ...\n"NOR);
		SYSTEM_D->add_loginuser( login_ob );
		input_to((: wait_for_login, login_ob :));
		return;
	}

	LOGIN_D->show_login_message(login_ob);

	if( !LOGIN_D->check_login_attacker(query_ip_number(login_ob)) )
	{
		login_ob->directly_receive(HIR"\n很抱歉，這個 IP["+query_ip_number(login_ob)+"] 的登入動作太過頻繁，請稍候再試。\n"NOR);
		//CHANNEL_D->channel_broadcast("sys", "登入："+query_ip_number(login_ob)+" 的登入動作太過頻繁，強制離線");
		//destruct(login_ob);
		logon_handle(INITIALIZE, login_ob);
		return;
	}

	logon_handle(INITIALIZE, login_ob);
}

int remove()
{
	foreach(object user in users())
	{
		if( !user->is_login_ob() ) continue;
		tell(user, HIR"\n很抱歉，登錄系統更新，麻煩您再重新登錄。\n"NOR);
		flush_messages(user);
		destruct(user);
	}
}

string query_name()
{
	return "玩家登入系統(PPL_LOGIN_D)";
}
