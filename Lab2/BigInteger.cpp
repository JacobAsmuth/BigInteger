#include "BigInteger.h"
#include <sstream>
using std::stringstream;
#include <algorithm>
using std::max;
#include <iomanip>
using std::setfill;
using std::setw;
using std::hex;
using std::uppercase;
#include <assert.h>
#include <exception>
using std::exception;
#include <random>
using std::random_device;
#include <math.h>
using std::ceil;
using std::min;

const BigInteger BigInteger::ZERO = 0;
const BigInteger BigInteger::ONE = 1;
const BigInteger BigInteger::TWO = 2;

BigInteger BigInteger::random( uint32_t bits, bool positives_only )
{
	random_device rd;
	uint32_t extra = bits % bits_per_value;
	uint32_t indexes = bits / bits_per_value;

	BigInteger res;
	res._bits.resize( indexes );

	if( !positives_only )
		res._negative = ( rd() & 1 ); //randomly true or false

	for( uint32_t& val : res._bits )
		val = rd(); //fill each index with random bits

	if( extra ) //we need a few bits to top us off
		res._bits.push_back( ( rd() >> ( bits_per_value - extra ) ) );

	return res;
}

BigInteger::BigInteger( uint32_t value, bool negative ): _negative( negative ), _bits( 1, value ) {}

BigInteger::BigInteger( uint64_t value, bool negative ): _negative( negative ), _bits( 2 )
{
	uint32_t* p = (uint32_t*)( &value ); //pointer gymnastics!
	_bits[0] = p[0];
	_bits[1] = p[1];
}

BigInteger::BigInteger( int32_t value ): _negative( value < 0 ), _bits( 1, std::abs( value ) ) {}

BigInteger::BigInteger( int64_t value ): _bits( 2 )
{
	if( value < 0 )
	{
		_negative = true;
		value = -value;
		uint32_t* p = (uint32_t*)( &value );
		_bits[0] = p[0];
		_bits[1] = p[1];
	}
	else
	{
		_negative = false;
		uint32_t* p = (uint32_t*)( &value );
		_bits[0] = p[0];
		_bits[1] = p[1];
	}
}

BigInteger::BigInteger( string value )
{
	try
	{
		if( value[0] == '-' )
		{
			_negative = true;
			value.erase( 0, 1 );
		}

		int cur_index = 0;
		for( int32_t i = (int32_t)value.length() - 1; i >= 0; i -= 8 )
		{
			int lower_idx = max( i - 7, 0 ); //grab at most the next down 8 characters. less if we hit our 0 index.
			string sub = value.substr( lower_idx, i - lower_idx + 1 );

			if( sub.length() == 0 )
				break;

			unsigned long v = std::stoul( sub, 0, 16 );

			//I promise these types have equivalent ranges :)
			assert( v == (uint32_t)v );

			_bits.push_back( (uint32_t)v );
		}
		trim();
	}
	catch( ... )
	{
		_bits.clear();
		_bits.push_back( 0 );
		throw exception( "Invalid hex number" );
	}
}

double BigInteger::log( uint32_t base )
{
	if( base <= 1 )
		throw exception( "Invalid logarithm base" );
	if( *this == ONE )
		return 0;
	if( base == 2 )
		return this->log2();
	else return this->log2() / std::log2( base );
}

BigInteger BigInteger::operator+( const BigInteger & rhs ) const
{
	if( this->_negative && rhs._negative )
	{
		BigInteger result = this->internal_add( rhs );
		result._negative = true;
		return result;
	}
	else if( this->_negative && !rhs._negative )
	{
		if( this->abs() > rhs )
		{
			BigInteger result = this->internal_sub( rhs );
			result._negative = true;
			return result;
		}
		else
		{
			BigInteger result = rhs.internal_sub( *this );
			result._negative = false;
			return result;
		}
	}
	else if( !this->_negative && rhs._negative )
	{
		if( rhs.abs() > *this )
		{
			BigInteger result = this->internal_sub( rhs );
			result._negative = true;
			return result;
		}
		else
		{
			return this->internal_sub( rhs );
		}
	}
	else return this->internal_add( rhs );
}

