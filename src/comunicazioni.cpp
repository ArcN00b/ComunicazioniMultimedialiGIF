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
	unsigned char simbolo;
	int R;
	int G;
	int B;
	int occorrenza;
}colorVet;

//Dichiaro le variabili globali utilizzate in futuro
pixelInfo *pixel, *palette;
colorVet *colori;
unsigned char *pixelGif, *pixelLzw;
char *dizionario[65536];
int larghezza, altezza, numColori, lzw;

int cercaDizionario(char stringa[], int fineDizionario) {

	//Variabile locale utile per cercare nel dizionario
	int i;

	//Scorro tutto il dizionario controllando che ci sia la stringa
	for (i = 0; i < fineDizionario; i++) {
		if(strcmp(stringa, dizionario[i]) == 0)
			return i;
	}

	//Se la stringa non viene trovata ritorno -1
	return -1;
}

int compressoreLZW() {

	int i, c, pos, prec, indiceLzw = 0;
	char temp[65536];
	int fineDizionario = numColori;

	printf("Comprimo con LZW\n");

	//Imposto il flag lzw a true
	lzw = 1;

	//Alloco lo spazio per inserire la lista di simboli
	pixelLzw = (unsigned char*) malloc(larghezza * altezza * sizeof(unsigned char));

	//Inizializzo il dizionario
	for(i = 0; i < numColori; i++) {

		//Scrivo i simboli che già conosco nel dizionario
		dizionario[i] = (char*) malloc(sizeof(char));
		itoa(i,dizionario[i], 10);
	}

	//Scorro tutti i pixel dell'immagine
	for(i = 0; i < larghezza * altezza; i++) {

		//Scrivo in forma di stringa il simbolo attuale
		sprintf(temp,"%u", pixelGif[i]);
		c = 1;

		//Cerco se la stringa attuale è presente nel dizionario
		do {
			prec = pos;
			pos = cercaDizionario(temp, fineDizionario);
			//Se temp è presente nel dizionario devo aggiungere a temp un simbolo
			if(pos != -1) {
				sprintf(temp,"%s%u", temp, pixelGif[i + c]);

				//Conto quante volte eseguo il ciclo per aggiornare correttamente i
				c++;

			//Altrimenti devo salvare l'indice attuale
			} else {

				//Aggiungo la stringa composta al dizionario
				dizionario[fineDizionario] = (char*) malloc((strlen(temp) + 1) * sizeof(char));
				strcpy(dizionario[fineDizionario++], temp);

				//Inserisco il codice di output nei pixel LZW a dimensione fissa 8 o 16bit
				if(fineDizionario < 256)
					pixelLzw[indiceLzw++] = prec;
				else {
					pixelLzw[indiceLzw++] = prec >> 8;
					pixelLzw[indiceLzw++] = prec & 255;
				}
			}
		} while(pos != -1 && i + c < larghezza * altezza);

		//Se ho raggiunto la dimensione massima del dizionario lo resetto
		if(fineDizionario == 65536) {
			fineDizionario = numColori;
		}

		//Se ho eseguito il ciclo almeno due volte devo aggiornare i
		if(c > 2)
			i += c - 2;
	}

	//Potrebbe non essere inserita nell'output l'ultima stringa cercata
	if(pos != -1) {
		i -= c + 2;
		dizionario[fineDizionario] = (char*) malloc((strlen(temp) + 1) * sizeof(char));
		strcpy(dizionario[fineDizionario], temp);
	}

	//Ritorno la grandezza del risultato della compressione
	return indiceLzw;
}

void decompressoreLZW(int indiceLzw) {

	int i, pos, indiceGif = 0;
	unsigned int j;
	int fineDizionario = numColori;

	printf("Decomprimo con LZW\n");

	//Inizializzo il dizionario
	for(i = 0; i < numColori; i++) {

		//Scrivo i simboli che già conosco nel dizionario
		dizionario[i] = (char*) malloc(2 * sizeof(char));
		itoa(i, dizionario[i], 10);
	}

	//Scorro tutti i pixel dell'immagine
	i = 0;
	while(i < indiceLzw) {

		//Ottengo la posizione dell'indice dal pixel lzw attuale
		if(fineDizionario < 256 - 1)
			pos = pixelLzw[i++];
		else {
			pos = (pixelLzw[i] << 8) + pixelLzw[i + 1];
			i += 2;
		}

		//Aggiorno il dizionario aggiungendo il nuovo codice a quello precedente
		if(fineDizionario != numColori)
			sprintf(dizionario[fineDizionario - 1], "%s%c", dizionario[fineDizionario - 1], dizionario[pos][0]);

		//Scrivo l'output in base al codice letto attualmente
		for(j = 0; j < strlen (dizionario[pos]); j++)
			pixelGif[indiceGif++] = (unsigned char) dizionario[pos][j];

		//Aggiungo la stringa composta al dizionario
		dizionario[fineDizionario] = (char*) malloc((strlen(dizionario[pos]) + 2) * sizeof(char));
		strcpy(dizionario[fineDizionario++], dizionario[pos]);

		//Se ho raggiunto la dimensione massima del dizionario lo resetto
		if(fineDizionario == 65536)
			fineDizionario = numColori;
	}
}

