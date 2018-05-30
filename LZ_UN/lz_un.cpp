#include <stdio.h>
#include <memory.h>
#include <string.h>
#include <stdlib.h>

#include "LZ.h"

FILE*	src;
FILE*	dst;
long	filesize;
long	dic_cnt;
long	dat_cnt;
long	tmppoint;

byte*	tmp;
byte*	UNCOMPRESSED_DATA;

long	win_point[ 2 ] = { 0, WIN_SIZE - 1 };
long	dst_point = 0;
long	cnt=0;
void move_window( int );
void decode();

byte get_length( int );
int get_offset( int );
byte get_byte( int );

void main( int argc, char* argv[] ) {


	dat_cnt = dic_cnt = 0;
	tmppoint = 0;

/*
	if( ( src = fopen( argv[ 1 ], "rb" ) ) == NULL ) {

		printf( "cannot find file\n" );
		return;
	}
*/
	src = fopen( "test.lz", "rb" );

	fseek( src, 0, SEEK_END);
	filesize = ftell( src );
	fseek( src, 0, SEEK_SET);

	tmp = ( byte* )malloc( filesize );
	memset( tmp, 0, filesize );

	fread( ( void* )( tmp ), filesize, 1, src );
	fclose( src );


	if( filesize < 100000 ) {

		UNCOMPRESSED_DATA = ( byte* )malloc( 100000 );
		memset( ( void* )UNCOMPRESSED_DATA, 0, 100000 );
	}
	else {

		UNCOMPRESSED_DATA = ( byte* )malloc( filesize * 10 + WIN_SIZE );
		memset( ( void* )UNCOMPRESSED_DATA, 0, filesize * 10 + WIN_SIZE );
	}

	while( tmppoint < filesize ) {

		decode();
	}
	


	dst = fopen( "b.bmp", "wb" );
	int cnt = win_point[ 1 ] - WIN_SIZE;
	fwrite( ( void* )( UNCOMPRESSED_DATA + WIN_SIZE ), 1, cnt , dst );
	fclose( dst );

	free( ( void* )UNCOMPRESSED_DATA );
	free( ( void* )tmp );

}

void decode() {

	int offset;
	byte length, ch;

	byte t;
	memcpy( ( void* )&t, ( void* )( tmp + tmppoint ), 1 );
	
	int typepoint = ( dat_cnt * 9 + dic_cnt * 26 ) % 8;
	if( ( t & ( 0x80 >> typepoint ) ) == ( 0x80 >> typepoint ) ) {

		// type : 1

		// offset
		offset = get_offset( typepoint );

		// length
		length = get_length( typepoint );
		
		// byte
		ch = get_byte( typepoint );


		memcpy( ( void* )( UNCOMPRESSED_DATA + win_point[ 1 ] + 1 ), ( void* )( UNCOMPRESSED_DATA + win_point[ 0 ] + offset ), length );
		memcpy( ( void* )( UNCOMPRESSED_DATA + win_point[ 1 ] + length + 1 ), ( void* )&ch, 1 );

		move_window( length + 1 );
		dic_cnt++;
	}
	else {

		// type : 0

		if( typepoint == 7 ) {

			memcpy( ( void* )( UNCOMPRESSED_DATA + win_point[ 1 ] + 1 ), ( void* )( tmp + tmppoint + 1 ), 1 );
			tmppoint += 2;
		}
		else {
			
			byte ch[ 2 ] = { 0, 0 };
			memcpy( ( void* )&ch[ 0 ], ( void* )( tmp + tmppoint ), 1 );
			ch[ 0 ] = ch[ 0 ] << ( typepoint + 1 );

			memcpy( ( void* )&ch[ 1 ], ( void* )( tmp + tmppoint + 1 ), 1 );
			ch[ 0 ] = ch[ 0 ] | ( ch[ 1 ] >> ( 7 - typepoint ) );

			memcpy( ( void* )( UNCOMPRESSED_DATA + win_point[ 1 ] + 1 ), ( void* )( &ch[ 0 ] ), 1 );

			tmppoint++;
		}

		move_window( 1 );
	
		dat_cnt++;
	}
}