BigInteger & BigInteger::operator+=( const BigInteger & rhs )
{
	return *this = *this + rhs;
}

BigInteger & BigInteger::operator++()
{
	return *this += ONE;
}

BigInteger BigInteger::operator++( int )
{
	BigInteger copy = *this;
	*this += ONE;
	return copy;
}

BigInteger BigInteger::operator-( const BigInteger & rhs ) const
{
	if( this->_negative && !rhs._negative ) // negative minus a positive is addition and the final answer is negative
	{
		BigInteger res = this->internal_add( rhs );
		res._negative = true;
		return res;
	}
	else if( !this->_negative && rhs._negative ) //positive minus a negative is addition and the final answer is always positive
	{
		return rhs.internal_add( *this );
	}
	else if( this->_negative && rhs._negative )
	{
		if( this->operator<( rhs ) )
		{
			BigInteger res = this->internal_sub( rhs );
			res._negative = true;
			return res;
		}
		else
		{
			BigInteger res = rhs.internal_sub( *this ); //if the left hand side is bigger than the left hand side ( -1 - -5 ) = 5 - 1
			res._negative = false;
			return res;
		}

	}
	else //both positive
	{
		if( this->operator<( rhs ) )
		{
			BigInteger res = rhs.internal_sub( *this );
			res._negative = true;
			return res;
		}
		else return internal_sub( rhs );
	}
}

BigInteger BigInteger::operator-() const
{
	BigInteger res = *this;
	res._negative = !res._negative;
	return res;
}

BigInteger& BigInteger::operator-=( const BigInteger & rhs )
{
	return *this = *this - rhs;
}

BigInteger & BigInteger::operator--()
{
	return *this -= ONE;
}

BigInteger BigInteger::operator--( int kind )
{
	BigInteger copy = *this;
	*this -= ONE;
	return copy;
}

BigInteger BigInteger::operator*( const BigInteger & rhs ) const
{
	BigInteger sum = 0;
	size_t num_digits = this->_bits.size();
	for( size_t idx = 0; idx < num_digits; ++idx )
	{
		for( int i = 0; i < bits_per_value; ++i )
		{
			uint32_t bit_idx = ( i * num_digits ) + idx;
			bool bit = this->get_bit( bit_idx );
			if( bit )
				sum += rhs << bit_idx;
		}
	}
	sum._negative = ( this->_negative != rhs._negative ); //different signs is negative, same sign is positive
	return sum;
}

BigInteger& BigInteger::operator*=( const BigInteger & rhs )
{
	return *this = *this * rhs;
}

BigInteger BigInteger::operator/( const BigInteger & rhs ) const
{
	return this->divide( rhs );
}

BigInteger & BigInteger::operator/=( const BigInteger & rhs )
{
	return *this = *this / rhs;
}

BigInteger BigInteger::operator%( const BigInteger & rhs ) const
{
	BigInteger remainder;
	this->divide( rhs.abs(), &remainder );
	return remainder;
}

BigInteger & BigInteger::operator%=( const BigInteger & rhs )
{
	return *this = *this % rhs;
}

bool BigInteger::operator>( const BigInteger & rhs ) const
{
	return !( this->operator<=( rhs ) );
}

bool BigInteger::operator>=( const BigInteger & rhs ) const
{
	return !( this->operator<( rhs ) );
}

//Comparison operators are O(n) complexity... :(
bool BigInteger::operator<( const BigInteger & rhs ) const
{
	if( this->_negative && !rhs._negative ) //negative is always less than positive
		return true;
	if( !this->_negative && rhs._negative ) //positive always less than a negative
		return false;

	//small optimization. only call the size functions once respectively
	int32_t this_size = ( int32_t )this->_bits.size();
	int32_t rhs_size = (int32_t)rhs._bits.size();

	if( this_size < rhs_size )
		return ( !this->_negative && !rhs._negative ); //if they're both positive then true else false
	if( this_size > rhs_size )
		return !( !this->_negative && !rhs._negative ); //if they're both positive then false else true;

	//the sizes are equal, so we can iterate over both with a single index
	for( int32_t i = this_size - 1; i >= 0; --i )
	{
		if( this->_bits[i] < rhs._bits[i] )
			return ( !this->_negative && !rhs._negative );
		if( this->_bits[i] > rhs._bits[i] )
			return !( !this->_negative && !rhs._negative );
	}

	//all ths bits are equal if we got here
	return false;
}

