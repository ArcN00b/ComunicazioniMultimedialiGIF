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
#include<bitset>
#include<iostream>

typedef struct{
	char *simbolo;
	int R;
	int G;
	int B;
	int occorrenze;
}colorVet;


colorVet *colori;
char *matriceImmagine;
unsigned char *paletteGlobale;
int larghezza,altezza,numColori,bitColore;

/*Al termine di questa funzione abbiamo una lista di colori ordinata con il rispettivo simbolo e occorrenza
 * in più abbiamo una matrice dell'immagine scritta tutta su un vettore
 * e le informazioni riguardanti l'immagine
 */
int LetturaFileXPM(char nomeFile[]){

	FILE *fin;
	char *line,hexString[2],simboloConf[bitColore];
	int cline=0,i,k,c=0,j=0;

	fin = fopen(nomeFile,"r+");
	if(fin == NULL){
		fclose(fin);
		return -1;
	}
	else{
		printf("file aperto correttamente\n");
	}

	line = (char*) malloc (256 * sizeof(char));

	while(!(feof(fin))){

		char *str;
		int t = 0;
		fscanf(fin, " %[^\n]",line);
		cline++;

		//queste variazioni agli indici vanno bene soltanto nel caso in cui il file sia stato convertito con gimp
		if(cline == 3){
			//andiamo a leggere tutti i parametri dell'immagine

			str = strtok(line," ");
			for(i=0;i<(strlen(str)-1);i++){
				str[i] = str[i+1];
			}

			str[strlen(str) - 1] = '\0';
			larghezza = atoi(str);

			while(str != NULL){

				str = strtok(NULL," ");
				switch(c){
				case 0:
					altezza = atoi(str);
					break;
				case 1:
					numColori = atoi(str);
					colori = (colorVet*) malloc (numColori * sizeof(colorVet));
					break;
				case 2:
					for(i=0;i<strlen(str);i++){
						if(str[i] == '"'){
							str[i] = '\0';
							//condizione di chiusura for
							i = strlen(str);
						}
					}
					bitColore = atoi(str);
					line = (char*) malloc (larghezza * bitColore * sizeof(char));
					//ricordati di leggere bene questo vettore o è la fine
					matriceImmagine = (char*) malloc (altezza * larghezza * bitColore *sizeof(char));
					break;
				}
				c++;
			}
		}

		else if((cline >= 4) && (cline <= numColori + 3)){

			colori[cline - 4].simbolo = (char*) malloc (bitColore*sizeof(char));
			//simbolo colore
			for(i=0;i<bitColore;i++){
				colori[cline - 4].simbolo[i] = line[i+1];
			}
			colori[cline - 4].simbolo[bitColore] = '\0';
			colori[cline - 4].occorrenze = 0;

			str = (char*) malloc (6*sizeof(char));
			for(i=0;i<strlen(line);i++){
				if((i > 6) && (i < 13)){
					str[t] = line[i];
					t++;
				}
			}

			//prendo le tre componenti RGB e le porto in decimale
			hexString[0] = str[0];
			hexString[1] = str[1];
			colori[cline - 4].R = strtol(hexString,NULL,16);
			hexString[0] = str[2];
			hexString[1] = str[3];
			colori[cline - 4].G = strtol(hexString,NULL,16);
			hexString[0] = str[4];
			hexString[1] = str[5];
			colori[cline - 4].B = strtol(hexString,NULL,16);

			//sono in ordine dal piu piccolo al piu grande, il colore successivo, ha almeno
			//due componenti maggiori di quello precedente
		}

		else if((cline > numColori + 3) && (cline < altezza + numColori + 4)){

			//il vettore che contiene tutta l'immagine è scritto riga per riga
			c = 0;
			for(i=1;i<(strlen(line) - 2);i++){

				//c è l'indice che mi dice quando sono stati salvati due simboli, quindi
				//quando posso prendere i due simboli per controllare l'occorrenza
				c++;
				//copio un carattere del simbolo all'interno della matrice
				matriceImmagine[j] = line[i];
				//j è l'indice con cui vado a salvari simboli nella matrice
				j++;
				if(c == bitColore){
					c = 0;
					for(j=bitColore-1;j>=0;j--){
						simboloConf[c] = line[i-j];
						c++;
					}
					c = 0;
					simboloConf[bitColore] = '\0';

					for(k=0;k<numColori;k++){
						//è una ricerca per simbolo non per valore non posso fare la dicotomica
						if(strcmp(simboloConf,colori[k].simbolo) == 0){
							colori[k].occorrenze++;
						}
					}
				}
			}
		}

		else{
				//controllo i casi che non rientrano in nessun if
		}
	}

	printf("finito\n");
	free(line);
	getch();
	fclose(fin);
	return 0;
}

