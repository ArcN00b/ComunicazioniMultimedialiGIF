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

typedef struct{
	unsigned char R;
	unsigned char G;
	unsigned char B;
	char *simbolo;
}paletteSimboli;

colorVet *colori;
paletteSimboli *paletteGlobale;
char *matriceImmagine;
unsigned char *matriceImmagineConvertita;
int larghezza,altezza,numColori,bitColore,coloriPalette;

/*Al termine di questa funzione abbiamo una lista di colori ordinata con il rispettivo simbolo e occorrenza
 * in più abbiamo una matrice dell'immagine scritta tutta su un vettore
 * e le informazioni riguardanti l'immagine
 */
int LetturaFileXPM(char nomeFile[]){

	FILE *fin;
	char *line,hexString[2],simboloConf[bitColore];
	int cline=0,i,k,c=0,j=0,t=0;

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
					matriceImmagineConvertita = (unsigned char*) malloc (altezza * larghezza * sizeof(unsigned char));
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
					for(t=bitColore-1;t>=0;t--){
						simboloConf[c] = line[i-t];
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

	printf("lettura del file XPM completata\n");
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
int CostruzionePaletteGlobale(){

	int i,k,j,t,c = 0,indiceColore = 0,flag = 0,numColoriAssegnati = 0,bitPrecisione;
	colorVet scambio;
	unsigned char verifica[3];

	//vado ad inizializzare il vettore che conterrà i simboli dei colori che si trovano dentro la palette
	paletteGlobale = (paletteSimboli*) malloc (coloriPalette * sizeof(paletteSimboli));

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

	paletteGlobale[k].R = (colori[k].R) & 0xFF;
	paletteGlobale[k].G = (colori[k].G) & 0xFF;
	paletteGlobale[k].B = (colori[k].B) & 0xFF;

	//vado a copiare all'interno il simbolo del colore che viene inserito nella palette
	paletteGlobale[k].simbolo = (char*) malloc (bitColore * sizeof(char));
	strcpy(paletteGlobale[k].simbolo,colori[k].simbolo);

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
			for(j=0;(j<coloriPalette && flag == 0);j++){
				if(((verifica[0] >> bitPrecisione) == (paletteGlobale[j].R >> bitPrecisione)) && (((verifica[1] >> bitPrecisione) == (paletteGlobale[j].G >> bitPrecisione))) && (((verifica[2] >> bitPrecisione) == (paletteGlobale[j].B >> bitPrecisione)))){
					break;
				}
				if(j == indiceColore){
					//devo scrivere nelle successive posizioni di j
					paletteGlobale[j+1].R = verifica[0]; //R
					paletteGlobale[j+1].G = verifica[1]; //G
					paletteGlobale[j+1].B = verifica[2]; //B
					paletteGlobale[numColoriAssegnati].simbolo = (char*) malloc (bitColore * sizeof(char));
					strcpy(paletteGlobale[numColoriAssegnati].simbolo,colori[k].simbolo);
					indiceColore = j + 1;
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
				for(j=0;j<indiceColore;j++){
					//controllo se il colore l'ho gia inserito all'interno della palette
					//con questo indice dovrei arrivare all'ultimo valore inserito
					if((verifica[0] == paletteGlobale[k].R) && ((verifica[1] == paletteGlobale[k].G)) && ((verifica[2] == paletteGlobale[k].B))){
						flag = 0;
						break;
					}
					else{
						flag = 1;
					}
				}
				if(flag == 1){
					//se non l'ho inserito lo inserisco alla prima posizione libera in teoria
					paletteGlobale[indiceColore+1].R = colori[k].R & 0xFF;
					paletteGlobale[indiceColore+1].G = colori[k].G & 0xFF;
					paletteGlobale[indiceColore+1].B = colori[k].B & 0xFF;
					paletteGlobale[numColoriAssegnati].simbolo = (char*) malloc (bitColore * sizeof(char));
					strcpy(paletteGlobale[numColoriAssegnati].simbolo,colori[k].simbolo);
					indiceColore++;
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
	/*printf("\n");
	j=0;
	for(i=0;i<coloriPalette;i++){
		std::bitset<8> x(paletteGlobale[i].R);
		std::cout << (x) <<'\n';
		printf("%d\n",int(paletteGlobale[i].R));
		std::bitset<8> y(paletteGlobale[i].G);
		std::cout << (y) <<'\n';
		printf("%d\n",int(paletteGlobale[i].G));
		std::bitset<8> z(paletteGlobale[i].B);
		std::cout << (z) <<'\n';
		printf("%d\n",int(paletteGlobale[i].B));
		printf("%s\n",paletteGlobale[i].simbolo);
		printf("\n");
	}*/

	printf("creazione della palette globale completata\n");
	getch();
	return 0;
}

//-------------------------------------------------------------------------------------
int CostruzioneMatriceImmagineGIF(){

	int i,c=0,j,flag=0,min,sommaDiff,t=0;
	int componenteRossa,componentVerde,componenteBlu;
	char *simbolo;
	unsigned char pos;

	simbolo = (char*) malloc (bitColore * sizeof(char));

	//leggo tutti i caratteri all'interno della matrice dell'immagine XPM
	for(i=1;i<=strlen(matriceImmagine);i++){
		//ciclo che prende ogni simbolo dell'immagine e verifica se si trova all'interno della palette

		simbolo[c] = matriceImmagine[i-1];
		c++;

		if((i % bitColore) == 0){
			//a questo punto io ho un simbolo e devo andare a recuperare il valore delle sue componenti

			c = 0;
			for(j=0;(j<numColori && flag == 0);j++){
				if(strcmp(simbolo,colori[j].simbolo) == 0){
					flag = 1;
					componenteRossa = colori[j].R;
					componentVerde = colori[j].G;
					componenteBlu = colori[j].B;
				}
			}
			flag = 0;

			//a questo punto abbiamo i valori numerici interi del colore e dobbiamo andare a fare la differenza con i colori all'interno
			//palette
			sommaDiff = 0;
			min = 1000;
			//scorro la palette
			for(j=0;j<coloriPalette;j++){

				//è univoco questo conto ?
				sommaDiff = abs(componenteRossa - int(paletteGlobale[j].R)) + abs(componentVerde - int(paletteGlobale[j].G)) + abs(componenteBlu - int(paletteGlobale[j].B));

				//piu la differenza è minore piu il colore si avvicina all'originale
				if(sommaDiff < min){
					min = sommaDiff;
					pos = j & 0xFF;
				}
			}

			//vado a copiare il valore della posizione del colore all'interno della matrice dell'immagine
			matriceImmagineConvertita[t] = pos;
			printf("%d\n",int(matriceImmagineConvertita[t]));

			//contatore che mi tiene conto del numero di simboli che ho riconosciuto
			t++;
		}
	}

	printf("costruzione matrice immagine GIF completata\n");
	getch();
	return 0;
}
//-------------------------------------------------------------------------------------



int LetturaFilePPM(char nomeFile[]){

	return 0;
}

//-----------------------------------------------------------------------------------------
int main(){

	int res, i,k=2,tipoPalette=0,c=0;
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

		while((c != 1) && ((c != 2)) && ((c != 3)) && ((c != 4)) && ((c != 5))){
			printf("da quanti colori vuoi che la palette sia formata ?\n");
			printf("1)16\n");
			printf("2)32\n");
			printf("3)64\n");
			printf("4)128\n");
			printf("5)256\n");
			scanf("%d",&c);
		}

		switch(c){
		case 1:
			coloriPalette = 16;
			res |= CostruzionePaletteGlobale();
			break;
		case 2:
			coloriPalette = 32;
			res |= CostruzionePaletteGlobale();
			break;
		case 3:
			coloriPalette = 64;
			res |= CostruzionePaletteGlobale();
			break;
		case 4:
			coloriPalette = 128;
			res |= CostruzionePaletteGlobale();
			break;
		case 5:
			coloriPalette = 256;
			res |= CostruzionePaletteGlobale();
			break;
		}

	}
	else if(tipoPalette == 2){
		//costruzione palette locale
	}

	res |= CostruzioneMatriceImmagineGIF();

	if(res < 0){
		printf("errore\n");
	}

	free(nomeFile);
	free(colori);
	return 0;
	//ricorda di togliere tutti i return 0 e di far ritornare la cosa giusta
}