bool BigInteger::operator<=( const BigInteger & rhs ) const
{
	if( this->_negative && !rhs._negative )
		return true;
	if( !this->_negative && rhs._negative )
		return false;

	//small optimization. only call the size functions once respectively
	int32_t this_size = ( int32_t )this->_bits.size();
	int32_t rhs_size = (int32_t)rhs._bits.size();

	if( this_size < rhs_size )
		return ( !this->_negative && !rhs._negative ); //if they're both positive then true else false
	if( this_size > rhs_size )
		return !( !this->_negative && !rhs._negative ); //if they're both positive then false else true

	//the sizes are equal, so we can iterate over both with a single index
	for( size_t i = this_size - 1; i >= 0; --i )
	{
		if( this->_bits[i] < rhs._bits[i] )
			return ( !this->_negative && !rhs._negative );
		if( this->_bits[i] > rhs._bits[i] )
			return !( !this->_negative && !rhs._negative );
	}

	//all ths bits are equal if we got here
	return true;
}

//same size, same bits, and same sign
bool BigInteger::operator==( const BigInteger & rhs ) const
{
	if( this->_bits.size() != rhs._bits.size() )
		return false;
	return this->_bits == rhs._bits && this->_negative == rhs._negative; //same sign, same bits
}

bool BigInteger::operator!=( const BigInteger& rhs ) const
{
	return !( this->operator==( rhs ) );
}

//unsigned 32 bit overflow is guaranteed behavior in C/C++ so I'm fine to rely on it.
BigInteger BigInteger::operator<<( uint32_t lshift ) const
{
	BigInteger copy = *this;
	if( lshift >= bits_per_value )
	{ //an optimization. If we're shifting over 70 bits, we can just add 64 new bits of LSB-side zeros and then do the actual last 6 bits manually
		copy._bits.insert( copy._bits.begin(), lshift / 32, 0 );
		lshift %= bits_per_value;
	}

	while( lshift-- > 0 )
	{
		int carry_bit = 0;
		for( uint32_t& val : copy._bits )
		{
			int temp_carry = ( val >> 31 ) & 1;
			val <<= 1;
			val |= carry_bit; //1 or 0
			carry_bit = temp_carry;
		}
		if( carry_bit )
			copy._bits.push_back( 1 );
	}
	return copy;
}

BigInteger & BigInteger::operator<<=( uint32_t lshift )
{
	if( lshift >= bits_per_value )
	{
		this->_bits.insert( this->_bits.begin(), lshift / 32, 0 );
		lshift %= bits_per_value;
	}

	while( lshift-- > 0 )
	{
		int carry_bit = 0;
		for( uint32_t& val : _bits )
		{
			int temp_carry = ( val >> 31 ) & 1;
			val <<= 1;
			//the least significant bit will be 0 after the above operation
			val |= carry_bit; //1 or 0
			//now the least significant bit will be equal to carry_bit
			carry_bit = temp_carry;
			//keep track of what the most significant bit was before the shift, since it got shifted to oblivion
		}
		if( carry_bit )
			_bits.push_back( 1 );
	}
	return *this;
}

BigInteger BigInteger::operator >> ( uint32_t rshift ) const
{
	BigInteger copy = *this;
	while( rshift-- > 0 )
	{
		int carry_bit = 0;
		for( int32_t i = (int32_t)copy._bits.size() - 1; i >= 0; --i )
		{
			int temp_carry = copy._bits[i] & 1;
			copy._bits[i] >>= 1;
			copy._bits[i] |= ( carry_bit << ( bits_per_value - 1 ) );
			carry_bit = temp_carry;
		}
	}
	copy.trim();
	return copy;
}

