//============================================================================
// Name        : comunicazioni.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : progetto di comunicazioni multimediali, Ansi-style
//============================================================================

#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<string>
#include<conio.h>
//#include<iostream>

//Struttura in cui vengono inseriti i valori dei pixel trovati
typedef struct pixelInfo{
	int R;
	int G;
	int B;
}pixelInfo;

//Struttura in cui vengono inseriti i colori trovati
typedef struct colorVet{
	char simbolo[3];
	int R;
	int G;
	int B;
	int occorrenze;
}colorVet;

//Dichiaro le variabili globali utilizzate in futuro
pixelInfo *pixel;
colorVet *colori;
int larghezza, altezza;

int LetturaFileXPM(char nomeFile[]){

	FILE *fin;
	char line[256],*str;
	unsigned int cline=0,numColori,bitColore,i,c=0;
	std::string::size_type sz = 0;

	fin = fopen(nomeFile,"r+");
	if(fin == NULL){
		fclose(fin);
		return -1;
	}
	else{
		printf("file aperto correttamente\n");
	}

	while(!(feof(fin))){

		fscanf(fin, " %[^\n]",line);
		cline++;

		if(cline == 4){
			//andiamo a leggere tutti i parametri dell'immagine

			str = strtok(line," ");
			for(i=0;i<(strlen(str)-1);i++){
				str[i] = str[i+1];
			}
			str[4] = '\0';
			larghezza = atoi(str);

			while(str != NULL){

				str = strtok(NULL," ");
				switch(c){
				case 0:
					altezza = atoi(str);
					break;
				case 1:
					numColori = atoi(str);
					break;
				case 2:
					for(i=0;i<strlen(str);i++){
						if(str[i] == '"'){
							str[i] = '\0';

							//costringo il ciclo for a finire
							//anche se secondo me dopo aver messo il terminatore di stringa
							//non dovrebbe servire
							i = strlen(str);
						}
					}
					bitColore = atoi(str);
					break;
				}
				c++;
			}
		}

		colori = (colorVet*) malloc (numColori * sizeof(colorVet));

		if((cline >= 5) && (cline <= 260)){
			//salviamo in una struct con tutti i colori dell'immagine

			str = strtok(line,"c");
			for(i=0;i<bitColore;i++){
				str[i] = str[i+1];
			}
			str[bitColore] = '\0';
			strcpy(colori[cline - 5].simbolo,str);
			printf("%s\n",colori[cline -5].simbolo);

		}

		//questa è solo una prova
		if(cline > 260){
			getch();
		}
	}

	fclose(fin);
	return 0;
}

int LetturaFilePPM(char nomeFile[]){

	//Dichiaro alcune variabili utili per la lettura da file
	FILE *fin;
	char line[256];
	int cline = 0, numColori, cpixel = 0;
	std::string::size_type sz = 0;

	//Apro il file per la lettura controllando che non ci siano problemi
	fin = fopen(nomeFile,"r+");
	if(fin == NULL) {
		fclose(fin);
		return -1;
	} else {
		printf("file aperto correttamente\n");
	}

	//leggo dal file fino a quando non viene letto completamente
	while (!(feof(fin))){

		//leggo una linea dal file
		fscanf(fin, " %[^\n]", line);
		cline++;

		//Nella terza riga del file ricavo larghezza e altezza immagine
		if (cline == 3) {
			larghezza = atoi(strtok(line," "));
			altezza = atoi(strtok(NULL,"\0"));
			pixel = (pixelInfo*) malloc (larghezza * altezza * sizeof(pixelInfo));

		//Nella quarta riga del file ricavo il numero dei colori
		} else if (cline == 4) {
			numColori = atoi(line);
			colori = (colorVet*) malloc (numColori * sizeof(colorVet));

		//Leggo le infomazioni di ogni pixel
		} else if (cline > 4) {

			//Inserisco le informazioni nella struttura pixel
			if (cline % 3 == 2)
				pixel[cpixel].R = atoi(line);
			else if(cline % 3 == 0)
				pixel[cpixel].G = atoi(line);
			else {
				pixel[cpixel].B = atoi(line);
				cpixel++;
			}
		}

	}
	return 0;
}

//Funzione che stampa a video i pixel in modo ordinato
void stampaPixel(){

	//Indici utilizzati per effettuare l'accesso a ogni componente della struttura
	int i;

	//Scorro tutta la struttura dati
	for(i = 0; i < larghezza * altezza; i++) {

		//Stampo ogni pixel
		printf("%d %d %d-", pixel[i].R, pixel[i].G, pixel[i].B);

		//Stampo la riga successiva
		if(i % larghezza == larghezza -1)
			printf("\n");
	}

	scanf("%d",&i);
}

int main(){

	int res, i,k=2;
	char *nomeFile,ext[3];

	//di sicuro ci sono modi piu efficaci
	res = strlen("assassins_creed_syndicate-1280x800.ppm");
	nomeFile = (char*) malloc (res*sizeof(char));
	strcpy(nomeFile,"assassins_creed_syndicate-1280x800.ppm");

	//ciclo che prende l'estensione del file
	for(i=res-1;i>=res-3;i--){
		ext[k] = nomeFile[i];
		k--;
	}

	//Controllo l'estensione per decidere quale funzione utilizzare
	if(strcmp(ext,"xpm")==0) {
		res = LetturaFileXPM(nomeFile);
	} else if(strcmp(ext,"ppm")==0) {
		res = LetturaFilePPM(nomeFile);
	} else {
		printf("errore\n");
	}

	//Stampo la struttura dati preparata in precedenza
	stampaPixel();

	//Libero le strutture precedentemente utilizzate
	free(nomeFile);
	free(colori);
	return 0;
}
