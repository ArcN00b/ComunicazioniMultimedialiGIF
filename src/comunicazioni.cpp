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

typedef struct{
	char simbolo[3];
	int R;
	int G;
	int B;
	int occorrenze;
}colorVet;

colorVet *colori;

int LetturaFileXPM(char nomeFile[]){

	FILE *fin;
	char line[256],*str;
	int cline=0,larghezza,altezza,numColori,bitColore,i,c=0;
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

	return 0;
}

int main(){

	int res, i,k=2;
	char *nomeFile,ext[3];

	//di sicuro ci sono modi piu efficaci
	res = strlen("arrow__the_flash-HD.xpm");
	nomeFile = (char*) malloc (res*sizeof(char));
	strcpy(nomeFile,"arrow__the_flash-HD.xpm");

	//ciclo che prende l'estensione del file
	for(i=res-1;i>=res-3;i--){
		ext[k] = nomeFile[i];
		k--;
	}

	if(strcmp(ext,"xpm")==0){
		res = LetturaFileXPM(nomeFile);
	}
	//if(strcmp(ext,"ppm")==0){
	//	res = LetturaFilePPM(nomeFile);
	//}

	if(res < 0){
		printf("errore\n");
	}

	free(nomeFile);
	free(colori);
	return 0;
}