BigInteger & BigInteger::operator>>=( uint32_t rshift )
{
	while( rshift-- > 0 )
	{
		int carry_bit = 0;
		for( int32_t i = (int32_t)_bits.size() - 1; i >= 0; --i )
		{
			int temp_carry = _bits[i] & 1;
			_bits[i] >>= 1;
			_bits[i] |= ( carry_bit << ( bits_per_value - 1 ) );
			carry_bit = temp_carry;
		}
	}
	trim();
	return *this;
}

uint32_t BigInteger::bits_allocated() const
{
	return _bits.size() * bits_per_value;
}

uint32_t BigInteger::bits_used() const
{
	return ( _bits.size() - 1 ) * bits_per_value + (uint32_t)std::log2( _bits.back() ) + 1;
}

BigInteger BigInteger::abs() const
{
	BigInteger copy = *this;
	copy._negative = false;
	return copy;
}

BigInteger BigInteger::pow( uint32_t power ) const
{
	if( power <= 0 )
		return 0;

	BigInteger res = *this;
	for( uint32_t i = 1; i < power; ++i )
		res *= *this;
	return res;
}

BigInteger BigInteger::pow( const BigInteger & power ) const
{
	if( power <= 0 )
		return 0;

	BigInteger res = *this;
	for( uint32_t i = 1; power >= i; ++i )
		res *= *this;
	return res;
}

bool BigInteger::get_bit( uint32_t bit ) const
{
	if( bit >= bits_allocated() )
		throw exception( "Bit number out of range!" );

	size_t array_index = bit / bits_per_value;
	size_t bit_index_in_array = bit % bits_per_value;

	return ( _bits[array_index] >> bit_index_in_array ) & 1;
}

void BigInteger::set_bit( uint32_t bit, bool value )
{
	if( bit >= bits_allocated() )
		throw exception( "Bit number out of range!" );

	size_t array_index = bit / bits_per_value;
	size_t bit_index_in_array = bit % bits_per_value;

	_bits[array_index] ^= ( -( (uint8_t)value ) ^ _bits[array_index] ) & ( 1 << bit_index_in_array );
}

void BigInteger::toggle_bit( uint32_t bit )
{
	if( bit >= bits_allocated() )
		throw exception( "Bit number out of range!" );

	size_t array_index = bit / bits_per_value;
	size_t bit_index_in_array = bit % bits_per_value;

	_bits[array_index] ^= 1 << bit_index_in_array;
}

string BigInteger::to_string( uint32_t base ) const
{
	if( base < 2 || base > 36 )
		return "Invalid Base";

	BigInteger remainder = ZERO, quotient = *this, bigint_base = base;
	stringstream ss;
	char buff[32];

	do
	{
		quotient = quotient.divide( bigint_base, &remainder );
		_itoa_s( remainder.to_uint32(), buff, sizeof( buff ), base );
		ss << buff;
	} while( quotient != ZERO );

	string result = ss.str();
	std::reverse( result.begin(), result.end() );

	if( _negative )
		return "-" + result;
	else return result;

}

double BigInteger::log2()
{
	return ( ( _bits.size() - 1 ) * bits_per_value ) + std::log2( _bits.back() );
}

uint32_t BigInteger::get_lowest_set_bit()
{
	for( size_t i = 0; i < _bits.size(); ++i )
		for( uint32_t bit = 0; bit < bits_per_value; ++bit )
			if( ( _bits[i] >> bit ) & 1 )
				return i * bits_per_value + bit;
	return -1;
}