int get_offset( int typepoint ) {

	byte offset[ 3 ] = { 0, 0, 0 };
	int _offset = 0;

/*
	if( typepoint <= 6 && typepoint >= 4 ) {	// 4,5,6

		// 3바이트 필요
		memcpy( ( void* )( &offset[ 0 ] ), ( void* )( tmp + tmppoint ), 1 );
		memcpy( ( void* )( &offset[ 1 ] ), ( void* )( tmp + tmppoint + 1 ), 1 );
		memcpy( ( void* )( &offset[ 2 ] ), ( void* )( tmp + tmppoint + 2 ), 1 );

		offset[ 0 ] = ( byte )( offset[ 0 ] << ( typepoint + 1 ) ) >> ( typepoint + 1 );
		offset[ 2 ] = ( byte )( offset[ 2 ] >> ( 11 - typepoint ) ) << ( 11 - typepoint );

		_offset = ( int )( offset[ 2 ] ) >> ( 11 - typepoint );
		_offset = _offset + ( ( int )(offset[ 1 ] ) << ( typepoint - 3 ) );
		_offset = _offset + ( ( int )(offset[ 0 ] ) << ( typepoint + 5 ) );
	}
	else if( typepoint < 4 ) {	// 0,1,2,3

		// 2바이트 필요
		memcpy( ( void* )( &offset[ 0 ] ), ( void* )( tmp + tmppoint ), 1 );
		memcpy( ( void* )( &offset[ 1 ] ), ( void* )( tmp + tmppoint + 1 ), 1 );

		offset[ 0 ] = ( int )( offset[ 0 ] ) << ( typepoint + 9 ) >> ( typepoint + 5 );
		offset[ 1 ] = offset[ 1 ] >> ( ( 11 - typepoint ) % 8 );

		_offset = offset[ 0 ] + ( int )offset[ 1 ];
	}
	else {	// 7

		memcpy( ( void* )( &offset[ 0 ] ), ( void* )( tmp + tmppoint + 1 ), 1 );
		memcpy( ( void* )( &offset[ 1 ] ), ( void* )( tmp + tmppoint + 2 ), 1 );

		_offset = offset[ 0 ] * 16 + ( offset[ 1 ] >> 4 );
	}
*/

	memcpy( ( void* )( &offset[ 0 ] ), ( void* )( tmp + tmppoint ), 1 );
	memcpy( ( void* )( &offset[ 1 ] ), ( void* )( tmp + tmppoint + 1 ), 1 );
	memcpy( ( void* )( &offset[ 2 ] ), ( void* )( tmp + tmppoint + 2 ), 1 );

	if( typepoint == 7 ) {

		_offset = ( offset[ 1 ] << 4 ) + ( offset[ 2 ] >> 4 );
	}
	else if( typepoint == 6 ) {

		_offset	  = ( ( ( int )offset[ 0 ] & 0x00000001 ) << 11 ) + 
					( ( ( int )offset[ 1 ] ) << 3 ) + 
					( ( ( int )offset[ 2 ] ) >> 5 );
	}
	else if( typepoint == 5 ) {

		_offset   = ( ( ( int )offset[ 0 ] & 0x00000003 ) << 10 ) + 
					( ( ( int )offset[ 1 ] ) << 2 ) + 
					( ( ( int )offset[ 2 ] ) >> 6 );
	}
	else if( typepoint == 4 ) {

		_offset   = ( ( ( int )offset[ 0 ] & 0x00000007 ) << 9 ) + 
					( ( ( int )offset[ 1 ] ) << 1 ) + 
					( ( ( int )offset[ 2 ] ) >> 7 );

	}
	else if( typepoint == 3 ) {

		_offset   = ( ( ( int )offset[ 0 ] & 0x0000000F ) << 8 ) + 
					( ( ( int )offset[ 1 ] ) );
	}
	else if( typepoint == 2 ) {

		_offset   = ( ( ( int )offset[ 0 ] & 0x0000001F ) << 7 ) + 
					( ( ( int )offset[ 1 ] ) >> 1 );
	}
	else if( typepoint == 1 ) {

		_offset   = ( ( ( int )offset[ 0 ] & 0x0000003F ) << 6 ) + 
					( ( ( int )offset[ 1 ] ) >> 2 );
	}
	else {

		_offset   = ( ( ( int )offset[ 0 ] & 0x0000007F ) << 5 ) + 
					( ( ( int )offset[ 1 ] ) >> 3 );
	}	 

//if( _offset < 1 )
//printf("%d\n", _offset );



	return _offset;
}

