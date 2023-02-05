#include <Windows.h>
#include <WinSock.h>  //一定要包含这个 
#include <stdio.h>  
#include "include/mysql.h"    //引入mysql头文件(一种方式是在vc++目录里面设置，一种是文件夹拷到工程目录，然后这样包含)  

#include <algorithm>
#include <vector>
#include <string>
#include <time.h>
#include <iostream>
//包含附加依赖项，也可以在工程--属性里面设置  
#pragma comment(lib,"wsock32.lib") 
#pragma comment(lib,"libmysql.lib")

using namespace std;


MYSQL mysql; //mysql连接
MYSQL_FIELD* fd;  //字段列数组
char field[32][32];  //存字段名二维数组
MYSQL_RES* res; //这个结构代表返回行的一个查询结果集
MYSQL_ROW column; //一个行数据的类型安全(type-safe)的表示，表示数据行的列
string query; //查询语句

//函数声明 
bool ConnectDatabase();
void FreeConnect();
void GetToday(int& year,int& mon,int& day,int& hour,int& min,int& sec);//当日日期（年月日时分秒）
vector<string> GetColumnName(string tablename);//获取当前表所有字段名
int GetColumnNum(string tablename);//获取当前啊表字段数量
void AutoUpdate(string tablename);//每日更新总表
bool IsClockin(string crewname);//查询是否打卡
bool ClockIn(string crewname);//进行今日打卡
bool ToClockIn(string managername, vector<string> abscentlist);//周日打卡
bool AddPerson(string managername, string crewname);//增加一个人员
bool Deduction(string managername,vector<string> columnname);//二级表自动扣工时，配合一级表做映射
bool Show(string tablename);

enum classType
{
	Sat,
	Sun,
	Summer,
	Winter
}Type;


int main(int argc, char** argv)
{
	string manager;
	string table;
	string absl;
	vector<string> ablist;
	if (!ConnectDatabase())
	{
		system("pause");
		return -1;
	}
	AutoUpdate("gp");
	cout << "请输入管理员名称：";
	cin >> manager;
	cout << "请问是要在星期六/星期天打卡？(Sat|Sun)：";
	cin >> table;
	cout << "请输入缺席人员名称，用空格隔开 (没有则认为全齐)";
	char key_space = getchar();//吃一个回车
	getline(cin, absl);
	absl += ' ';
	string tmp;
	for (int i = 0; i < absl.size(); i++)
	{
		if (absl[i] != ' ')
			tmp += absl[i];
		else
		{
			ablist.push_back(tmp);
			tmp.clear();
		}
	}

	ToClockIn(manager + "_" + table, ablist);
	ClockIn(manager);

	//AddPerson("Nomel", "Kity_2");
	//QueryDatabase2();
	//DeleteData();
	Show(manager + "_" + table);
	FreeConnect();
	return 0;
}








//连接数据库  
bool ConnectDatabase()
{
	//初始化mysql  
	mysql_init(&mysql);  //连接mysql，数据库  
	const char host[] = "localhost";
	const char user[] = "root";
	const char psw[] = "bichang123";
	const char database[] = "BC_ClockIn";
	const int port = 3306;
	//返回false则连接失败，返回true则连接成功  
	if (!(mysql_real_connect(&mysql, host, user, psw, database, port, NULL, 0)))
		//中间分别是主机，用户名，密码，数据库名，端口号（可以写默认0或者3306等），可以先写成参数再传进去  
	{
		printf("Error connecting to database:%s\n", mysql_error(&mysql));
		return false;
	}
	else
	{
		printf("Database Connected...\n");
		return true;
	}
}
//释放资源  
void FreeConnect()
{
	mysql_free_result(res);  //释放一个结果集合使用的内存。
	mysql_close(&mysql);	 //关闭一个服务器连接。
}

/***************************数据库操作***********************************/
//其实所有的数据库操作都是先写个sql语句，然后用mysql_query(&mysql,query)来完成，包括创建数据库或表，增删改查  
//查询数据  
//bool QueryDatabase2()
//{
//	mysql_query(&mysql, "set names gbk");
//	//返回0 查询成功，返回1查询失败  
//	if (mysql_query(&mysql, "select * from user"))        //执行SQL语句  
//	{
//		printf("Query failed (%s)\n", mysql_error(&mysql));
//		return false;
//	}
//	else
//	{
//		printf("query success\n");
//	}
//	res = mysql_store_result(&mysql);
//	//打印数据行数  
//	printf("number of dataline returned: %d\n", mysql_affected_rows(&mysql));
//	for (int i = 0; fd = mysql_fetch_field(res); i++)  //获取字段名  
//		strcpy_s(field[i], fd->name);
//	int j = mysql_num_fields(res);  // 获取列数  
//	for (int i = 0; i < j; i++)  //打印字段  
//		printf("%10s\t", field[i]);
//	printf("\n");
//	while (column = mysql_fetch_row(res))
//	{
//		for (int i = 0; i < j; i++)
//			printf("%10s\t", column[i]);
//		printf("\n");
//	}
//	return true;
//}
//
////删除数据  
//bool DeleteData()
//{
//	/*sprintf(query, "delete from user where id=6");*/
//	char query[100];
//	printf("please input the sql:\n");
//	//gets(query);  //这里手动输入sql语句  
//	if (mysql_query(&mysql, query))        //执行SQL语句  
//	{
//		printf("Query failed (%s)\n", mysql_error(&mysql));
//		return false;
//	}
//	else
//	{
//		printf("Insert success\n");
//		return true;
//	}
//}

