#include <Windows.h>
#include <WinSock.h>  //一定要包含这个 
#include <stdio.h>  
#include "include/mysql.h"    //引入mysql头文件(一种方式是在vc++目录里面设置，一种是文件夹拷到工程目录，然后这样包含)  

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
void GetToday(int& year,int& mon,int& day);
bool IsClockin(string crewname);//查询是否打卡
bool ClockIn(string crewname);//进行今日打卡
bool AddPerson(string managername, string crewname);//增加一个人员
bool Deduction(string managername);//二级表自动扣工时，配合一级表做映射
int main(int argc, char** argv)
{
	string s = "Nomel";
	ConnectDatabase();
	//ClockIn(s);
	//AddPerson("Nomel", "Kity_2");
	//QueryDatabase2();
	//DeleteData();
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
	const char psw[] = "123456";
	const char database[] = "test_cpp";
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
		printf("Connected...\n");
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

void GetToday(int& year,int& mon,int& day)
{
	time_t now = time(NULL);
	struct tm tm_t;
	localtime_s(&tm_t, &now);
	year = tm_t.tm_year + 1900;
	mon = tm_t.tm_mon + 1;
	day = tm_t.tm_mday;
}

bool Deduction(string managername)
{
	//先查表里面有多少字段
	int num = 0;
	string query_count = "select count(*) from information_schema.columns where table_name = " + (string)"\'" + managername + (string)"\'";
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

	//再查总表
	query = "select * from " + managername;//查询一级表所有被管理的人员，等会儿映射到二级表对应扣工时
	mysql_query(&mysql, "set names gbk"); //设置编码格式（SET NAMES GBK也行），否则cmd下中文乱码  
	//返回0 查询成功，返回1查询失败  
	if (mysql_query(&mysql, query.c_str())) {        // 执行指定为一个空结尾的字符串的SQL查询。	
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
		cout << "There is no data about " + managername + " inside the table!" << endl;
		return false;
	}
	
	MYSQL_ROW reserved;
	string query_tmp;
	while (column = mysql_fetch_row(res))		//对每一个名字映射去二级表，制作一条对应的查询语句，这个表的特性是只有一列且实时更新
	{
		for (int i = 0; i < num; i++) {

			query_tmp = "update gp set " + (string)(column[i]) + " = " + (string)(column[i]) + "-2";
			if (mysql_query(&mysql, query_tmp.c_str())) {        // 执行指定为一个空结尾的字符串的SQL查询。	
				cout << "Query failed" + (string)":" + (string)mysql_error(&mysql) << endl;
				return false;
			}
		}
	}
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
	//打印数据行数  
	//printf("number of dataline returned: %lld\n", mysql_affected_rows(&mysql));
	////获取字段的信息  
	//char* str_field[32];  //定义一个字符串数组存储字段信息(放在了常量区，不能直接被改变相当于前面默认了const)
	//for (int i = 0; i < 4; i++)   //在已知字段数量的情况下获取字段名 
	//{
	//	str_field[i] = mysql_fetch_field(res)->name;	//返回一个所有字段结构的数组。
	//}
	//for (int i = 0; i < 4; i++)   //打印字段  
	//		printf("%20s\t", str_field[i]);
	//printf("\n");
	////打印获取的数据 
	//由于fetch函数会读取res直到为空，所以colum出while时为空，要备份最后一条
	MYSQL_ROW reserved = column;
	while (column = mysql_fetch_row(res))   //获取最新一行 
	{
		//printf("%20s\t%20s\t%30s\t%12s\n", column[0], column[1], column[2], column[3]);//column是列数组
		reserved = column;
	}
	//对比当前时间判断是否打卡
	int year_now,mon_now, day_now;
	GetToday(year_now,mon_now, day_now);
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
	int year_now,mon_now, day_now;
	GetToday(year_now,mon_now, day_now);//获取当前日期
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
		//打卡成功就映射去二级表扣工时
		Deduction(managername);
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