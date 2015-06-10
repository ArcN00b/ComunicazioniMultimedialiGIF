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
//#include<bitset>
//#include<iostream>
//#include<math.h>

//Struttura in cui vengono inseriti i valori dei pixelPPM trovati
typedef struct pixelInfo{
	int R;
	int G;
	int B;
}pixelInfo;

//Struttura in cui vengono inseriti i colori trovati
typedef struct colorVet{
	unsigned char simbolo[5];
	int R;
	int G;
	int B;
	int occorrenza;
}colorVet;

//Dichiaro le variabili globali utilizzate in futuro
pixelInfo *pixelPPM, *palette[16];
colorVet *colori[16];
unsigned char *pixelGif[16], *pixelLzw[16];
char *pixelXpm;
int larghezza, altezza, coloriPalette, bitColore, bitPrecisione, numPalette, lzw;
//Vettore utilizzato per conoscere la quantità di pixel scritta e letta da file
int lunghezzaPixel[16];

//-----------------------------------------------------------------------------------------------------
/* Funzione che trova sceglie la palette locale da associare al pixel indicato dall'indice
 */
int selezionaPalette(int indice, int selettore) {

	//Se la palette è una unica non devo aggiornare l'indice
	if(numPalette == 1 || indice == 0)
		return selettore;

	//Alcuni controlli che identificano il settore giusto
	if((indice % larghezza) % int(larghezza / numPalette) == 0 && indice % larghezza != larghezza - numPalette + 1)
		selettore ++;
	if(indice % larghezza == 0)
		selettore -= numPalette;
	if(indice % int(altezza * larghezza / numPalette) == 0 && (indice < larghezza * altezza - larghezza))
		selettore += numPalette;

	//ritorno la posizione trovata
	return selettore;
}

//-----------------------------------------------------------------------------------------------------
/* Funzione che trova la posizione di un insieme di numeri all'interno di un dizionario numerico
 * se non è possibile trovare la posizione ritorna il valore -1
 */
int cercaDizionario(int attuale[], int *dizionario[], int fineDizionario) {

	//Variabile locale utile per cercare nel dizionario
	int i = 0, j, f;

	//Scorro tutto il dizionario controllando che ci sia la stringa
	while(i < fineDizionario) {

		//Assegno i valori alle variabili di controllo della ricerca
		j = -1;
		f = 0;

		//Scorro le stringhe attuali
		do {

			//Aggiorno j
			j++;

			//Controllo se ci sono delle differenze
			if(attuale[j] != dizionario[i][j])
				f = 1;

		//Continuo finchè non ci sono differenze oppure finchè non arrivo a fine stringhe
		} while(f == 0 && attuale[j] != -1 && dizionario[i][j] != -1);

		//Controllo che i vettori confrontati abbiano la stessa lunghezza
		if(attuale[j] == dizionario[i][j])
			return i;

		//Aggiorno i
		i++;
	}

	//Se la stringa non viene trovata ritorno -1
	return -1;
}

//-----------------------------------------------------------------------------------------------------
/* Funzione che, tramite utilizzo dell'algoritmo LZW, comprime i pixelPPM per il file gif
 */
