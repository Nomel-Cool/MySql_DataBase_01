#include <Windows.h>
#include <WinSock.h>  //һ��Ҫ������� 
#include <stdio.h>  
#include "include/mysql.h"    //����mysqlͷ�ļ�(һ�ַ�ʽ����vc++Ŀ¼�������ã�һ�����ļ��п�������Ŀ¼��Ȼ����������)  

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
void GetToday(int& year,int& mon,int& day);
bool IsClockin(string crewname);//��ѯ�Ƿ��
bool ClockIn(string crewname);//���н��մ�
bool AddPerson(string managername, string crewname);//����һ����Ա
bool Deduction(string managername);//�������Զ��۹�ʱ�����һ������ӳ��
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
//�������ݿ�  
bool ConnectDatabase()
{
	//��ʼ��mysql  
	mysql_init(&mysql);  //����mysql�����ݿ�  
	const char host[] = "localhost";
	const char user[] = "root";
	const char psw[] = "123456";
	const char database[] = "test_cpp";
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
		printf("Connected...\n");
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
	//�Ȳ�������ж����ֶ�
	int num = 0;
	string query_count = "select count(*) from information_schema.columns where table_name = " + (string)"\'" + managername + (string)"\'";
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

	//�ٲ��ܱ�
	query = "select * from " + managername;//��ѯһ�������б��������Ա���Ȼ��ӳ�䵽�������Ӧ�۹�ʱ
	mysql_query(&mysql, "set names gbk"); //���ñ����ʽ��SET NAMES GBKҲ�У�������cmd����������  
	//����0 ��ѯ�ɹ�������1��ѯʧ��  
	if (mysql_query(&mysql, query.c_str())) {        // ִ��ָ��Ϊһ���ս�β���ַ�����SQL��ѯ��	
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
		cout << "There is no data about " + managername + " inside the table!" << endl;
		return false;
	}
	
	MYSQL_ROW reserved;
	string query_tmp;
	while (column = mysql_fetch_row(res))		//��ÿһ������ӳ��ȥ����������һ����Ӧ�Ĳ�ѯ��䣬������������ֻ��һ����ʵʱ����
	{
		for (int i = 0; i < num; i++) {

			query_tmp = "update gp set " + (string)(column[i]) + " = " + (string)(column[i]) + "-2";
			if (mysql_query(&mysql, query_tmp.c_str())) {        // ִ��ָ��Ϊһ���ս�β���ַ�����SQL��ѯ��	
				cout << "Query failed" + (string)":" + (string)mysql_error(&mysql) << endl;
				return false;
			}
		}
	}
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
	//��ӡ��������  
	//printf("number of dataline returned: %lld\n", mysql_affected_rows(&mysql));
	////��ȡ�ֶε���Ϣ  
	//char* str_field[32];  //����һ���ַ�������洢�ֶ���Ϣ(�����˳�����������ֱ�ӱ��ı��൱��ǰ��Ĭ����const)
	//for (int i = 0; i < 4; i++)   //����֪�ֶ�����������»�ȡ�ֶ��� 
	//{
	//	str_field[i] = mysql_fetch_field(res)->name;	//����һ�������ֶνṹ�����顣
	//}
	//for (int i = 0; i < 4; i++)   //��ӡ�ֶ�  
	//		printf("%20s\t", str_field[i]);
	//printf("\n");
	////��ӡ��ȡ������ 
	//����fetch�������ȡresֱ��Ϊ�գ�����colum��whileʱΪ�գ�Ҫ�������һ��
	MYSQL_ROW reserved = column;
	while (column = mysql_fetch_row(res))   //��ȡ����һ�� 
	{
		//printf("%20s\t%20s\t%30s\t%12s\n", column[0], column[1], column[2], column[3]);//column��������
		reserved = column;
	}
	//�Աȵ�ǰʱ���ж��Ƿ��
	int year_now,mon_now, day_now;
	GetToday(year_now,mon_now, day_now);
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
	int year_now,mon_now, day_now;
	GetToday(year_now,mon_now, day_now);//��ȡ��ǰ����
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
		//�򿨳ɹ���ӳ��ȥ������۹�ʱ
		Deduction(managername);
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