void GetToday(int& year,int& mon,int& day,int& hour,int& min,int& sec)
{
	time_t now = time(NULL);
	struct tm tm_t;
	localtime_s(&tm_t, &now);
	year = tm_t.tm_year + 1900;
	mon = tm_t.tm_mon + 1;
	day = tm_t.tm_mday;
	hour = tm_t.tm_hour;
	min = tm_t.tm_min;
	sec = tm_t.tm_sec;
}
int GetColumnNum(string tablename)
{
	//先查表里面有多少字段
	int num = 0;
	string query_count = "select count(*) from information_schema.columns where table_name = " + (string)"\'" + tablename + (string)"\'";
	if (mysql_query(&mysql, query_count.c_str())) {        // 执行指定为一个空结尾的字符串的SQL查询。	
		cout << "Query failed" + (string)":" + (string)mysql_error(&mysql) << endl;
		return false;
	}
	if (!(res = mysql_store_result(&mysql)))    //获得sql语句结束后返回的结果集  
	{
		printf("Couldn't get result from %s\n", mysql_error(&mysql));
		return false;
	}
	while (column = mysql_fetch_row(res))
	{
		num = (**column) - '0';
	}
	return num;
}
vector<string> GetColumnName(string tablename)
{
	/*SELECT

	column_name

		FROM

		information_schema.COLUMNS

		WHERE

		table_name = 'Saturday' and column_name < >'time';
	*/
	vector<string> ret;
	string query_colname = "select COLUMN_NAME from INFORMATION_SCHEMA.Columns where table_name= " + (string)"\'" + tablename + (string)"\'";
	if (mysql_query(&mysql, query_colname.c_str())) {        // 执行指定为一个空结尾的字符串的SQL查询。	
		cout << "Query failed" + (string)":" + (string)mysql_error(&mysql) << endl;
		return ret;
	}
	if (!(res = mysql_store_result(&mysql)))    //获得sql语句结束后返回的结果集  
	{
		printf("Couldn't get result from %s\n", mysql_error(&mysql));
		return ret;
	}
	while (column = mysql_fetch_row(res))
	{
		ret.push_back(*column);
	}
	//由于mysql的原因，主键字段会放到最后显示，需要手动做调整
	//std::rotate(ret.begin(), ret.end() - 1, ret.end());
	return ret;
}
void AutoUpdate(string tablename)
{
	int num = 0;
	int year, mon, day, hour, min, sec;
	GetToday(year, mon, day, hour, min, sec);
	string time = to_string(year) + "-" + to_string(mon) + "-" + to_string(day);
	string isToday = "select * from gp where time = " + time;
	if (mysql_query(&mysql, isToday.c_str())) {        // 执行指定为一个空结尾的字符串的SQL查询。	
		cout << "Query failed" + (string)":" + (string)mysql_error(&mysql) << endl;
		return;
	}
	if (!(res = mysql_store_result(&mysql)))    
	{
		printf("Couldn't get result from %s\n", mysql_error(&mysql));
		return;
	}
	while (column = mysql_fetch_row(res))	
	{
		num++;
	}
	if (num != 0)return;	//如果有数据说明已经更新过直接退出函数 
	vector<string> colname = GetColumnName(tablename);
	query = "insert into " + tablename + " select time + 1";
	for (int i = 1; i < colname.size(); i++)
		query += "," + colname[i];
	query += " from " + tablename + " order by time desc limit 1";
	mysql_query(&mysql, query.c_str());
	//update gp set time = '2023-02-05' where time in (select t.time from (select * from gp order by time desc limit 1) as t); 
	query = "update gp set time = " + (string)"\'" + time + (string)"\'" + "where time in (select t.time from (select * from gp order by time desc limit 1) as t)";;
	mysql_query(&mysql, query.c_str());
}
bool Deduction(string managername,vector<string> columnname)
{
	string query_1 = "select * from " + managername;
	if (mysql_query(&mysql, query_1.c_str())) {        // 执行指定为一个空结尾的字符串的SQL查询。	
		cout << "Query failed" + (string)":" + (string)mysql_error(&mysql) << endl;
		return false;
	}
	if (!(res = mysql_store_result(&mysql)))    //获得sql语句结束后返回的结果集  
	{
		printf("Couldn't get result from %s\n", mysql_error(&mysql));
		return false;
	}
	//获取最新的一条用于减工时
	MYSQL_ROW reserved = column;
	while (column = mysql_fetch_row(res))
	{
		reserved = column;
	}

	string query_deduct;

	int num = GetColumnNum(managername);
	for (int i = 1; i < num; i++)
	{
		query_deduct = "update gp set ";
		//先强转char*再用atoi转整型才能运算
		query_deduct += columnname[i] + "=" + columnname[i] + "-" + to_string((atoi((char*)reserved[i]) * 2)) + " where time = '" + reserved[0] + (string)"\'";
		if (mysql_query(&mysql, query_deduct.c_str())) {        // 执行指定为一个空结尾的字符串的SQL查询。	
			cout << "Query failed" + (string)":" + (string)mysql_error(&mysql) << endl;
			return false;
		}
	}
	return true;
}
bool ToClockIn(string managername, vector<string> abscentlist)
{
	//先获取打卡时间
	int year, mon, day, hour, min, sec;
	GetToday(year, mon, day, hour, min, sec);
	string time = to_string(year) + "-" + to_string(mon) + "-" + to_string(day) + " " + to_string(hour) + ":" + to_string(min) + ":" + to_string(sec);


	//再制作查询语句
	vector<string> colname = GetColumnName(managername);
	string query_insert = "insert into " + managername + " (";
	for (int i = 0; i < colname.size() - 1; i++)//这里最后一个段名手动补齐
	{
		query_insert += (colname[i] + ",");
	}
	query_insert += colname[colname.size() - 1];//最后一句单独添加

	query_insert += ") values (\'" + time + "\',";
	for (int i = 1; i < colname.size() - 1; i++)//这里最后一个要收尾，所以单独处理-1
	{
		if (find(abscentlist.begin(), abscentlist.end(), colname[i]) == abscentlist.end())//出席了
			query_insert += ("1,");
		else
			query_insert += ("0,");
	}
	if (find(abscentlist.begin(), abscentlist.end(), colname[colname.size() - 1]) == abscentlist.end())//出席了
		query_insert += "1)";
	else
		query_insert += "0)";

	//这样就制作出了按照不出席名单来制作插入值
	if (mysql_query(&mysql, query_insert.c_str())) {        // 执行指定为一个空结尾的字符串的SQL查询。	
		cout << "Query failed" + (string)":" + (string)mysql_error(&mysql) << endl;
		return false;
	}
	Deduction(managername, colname);
	return true;
}

