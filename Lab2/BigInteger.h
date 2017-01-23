#pragma once
#include <vector>
using std::vector;
#include <ostream>
using std::ostream;
#include <istream>
using std::istream;
#include <string>
using std::string;

class BigInteger
{
public:
	const static BigInteger ZERO;
	const static BigInteger ONE;
	const static BigInteger TWO;

	//Generate a random BigInteger with the passed number of bits.
	static BigInteger random( uint32_t bits, bool positives_only = false );

	//Constructor from an unsigned 32 bit integer value. 
	BigInteger( uint32_t value = 0, bool negative = false );
	//Constructor from an unsigned 64 bit integer value.
	BigInteger( uint64_t value, bool negative = false );
	//Constructor from a signed 32 bit integer value.
	BigInteger( int32_t value );
	//Constructor from a signed 64 bit integer value.
	BigInteger( int64_t value );
	//Constructor from a hexadecimal string value.
	BigInteger( string value );

	double log( uint32_t base );
	inline bool even() const { return ( _bits[0] % 2 ) == 0; }
	inline bool odd() const { return ( _bits[0] % 2 ) != 0; }

	//Returns true/false if the object is negative/positive
	inline bool negative() const { return _negative; }
	//Sets the number to be negative or not
	inline void negative( bool neg ) { _negative = neg; }
	//Returns the number of bits allocated in the internal data structure aka the max value that 'bits_used' can be before the data structure needs to grow
	uint32_t bits_allocated() const;
	//Returns the smallest number of bits the stored number could fit within
	uint32_t bits_used() const;

	//Returns the absolute value of the number(sets it to true)
	BigInteger abs() const;
	//Returns the number raised to the passed power
	BigInteger pow( uint32_t power ) const;
	//Returns the number raised to the passed power
	BigInteger pow( const BigInteger& power ) const;
	//Returns integer division of *this / rhs. Allows you to catch the remainder if desired.
	//If you need both the quotient and the remainder, this is twice as efficient as using 
	//the division then modulus operator
	BigInteger divide( const BigInteger& rhs, BigInteger* remainder = nullptr ) const;

	//Returns true(1) or false(0) of the given bit index. 
	//Throws exception object if bit is >= bits_allocated()
	bool get_bit( uint32_t bit ) const;
	//Set the passsed bit index to true(1) or false(0). 
	//Throws exception object if bit is >= bits_allocated()
	void set_bit( uint32_t bit, bool value );
	//Switches the value of the passed bit index
	//Throws exception object if bit is >= bits_allocated()
	void toggle_bit( uint32_t bit );

	//Binary addition operator overload. Works the same as uint32_t's operator+
	BigInteger operator+( const BigInteger& rhs ) const;
	//Binary += operator overload. Works the same as uint32_t's operator+=
	BigInteger& operator+=( const BigInteger& rhs );
	//Unary prefix increment operator overload. Works the same as uint32_t's operator++
	BigInteger& operator++();
	//Unary postfix increment operator overload. Works the same as uint32_t's ++operator
	BigInteger operator++( int );

	//Unary negation operator overload. Works the same as uint32_t's -operator
	BigInteger operator-( const BigInteger& rhs ) const;
	//Binary subtraction operator overload. Works the same as uint32_t's operator-
	BigInteger operator-() const;
	//Binary -= operator overload. Works the same as uint32_t's operator-=
	BigInteger& operator-=( const BigInteger& rhs );
	//Unary prefix decrement operator overload. Works the same as uint32_t's --operator
	BigInteger& operator--();
	//Unary postfix decrement operator overload.Works the same as uint32_t's operator--
	BigInteger operator--( int kind );

	//Binary multiplication operator overload. Works the same as uint32_t's operator*
	BigInteger operator*( const BigInteger& rhs ) const;
	//Binary *= operator overload. Works the same as uint32_t's operator*=
	BigInteger& operator*=( const BigInteger& rhs );

	//Binary division operator overload. Works the same as uint32_t's operator/
	BigInteger operator/( const BigInteger& rhs ) const;
	//Binary /= operator overload. Works the same as uint32_t's operator/=
	BigInteger& operator/=( const BigInteger& rhs );

	BigInteger operator%( const BigInteger& rhs ) const;
	BigInteger& operator%=( const BigInteger& rhs );

	//Binary > operator overload. Works the same as uint32_t's operator>
	bool operator>( const BigInteger& rhs ) const;
	//Binary >= operator overload. Works the same as uint32_t's operator>=
	bool operator>=( const BigInteger& rhs ) const;

	//Binary < operator overload. Works the same as uint32_t's operator<
	bool operator<( const BigInteger& rhs ) const;
	//Binary <= operator overload. Works the same as uint32_t's operator<=
	bool operator<=( const BigInteger& rhs ) const;

	//Binary == operator overload. Works the same as uint32_t's operator==
	bool operator==( const BigInteger& rhs ) const;
	//Binary != operator overload. Works the same as uint32_t's operator!=
	bool operator!=( const BigInteger& rhs ) const;

	//Binary left shift operator overload. Works the same as uint32_t's operator<<
	BigInteger operator<<( uint32_t lshift ) const;
	//Binary <<= operator overload. Works the same as uint32_t's operator<<=
	BigInteger& operator<<=( uint32_t lshift );

	//Binary right shift operator overload. Works the same as uint32_t's operator>>
	BigInteger operator>>( uint32_t rshift ) const;
	//Binary >>= operator overload. Works the same as uint32_t's operator>>=
	BigInteger& operator>>=( uint32_t rshift );

	//Returns a hexadecimal string representation of the object
	string to_string( uint32_t base = 10 ) const;
	//Ostream overload, shortcut for outputting 'to_string()'
	friend ostream& operator<<( ostream& os, const BigInteger& rhs );
	//Istream overload, shortcut for reading in string constructor
	friend istream& operator>>( istream& is, BigInteger& rhs );

	uint32_t to_uint32() { return _bits[0]; }
	int32_t to_int32() { return _negative ? -(int32_t)_bits[0] : (int32_t)_bits[0]; }
	uint64_t to_uint64() { uint64_t r; memcpy( &r, &_bits[0], sizeof( uint64_t ) ); return r; }
	int64_t to_int64() { int64_t r; memcpy( &r, &_bits[0], sizeof( int64_t ) ); return _negative ? -r : r; }

	operator string() const { return to_string(); }

private:
	const static uint32_t bits_per_value = sizeof( uint32_t ) * 8;
	
	double log2();

	//Returns the index of the first set bit, starting from least significant bit working upwards
	uint32_t get_lowest_set_bit();

	BigInteger internal_add( const BigInteger& rhs ) const;
	BigInteger internal_sub( const BigInteger& rhs ) const;
	void trim();

	static inline vector<uint32_t>* bigger_array( const vector<uint32_t>& _1, const vector<uint32_t>& _2 );
	static inline vector<uint32_t>* smaller_array( const vector<uint32_t>& _1, const vector<uint32_t>& _2 );
	
	bool _negative = false;
	vector<uint32_t> _bits;
};