int compressoreLZW(int indice) {

	int i, j, c, pos, prec, indiceLzw = 0;
	int temp[65536], *dizionario[65536];
	int fineDizionario = coloriPalette;

	printf("Comprimo con LZW\n");

	//Imposto il flag lzw a true
	lzw = 1;

	//Alloco lo spazio per inserire la lista di simboli
	pixelLzw[indice] = (unsigned char*) malloc(larghezza * altezza * sizeof(unsigned char));

	//Inizializzo il dizionario
	for(j = 0; j < coloriPalette; j++) {

		//Scrivo i simboli che già conosco nel dizionario
		dizionario[j] = (int*) malloc(2 * sizeof(int));
		dizionario[j][0] = j;
		dizionario[j][1] = -1;
	}

	//Scorro tutti i pixelPPM dell'immagine
	for(i = 0; i < lunghezzaPixel[indice]; i++) {

		//Scrivo in forma di stringa il simbolo attuale
		temp[0] = pixelGif[indice][i];
		temp[1] = -1;
		c = 1;

		//Cerco se la stringa attuale è presente nel dizionario
		do {
			prec = pos;
			pos = cercaDizionario(temp, dizionario, fineDizionario);

			//Se temp è presente nel dizionario devo aggiungere a temp un simbolo
			if(pos != -1) {
				temp[c] = pixelGif[indice][i + c];
				temp[c + 1] = -1;

				//Conto quante volte eseguo il ciclo per aggiornare correttamente i
				c++;

			//Altrimenti devo salvare l'indice attuale
			} else {

				//Aggiungo la stringa composta al dizionario
				dizionario[fineDizionario] = (int*) malloc((c + 2) * sizeof(int));
				for(j = 0; j <= c; j++)
					dizionario[fineDizionario][j] = temp[j];

				//Inserisco il codice di output nei pixelPPM LZW a dimensione fissa 8 o 16bit
				if(fineDizionario < 256)
					pixelLzw[indice][indiceLzw++] = prec;
				else {
					pixelLzw[indice][indiceLzw++] = prec >> 8;
					pixelLzw[indice][indiceLzw++] = prec & 255;
				}
			}
		} while(pos != -1 && i + c < larghezza * altezza);

		//Aggiorno la fine del dizionario
		fineDizionario++;

		//Se ho raggiunto la dimensione massima del dizionario lo resetto
		if(fineDizionario == 65536) {
			fineDizionario = coloriPalette;

			//Azzero il dizionario
			for(j = 0; j < 65536; j++)
				free(dizionario[j]);

			//Inizializzo il dizionario
			for(j = 0; j < coloriPalette; j++) {

				//Scrivo i simboli che già conosco nel dizionario
				dizionario[j] = (int*) malloc(2 * sizeof(int));
				dizionario[j][0] = j;
				dizionario[j][1] = -1;
			}
		}

		//Se ho eseguito il ciclo almeno due volte devo aggiornare i
		if(c > 2)
			i += c - 2;
	}

	//Potrebbe non essere inserita nell'output l'ultima stringa cercata
	if(pos != -1) {
		//Aggiungo la stringa composta al dizionario
		dizionario[fineDizionario] = (int*) malloc((c + 2) * sizeof(int));
		for(j = 0; j <= c; j++)
			dizionario[fineDizionario][j] = dizionario[pos][j];
	}

	//Ritorno la grandezza del risultato della compressione
	return indiceLzw;
}

//-----------------------------------------------------------------------------------------------------
/* Funzione che, tramite utilizzo dell'algoritmo LZW, decomprime i pixelPPM per il file raw
 */