bool IsClockin(string crewname)
{
	query = "select * from crew where name = " + (string)"\'" + crewname + (string)"\'"; //执行查询语句，这里是查询所有，user是表名，不用加引号，用strcpy也可以 
	mysql_query(&mysql, "set names gbk"); //设置编码格式（SET NAMES GBK也行），否则cmd下中文乱码  
	//返回0 查询成功，返回1查询失败  
	if (mysql_query(&mysql, query.c_str())){        // 执行指定为一个空结尾的字符串的SQL查询。	
		cout << "Query failed" + (string)":" + (string)mysql_error(&mysql) << endl;
		return false;
	}
	//获取结果集  
	if (!(res = mysql_store_result(&mysql)))    //获得sql语句结束后返回的结果集  
	{
		printf("Couldn't get result from %s\n", mysql_error(&mysql));
		return false;
	}
	//查询结果数量
	if (mysql_affected_rows(&mysql) == 0)
	{
		cout << "There is no data about " + crewname + " inside the table!" << endl;
		return false;
	}
	MYSQL_ROW reserved = column;
	while (column = mysql_fetch_row(res))   //获取最新一行 
	{
		//printf("%20s\t%20s\t%30s\t%12s\n", column[0], column[1], column[2], column[3]);//column是列数组
		reserved = column;
	}
	//对比当前时间判断是否打卡
	int year_now, mon_now, day_now, hour_now, min_now, sec_now;
	GetToday(year_now, mon_now, day_now, hour_now, min_now, sec_now);
	int mon = (reserved[2][5] - 48) * 10 + (reserved[2][6] - 48);
	int day = (reserved[2][8] - 48) * 10 + (reserved[2][9] - 48);
	if ((mon == mon_now) && (day == day_now) && reserved[3])
	{
		//cout << crewname+"今日已打卡" << endl;
		return true;
	}
	//cout << crewname + "今日未打卡" << endl;
	return false;
}

