#include <Windows.h>
#include <WinSock.h>  //һ��Ҫ������� 
#include <stdio.h>  
#include "include/mysql.h"    //����mysqlͷ�ļ�(һ�ַ�ʽ����vc++Ŀ¼�������ã�һ�����ļ��п�������Ŀ¼��Ȼ����������)  

#include <algorithm>
#include <vector>
#include <string>
#include <time.h>
#include <iostream>
//�������������Ҳ�����ڹ���--������������  
#pragma comment(lib,"wsock32.lib") 
#pragma comment(lib,"libmysql.lib")

using namespace std;


MYSQL mysql; //mysql����
MYSQL_FIELD* fd;  //�ֶ�������
char field[32][32];  //���ֶ�����ά����
MYSQL_RES* res; //����ṹ�������е�һ����ѯ�����
MYSQL_ROW column; //һ�������ݵ����Ͱ�ȫ(type-safe)�ı�ʾ����ʾ�����е���
string query; //��ѯ���

//�������� 
bool ConnectDatabase();
void FreeConnect();
void GetToday(int& year,int& mon,int& day,int& hour,int& min,int& sec);//�������ڣ�������ʱ���룩
vector<string> GetColumnName(string tablename);//��ȡ��ǰ�������ֶ���
int GetColumnNum(string tablename);//��ȡ��ǰ�����ֶ�����
void AutoUpdate(string tablename);//ÿ�ո����ܱ�
bool IsClockin(string crewname);//��ѯ�Ƿ��
bool ClockIn(string crewname);//���н��մ�
bool ToClockIn(string managername, vector<string> abscentlist);//���մ�
bool AddPerson(string managername, string crewname);//����һ����Ա
bool Deduction(string managername,vector<string> columnname);//�������Զ��۹�ʱ�����һ������ӳ��
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
	if (!ConnectDatabase())return -1;
	AutoUpdate("gp");
	cout << "���������Ա���ƣ�";
	cin >> manager;
	cout << "������Ҫ��������/������򿨣�(Sat|Sun)��";
	cin >> table;
	cout << "������ȱϯ��Ա���ƣ��ÿո���� (û������Ϊȫ��)";
	char key_space = getchar();//��һ���س�
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