int decompressoreLZW(int indiceLzw, int indice) {

	int i, j, pos, indiceGif = 0;
	int *dizionario[65536], numCifre[65536];
	int fineDizionario = coloriPalette;

	printf("Decomprimo con LZW\n");

	//Inizializzo il dizionario
	for(j = 0; j < coloriPalette; j++) {

		//Scrivo i simboli che già conosco nel dizionario
		dizionario[j] = (int*) malloc(sizeof(int));
		dizionario[j][0] = j;
		numCifre[j] = 1;
	}

	//Scorro tutti i pixelPPM dell'immagine
	i = 0;
	while(i < indiceLzw) {

		//Ottengo la posizione dell'indice dal pixelPPM lzw attuale
		if(fineDizionario < 256) {
			pos = pixelLzw[indice][i++];
		} else {
			pos = (pixelLzw[indice][i] << 8) + pixelLzw[indice][i + 1];
			i += 2;
		}

		//Aggiorno il dizionario aggiungendo il nuovo codice a quello precedente
		if(fineDizionario != coloriPalette) {
			dizionario[fineDizionario - 1][numCifre[fineDizionario - 1]] = dizionario[pos][0];
			numCifre[fineDizionario -1]++;
		}

		//Scrivo l'output in base al codice letto attualmente
		for(j = 0; j < numCifre[pos]; j++) {
			pixelGif[indice][indiceGif] = (unsigned char) dizionario[pos][j];
			indiceGif++;
		}

		//Aggiungo la stringa composta al dizionario
		dizionario[fineDizionario] = (int*) malloc((numCifre[pos] + 3) * sizeof(int));
		numCifre[fineDizionario] = numCifre[pos];
		for(j = 0; j < numCifre[pos]; j++)
			dizionario[fineDizionario][j] = dizionario[pos][j];

		//Aggiorno l'indice di fine dizionario
		fineDizionario++;

		//Se ho raggiunto la dimensione massima del dizionario lo resetto
		if(fineDizionario == 65536) {
			fineDizionario = coloriPalette;

			//Azzero il dizionario
			for(j = 0; j < 65536; j++){
				free(dizionario[j]);
				numCifre[j] = 0;
			}

			//Inizializzo il dizionario
			for(j = 0; j < coloriPalette; j++) {

				//Scrivo i simboli che già conosco nel dizionario
				dizionario[j] = (int*) malloc(sizeof(int));
				dizionario[j][0] = j;
				numCifre[j] = 1;
			}
		}
	}

	//ritorno la lunghezza dei pixel generati
	return indiceGif;
}

//-----------------------------------------------------------------------------------------------------
/* Funzione che legge un file in formato ASCII ppm e compila le strutture dati di conseguenza
 */
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

		//Leggo le infomazioni di ogni pixelPPM
		if (cline > 4) {

			//Inserisco le informazioni nella struttura pixelPPM
			if (cline % 3 == 2)
				pixelPPM[cpixel].R = atoi(line);
			else if(cline % 3 == 0)
				pixelPPM[cpixel].G = atoi(line);
			else {
				pixelPPM[cpixel].B = atoi(line);
				cpixel++;
			}

		//Nella terza riga del file ricavo larghezza e altezza immagine
		} else if (cline == 3) {
			larghezza = atoi(strtok(line," "));
			altezza = atoi(strtok(NULL,"\0"));

			//Alloco la struttura dati che contiene tutti i dati dei pixelPPM
			pixelPPM = (pixelInfo*) malloc (larghezza * altezza * sizeof(pixelInfo));
		}
	}

	//Chiudo il file e ritorno un valore di default
	fclose(fin);
	return 0;
}

//-----------------------------------------------------------------------------------------------------
/* Funzione che legge un file in formato ASCII gif e compila le strutture dati di conseguenza
 */
