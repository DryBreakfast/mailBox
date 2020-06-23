#include <iostream>
#include "MailBox.h"
#include "windows.h"
#include <string>
#include <cstdio>
#include <list>
using namespace std;

MailBox::MailBox(TCHAR* path_to_mail,unsigned max_size_of_mail)
{
	DWORD dwCount;
	unsigned size = 0;
	mail_handle = CreateFile(path_to_mail, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);

	if (mail_handle == INVALID_HANDLE_VALUE)
	{
		mail_handle = CreateFile(path_to_mail, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_NEW, 0, 0);
		if (mail_handle != INVALID_HANDLE_VALUE)
		{
			WriteFile(mail_handle, &size, sizeof(unsigned), &dwCount, 0);
			WriteFile(mail_handle, &size, sizeof(unsigned), &dwCount, 0);
			WriteFile(mail_handle, &max_size_of_mail, sizeof(unsigned), &dwCount, 0);
		}
	}
}

void MailBox::Add_Mess(TCHAR* message)
{
	DWORD number_of_bytes_written;
	DWORD file_size_high = 0;
	long distance_to_move_high = 0;

	unsigned max_size_of_mail=0;
	unsigned current_size_of_mess=0;
	
	unsigned current_file_size = GetFileSize(mail_handle, &file_size_high);
	unsigned size_of_mess = _tcslen(message) * sizeof(TCHAR);

	SetFilePointer(mail_handle, sizeof(unsigned)*2, &distance_to_move_high, FILE_BEGIN);
	ReadFile(mail_handle, &max_size_of_mail, sizeof(unsigned), &number_of_bytes_written, 0);

	if (current_file_size + size_of_mess + sizeof(unsigned) > max_size_of_mail) {
		cout << "Maximum size of mail is reached\n";
		return;
	}

	SetFilePointer(mail_handle, 0, &distance_to_move_high, FILE_END);
	WriteFile(mail_handle, &size_of_mess, sizeof(unsigned), &number_of_bytes_written, 0);
	WriteFile(mail_handle, message, size_of_mess, &number_of_bytes_written, 0);

	unsigned num_of_mess = Count_Mess();
	num_of_mess++;

	SetFilePointer(mail_handle, 0, &distance_to_move_high, FILE_BEGIN);
	WriteFile(mail_handle, &num_of_mess, sizeof(unsigned), &number_of_bytes_written, 0);

	ReadFile(mail_handle, &current_size_of_mess, sizeof(unsigned), &number_of_bytes_written, 0);
	size_of_mess += current_size_of_mess;
	SetFilePointer(mail_handle, sizeof(unsigned), &distance_to_move_high, FILE_BEGIN);
	WriteFile(mail_handle, &size_of_mess, sizeof(unsigned), &number_of_bytes_written, 0);
}

unsigned MailBox::Count_Mess()
{
	DWORD numbers_of_bytes_written;
	long file_size_high = 0;

	unsigned result=0;

	SetFilePointer(mail_handle, 0, &file_size_high, FILE_BEGIN);
	ReadFile(mail_handle, &result, sizeof(unsigned), &numbers_of_bytes_written, 0);

	return result;
}

PMailBox_Header MailBox::Read_Mess(unsigned number_of_mess)
{
	DWORD numbers_of_bytes_written;
	long distance_to_move_high = 0;

	PMailBox_Header result = 0;
	
	unsigned size_of_mess;
	unsigned i = 0;

	if (number_of_mess < Count_Mess() && number_of_mess >= 0)
	{
		SetFilePointer(mail_handle, sizeof(unsigned)*3, &distance_to_move_high, FILE_BEGIN);

		while (i < number_of_mess)
		{
			ReadFile(mail_handle, &size_of_mess, sizeof(unsigned), &numbers_of_bytes_written, 0);
			SetFilePointer(mail_handle, size_of_mess, &distance_to_move_high, FILE_CURRENT);
			++i;
		}

		ReadFile(mail_handle, &size_of_mess, sizeof(unsigned), &numbers_of_bytes_written, 0);

		TCHAR* mess = new TCHAR[size_of_mess / sizeof(TCHAR) + 1];
		memset(mess, 0, size_of_mess + sizeof(TCHAR));

		ReadFile(mail_handle, mess, size_of_mess, &numbers_of_bytes_written, 0);

		result = new MailBox_Header();
		result->message = mess;
		result->length_of_mess = size_of_mess / sizeof(TCHAR);
	}
	return result;
}