/*int LetturaFileXPM(char nomeFile[]) {

	FILE *fin;
	char line[256],*str;
	unsigned int cline=0,numColori,bitColore,i,c=0;
	std::string::size_type sz = 0;

	//Apro il file per la lettura controllando che non ci siano problemi
	fin = fopen(nomeFile,"r+");
	if(fin == NULL) {
		fclose(fin);
		return -1;
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

		//questa Ã¨ solo una prova
		if(cline > 260){
			getch();
		}
	}

	//Chiudo il file e ritorno un valore di default
	fclose(fin);
	return 0;
}*/

int LetturaFilePPM(char nomeFile[]) {

	//Dichiaro alcune variabili utili per la lettura da file
	FILE *fin;
	char line[256];
	int cline = 0, cpixel = 0;
	std::string::size_type sz = 0;

	//Apro il file per la lettura controllando che non ci siano problemi
	fin = fopen(nomeFile,"r+");
	if(fin == NULL) {
		fclose(fin);
		return -1;
	}

	//leggo completamente il file
	while (!(feof(fin))){

		//leggo una linea dal file
		fscanf(fin, " %[^\n]", line);
		cline++;

		//Leggo le infomazioni di ogni pixel
		if (cline > 4) {

			//Inserisco le informazioni nella struttura pixel
			if (cline % 3 == 2)
				pixel[cpixel].R = atoi(line);
			else if(cline % 3 == 0)
				pixel[cpixel].G = atoi(line);
			else {
				pixel[cpixel].B = atoi(line);
				cpixel++;
			}

		//Nella terza riga del file ricavo larghezza e altezza immagine
		} else if (cline == 3) {
			larghezza = atoi(strtok(line," "));
			altezza = atoi(strtok(NULL,"\0"));

			//Alloco la struttura dati che contiene tutti i dati dei pixel
			pixel = (pixelInfo*) malloc (larghezza * altezza * sizeof(pixelInfo));

		//Nella quarta riga del file ricavo il numero dei colori
		} else if (cline == 4) {
			numColori = larghezza * altezza;

			//Alloco la struttura dati contenente la lista di colori dell'immagine originale
			colori = (colorVet*) malloc (numColori * sizeof(colorVet));
		}
	}

	//Chiudo il file e ritorno un valore di default
	fclose(fin);
	return 0;
}

int LetturaFileGIF(char nomeFile[]) {

	//Dichiaro alcune variabili utili per la lettura da file
	FILE *fin;
	char line[256];
	int cline = 0, cpixel = 0, ccolori = 0, i;
	numColori = 256; // temporaneo
	std::string::size_type sz = 0;

	//Apro il file per la lettura controllando che non ci siano problemi
	fin = fopen(nomeFile,"r+");
	if(fin == NULL) {
		fclose(fin);
		return -1;
	}

	//leggo completamente il file
	while (!(feof(fin))){

		//leggo una linea dal file
		fscanf(fin, " %[^\n]", line);
		cline++;

		//Leggo le informazioni di ogni pixel
		if(cline > numColori + 1) {
			if(lzw == 0)
				pixelGif[cpixel++] = (unsigned char) atoi(line);
			else
				pixelLzw[cpixel++]  = (unsigned char) atoi(line);

		//Leggo le informazioni delle palette
		} else if(cline > 1 && cline <= numColori + 1){
			//printf("%s\n", line);
			palette[ccolori].R = atoi(strtok(line, " "));
			palette[ccolori].G = atoi(strtok(NULL, " "));
			palette[ccolori++].B = atoi(strtok(NULL, "\0"));

		//Ricavo le informazioni sull'immagine
		} else {
			larghezza = atoi(strtok(line, " "));
			altezza = atoi(strtok(NULL, " "));
			numColori = atoi(strtok(NULL, " "));
			lzw = atoi(strtok(NULL, "\0"));
			palette = (pixelInfo*) malloc(numColori * sizeof(pixelInfo));
			pixelGif = (unsigned char*) malloc(larghezza * altezza * sizeof(unsigned char));
			pixelLzw = (unsigned char*) malloc(larghezza * altezza * sizeof(unsigned char));
		}
	}

	//Decomprimo i pixel se sono stati compressi
	if (lzw == 1)
		decompressoreLZW(cpixel - 1);

	//Alloco la struttura dati che contiene tutti i dati dei pixel
	pixel = (pixelInfo*) malloc (larghezza * altezza * sizeof(pixelInfo));

	//Associo ad ogni pixel il valore ricavato dalla palette
	for(i = 0; i < larghezza * altezza; i++) {
		pixel[i].R = palette[(int) pixelGif[i]].R;
		pixel[i].G = palette[(int) pixelGif[i]].G;
		pixel[i].B = palette[(int) pixelGif[i]].B;
	}

	//Chiudo il file e ritorno un valore di default
	fclose(fin);
	return 0;
}

