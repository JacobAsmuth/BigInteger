#include "BigInteger.h"
#include <iostream>
using std::cout;
using std::cin;
#include <assert.h>
#include <string>
using std::string;
#include <math.h>

int main()
{
	BigInteger a( "70000000" );



	BigInteger _1, _2;
	uint32_t choice;

	cout << "Enter a number(hexadecimal). \n";
	cin >> _1;

	cout << "Now enter another one(hexadecimal).\n";
	cin >> _2;

	while( true )
	{
		cout << "What would you like to do?\n"
			<< "1. Compare the values\n"
			<< "2. Add the values\n"
			<< "3. Subtract the second from the first\n"
			<< "4. Multiply the values\n"
			<< "5. Divide the values\n"
			<< "6. Modulus the values\n"
			<< "7. Left shift the first value\n"
			<< "8. Right shift the first value\n"
			<< "9. See information about values\n";

		cin >> choice;

		switch( choice )
		{
		case 1:
			if( _1 < _2 )
				cout << "Value 1 is less than value 2.\n";
			else if( _1 == _2 )
				cout << "Value 1 is equal to value 2.\n";
			else
				cout << "Value 1 is greater than value 2.\n";
			break;
		case 2:
			cout << _1 + _2 << '\n'; break;
		case 3:
			cout << _2 - _1 << '\n'; break;
		case 4:
			cout << _1 * _2 << '\n'; break;
		case 5:
			cout << _1 / _2 << '\n'; break;
		case 6:
			cout << _1 % _2 << '\n'; break;
		case 7:
			cout << ( _1 << 1 ) << '\n'; break;
		case 8:
			cout << ( _1 >> 1 ) << '\n'; break;
		case 9:
			cout << "The first number is " << _1.bits_used() << " bits long and has " << (uint32_t)_1.log( 10 ) + 1 << " decimal digits.\n";
			cout << "The second number is " << _2.bits_used() << " bits long and has " << (uint32_t)_2.log( 10 ) + 1 << " decimal digits.\n";
			break;
		default:
			cout << "That's not an option. Please enter the number of the option you'd like to choose.\n";
		}

		cout << "Now what would you like to do?\n"
			<< "1. Perform another operation\n"
			<< "2. Change the first value\n"
			<< "3. Change the second value\n"
			<< "4. Quit\n";
		cin >> choice;

		system( "CLS" );
		switch( choice )
		{
		case 1:
			continue;
		case 2:
			cout << "Enter the hexadecimal value now\n";
			cin >> _1;
			continue;
		case 3:
			cout << "Enter the hexadecimal value now\n";
			cin >> _2;
			continue;
		case 4:
			return 0;
		}
	}
	return 0;
}