int LetturaFileGIF(char nomeFile[]) {

	//Dichiaro alcune variabili utili per la lettura da file
	FILE *fin;
	char line[256];
	int cline = 0, ccolori = 0, i, c[16], selettore = 0;
	coloriPalette = 256; // temporaneo
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

		//Se è presente il separatore aggiorno gli indici
		if(line[0] == ',') {
			i++;
			cline = 1;
			ccolori = 0;

			//Leggo le informazioni di ogni pixelPPM
		} else if(cline > coloriPalette + 1) {
			if(lzw == 0)
				pixelGif[i][lunghezzaPixel[i]++] = (unsigned char) atoi(line);
			else
				pixelLzw[i][lunghezzaPixel[i]++]  = (unsigned char) atoi(line);

		//Leggo le informazioni delle palette
		} else if(cline > 1 && cline <= coloriPalette + 1){
			palette[i][ccolori].R = atoi(strtok(line, " "));
			palette[i][ccolori].G = atoi(strtok(NULL, " "));
			palette[i][ccolori++].B = atoi(strtok(NULL, "\0"));

		//Ricavo le informazioni sull'immagine
		} else {
			larghezza = atoi(strtok(line, " "));
			altezza = atoi(strtok(NULL, " "));
			coloriPalette = atoi(strtok(NULL, " "));
			numPalette = atoi(strtok(NULL, " "));
			lzw = atoi(strtok(NULL, "\0"));
			for(i = 0; i < numPalette * numPalette; i++) {
				palette[i] = (pixelInfo*) malloc(coloriPalette * sizeof(pixelInfo));
				pixelGif[i] = (unsigned char*) malloc(larghezza * altezza * sizeof(unsigned char));
				pixelLzw[i] = (unsigned char*) malloc(larghezza * altezza * sizeof(unsigned char));
				lunghezzaPixel[i] = 0;
			}
			i = 0;
		}
	}

	//Elimino l'ultima linea (poichè vuota) dai pixel utilizzabili
	lunghezzaPixel[numPalette * numPalette - 1]--;

	//Decomprimo i pixelPPM se sono stati compressi
	if (lzw == 1) {
		for(i = 0; i < numPalette * numPalette; i++) {
			lunghezzaPixel[i] = decompressoreLZW(lunghezzaPixel[i], i);
		}
	}

	//Alloco la struttura dati che contiene tutti i dati dei pixelPPM
	pixelPPM = (pixelInfo*) malloc (larghezza * altezza * sizeof(pixelInfo));

	//Inizializzo il contatore delle occorrenze
	for(i = 0; i < numPalette * numPalette; i++)
		c[i] = 0;

	//Associo ad ogni pixelPPM il valore ricavato dalla palette
	for(i = 0; i < larghezza * altezza; i++) {

		//Trovo lo spazio da utilizzare nelle strutture
		selettore = selezionaPalette(i, selettore);
		//printf("i = %d, selettore = %d\n", i, selettore);
		pixelPPM[i].R = palette[selettore][(int) pixelGif[selettore][c[selettore]]].R;
		pixelPPM[i].G = palette[selettore][(int) pixelGif[selettore][c[selettore]]].G;
		pixelPPM[i].B = palette[selettore][(int) pixelGif[selettore][c[selettore]]].B;
		c[selettore]++;
	}

	//Chiudo il file e ritorno un valore di default
	fclose(fin);
	return 0;
}

/* Funzione che conta le occorrenze dei colori nei pixelPPM originali della foto riodinandoli in modo
 * decrescente, infine crea le palette e trova il nuovo colore dei pixelPPM associando l'indice corretto
 */