//Funzione che trova la posizione di un colore all'interno della lista colori
void compilaVettoreColori() {

	//Indici di ricerca
	int i, j, pos;
	colorVet temp;

	//Azzero le occorrenze del vettore dei colori
	for(i = 0; i < numColori; i++)
		colori[i].occorrenza = 0;

	//Scorro tutti i pixel presenti nell'immagine
	for(pos = 0; pos < larghezza * altezza; pos++) {

		//Scorro la lista dei colori per trovare se il colore Ã¨ giÃ  presente o meno
		i = 0;
		while(i < numColori && colori[i].occorrenza != 0 && !(pixel[pos].R == colori[i].R &&
				pixel[pos].G == colori[i].G && pixel[pos].B == colori[i].B))
			i++;

		//Se ho trovato una corrispondenza aggiorno l'occorrenza
		if(colori[i].occorrenza != 0) {

			//Aggiorno la lista dei colori
			colori[i].occorrenza++;

			//Trovo la posizione in cui spostare il colore appena aggiornato
			j = i - 1;
			while(j >= 0 && colori[j].occorrenza < colori[i].occorrenza)
				j--;

			//Effettuo lo scambio di posizione
				temp = colori[j + 1];
				colori[j + 1] = colori[i];
				colori[i] = temp;

		//Inserisco il colore in una nuova posizione
		} else {
			colori[i].R = pixel[pos].R;
			colori[i].G = pixel[pos].G;
			colori[i].B = pixel[pos].B;
			colori[i].occorrenza++;
		}
	}
}

//Funzione che crea le palette e trova il nuovo colore dei pixel
void creaDatiGif(int grandezza) {

	//Dichiaro alcune variabili locali
	int i, j, min, somma;

	//Inizializzo le strutture dati
	numColori = grandezza;
	palette = (pixelInfo*) malloc(numColori * sizeof(colorVet));
	pixelGif = (unsigned char*) malloc(larghezza * altezza * sizeof(unsigned char));

	//Prendo i primi grandezza colori dal vettore colori
	for(i = 0; i < numColori; i++) {
		palette[i].R = colori[i].R;
		palette[i].G = colori[i].G;
		palette[i].B = colori[i].B;
	}

	//Trovo il nuovo colore dei pixel
	for(i = 0; i < larghezza * altezza; i++) {

		//Imposto la ricerca del minimo
		min = 160000000;

		//Scansiono tutta la palette
		for(j = 0; j < numColori; j++) {

			//Cerco la distanza minima tra il colore originale e quelli nella palette
			somma = abs(pixel[i].R - palette[j].R) + abs(pixel[i].G - palette[j].G) + abs(pixel[i].B - palette[j].B);
			if(somma < min) {
				min = somma;
				pixelGif[i] = (unsigned char) j;
			}
		}
	}
}

//Funzione che scrive i pixel all'interno del file
int scriviColori() {

	//Variabili locali utili
	int i = 0;

	//Apro il file per scrivere i colori
	FILE *fout = fopen("ListaColori.txt", "w");
	if(fout == NULL) {
		fclose(fout);
		return -1;
	}

	//Scorro tutta la struttura dati
	while(i < numColori && colori[i].occorrenza != 0) {

		//Scrivo all'interno del file
		fprintf(fout, "R %d:G %d:B %d:O %d\n", colori[i].R, colori[i].G, colori[i].B, colori[i].occorrenza);
		i++;
	}

	//Chiudo il file e ritorno un valore di default
	fclose(fout);
	return 0;
}

