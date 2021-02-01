#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "mpi.h"
#define REZERWA 500
#define MAGAZYN 1
#define WYJAZD 2
#define ZALADUNEK 3
#define ROZLADUNEK 4
#define POMOC -2
#define TANKOWANIE 2000
int ZAJEDZ = 1, NIE_ZAJEZDZAJ = 0;
int paliwo = 2000;
int liczba_procesow;
int nr_procesu;
int ilosc_tirow;
int ilosc_miejsc = 4;
int ilosc_miejsc_zajetych = 0;
int tag = 1;
int wyslij[2];
int odbierz[2];
MPI_Status mpi_status;
void Wyslij(int nr_tira, int stan) {
    wyslij[0] = nr_tira;
    wyslij[1] = stan;
    MPI_Send(&wyslij, 2, MPI_INT, 0, tag, MPI_COMM_WORLD);
    sleep(1);
}

void Magazyn(int liczba_procesow) {
    int nr_tira, status;
    ilosc_tirow = liczba_procesow - 1;
    sleep(2);
    while(ilosc_miejsc <= ilosc_tirow) {
        MPI_Recv(&odbierz, 2, MPI_INT, MPI_ANY_SOURCE, tag, MPI_COMM_WORLD, &mpi_status);
        nr_tira = odbierz[0];
        status = odbierz[1];
        if(status == 1) {
            printf("TIR %d jest teraz na magazynie\n", nr_tira);
        }
        else if(status == 2) {
            printf("TIR %d wyjezdza z magazynu z miejsca %d\n", nr_tira, ilosc_miejsc_zajetych);
            ilosc_miejsc_zajetych--;
        }
        else if(status == 3) {
            printf("TIR %d laduje\n", nr_tira);
        }
        else if(status == 4 || status == -2) {
            if(ilosc_miejsc_zajetych < ilosc_miejsc) {
                ilosc_miejsc_zajetych++;
                printf("TIR zajechal\n");
                MPI_Send(&ZAJEDZ, 1, MPI_INT, nr_tira, tag, MPI_COMM_WORLD);
            }
            else {
                MPI_Send(&NIE_ZAJEZDZAJ, 1, MPI_INT, nr_tira, tag, MPI_COMM_WORLD);
            }      
        }
    }
}

void Tir() {
    int stan, i;
    stan = WYJAZD;
    while(1) {
        if(stan == 1) {
            if(rand()%2 == 1) {
                stan = WYJAZD;
                paliwo = TANKOWANIE;
                printf("TIR %d prosi o pozwolenie na wyjazd\n", nr_procesu);
                Wyslij(nr_procesu, stan);
            }
            else {
                Wyslij(nr_procesu, stan);
            }
        }
        else if(stan == 2) {
            printf("Wyjechalem, TIR %d\n", nr_procesu);
            stan = ZALADUNEK;
            Wyslij(nr_procesu, stan);
        }
        else if(stan == 3) {
            paliwo -= rand()%500;
            if(paliwo <= REZERWA) {
                stan = ROZLADUNEK;
                printf("TIR chce zajechac do magazynu\n");
                Wyslij(nr_procesu, stan);
            }
            else {
                for(i = 0; rand()%10000; i++);
            }            
        }
        else if(stan == 4) {
            int temp;
            MPI_Recv(&temp, 1, MPI_INT, 0, tag, MPI_COMM_WORLD, &mpi_status);
            if(temp == ZAJEDZ) {
                stan = MAGAZYN;
                printf("TIR o numerze %d zajezdza do magazynu\n", nr_procesu);
            }
            else {
                paliwo -= rand()%500;
                if(paliwo > 0) {
                    Wyslij(nr_procesu, stan);
                }
                else {
                    stan = POMOC;
                    printf("TIR %d skonczylo się paliwo! Oczekuje na pomoc z magazynu.\n", nr_procesu);
                    Wyslij(nr_procesu, stan);
                }
            }
        }
        else if(stan == -2) {
            int temp;
            MPI_Recv(&temp, 1, MPI_INT, 0, tag, MPI_COMM_WORLD, &mpi_status);
            if(temp == ZAJEDZ) {
                stan = MAGAZYN;
                printf("TIR o numerze %d otrzymał pomoc i zajezdza do magazynu\n", nr_procesu);
            }
            else {
                printf("Poczekam sobie jeszcze na pomoc. TIR %d\n", nr_procesu);
            }
            Wyslij(nr_procesu, stan);
        }
    }       
}

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &nr_procesu);
    MPI_Comm_size(MPI_COMM_WORLD, &liczba_procesow);
    srand(time(NULL));
    if(nr_procesu == 0)
        Magazyn(liczba_procesow);
    else
        Tir();
    MPI_Finalize();
    return 0;
}