void MailBox::Delete_Mess(unsigned number_of_mess)
{
	DWORD numbers_of_bytes_written;
	DWORD file_size_high = 0;
	long distance_to_move_high = 0;
	
	unsigned current_file_size = GetFileSize(mail_handle, &file_size_high);
	int bytes_before_del = 0;
	int bytes_after_del = 0;
	unsigned bytes_of_mess;
	unsigned new_file_size = 0;
	unsigned i = 0;
	
	if (number_of_mess < Count_Mess() && number_of_mess >= 0)
	{
		bytes_before_del += sizeof(unsigned) * 3;
		SetFilePointer(mail_handle, sizeof(unsigned) * 3, &distance_to_move_high, FILE_BEGIN);
		
		while (i <= number_of_mess)
		{
			ReadFile(mail_handle, &bytes_of_mess, sizeof(unsigned), &numbers_of_bytes_written, 0);
			SetFilePointer(mail_handle, bytes_of_mess, &distance_to_move_high, FILE_CURRENT);

			if (i != number_of_mess)
			{
				bytes_before_del += sizeof(unsigned);
				bytes_before_del += bytes_of_mess;
			}
			else
			{
				bytes_after_del += sizeof(unsigned);
				bytes_after_del += bytes_of_mess;
			}
			i++;
		}

		byte* mess_after_dell = new byte[current_file_size - bytes_before_del - bytes_after_del];
		ReadFile(mail_handle, mess_after_dell, current_file_size - bytes_before_del - bytes_after_del, &numbers_of_bytes_written, 0);
		SetFilePointer(mail_handle, bytes_before_del, &distance_to_move_high, FILE_BEGIN);
		SetEndOfFile(mail_handle);

		WriteFile(mail_handle, mess_after_dell, current_file_size - bytes_before_del - bytes_after_del, &numbers_of_bytes_written, 0);

		unsigned num_of_mess = Count_Mess();
		--num_of_mess;
		SetFilePointer(mail_handle, 0, &distance_to_move_high, FILE_BEGIN);
		WriteFile(mail_handle, &num_of_mess, sizeof(unsigned), &numbers_of_bytes_written, 0);

		ReadFile(mail_handle, &new_file_size, sizeof(unsigned), &numbers_of_bytes_written, 0);
		new_file_size -= bytes_after_del - sizeof(unsigned);
		SetFilePointer(mail_handle, sizeof(unsigned), &distance_to_move_high, FILE_BEGIN);
		WriteFile(mail_handle, &new_file_size, sizeof(unsigned), &numbers_of_bytes_written, 0);
	}
}

void MailBox::Delete_All_Mess()
{
	DWORD numbers_of_bytes_written;
	long distance_to_move_high = 0;

	unsigned current_file_size = 0;

	SetFilePointer(mail_handle, 0, &distance_to_move_high, FILE_BEGIN);
	WriteFile(mail_handle, &current_file_size, sizeof(unsigned), &numbers_of_bytes_written, 0);
	WriteFile(mail_handle, &current_file_size, sizeof(unsigned), &numbers_of_bytes_written, 0);
	SetFilePointer(mail_handle, sizeof(unsigned), &distance_to_move_high, FILE_CURRENT);
	SetEndOfFile(mail_handle);
}

void MailBox::Info_Mail()
{
	DWORD file_size_high = 0;
	DWORD numbers_of_bytes_written;
	long distance_to_move_high = 0;
	
	unsigned max_size_of_mail = 0;
	unsigned num_of_mess = 0;
	unsigned current_file_size = GetFileSize(mail_handle, &file_size_high);

	SetFilePointer(mail_handle, 0, &distance_to_move_high, FILE_BEGIN);
	ReadFile(mail_handle, &num_of_mess, sizeof(unsigned), &numbers_of_bytes_written, 0);
	SetFilePointer(mail_handle, sizeof(unsigned), &distance_to_move_high, FILE_CURRENT);
	ReadFile(mail_handle, &max_size_of_mail, sizeof(unsigned), &numbers_of_bytes_written, 0);
	cout << "The number of messages -> " << num_of_mess << endl << "Using mail space -> " << current_file_size << "/" << max_size_of_mail << endl<<endl;
}



unsigned MailBox::Get_CSum(TCHAR* buffer, int size_buff)
{
	unsigned long crc_matrix[256];
	unsigned long crc;

	for(int i=0; i<256; ++i) {
		crc=i;
		for(int j=0; j<8; ++j) {
			if (crc & 1)
				crc=(crc >> 1) ^ 0xedb88320UL;
			else
				crc=crc >> 1;
		}
		crc_matrix[i] = crc;
	}

	while (size_buff--) {
		if (size_buff==3)
			crc = crc;
		crc=crc_matrix[(crc ^ *buffer++) & 0xff] ^ (crc >> 8);
	}
	return crc ^ 0xffffffffUL;
}

