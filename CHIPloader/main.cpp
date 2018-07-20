#include <iostream>
#include <fstream>
#include <windows.h> //needed for Sleep();

using namespace std;

fstream ofs;

int main()
{
    char name[16];
    cout << "Enter file name of game to load in EMU-8\nEnsure the game is located in the same folder as the program\nEnsure game has .bin file extension" << endl;
    cin >> name;
    cout << name << " is selected game." << endl;
    ofs.open("temp.txt", ios::out | ios::trunc);
    ofs << name << ".bin";
    ofs.close();
    Sleep(2000);
    return 0;
}