void creaDatiGifDaPPM() {

	/*
	 * Blocco per la conta delle occorrenze e il riodino dei colori
	 */
	int i, j, k, pos, min, somma, c[16], selettore = 0;
	colorVet temp;

	//Azzero le occorrenze del vettore dei colori
	for(i = 0; i < numPalette * numPalette; i++) {

		//Alloco la struttura dati contenente la lista di colori dell'immagine originale
		colori[i] = (colorVet*) malloc (larghezza * altezza * sizeof(colorVet));
		c[i] = 0;

		//Azzero tutte le occorrenze
		for(j = 0; j < larghezza * altezza; j++) {
			colori[i][j].occorrenza = 0;
		}
	}

	//Scorro tutti i pixelPPM presenti nell'immagine
	for(pos = 0; pos < larghezza * altezza; pos++) {

		//Scorro la lista dei colori per trovare se il colore è già presente o meno
		i = 0;
		selettore = selezionaPalette(pos, selettore);
		//printf("pos = %d, selettore = %d\n", pos, selettore);
		while(i <= c[selettore] && colori[selettore][i].occorrenza != 0 && !(pixelPPM[pos].R == colori[selettore][i].R &&
				pixelPPM[pos].G == colori[selettore][i].G && pixelPPM[pos].B == colori[selettore][i].B)) {
			i++;
		}
		//Se ho trovato una corrispondenza aggiorno l'occorrenza
		if(colori[selettore][i].occorrenza != 0) {

			//Aggiorno la lista dei colori
			colori[selettore][i].occorrenza++;

			//Trovo la posizione in cui spostare il colore appena aggiornato
			j = i - 1;
			while(j >= 0 && colori[selettore][j].occorrenza < colori[selettore][i].occorrenza)
				j--;

			//Effettuo lo scambio di posizione
			temp = colori[selettore][j + 1];
			colori[selettore][j + 1] = colori[selettore][i];
			colori[selettore][i] = temp;

		//Inserisco il colore in una nuova posizione
		} else {
			colori[selettore][i].R = pixelPPM[pos].R;
			colori[selettore][i].G = pixelPPM[pos].G;
			colori[selettore][i].B = pixelPPM[pos].B;
			colori[selettore][i].occorrenza++;
			c[selettore]++;
		}
	}

	/*
	 * Blocco la ricerca del nuovo colore di ogni pixelPPM
	 */

	//Inizializzo le strutture dati
	for(i = 0; i < numPalette * numPalette; i++) {
		palette[i] = (pixelInfo*) malloc(coloriPalette * sizeof(colorVet));
		pixelGif[i] = (unsigned char*) malloc(larghezza * altezza * sizeof(unsigned char));
	}

	//Prendo i primi grandezza colori dal vettore colori
	for(i = 0; i < numPalette * numPalette; i++) {
		c[i] = 0;
		j = 0;

		//Scorro la lista dei colori
		while(j <= larghezza * altezza && c[i] < coloriPalette) {

			//Scorro la palette controllando che il colore non sia già presente
			k = 0;
			while(k <= c[i] && !(palette[i][k].R >> bitPrecisione == colori[i][j].R >> bitPrecisione && palette[i][k].G >> bitPrecisione == colori[i][j].G >> bitPrecisione &&
					palette[i][k].B >> bitPrecisione == colori[i][j].B >> bitPrecisione))
				k++;

			//Se è vera allora ho un colore già presente
			if(k <= c[i])
				j++;

			//In caso contrario aggiungo il colore alla palette
			else {
				palette[i][c[i]].R = colori[i][j].R;
				palette[i][c[i]].G = colori[i][j].G;
				palette[i][c[i]].B = colori[i][j].B;
				c[i]++;
				j++;
			}
		}
	}

	//Se le palette sono composte con meno colori di quelli che vengono selezionati
	//Prendo i primi grandezza colori dal vettore colori
	for(i = 0; i < numPalette * numPalette; i++) {
		j = 0;

		//Scorro la lista dei colori
		while(j <= larghezza * altezza && c[i] < coloriPalette) {

			//Scorro la palette controllando che il colore non sia già presente
			k = 0;
			while(k <= c[i] && !(palette[i][k].R == colori[i][j].R && palette[i][k].G == colori[i][j].G &&
					palette[i][k].B == colori[i][j].B))
				k++;

			//Se è vera allora ho un colore già presente
			if(k <= c[i])
				j++;

			//In caso contrario aggiungo il colore alla palette
			else {
				palette[i][c[i]].R = colori[i][j].R;
				palette[i][c[i]].G = colori[i][j].G;
				palette[i][c[i]].B = colori[i][j].B;
				c[i]++;
				j++;
			}
		}
	}

	//Inizializzo il contatore delle occorrenze
	for(i = 0; i < numPalette * numPalette; i++)
		lunghezzaPixel[i] = 0;

	//Trovo il nuovo colore dei pixelPPM
	selettore = 0;
	for(i = 0; i < larghezza * altezza; i++) {

		//Imposto la ricerca del minimo
		min = 160000000;
		selettore = selezionaPalette(i, selettore);

		//Scansiono tutta la palette
		for(j = 0; j < coloriPalette; j++) {

			//Cerco la distanza minima tra il colore originale e quelli nella palette
			somma = abs((pixelPPM[i].R) - (palette[selettore][j].R)) + abs((pixelPPM[i].G) - (palette[selettore][j].G)) +
					abs((pixelPPM[i].B) - (palette[selettore][j].B));
			if(somma < min) {
				min = somma;
				pixelGif[selettore][lunghezzaPixel[selettore]] = (unsigned char) j;
			}

			//Se ho già trovato il minimo
			if(min == 0)
				j += coloriPalette;
		}

		//Aggiorno il contatore dei pixel
		lunghezzaPixel[selettore]++;
	}
}

