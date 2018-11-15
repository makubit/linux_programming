//cwiczenie z bibliotekami w linuxie
//
//vim: sts=2 sw=2 et:

#include<string.h>

/******************************************************************************
 * * do zadanego bufora wpisywane są kolejne ciągi, oddzielane blokami spacji
 * * zwracana jest liczba zapełnionych komórek
 * *
 * * wejście:
 * * początek bufora i jego długość
 * * tablica wskaźników na łańcuchy
 * * liczba określająca ilość spacji separujących
 * ******************************************************************************/

int fillbuf( char * buf, int blen, char * txts[], int splen ) {

	int count=0;
	for( char ** str = txts; str ; str++ ) {
		int slen = strlen(*str);
		if( slen > blen ) return count;
		strncpy(buf,*str,slen);
		buf += slen;
		blen-= slen;
		count += slen;
		for( int sp=0; sp<splen && blen>0; sp++,blen--,count++ ) {
			*(buf++) = ' ';
		}
	}
	return count;
}