byte get_length( int typepoint ) {

	byte length[ 3 ]	= { 0, 0, 0 };
	byte _length = 0;

	memcpy( ( void* )( &length[ 0 ] ), ( void* )( tmp + tmppoint + 1 ), 1 );
	memcpy( ( void* )( &length[ 1 ] ), ( void* )( tmp + tmppoint + 2 ), 1 );
	memcpy( ( void* )( &length[ 2 ] ), ( void* )( tmp + tmppoint + 3 ), 1 );
/*
	if( typepoint < 3 ) {

		length[ 0 ] = length[ 0 ] << ( typepoint + 5 );
		length[ 0 ] = length[ 0 ] >> ( typepoint + 3 );
		length[ 1 ] = ( ( byte )length[ 1 ] ) >> ( 6 - typepoint );
		_length = length[ 0 ] + length[ 1 ];
	}
	else if( typepoint < 7 ) {

		_length = ( byte )( ( length[ 1 ] ) << ( typepoint - 3 ) ) >> 3;
	}
	else {

//		_length = ( byte )( length[ 1 ] << 9 >> 8 ) + ( length[ 2 ] >> 7 );
		_length = ( ( byte )( length[ 1 ] & 0x000F ) << 4 >> 3 )+ ( ( byte )length[ 2 ] >> 7);
	}
*/

	if( typepoint == 7 ) {

		_length = ( ( length[ 1 ] & 0x000F ) << 1 ) + ( ( length[ 2 ] & 0x0080 ) >> 7 );
	}
	else if( typepoint == 6 ) {

		_length = ( length[ 1 ] & 0x001F );
	}
	else if( typepoint == 5 ) {

		_length = ( length[ 1 ] & 0x003F ) >> 1;
	}
	else if( typepoint == 4 ) {

		_length = ( length[ 1 ] & 0x007F ) >> 2;
	}
	else if( typepoint == 3 ) {

		_length = ( length[ 1 ] & 0x00FF ) >> 3;
	}
	else if( typepoint == 2 ) {

		_length = ( ( length[ 0 ] & 0x0001 ) << 4 ) + ( ( length[ 1 ] & 0x00FF ) >> 4 );
	}
	else if( typepoint == 1 ) {

		_length = ( ( length[ 0 ] & 0x0003 ) << 3 ) + ( ( length[ 1 ] & 0x00FF ) >> 5 );
	}
	else {

		_length = ( ( length[ 0 ] & 0x0007 ) << 2 ) + ( ( length[ 1 ] & 0x00FF ) >> 6 );
	}	
	

	return _length;

}


byte get_byte( int typepoint ) {

	byte ch[ 3 ] = { 0, 0, 0 };
	byte _ch = 0;

	memcpy( ( void* )( &ch[ 0 ] ), ( void* )( tmp + tmppoint + 2 ), 1 );
	memcpy( ( void* )( &ch[ 1 ] ), ( void* )( tmp + tmppoint + 3 ), 1 );
	memcpy( ( void* )( &ch[ 2 ] ), ( void* )( tmp + tmppoint + 4 ), 1 );

	if( typepoint == 7 ) {

		_ch = ( ch[ 1 ] << 1 ) + ( ch[ 2 ] >> 7 );
		tmppoint += 4;
	}
	else if( typepoint == 6 ) {

		_ch = ch[ 1 ];
		tmppoint += 4;
	}
	else if( typepoint == 5 ) {

		_ch = ( ch[ 0 ] << 7 ) + ( ch[ 1 ] >> 1 );
		tmppoint += 3;
	}
	else if( typepoint == 4 ) {

		_ch = ( ch[ 0 ] << 6 ) + ( ch[ 1 ] >> 2 );
		tmppoint += 3;
	}
	else if( typepoint == 3 ) {

		_ch = ( ch[ 0 ] << 5 ) + ( ch[ 1 ] >> 3 );
		tmppoint += 3;
	}
	else if( typepoint == 2 ) {

		_ch = ( ch[ 0 ] << 4 ) + ( ch[ 1 ] >> 4 );
		tmppoint += 3;
	}
	else if( typepoint == 1 ) {

		_ch = ( ch[ 0 ] << 3 ) + ( ch[ 1 ] >> 5 );
		tmppoint += 3;
	}
	else {

		_ch = ( ch[ 0 ] << 2 ) + ( ch[ 1 ] >> 6 );
		tmppoint += 3;
	}	



/*
	if( typepoint < 6 ) {

		_ch = ( ch[ 0 ] << ( typepoint + 2 ) ) + ( ch[ 1 ] >> ( 6 - typepoint ) );
		tmppoint += 3;
	}
	else if( typepoint < 7 ) {

		_ch = ch[ 1 ];
		tmppoint += 4;
	}
	else {

//		_ch = ( ch[ 1 ] << 1 ) + ( ch[ 2 ] >> 7 );
		_ch = ( byte )( ch[ 1 ] ) * 2 + ( ( byte )ch[ 2 ] >> 7 );
		tmppoint += 4;
	}
*/
	return _ch;
}

void move_window( int i ) {

	win_point[ 0 ] += i;
	win_point[ 1 ] += i;
}