BigInteger BigInteger::internal_add( const BigInteger & rhs ) const
{
	//If they're equal, bigger will be this->_bits, and smaller will be rhs._bits
	vector<uint32_t>* bigger = bigger_array( this->_bits, rhs._bits );
	vector<uint32_t>* smaller = smaller_array( this->_bits, rhs._bits );

	BigInteger ret;
	ret._bits.resize( bigger->size() );

	//the bits that need to be added together
	uint32_t carry = 0;
	for( size_t i = 0; i < smaller->size(); ++i )
	{
		int64_t res = (int64_t)( *smaller )[i] + ( *bigger )[i] + carry;
		if( res > UINT32_MAX )
		{
			carry = 1;
			res -= (int64_t)UINT32_MAX + 1;
		}
		else carry = 0;

		ret._bits[i] = (uint32_t)res;
	}

	//now we have to handle the carry and the extra
	for( size_t i = smaller->size(); i < bigger->size(); ++i )
	{
		int64_t res = (int64_t)( *bigger )[i] + carry;
		if( res > UINT32_MAX )
		{
			carry = 1;
			res -= (int64_t)UINT32_MAX + 1;
		}
		else carry = 0;

		ret._bits[i] = (uint32_t)res;
	}

	if( carry )
		ret._bits.push_back( carry );

	return ret;
}

BigInteger BigInteger::internal_sub( const BigInteger & rhs ) const
{
	//If they're equal, bigger will be this->_bits, and smaller will be rhs._bits
	vector<uint32_t>* bigger = bigger_array( this->_bits, rhs._bits );
	vector<uint32_t>* smaller = smaller_array( this->_bits, rhs._bits );

	BigInteger ret;
	ret._bits.resize( bigger->size() );

	uint32_t borrow = 0;
	for( size_t i = 0; i < smaller->size(); ++i )
	{
		int64_t res = (int64_t)( *bigger )[i] - ( *smaller )[i] - borrow;
		if( res < 0 )
		{
			borrow = 1;
			res += (int64_t)UINT32_MAX + 1;
		}
		else borrow = 0;

		ret._bits[i] = (uint32_t)res;
	}

	for( size_t i = smaller->size(); i < bigger->size(); ++i )
	{
		int64_t res = (int64_t)( *bigger )[i] - borrow;
		if( res < 0 )
		{
			borrow = 1;
			res += (int64_t)UINT32_MAX + 1;
		}
		else borrow = 0;

		ret._bits[i] = (uint32_t)res;
	}

	assert( borrow == 0 ); //this is only possible if 'this' < rhs
	ret.trim();

	return ret;
}

BigInteger BigInteger::divide( const BigInteger & rhs, BigInteger* outer_remainder ) const
{
	BigInteger quotient = 0, remainder = 0;
	uint32_t bits = bits_allocated();

	for( int32_t i = bits - 1; i >= 0; --i )
	{
		remainder <<= 1;
		remainder.set_bit( 0, this->get_bit( i ) );
		if( remainder >= rhs )
		{
			remainder -= rhs;
			if( quotient.bits_allocated() <= (uint32_t)i )
				quotient._bits.resize( ( i / bits_per_value ) + 1 );
			quotient.set_bit( i, 1 );
		}
	}

	quotient.trim();

	//negative if one is negative, positive if two or zero are negative
	if( quotient != ZERO )
		quotient._negative = this->_negative != rhs._negative;
	if( remainder != ZERO )
		remainder._negative = quotient._negative;

	if( outer_remainder != nullptr )
		*outer_remainder = remainder;

	return quotient;
}


//If we have extra zeros in the MSBs, remove them
void BigInteger::trim()
{
	while( _bits.size() > 1 && _bits.back() == 0 )
		_bits.pop_back();
}

//If they're equal, this function returns &_1
vector<uint32_t>* BigInteger::bigger_array( const vector<uint32_t>& _1, const vector<uint32_t>& _2 )
{
	if( _1.size() >= _2.size() )
		return &const_cast<vector<uint32_t>&>( _1 );
	else return &const_cast<vector<uint32_t>&>( _2 );
}

//If they're equal, this function returns &_2
vector<uint32_t>* BigInteger::smaller_array( const vector<uint32_t>& _1, const vector<uint32_t>& _2 )
{
	if( _1.size() < _2.size() )
		return &const_cast<vector<uint32_t>&>( _1 );
	else return &const_cast<vector<uint32_t>&>( _2 );
}

ostream& operator<<( ostream& os, const BigInteger& rhs )
{
	return os << rhs.to_string();
}

istream& operator >> ( istream & is, BigInteger & rhs )
{
	string s;
	is >> s;
	rhs = BigInteger( s );
	return is;
}