//�������ݿ�  
bool ConnectDatabase()
{
	//��ʼ��mysql  
	mysql_init(&mysql);  //����mysql�����ݿ�  
	const char host[] = "localhost";
	const char user[] = "root";
	const char psw[] = "bichang123";
	const char database[] = "BC_ClockIn";
	const int port = 3306;
	//����false������ʧ�ܣ�����true�����ӳɹ�  
	if (!(mysql_real_connect(&mysql, host, user, psw, database, port, NULL, 0)))
		//�м�ֱ����������û��������룬���ݿ������˿ںţ�����дĬ��0����3306�ȣ���������д�ɲ����ٴ���ȥ  
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
//�ͷ���Դ  
void FreeConnect()
{
	mysql_free_result(res);  //�ͷ�һ���������ʹ�õ��ڴ档
	mysql_close(&mysql);	 //�ر�һ�����������ӡ�
}

/***************************���ݿ����***********************************/
//��ʵ���е����ݿ����������д��sql��䣬Ȼ����mysql_query(&mysql,query)����ɣ������������ݿ�����ɾ�Ĳ�  
//��ѯ����  
//bool QueryDatabase2()
//{
//	mysql_query(&mysql, "set names gbk");
//	//����0 ��ѯ�ɹ�������1��ѯʧ��  
//	if (mysql_query(&mysql, "select * from user"))        //ִ��SQL���  
//	{
//		printf("Query failed (%s)\n", mysql_error(&mysql));
//		return false;
//	}
//	else
//	{
//		printf("query success\n");
//	}
//	res = mysql_store_result(&mysql);
//	//��ӡ��������  
//	printf("number of dataline returned: %d\n", mysql_affected_rows(&mysql));
//	for (int i = 0; fd = mysql_fetch_field(res); i++)  //��ȡ�ֶ���  
//		strcpy_s(field[i], fd->name);
//	int j = mysql_num_fields(res);  // ��ȡ����  
//	for (int i = 0; i < j; i++)  //��ӡ�ֶ�  
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
////ɾ������  
//bool DeleteData()
//{
//	/*sprintf(query, "delete from user where id=6");*/
//	char query[100];
//	printf("please input the sql:\n");
//	//gets(query);  //�����ֶ�����sql���  
//	if (mysql_query(&mysql, query))        //ִ��SQL���  
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
	//�Ȳ�������ж����ֶ�
	int num = 0;
	string query_count = "select count(*) from information_schema.columns where table_name = " + (string)"\'" + tablename + (string)"\'";
	if (mysql_query(&mysql, query_count.c_str())) {        // ִ��ָ��Ϊһ���ս�β���ַ�����SQL��ѯ��	
		cout << "Query failed" + (string)":" + (string)mysql_error(&mysql) << endl;
		return false;
	}
	if (!(res = mysql_store_result(&mysql)))    //���sql�������󷵻صĽ����  
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
	if (mysql_query(&mysql, query_colname.c_str())) {        // ִ��ָ��Ϊһ���ս�β���ַ�����SQL��ѯ��	
		cout << "Query failed" + (string)":" + (string)mysql_error(&mysql) << endl;
		return ret;
	}
	if (!(res = mysql_store_result(&mysql)))    //���sql�������󷵻صĽ����  
	{
		printf("Couldn't get result from %s\n", mysql_error(&mysql));
		return ret;
	}
	while (column = mysql_fetch_row(res))
	{
		ret.push_back(*column);
	}
	//����mysql��ԭ�������ֶλ�ŵ������ʾ����Ҫ�ֶ�������
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
	if (mysql_query(&mysql, isToday.c_str())) {        // ִ��ָ��Ϊһ���ս�β���ַ�����SQL��ѯ��	
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
	if (num != 0)return;	//���������˵���Ѿ����¹�ֱ���˳����� 
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
	if (mysql_query(&mysql, query_1.c_str())) {        // ִ��ָ��Ϊһ���ս�β���ַ�����SQL��ѯ��	
		cout << "Query failed" + (string)":" + (string)mysql_error(&mysql) << endl;
		return false;
	}
	if (!(res = mysql_store_result(&mysql)))    //���sql�������󷵻صĽ����  
	{
		printf("Couldn't get result from %s\n", mysql_error(&mysql));
		return false;
	}
	//��ȡ���µ�һ�����ڼ���ʱ
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
		//��ǿתchar*����atoiת���Ͳ�������
		query_deduct += columnname[i] + "=" + columnname[i] + "-" + to_string((atoi((char*)reserved[i]) * 2)) + " where time = '" + reserved[0] + (string)"\'";
		if (mysql_query(&mysql, query_deduct.c_str())) {        // ִ��ָ��Ϊһ���ս�β���ַ�����SQL��ѯ��	
			cout << "Query failed" + (string)":" + (string)mysql_error(&mysql) << endl;
			return false;
		}
	}
	return true;
}
bool ToClockIn(string managername, vector<string> abscentlist)
{
	//�Ȼ�ȡ��ʱ��
	int year, mon, day, hour, min, sec;
	GetToday(year, mon, day, hour, min, sec);
	string time = to_string(year) + "-" + to_string(mon) + "-" + to_string(day) + " " + to_string(hour) + ":" + to_string(min) + ":" + to_string(sec);


	//��������ѯ���
	vector<string> colname = GetColumnName(managername);
	string query_insert = "insert into " + managername + " (";
	for (int i = 0; i < colname.size() - 1; i++)//�������һ�������ֶ�����
	{
		query_insert += (colname[i] + ",");
	}
	query_insert += colname[colname.size() - 1];//���һ�䵥�����

	query_insert += ") values (\'" + time + "\',";
	for (int i = 1; i < colname.size() - 1; i++)//�������һ��Ҫ��β�����Ե�������-1
	{
		if (find(abscentlist.begin(), abscentlist.end(), colname[i]) == abscentlist.end())//��ϯ��
			query_insert += ("1,");
		else
			query_insert += ("0,");
	}
	if (find(abscentlist.begin(), abscentlist.end(), colname[colname.size() - 1]) == abscentlist.end())//��ϯ��
		query_insert += "1)";
	else
		query_insert += "0)";

	//�������������˰��ղ���ϯ��������������ֵ
	if (mysql_query(&mysql, query_insert.c_str())) {        // ִ��ָ��Ϊһ���ս�β���ַ�����SQL��ѯ��	
		cout << "Query failed" + (string)":" + (string)mysql_error(&mysql) << endl;
		return false;
	}
	Deduction(managername, colname);
	return true;
}

bool IsClockin(string crewname)
{
	query = "select * from crew where name = " + (string)"\'" + crewname + (string)"\'"; //ִ�в�ѯ��䣬�����ǲ�ѯ���У�user�Ǳ��������ü����ţ���strcpyҲ���� 
	mysql_query(&mysql, "set names gbk"); //���ñ����ʽ��SET NAMES GBKҲ�У�������cmd����������  
	//����0 ��ѯ�ɹ�������1��ѯʧ��  
	if (mysql_query(&mysql, query.c_str())){        // ִ��ָ��Ϊһ���ս�β���ַ�����SQL��ѯ��	
		cout << "Query failed" + (string)":" + (string)mysql_error(&mysql) << endl;
		return false;
	}
	//��ȡ�����  
	if (!(res = mysql_store_result(&mysql)))    //���sql�������󷵻صĽ����  
	{
		printf("Couldn't get result from %s\n", mysql_error(&mysql));
		return false;
	}
	//��ѯ�������
	if (mysql_affected_rows(&mysql) == 0)
	{
		cout << "There is no data about " + crewname + " inside the table!" << endl;
		return false;
	}
	MYSQL_ROW reserved = column;
	while (column = mysql_fetch_row(res))   //��ȡ����һ�� 
	{
		//printf("%20s\t%20s\t%30s\t%12s\n", column[0], column[1], column[2], column[3]);//column��������
		reserved = column;
	}
	//�Աȵ�ǰʱ���ж��Ƿ��
	int year_now, mon_now, day_now, hour_now, min_now, sec_now;
	GetToday(year_now, mon_now, day_now, hour_now, min_now, sec_now);
	int mon = (reserved[2][5] - 48) * 10 + (reserved[2][6] - 48);
	int day = (reserved[2][8] - 48) * 10 + (reserved[2][9] - 48);
	if ((mon == mon_now) && (day == day_now) && reserved[3])
	{
		//cout << crewname+"�����Ѵ�" << endl;
		return true;
	}
	//cout << crewname + "����δ��" << endl;
	return false;
}