bool MailBox::Check_CSum_Of_Mail()
{
	DWORD numbers_of_bytes_written;
	long distance_to_move_high = 0;
	DWORD file_size_high = 0;
	unsigned file_size = 0;
	file_size = GetFileSize(mail_handle, &file_size_high) - 4;

	TCHAR* file_content = new TCHAR[file_size];
	SetFilePointer(mail_handle, 0, &distance_to_move_high, FILE_BEGIN);
	ReadFile(mail_handle, file_content, file_size, &numbers_of_bytes_written, 0);

	unsigned current_CSum = Get_CSum(file_content, file_size);
	unsigned prev_CSum;
	ReadFile(mail_handle, &prev_CSum, sizeof(unsigned), &numbers_of_bytes_written, 0);
	return current_CSum == prev_CSum;
}


void MailBox::Write_CSum()
{
	DWORD numbers_of_bytes_written;
	long distance_to_move_high = 0;

	unsigned size_of_mails = 0;
	SetFilePointer(mail_handle, sizeof(unsigned), &distance_to_move_high, FILE_BEGIN);
	ReadFile(mail_handle, &size_of_mails, sizeof(unsigned), &numbers_of_bytes_written, 0);

	unsigned current_file_size = sizeof(unsigned) * 3 + Count_Mess() * sizeof(unsigned) + size_of_mails;

	TCHAR* file_content = new TCHAR[current_file_size];
	SetFilePointer(mail_handle, 0, &distance_to_move_high, FILE_BEGIN);
	ReadFile(mail_handle, file_content, current_file_size, &numbers_of_bytes_written, 0);

	unsigned current_CSum = Get_CSum(file_content, current_file_size);
	WriteFile(mail_handle, &current_CSum, sizeof(unsigned), &numbers_of_bytes_written, 0);
	SetEndOfFile(mail_handle);
}

void MailBox::Close_Mail()
{
	Write_CSum();
	CloseHandle(mail_handle);
}

void MailBox::Delete_Prev_CSum()
{
	long distance_to_move_high = 0;
	DWORD file_size_high = 0;

	unsigned current_file_size = GetFileSize(mail_handle, &file_size_high)-4;
	SetFilePointer(mail_handle, current_file_size, &distance_to_move_high, FILE_BEGIN);
	SetEndOfFile(mail_handle);
}

