#include <iostream>
#include <windows.h> //needed for Sleep();

using namespace std;

int main()
{
    char name[16];
    cout << "Enter file name of game to load in CHIPmu\nEnsure the game is located in the same folder as the program" << endl;
    cin >> name;
    cout << name << " is selected game." << endl;
    Sleep(2000);
    return 0;
}