bool ClockIn(string managername)
{
	//����Ƿ��Ѵ�
	if (IsClockin(managername))
	{
		cout << managername + "�����Ѵ򿨣������ٴ�����������Ҫ���ҹ���Ա" << endl;
		return false;
	}
	//δ�򿨾�ִ��Insertָ���crew�������һ������Ϣ
	int year_now, mon_now, day_now, hour_now, min_now, sec_now;
	GetToday(year_now, mon_now, day_now, hour_now, min_now, sec_now);//��ȡ��ǰ����
	query = "insert crew(name,time,clock_in) values (" + (string)"\'" + managername + (string)"\'" + "," + (string)"\'" + to_string(year_now)  + "-" +  to_string(mon_now) + "-" + to_string(day_now) + (string)"\'" + "," + "1)";
	mysql_query(&mysql, "set names gbk"); //���ñ����ʽ��SET NAMES GBKҲ�У�������cmd����������  
	//����0 ��ѯ�ɹ�������1��ѯʧ��  
	if (mysql_query(&mysql, query.c_str())) {        // ִ��ָ��Ϊһ���ս�β���ַ�����SQL��ѯ��	
		cout << "Query failed" + (string)":" + (string)mysql_error(&mysql) << endl;
		return false;
	}
	else
	{
		cout << managername + "�򿨳ɹ�" << endl;
	}
	return true;
}

//�䵱һ�������ڶ�����Ĺ�ʱ�۳�ӳ��
bool AddPerson(string managername ,string crewname)
{
	//ע�����������ֶ�Ϊnot null�������÷ǿ�Ĭ��ֵ������nullֵ���ֶ��޷�update�ģ������Ҫ���̸���������ֶεĻ���
	int count = 0;
	MYSQL_ROW reserved = column;
	string query_tmp = "select COLUMN_NAME from INFORMATION_SCHEMA.Columns where table_name= " + (string)"\'" + managername + (string)"\'";
	mysql_query(&mysql, "set names gbk");
	if (mysql_query(&mysql, query_tmp.c_str())) {        // ִ��ָ��Ϊһ���ս�β���ַ�����SQL��ѯ��	
		cout << "Query failed" + (string)":" + (string)mysql_error(&mysql) << endl;
		return false;
	}
	//��ȡ�������е��ֶ�����
	if (!(res = mysql_store_result(&mysql)))    //���sql�������󷵻صĽ����  
	{
		printf("Couldn't get result from %s\n", mysql_error(&mysql));
		return false;
	}
	//������Ƿ�Ϊ�գ���Ϊû�оͿ������
	while (column = mysql_fetch_row(res))   //��ȡ����һ�� 
	{
		count++;
		reserved = column;
	}
	string last = *reserved;	//��ȡ���һ��Ա���ı��
	int number = 0;
	//�������һ����Ա���ı��
	size_t pos = last.find("_");
	for (; pos<last.size()-1; pos++)
	{
		number = number * 10 + last[pos + 1] - '0';
	}
	//���һ��Ա��������
	query = "alter table " + managername + " add p_" + to_string(number + 1) + " varchar(12) not null default \"HELLO\"";
	if (mysql_query(&mysql, query.c_str())) {        // ִ��ָ��Ϊһ���ս�β���ַ�����SQL��ѯ��	
		cout << "Query failed" + (string)":" + (string)mysql_error(&mysql) << endl;
		return false;
	}
	string query1 = "update " + managername + " set p_" + to_string(number + 1) + "= " + (string)"\'" + crewname + (string)"\'";
	if (mysql_query(&mysql, query1.c_str())) {        // ִ��ָ��Ϊһ���ս�β���ַ�����SQL��ѯ��	
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
	mysql_query(&mysql, "set names gbk"); //���ñ����ʽ��SET NAMES GBKҲ�У�������cmd����������  
	if (mysql_query(&mysql, query_show.c_str())) {        // ִ��ָ��Ϊһ���ս�β���ַ�����SQL��ѯ��	
		cout << "Query failed" + (string)":" + (string)mysql_error(&mysql) << endl;
		return false;
	}
	if (!(ALL_RES = mysql_store_result(&mysql)))    //���sql�������󷵻صĽ����  
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