void menu(string path_catalog) {

	int choice = 0;
	
	int num;
	int count = 1;
	
	WIN32_FIND_DATA search;
	MailBox* mail = 0;
	HANDLE hFind = 0;
	
	string path_file, search_link, name, del_name;
	string mess = "";
	
	list<string> name_list;
	unsigned max_size_of_mail = 0;

	while (choice != 12)
	{
		cout << "\n-------------------Menu-------------------\n\n1 - Create mailbox\n2 - Change mailbox\n3 - The number of messages in the mailbox\n4 - Read message without deleting\n5 - Read message with deleting\n6 - Add message\n7 - Delete message\n8 - Delete all messages\n9 - The total number of mailboxes\n10 - Delete mailbox\n11 - Information about active mailbox\n12 - Exit\n\n";
		if(name == "" || del_name == name)
			cout << "There is no active mailbox\n\nEnter the number -> ";
		else
			cout << name << " is active\n\nEnter the number -> ";
		cin >> choice;
		string m1, m2;
		switch (choice) {
		case 1:
			if (mail != 0) {
				mail->Delete_Prev_CSum();
				mail->Close_Mail();
			}
			cout << "Enter the size of mailbox -> ";
			cin>>max_size_of_mail;
			cout << "Enter the name of mailbox -> ";
			cin >> m1;
			getline(cin, m2);
			name = m1 + m2;
			name += ".txt";
			path_file = path_catalog;
			path_file += name;
			mail = new MailBox((TCHAR*)(path_file.c_str()),max_size_of_mail);
			mail->Write_CSum();
			cout << "Created\n";
			break;
		case 2:
			name_list = {};
			search_link = path_catalog;
			search_link += "*.txt";
			hFind = FindFirstFile(search_link.c_str(), &search);
			if (hFind == INVALID_HANDLE_VALUE) {
				cout << "\nThere are no mailboxes\n";
				break;
			}
				do {
					if (!(search.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY) && !(search.dwFileAttributes == FILE_ATTRIBUTE_DEVICE))
						name_list.push_back(search.cFileName);
				} while (FindNextFile(hFind, &search) != NULL);
			
			FindClose(hFind);
			count = 1;
			for (string el : name_list) 
			{
				cout << count << " - " << el<<endl;
				count++;
			}

			cout << "Enter the number of mailbox -> ";
			cin >> num;
			path_file = path_catalog;
			count = 0;
			for (string el : name_list)
			{
				if (num == count + 1) {
					path_file += el;
					name = el;
					break;
				}
				else ++count;
			}

			if (mail != 0) {
				mail->Delete_Prev_CSum();
				mail->Close_Mail();
			}

			mail = new MailBox((TCHAR*)(path_file.c_str()), max_size_of_mail);
			if (!mail->Check_CSum_Of_Mail()) {
				CloseHandle(mail->mail_handle);
				mail = 0;
				cout << "\nIntegrity of " << name << " is broken\n";
				name = "";
			}
			else	
			cout << "Mailbox " << name << " is active\n";
			break;
		case 3:
			if (mail != 0)
				cout << "The number of messages ->" << mail->Count_Mess() << endl;
			else cout << "Firstly create a mailbox\n";
			break;
		case 4:
			if (mail != 0) {
				cout << "Enter the number of message -> ";
				cin >> num;
				cout << "Message is:\n" << mail->Read_Mess(num - 1)->message << endl;
			}
			else cout << "Firstly create or choose a mailbox\n";
			break;
		case 5:
			if (mail != 0) {
				cout << "Enter the number of message -> ";
				cin >> num;
				cout << "Message is:\n" << mail->Read_Mess(num - 1)->message << endl;
				mail->Delete_Prev_CSum();
				mail->Delete_Mess(num - 1);
				mail->Write_CSum();
			}
			else cout << "Firstly create or choose a mailbox\n";
			break;
		case 6:
			if (mail != 0) {
				cout << "Enter the message -> ";
				cin >> m1;
				getline(cin, m2);
				mess = m1 + m2;
				mail->Delete_Prev_CSum();
				mail->Add_Mess((TCHAR*)(mess.c_str()));
				mail->Write_CSum();
			}
			else cout << "Firstly create or choose a mailbox\n";
			break;
		case 7:
			if (mail != 0) {
				cout << "Enter the number of message -> ";
				cin >> num;
				mail->Delete_Prev_CSum();
				mail->Delete_Mess(num - 1);
				mail->Write_CSum();
				cout << "Message is deleted\n" << endl;
			}
			else cout << "Firstly create or choose a mailbox\n";
			break;
		case 8:
			if (mail != 0) {
				mail->Delete_All_Mess();
				mail->Write_CSum();
			}
			else cout << "Firstly create or choose a mailbox\n";
			break;
		case 9:
			count = 0;
			name_list = {};
			search_link = path_catalog;
			search_link += "*.txt";
			hFind = FindFirstFile(search_link.c_str(), &search);
			if (hFind == INVALID_HANDLE_VALUE) {
				cout << "\nThere are no mailboxes\n";
				break;
			}
				do{
					if (!(search.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY) && !(search.dwFileAttributes == FILE_ATTRIBUTE_DEVICE))
						++count;
				} while (FindNextFile(hFind, &search) != NULL);
			
			FindClose(hFind);
			cout << "The total number of mailboxes -> " << count << endl;
			break;
		case 10:
			name_list = {};
			search_link = path_catalog;
			search_link += "*.txt";
			hFind = FindFirstFile(search_link.c_str(), &search);
			if (hFind == INVALID_HANDLE_VALUE) {
				cout << "\nThere are no mailboxes\n";
				break;
			}
				do {
					if (!(search.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY) && !(search.dwFileAttributes == FILE_ATTRIBUTE_DEVICE)) {
						name_list.push_back(search.cFileName);
					}
				} while (FindNextFile(hFind, &search) != NULL);
			
			FindClose(hFind);

			count = 1;
			for (string el : name_list)
			{
				cout << count << " - " << el << endl;
				count++;
			}
			
			cout << "Enter the number of mailbox -> ";
			cin >> num;
			path_file = path_catalog;

			count = 0;
			for (string el : name_list)
			{
				if (num == count + 1) {
					path_file += el;
					del_name = el;
					break;
				}
				else count++;
			}

			search_link = path_catalog;
			search_link += del_name;
			if (name == del_name) {
				CloseHandle(mail->mail_handle);
				mail = 0;
			}

			remove(search_link.c_str());
			break;
		case 11:
			if (mail != 0) {
				cout << "\nThe name of mailbox -> " << name << endl;
				mail->Info_Mail();
			}
			else cout << "Firstly create or choose a mailbox\n";
			break;
		default: break;
		}
	} 

	if (mail != 0) {
		mail->Delete_Prev_CSum();
		mail->Close_Mail();
	}
}


int main(int argc, char** argv)
{
	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);
	setlocale(LC_ALL, "ukrainian");
	string path_catalog = (string)argv[1] + "MailCatalog\\";
	if (!CreateDirectory(path_catalog.c_str(), NULL))
		cout << "Catalog is already created\n";
	else cout << "Catalog is created\n";

	menu(path_catalog);
}