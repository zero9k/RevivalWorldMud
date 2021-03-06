#include <daemon.h>
#include <socket.h>
#include <socket_err.h>

#define monitor(x)		CHANNEL_D->channel_broadcast("nch", "HTTP_CLIENT_D: " + x)
#define ADDR_SYNTAX		"http://%s/%s"
#define RESOLVE_TIMEOUT		10
#define PROCESS_TIMEOUT		30
#undef DEBUG

class socket_info
{
	int received, socket_fd, connect_port;
	int write_fd, connect_handle;
	string connect_host, connect_ip, get_path;
	string write_buffer, read_buffer;
	function callback;
}
mapping connections = allocate_mapping(0);
mapping resolves = allocate_mapping(0);
mapping resolve_catch = allocate_mapping(0);

void get_url(string url);

void create()
{
	//get_url("http://fs.mis.kuas.edu.tw/index.html");
}

void socket_shutdown(int fd)
{
	class socket_info soc = connections[fd];

	socket_close(fd);
	soc->received = 0;
	soc->socket_fd = 0;
	soc->write_fd = 0;
	soc->connect_port = 0;
	soc->connect_ip = 0;
	soc->connect_host = 0;
	soc->read_buffer = 0;
	soc->write_buffer = 0;

	// Restore socket info
	connections[fd] = soc;
	map_delete(connections, fd);
	return;
}

void socket_send(int fd, mixed msg)
{
	int res;
	class socket_info soc = connections[fd];

#ifdef DEBUG
	//monitor(sprintf("socket_send(%d) = %O", fd, soc));
	monitor("送出訊息: fd: " + fd + ", message: " + msg);
#endif

	if( !msg || msg == "" ) return; 
	// 尚未收到 socket write callback 前，將欲傳送的訊息存入 buffer
	if( !soc->write_fd )
	{
		if( !soc->write_buffer ) soc->write_buffer = "";
		soc->write_buffer += msg;
		return;
	}
	// 如果 buffer 中有訊息尚未送出，則與本次訊息同時送出
	if( soc->write_buffer )
	{
		soc->write_buffer += msg;
		res = socket_write(fd, soc->write_buffer);
	}
	else res = socket_write(fd, msg);

	switch(res) {

	case EESUCCESS:		/* 成功 */
	case EECALLBACK:	/* 等待完成 */
		soc->write_buffer = 0;
		break;
	case EEALREADY:		/* 程序已進行 */
	case EEWOULDBLOCK:	/* 程序停滯 */
	case EESEND:		/* 傳送資料錯誤 */
		if( !soc->write_buffer ) soc->write_buffer = msg;
		call_out((: socket_send :), 1, fd, "");
		break;
	default:		/* 其他錯誤 */
		monitor("socket_send: " + socket_error(res));
		// try again
		socket_shutdown(fd);
		//do_again();
		//do_next();
		break;
	}
	// Restore socket info
	connections[fd] = soc;
}

void receive_data(int fd, string msg)
{ 
	class socket_info soc = connections[fd];

#ifdef DEBUG
	//monitor(sprintf("socket_send(%d) = %O", fd, soc));
#endif

	if( !soc->read_buffer ) soc->read_buffer = "";
	soc->read_buffer += msg;
	//monitor(msg);

	// Restore socket info
	connections[fd] = soc;
}

void write_data(int fd)
{
	class socket_info soc = connections[fd];

#ifdef DEBUG
	//monitor(sprintf("write_data(%d) = %O", fd, soc));
#endif

	soc->write_fd = fd;
	//remove_call_out(connect_handle);

	// 如果收到 socket write callback 時
	// buffer 內有訊息未傳送則立即傳送，並清除 buffer
	if( soc->write_buffer )
	{
		socket_write(fd, soc->write_buffer);
		soc->write_buffer = 0;
	}

	// Restore socket info
	connections[fd] = soc;
}

void connection_close(int fd)
{
	class socket_info soc = connections[fd];

#ifdef DEBUG
	monitor("Connection closed by " + soc->connect_ip +".\n");
#endif

	if( find_call_out(soc->connect_handle) )
		remove_call_out(soc->connect_handle);
	//monitor(html_remove(soc->read_buffer));
	if( functionp(soc->callback) ) evaluate(soc->callback, soc->read_buffer);
	socket_shutdown(fd);
}

void connect_timeout(int fd)
{
	class socket_info soc = connections[fd];

#ifdef DEBUG
	monitor("connect: Unable to connect to remote host " + soc->connect_ip + ": Connection timeout\n");
#endif

	if( functionp(soc->callback) ) evaluate(soc->callback, soc->read_buffer);
	socket_shutdown(fd);
}

void do_connect(class socket_info soc)
{
	int res;
	string addr;

	//remove_call_out(resolve_handle);

	soc->socket_fd = socket_create(STREAM, (: receive_data :), (: connection_close :));

	if( soc->socket_fd < 0 )
	{
		monitor("do_connect: Unable to create socket: " + socket_error(soc->socket_fd) + "\n");
		return;
	}
	addr = soc->connect_ip + " " + soc->connect_port;
	res = socket_connect(soc->socket_fd, addr, (: receive_data :), (: write_data :));
	if( res != EESUCCESS )
	{
		monitor("do_connect: Unable to connect remote host: " + socket_error(res) + "\n");
		socket_shutdown(soc->socket_fd);
		return;
	}
	soc->connect_handle = call_out((: connect_timeout :), PROCESS_TIMEOUT, soc->socket_fd);

	// Create socket info
	connections[soc->socket_fd] = soc;

	socket_send(soc->socket_fd, sprintf("GET "ADDR_SYNTAX"\n\n", soc->connect_host, soc->get_path));
}

void resolve_callback(string addr, string ip, int key)
{
	class socket_info soc;

	if( undefinedp(soc = resolves[key]) ) return;
	map_delete(resolves, key);

	if( !addr || !ip )
	{
		monitor("resolve: " + (ip || addr) + " - Unknown host.\n");
		return;
	}
	resolve_catch[addr] = ip;
	soc->connect_ip = ip;
	do_connect(soc);
}

void resolve_timeout(int key)
{
	class socket_info soc;

	if( undefinedp(soc = resolves[key]) ) return;
	monitor("resolve: Unable to resolve remote host " + soc->connect_host + ": Timeout.\n");
	map_delete(resolves, key);
}

varargs void get_url(string url, function callback)
{
	int port;
	string host, path;
	class socket_info soc = new(class socket_info);

	if( !url ) error("Too few arguments to get_url.");
	sscanf(url, ADDR_SYNTAX, host, path);
	if( !host ) error("The URL syntax is " + sprintf(ADDR_SYNTAX, "<host>", "<path>"));
	sscanf(host, "%s:%s", host, port);	
	if( !port ) port = 80;
	soc->connect_host = host;
	soc->connect_port = port;
	soc->get_path = path;
	soc->callback = callback;
	// resolve catch
	if( !undefinedp(resolve_catch[host]) )
	{
		host = resolve_catch[host];
	}
	if( sscanf(host, "%*d.%*d.%*d.%*d") != 4 )
	{
		int key;

		monitor(sprintf("解析主機位置: %O", host));
		key = resolve(host, (: resolve_callback :));
		call_out((: resolve_timeout :), RESOLVE_TIMEOUT, key);
		resolves[key] = soc;
		return;
	}
	soc->connect_ip = host;
	do_connect(soc);
	return;
}
mapping query_resolves()
{
	return resolves;
}
mapping query_connections()
{
	return connections;
}

string query_name()
{
	return "HTTP 瀏覽器系統(HTTP_CLIENT_D)";
}