//-----------------------------------------------------------------------------------------------------
/* Funzione utilizzata per scrivere un file in formato .PPM dai dati elaborati
 */
int scriviPPM(char nomeFile[]) {

	//Dichiaro alcune variabili locali utili
	int i;

	//Cambio estensione al nome del file
	nomeFile[strlen(nomeFile) - 6] = 'p';
	nomeFile[strlen(nomeFile) - 5] = 'm';
	nomeFile[strlen(nomeFile) - 4] = '.';
	nomeFile[strlen(nomeFile) - 3] = 'p';
	nomeFile[strlen(nomeFile) - 2] = 'p';
	nomeFile[strlen(nomeFile) - 1] = 'm';

	//Apro il file per scrivere i colori
	FILE *fout = fopen(nomeFile, "w");
	if(fout == NULL) {
		fclose(fout);
		return -1;
	}

	//Scrivo l'intestazione del file
	fprintf(fout, "P3\n#CREATOR CAMPI ARMARI\n%d %d\n255\n", larghezza, altezza);

	//Scrivo il contenuto dei pixelPPM
	for(i = 0; i < larghezza * altezza; i++)
		fprintf(fout, "%d\n%d\n%d\n", pixelPPM[i].R, pixelPPM[i].G, pixelPPM[i].B);

	//Chiudo il file e ritorno un valore di default
	fclose(fout);
	return 0;
}

//-----------------------------------------------------------------------------------------------------
/* Funzione utilizzata per scrivere un file in formato .GIF dai dati elaborati
 */
int scriviGIF(char nomeFile[], unsigned char *daScrivere[]) {

	//Dichiaro alcune variabili locali utili
	int i, j;

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

	//Scrivo larghezza, altezza, numero colori e un flag lzw
	fprintf(fout, "%d %d %d %d %d\n", larghezza, altezza, coloriPalette, numPalette, lzw);

	//Scrivo il resto del file gif
	for(i = 0; i < numPalette * numPalette; i++) {

		//Scrivo tutta la palette globale
		for(j = 0; j < coloriPalette; j++)
			fprintf(fout, "%d %d %d\n", palette[i][j].R, palette[i][j].G, palette[i][j].B);

		//Scrivo il valore dei pixel
		for(j = 0; j < lunghezzaPixel[i]; j++)
			fprintf(fout, "%u\n", daScrivere[i][j]);

		//Controllo se scrivere o meno il separatore di palette
		if(i != numPalette * numPalette - 1)
			fprintf(fout, ",\n");
	}

	//Chiudo il file e ritorno un valore di default
	fclose(fout);
	return 0;
}

