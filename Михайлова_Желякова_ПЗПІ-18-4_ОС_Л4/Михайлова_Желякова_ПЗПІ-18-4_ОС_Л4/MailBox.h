#pragma once
#include <tchar.h>
#include <windows.h>

typedef struct _MailBox_Header
{
	unsigned length_of_mess;
	TCHAR* message;
} MailBox_Header, *PMailBox_Header;

class MailBox
{
public:
	HANDLE mail_handle;
	MailBox(TCHAR* path_to_mail,unsigned max_size_of_mail);
	void Add_Mess(TCHAR* message);
	PMailBox_Header Read_Mess(unsigned number_of_mess);
	void Delete_Mess(unsigned number_of_mess);
	unsigned Count_Mess();
	void Delete_All_Mess();
	void Info_Mail();
	unsigned Get_CSum(TCHAR* buffer, int size_buff);
	bool Check_CSum_Of_Mail();
	void Write_CSum();
	void Close_Mail();
	void Delete_Prev_CSum();
};