/*
 * in questa funzione andiamo a costruire la palette globale dell'immagine basandoci sulle occorrenze dei vari colori
 * all'interno dell'immagine, e basandoci sul numero di colori con cui ci viene richiesta la palette
 * coloriPalette: è il numero di colori con il quale voglio costruire la palette
 */
int CostruzionePaletteGlobale(int coloriPalette){

	int i,k,j,t,c = 0,indiceColore = 0,flag = 0,numColoriAssegnati = 0,bitPrecisione;
	colorVet scambio;
	unsigned char verifica[3];

	//lo moltiplico per tre, perche 3 sono le componenti del colore
	paletteGlobale = (unsigned char*) malloc (3*coloriPalette*sizeof(unsigned char));

	//vado a settare i bit per controllare i colori simili
	bitPrecisione = 6;

	//ordino il vettore che contiene tutti i colori per occorrenze
	//questa operazione andrebbe fatta al momento dell'inserimento
	for(i=0;i<numColori-1;i++){
		for(j=i+1;j<numColori;j++){
			if(colori[j].occorrenze > colori[i].occorrenze){
				scambio = colori[i];
				colori[i] = colori[j];
				colori[j] = scambio;
			}
		}
	}

//----------------------------------------------------------------------------------------------
	//funzionaaaaa
	//con questa funzione se vogliamo differenziare ancora di piu la palette allora basta aumentare
	//il numero di bit che controlliamo che non siano uguali
	//vado a riempire la palette di colori
	flag = 0;
	k = 0;
	j = 0;

	paletteGlobale[k*3] = (colori[k].R) & 0xFF;
	paletteGlobale[(k*3)+1] = (colori[k].G) & 0xFF;
	paletteGlobale[(k*3)+2] = (colori[k].B) & 0xFF;
	//aggiungo uno ai colori assegnati all'interno della palette
	numColoriAssegnati++;
	//indico l'ultima posizione scritta
	indiceColore = j;

	for(k=1;k<numColori;k++){
		if(numColoriAssegnati < coloriPalette){
			flag = 0;
			verifica[0] = colori[k].R & 0xFF;
			verifica[1] = colori[k].G & 0xFF;
			verifica[2] = colori[k].B & 0xFF;
			for(j=0;(j<3*coloriPalette && flag == 0);j=j+3){
				if(((verifica[0] >> bitPrecisione) == (paletteGlobale[j] >> bitPrecisione)) && (((verifica[1] >> bitPrecisione) == (paletteGlobale[j+1] >> bitPrecisione))) && (((verifica[2] >> bitPrecisione) == (paletteGlobale[j+2] >> bitPrecisione)))){
					break;
				}
				if(j == indiceColore){
					//devo scrivere nelle successive posizioni di j
					paletteGlobale[j+3] = verifica[0]; //R
					paletteGlobale[j+4] = verifica[1]; //G
					paletteGlobale[j+5] = verifica[2]; //B
					indiceColore = j + 3;
					flag = 1;
					numColoriAssegnati++;
				}
			}
		}
		else{
			break;
		}
	}

	printf("numColoriAssegnati = %d\n",numColoriAssegnati);

	//nel caso in cui i colori che non sono simili non bastano per riempire tutta la palette
	//faccio un ciclo in cui vado a reinserire tenendo sempre come ordinamento le occorrenze per questi ultimi
	//cioè dopo l'inserimento di tutti i colori non simili, gli ultimi colori saranno inseriti in base alle
	//occorrenze calcolate precedentemente.
	if(numColoriAssegnati < coloriPalette){
		for(k=1;k<numColori;k++){
			if(numColoriAssegnati < coloriPalette){
				flag = 1;
				verifica[0] = colori[k].R & 0xFF;
				verifica[1] = colori[k].G & 0xFF;
				verifica[2] = colori[k].B & 0xFF;
				for(j=0;j<indiceColore;j=j+3){
					//controllo se il colore l'ho gia inserito all'interno della palette
					//con questo indice dovrei arrivare all'ultimo valore inserito
					if((verifica[0] == paletteGlobale[j]) && ((verifica[1] == paletteGlobale[j+1])) && ((verifica[2] == paletteGlobale[j+2]))){
						flag = 0;
						break;
					}
					else{
						flag = 1;
					}
				}
				if(flag == 1){
					//se non l'ho inserito lo inserisco alla prima posizione libera in teoria
					paletteGlobale[indiceColore+3] = colori[k].R & 0xFF;
					paletteGlobale[indiceColore+4] = colori[k].G & 0xFF;
					paletteGlobale[indiceColore+5] = colori[k].B & 0xFF;
					indiceColore = indiceColore + 3;
					numColoriAssegnati++;
				}
			}
			else{
				break;
			}
		}
	}
	printf("numColoriAssegnati = %d\n",numColoriAssegnati);
//------------------------------------------------------------------------------------------------------------

	//stampa della palette
	//stampa in bit 0 e 1 delle tre componenti dei colori
	printf("\n");
	j=0;
	for(i=0;i<3*coloriPalette;i++){
		std::bitset<8> x(paletteGlobale[i]);
		std::cout << (x) <<'\n';
		if(((i+1) % 3) == 0){
			printf("\n");
		}
	}

	getch();
	return 0;
}