int scriviPPM(char nomeFile[]) {

	//Dichiaro alcune variabili locali utili
	int i;

	//Cambio estensione al nome del file
	nomeFile[strlen(nomeFile) - 3] = 'p';
	nomeFile[strlen(nomeFile) - 2] = 'p';
	nomeFile[strlen(nomeFile) - 1] = 'm';

	//Apro il file per scrivere i colori
	FILE *fout = fopen(nomeFile, "w");
	if(fout == NULL) {
		fclose(fout);
		return -1;
	}

	//Scrivo larghezza, altezza e numero colori
	fprintf(fout, "P3\n#CREATOR CAMPI ARMARI\n%d %d\n255\n", larghezza, altezza);

	//Scrivo il contenuto dei pixel
	for(i = 0; i < larghezza * altezza; i++)
		fprintf(fout, "%d\n%d\n%d\n", pixel[i].R, pixel[i].G, pixel[i].B);

	//Chiudo il file e ritorno un valore di default
	fclose(fout);
	return 0;
}

int scriviGIF(char nomeFile[], unsigned char daScrivere[], int grandezza) {

	//Dichiaro alcune variabili locali utili
	int i;

	//Cambio estensione al nome del file
	nomeFile[strlen(nomeFile) - 3] = 'g';
	nomeFile[strlen(nomeFile) - 2] = 'i';
	nomeFile[strlen(nomeFile) - 1] = 'f';

	//Apro il file per scrivere i colori
	FILE *fout = fopen(nomeFile, "w");
	if(fout == NULL) {
		fclose(fout);
		return -1;
	}

	//Scrivo larghezza, altezza e numero colori
	fprintf(fout, "%d %d %d %d\n", larghezza, altezza, numColori, lzw);

	//Scrivo tutta la palette
	for(i = 0; i < numColori; i++) {
		fprintf(fout, "%d %d %d\n", palette[i].R, palette[i].G, palette[i].B);
		//printf("%d %d %d\n", palette[i].R, palette[i].G, palette[i].B);
	}
	//Scrivo il contenuto dei pixel
	for(i = 0; i < grandezza; i++)
		fprintf(fout, "%u\n", daScrivere[i]);

	//Chiudo il file e ritorno un valore di default
	fclose(fout);
	return 0;
}

int main(){

	int res, i,k=2, grandezzaLzw;
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
	if(strcmp(ext,"xpm") == 0) {
		//res = LetturaFileXPM(nomeFile);
	} else if(strcmp(ext,"ppm") == 0) {

		//Eseguo la lettura da file .ppm
		if(LetturaFilePPM(nomeFile) == 0)
			printf("Lettura immagine completata\n");

		//Compilo la struttura che contiene la lista dei colori
		printf("Compilo vettore colori\n");
		compilaVettoreColori();

		//Scrivo la lista dei colori nel file
		if(scriviColori() == 0)
			printf("Scrittura colori completata\n");

		//Creo la palette globale
		printf("Creo dati gif\n");
		creaDatiGif(256);

		grandezzaLzw = compressoreLZW();

		//Scrivo l'immagine gif
		if(lzw == 1) {
			if(scriviGIF(nomeFile, pixelLzw, grandezzaLzw) == 0)
				printf("Scrittura gif lzw avvenuta con successo\n");
			free(pixelLzw);

		} else {
			if(scriviGIF(nomeFile, pixelGif, larghezza * altezza) == 0)
				printf("Scrittura gif avvenuta con successo\n");
		}

		//Libero le strutture dati
		free(colori);
		free(palette);
		free(pixel);
		free(pixelGif);

		//Leggo l'immagine gif
		strcpy(nomeFile,"assassins_creed_syndicate-1280x800.gif");
		LetturaFileGIF(nomeFile);

		//Scrivo il file.ppm
		strcpy(nomeFile,"assassins_creed_syndicate-1280x800_prova.gif");
		scriviPPM(nomeFile);

	} else {
		printf("errore\n");
	}

	//scanf("%d",&i);
	/*//Libero le strutture precedentemente utilizzate
	free(nomeFile);
	free(colori);
	free(palette);
	free(pixel);
	free(pixelGif);*/
	return 0;
}