bool ClockIn(string managername)
{
	//检查是否已打卡
	if (IsClockin(managername))
	{
		cout << managername + "今日已打卡，无需再打，如有特殊需要请找管理员" << endl;
		return false;
	}
	//未打卡就执行Insert指令，往crew表中添加一条打卡信息
	int year_now, mon_now, day_now, hour_now, min_now, sec_now;
	GetToday(year_now, mon_now, day_now, hour_now, min_now, sec_now);//获取当前日期
	query = "insert crew(name,time,clock_in) values (" + (string)"\'" + managername + (string)"\'" + "," + (string)"\'" + to_string(year_now)  + "-" +  to_string(mon_now) + "-" + to_string(day_now) + (string)"\'" + "," + "1)";
	mysql_query(&mysql, "set names gbk"); //设置编码格式（SET NAMES GBK也行），否则cmd下中文乱码  
	//返回0 查询成功，返回1查询失败  
	if (mysql_query(&mysql, query.c_str())) {        // 执行指定为一个空结尾的字符串的SQL查询。	
		cout << "Query failed" + (string)":" + (string)mysql_error(&mysql) << endl;
		return false;
	}
	else
	{
		cout << managername + "打卡成功" << endl;
	}
	return true;
}

//充当一级表，用于二级表的工时扣除映射
bool AddPerson(string managername ,string crewname)
{
	//注意让新增的字段为not null并且设置非空默认值，否则null值的字段无法update的（如果你要立刻改这个新增字段的话）
	int count = 0;
	MYSQL_ROW reserved = column;
	string query_tmp = "select COLUMN_NAME from INFORMATION_SCHEMA.Columns where table_name= " + (string)"\'" + managername + (string)"\'";
	mysql_query(&mysql, "set names gbk");
	if (mysql_query(&mysql, query_tmp.c_str())) {        // 执行指定为一个空结尾的字符串的SQL查询。	
		cout << "Query failed" + (string)":" + (string)mysql_error(&mysql) << endl;
		return false;
	}
	//获取表中所有的字段名字
	if (!(res = mysql_store_result(&mysql)))    //获得sql语句结束后返回的结果集  
	{
		printf("Couldn't get result from %s\n", mysql_error(&mysql));
		return false;
	}
	//不检查是否为空，因为没有就可以添加
	while (column = mysql_fetch_row(res))   //获取最新一行 
	{
		count++;
		reserved = column;
	}
	string last = *reserved;	//获取最后一个员工的编号
	int number = 0;
	//计算出下一个新员工的编号
	size_t pos = last.find("_");
	for (; pos<last.size()-1; pos++)
	{
		number = number * 10 + last[pos + 1] - '0';
	}
	//添加一个员工及其编号
	query = "alter table " + managername + " add p_" + to_string(number + 1) + " varchar(12) not null default \"HELLO\"";
	if (mysql_query(&mysql, query.c_str())) {        // 执行指定为一个空结尾的字符串的SQL查询。	
		cout << "Query failed" + (string)":" + (string)mysql_error(&mysql) << endl;
		return false;
	}
	string query1 = "update " + managername + " set p_" + to_string(number + 1) + "= " + (string)"\'" + crewname + (string)"\'";
	if (mysql_query(&mysql, query1.c_str())) {        // 执行指定为一个空结尾的字符串的SQL查询。	
		cout << "Query failed" + (string)":" + (string)mysql_error(&mysql) << endl;
		return false;
	}
	return true;
}

bool Show(string tablename)
{
	MYSQL_ROW ALL_COLUMN;
	MYSQL_RES* ALL_RES;
	int year, mon, day, hour, min, sec;
	GetToday(year, mon, day, hour, min, sec);
	string time = to_string(year) + "-" + to_string(mon) + "-" + to_string(day);

	string query_show = "select * from " + tablename + " where time = \'" + time + "\'";
	mysql_query(&mysql, "set names gbk"); //设置编码格式（SET NAMES GBK也行），否则cmd下中文乱码  
	if (mysql_query(&mysql, query_show.c_str())) {        // 执行指定为一个空结尾的字符串的SQL查询。	
		cout << "Query failed" + (string)":" + (string)mysql_error(&mysql) << endl;
		return false;
	}
	if (!(ALL_RES = mysql_store_result(&mysql)))    //获得sql语句结束后返回的结果集  
	{
		printf("Couldn't get result from %s\n", mysql_error(&mysql));
		return false;
	}
	int num = GetColumnNum(tablename);
	while (ALL_COLUMN = mysql_fetch_row(ALL_RES))
	{
		for (int i = 0; i < num; i++)
		{
			cout << ALL_COLUMN[i] << "\t";
		}
	}
	cout << endl;
	return true;
}