int LetturaFilePPM(char nomeFile[]){

	return 0;
}

int main(){

	int res, i,k=2,coloriPalette=0,tipoPalette=0;
	char *nomeFile,ext[3];

	//di sicuro ci sono modi piu efficaci
	res = strlen("2.xpm");
	nomeFile = (char*) malloc (res*sizeof(char));
	strcpy(nomeFile,"2.xpm");

	//ciclo che prende l'estensione del file
	for(i=res-1;i>=res-3;i--){
		ext[k] = nomeFile[i];
		k--;
	}

	if(strcmp(ext,"xpm")==0){
		res |= LetturaFileXPM(nomeFile);
	}
	//if(strcmp(ext,"ppm")==0){
	//	res |= LetturaFilePPM(nomeFile);
	//}

	while((tipoPalette != 1) && (tipoPalette != 2)){
		printf("Per la coversione vuoi fare usare una palette globale o una palette locale ?\n");
		printf("1)palette globale\n");
		printf("2)palette locale\n");
		scanf("%d",&tipoPalette);
	}

	if(tipoPalette == 1){
		//costruzione palette globale

		while((coloriPalette != 1) && ((coloriPalette != 2)) && ((coloriPalette != 3)) && ((coloriPalette != 4)) && ((coloriPalette != 5))){
			printf("da quanti colori vuoi che la palette sia formata ?\n");
			printf("1)16\n");
			printf("2)32\n");
			printf("3)64\n");
			printf("4)128\n");
			printf("5)256\n");
			scanf("%d",&coloriPalette);
		}

		switch(coloriPalette){
		case 1:
			res = CostruzionePaletteGlobale(16);
			break;
		case 2:
			res = CostruzionePaletteGlobale(32);
			break;
		case 3:
			res = CostruzionePaletteGlobale(64);
			break;
		case 4:
			res = CostruzionePaletteGlobale(128);
			break;
		case 5:
			res = CostruzionePaletteGlobale(256);
			break;
		}

	}
	else if(tipoPalette == 2){
		//costruzione palette locale
	}

	if(res < 0){
		printf("errore\n");
	}

	free(nomeFile);
	free(colori);
	return 0;
}
