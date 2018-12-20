//drugie zadanie
//piszemy program, kóßry przyjmuje 3 libczy float
//1- dolna agranica
//2- górna granica
//3-krok
//w dostepach czasu delta t, mierzonych jak najdokladniej ma byc wyposywany na erkan jakis znak (UNICODE DO CZASZKI)
//dartosc dt to srednia arytmetyczna z granic
//program obsługuje 2 sygnału usr1 i 2
//każdy z nich powoduje zmiane wartosc dt o krok (pryz czym usr1 zmiejsza, usr2 zwieksza)
//jeżeli dt wyjdzie poza granicę dolną lub górną -> wypisywane ostrzerzenie i nie ma wypisywanych znaków
//progeam czeka na kolejne sygnały, które pozwolą mu pracowac (zmniejszenie lub zwiekszenie )
// signal SIGALEM powoduje wyświetlenie wszystkie wartosci, krok, dt, gorna, dolna granica etc
//sygnały wysyłamy z basha (można też skrypcik)
//
//nanosleep, może clock nanosleep
//sleep jest przeryway przez sygnały -> sleep bedzie za któtki
//techniki dosypiania; trzeba sprawdzić ile czasu jeszcze brakuje do spania
//lub inna technika: jedna z wersji nanosleep przyjmuje flage aps?? mówi do kiedy mamy spać ABSTIME??



