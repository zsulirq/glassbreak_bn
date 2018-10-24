#include <stdio.h>  
#include <string.h>  
#include <string>  
#include <vector>  
#include <iostream>  
#include <algorithm>  
using namespace std;  
  
#define USR_NUM 5  
#define MAX_OPEN 5  
#define MAX_FILE_NUM 50  
#define for_each(x,y) for((x) = (y).begin(); (x) != (y).end(); (x)++)  
#define find_elem(x,y) find((x).begin(), (x).end(), y);  
string usrs[] = {"john","jim","tom","jialrs","kate"};  
  
struct mfd_item  
{  
    string usr_name;  
    int dir_pt;  
};  
  
mfd_item mfds[USR_NUM];  
  
struct dir_item  
{  
    dir_item(string name, int f_pt, string pri)  
        {  
            file_name = name;  
            file_pt = f_pt;  
            privilage = pri;  
        }  
    string file_name;  
    int state;  
    int file_pt;  
    string privilage;  
};  
vector<dir_item> ufds[USR_NUM];  
vector<string> opened[USR_NUM];  
string files[MAX_FILE_NUM];  
bool used[MAX_FILE_NUM];  
int file_cnt = 0;  
  
string usr;  
int usr_id;  
  
string get()  
{  
    string file_name;  
    cout << "Input file name:";  
    cin >> file_name;  
    return file_name;  
}  
  
vector<dir_item>::iterator find_usr_file(string file_name)  
{  
    vector<dir_item>::iterator it;  
    for_each(it, ufds[usr_id])  
        if( it->file_name == file_name)  
            break;  
    return it;  
}  
  
void m_create()  
{  
    if( file_cnt >= MAX_FILE_NUM)  
    {  
        cout << " Has no space for you !\n";  
        return;  
    }  
    string file_name, privilage;  
    int pt;  
    file_name = get();  
    cout << "privlage:";  
    cin >> privilage;  
    for(int i = 0; i < MAX_FILE_NUM; i++)  
        if( !used[i])  
        {  
            pt = i;  
            file_cnt ++;  
            break;  
        }  
    ufds[usr_id].push_back(dir_item(file_name, pt, privilage));  
    cout << "Successfully add new file " << file_name << endl;  
}  
  
void m_del()  
{  
    string file_name = get();  
    vector<dir_item>::iterator it = find_usr_file(file_name);  
    if(it == ufds[usr_id].end())  
    {  
        cout << "You has no file !" << endl;  
        return;  
    }  
    if(it->privilage[1] != 'w')  
    {  
        cout << "You has no privilage to delete the file !" << endl;  
        return;  
    }  
    char ch = 'n';  
    cout << "Do you really want to delete the file ?(y/n):";  
    cin >> ch;  
    if( ch == 'y')  
    {  
        used[it->file_pt] = false;  
        files[it->file_pt] = "";  
        ufds[usr_id].erase(it);  
        cout << "Delete successful !" << endl;  
    }  
}  
  
void m_open()  
{  
    string file_name = get();  
    vector<dir_item>::iterator it = find_usr_file(file_name);  
    if(it == ufds[usr_id].end())  
    {  
        cout << "You has no file !" << endl;  
        return;  
    }  
    vector<string>::iterator it2;  
    it2 = find_elem(opened[usr_id], file_name);  
    if(it2 != opened[usr_id].end())  
    {  
        cout << "Already opened !" << endl;  
        return;  
    }  
    opened[usr_id].push_back(file_name);  
    cout << "Open successful !" << endl;  
}  
  
void m_close()  
{  
    string file_name = get();  
    vector<string>::iterator it;  
    it = find_elem(opened[usr_id],file_name);  
    if(it != opened[usr_id].end())  
    {  
        opened[usr_id].erase(it);  
        cout << "Closed successful !\n";  
        return;  
    }  
    cout << "You have not opened the file !\n";  
}  
  
void m_read()  
{  
    string file_name = get();  
    vector<dir_item>::iterator it = find_usr_file(file_name);  
    if(it == ufds[usr_id].end())  
    {  
        cout << "You has no file !" << endl;  
        return;  
    }  
    if(it->privilage[0] == 'r')  
    {  
        cout << file_name << "as follow : " << endl;  
        cout << files[it->file_pt] << endl;  
    }  
    else  
        cout << "You has no privilage to read !";  
}  
  
void m_write()  
{  
    string file_name = get();  
    vector<dir_item>::iterator it = find_usr_file(file_name);  
    if(it == ufds[usr_id].end())  
    {  
        cout << "You has no file !" << endl;  
        return;  
    }  
    if(it->privilage[1] == 'w')  
    {  
        cout << file_name << "Origin data is as follow : " << endl;  
        cout << files[it->file_pt] << endl;  
        cout << "Write new data:\n";  
        cin >> files[it->file_pt];  
    }  
    else  
        cout << "You has no privilage to write !";  
}  
string f[] = {"create", "delete", "open", "close", "read", "write"};  
void (*funcs[])() = {m_create, m_del, m_open, m_close, m_read, m_write};  
  
void init()  
{  
    memset(used, 0, sizeof(bool)*MAX_FILE_NUM);  
    for(int i = 0; i < USR_NUM; i++)  
    {  
        mfds[i].usr_name = usrs[i];  
        mfds[i].dir_pt = i;  
    }  
}  
  
int find_usr(string usr)  
{  
    for(int i = 0; i < USR_NUM; i++)  
        if(usr == usrs[i])  
            return i;  
    return -1;  
}  
  
int main()  
{  
    init();  
  lable1:  
    while(true)  
    {      
        cout << "Login : ";  
        cin >> usr;  
        usr_id = find_usr(usr);  
        if( usr_id == -1)  
            cout << "Invailed usr name !\n";  
        else  
            break;  
    }  
    string command;  
    cout << "command usage :\n"  
         << "  create for make a new file;\n"  
         << "  delete for delete a file;\n"  
         << "  open   for open a file;\n"  
         << "  close  for close a opeed file;\n"  
         << "  write  for write data into a file;\n"  
         << "  read   for read data form file;\n"  
         << "  logout for change user;\n"  
         << "  quit   for quit.\n";  
    while(true)  
    {  
        cout << usr << " $ ";  
        cin >> command;  
        int i = 0;  
        for( ; i < 6; i++)  
            if( f[i] == command)  
                break;  
        if( i < 6)  
            funcs[i]();  
        else if (command == "logout")  
            goto lable1;  
        else if (command == "quit")  
            break;  
        else  
            cout << "unknown command!\n";  
    }  
}  