int main(){

	/* ****************************************************************** *
	 * Lettura del file                                                   *
	 * ****************************************************************** */

	//Alcune variabili locali utili alla gestione dell'applicativo
	int i, j = 2, input;
	char nomeFile[1024], estensione[3];

	//Chiedo all'utente il file da utilizzare
	printf("Inserire il nome del file da aprire\n");
	scanf("%s", nomeFile);

	//Controllo che sia stato selezionato un file
	while(strlen(nomeFile) == 0) {

		//Esecuzione errata del programma
		fprintf(stderr, "Specificare un file da utilizzare (trascinarlo in questa finestra è il modo più semplice)\n");
	}

	//Ricavo l'estensione del file
	for(i = 1; i <= 3; i++)
		estensione[j--] = nomeFile[strlen(nomeFile) - i];


	/* ****************************************************************** *
	 * Elaborazione di un .xpm                                            *
	 * ****************************************************************** */
	if(strcmp(estensione, "xpm") == 0) {

		//Eseguo la lettura da file .xpm
		//if(LetturaFileXPM(nomeFile) == 0)
			printf("Lettura immagine completata\n");

	/* ****************************************************************** *
	 * Elaborazione di un .ppm                                            *
	 * ****************************************************************** */
	} else if(strcmp(estensione,"ppm") == 0) {

		//Eseguo la lettura da file .ppm
		if(LetturaFilePPM(nomeFile) == 0)
			printf("Lettura immagine completata\n");

		//Stampo il menù a video
		input = 0;
		while(input != 1 && input != 2 && input != 3 && input != 4 && input != 5 && input != 6 && input != 7 && input != 8){
			printf("Quanti colori uso per comporre la palette?\n");
			printf("1)2\n");
			printf("2)4\n");
			printf("3)8\n");
			printf("4)16\n");
			printf("5)32\n");
			printf("6)64\n");
			printf("7)128\n");
			printf("8)256\n");
			scanf("%d",&input);
		}

		//Controllo l'input appena inserito
		switch(input){
		case 1:
			coloriPalette = 2;
			break;
		case 2:
			coloriPalette = 4;
			break;
		case 3:
			coloriPalette = 8;
			break;
		case 4:
			coloriPalette = 16;
			break;
		case 5:
			coloriPalette = 32;
			break;
		case 6:
			coloriPalette = 64;
			break;
		case 7:
			coloriPalette = 128;
			break;
		case 8:
			coloriPalette = 256;
			break;
		}

		//Stampo il menù a video per la gestione del numero palette
		input = 0;
		while(input != 1 && input != 2 && input != 3 && input != 4){
			printf("Quante palette uso?\n");
			printf("1)Una globale\n");
			printf("2)2x2\n");
			printf("3)3x3\n");
			printf("4)4x4\n");
			scanf("%d",&input);
		}

		//Controllo l'input appena inserito
		switch(input){
		case 1:
			numPalette = 1;
			break;
		case 2:
			numPalette = 2;
			break;
		case 3:
			numPalette = 3;
			break;
		case 4:
			numPalette = 4;
			break;
		}

		//Stampo il menù a video
		input = -1;
		while(input < 0 || input > 8){
			printf("Quanti bit di precisione uso nella scelta dei colori? (0 <= input <= 8)\n");
			scanf("%d",&input);
		}

		//Assegno il numero inserito alla variabile globale bitPrecisione
		bitPrecisione = input;

		//Creo la palette globale
		printf("Creo dati gif\n");
		creaDatiGifDaPPM();

		//Stampo il menù a video
		input = 0;
		while(input != 1 && input != 2){
			printf("Utilizzare compressione LZW?\n");
			printf("1)Si\n");
			printf("2)No\n");
			scanf("%d",&input);
		}

		//Controllo l'input appena inserito
		if(input == 1)
			for(i = 0; i < numPalette * numPalette; i++)
				lunghezzaPixel[i] = compressoreLZW(i);

		//Scrivo l'immagine gif
		if(lzw == 1) {
			if(scriviGIF(nomeFile, pixelLzw) == 0)
				printf("Scrittura gif lzw avvenuta con successo\n");

		} else {
			if(scriviGIF(nomeFile, pixelGif) == 0)
				printf("Scrittura gif avvenuta con successo\n");
		}

	/* ****************************************************************** *
	 * Elaborazione di un .gif                                            *
	 * ****************************************************************** */
	} else if(strcmp(estensione, "gif") == 0) {

		//Leggo l'immagine gif
		LetturaFileGIF(nomeFile);
		printf("Lettura immagine completata\n");

		//Stampo il menù a video
		input = 0;
		while(input != 1 && input != 2){
			printf("In che formato decomprimere il file .gif?\n");
			printf("1)XPM\n");
			printf("2)PPM\n");
			scanf("%d",&input);
		}

		//Controllo l'input appena inserito
		switch(input){
		case 1:
			//scriviXPM(argv[1]);
			break;
		case 2:
			scriviPPM(nomeFile);
			break;
		}

	//In caso di errori
	} else
		printf("Formato non riconosciuto\n");

	return 0;
}
