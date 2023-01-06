#include <iostream>  
using namespace std;
int main() {
	int choice;
	cout << "Enter 1, 2 or 3: ";
	cin >> choice;
	switch (choice)
	{
	case 1: 
	case 2: 
	case 3: 
		cout << "Choice 3\n"; break;
	default: 
		cout << "Not 1, 2 or 3\n"; break;
	}
}
