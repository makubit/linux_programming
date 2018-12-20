# $1 - sciezka do fifo
# $2- komunikat obowiazkowy
# $3 - odstep czasu w sek

usage() {
	echo "$0 <sciezka do FIFO> <komunikat> [ <opoznienie (float)> ]"
	echo "domysne opoznienie 1.0sek"

}

prerr() {
	{ echo "$1" ; echo ; usage ; } 1>&2
	exit $2
}

#sprawdzenie istnienia i tylu pliku
[ -z $1 ] && { prerr " brak oobowiakowego parametru" 1 ;}
[ -p "$1" ] || { prerr " niepoprawny parametr \" sciezka do FIFO\"" 1;}

#przyjecie domyslnej wartosci dla opóźnienia
SLEEP=${3:-1.0}

#otwarcie zapisu
exec 1> "$1"

while : ; do
	echo -ne "$2"
	sleep $SLEEP
done
