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
pixelInfo *pixelPPM, *palette;
colorVet *colori;
unsigned char *pixelGif, *pixelLzw;
char *pixelXpm;
int larghezza, altezza, coloriPalette, lzw, bitColore, numColori;

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

		do {

			//Aggiorno j
			j++;

			//Controllo se ci sono delle differenze
			if(attuale[j] != dizionario[i][j])
				f = 1;

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
int compressoreLZW() {

	int i, j, c, pos, prec, indiceLzw = 0;
	int temp[65536], *dizionario[65536];
	int fineDizionario = coloriPalette;

	printf("Comprimo con LZW\n");

	//Imposto il flag lzw a true
	lzw = 1;

	//Alloco lo spazio per inserire la lista di simboli
	pixelLzw = (unsigned char*) malloc(larghezza * altezza * sizeof(unsigned char));

	//Inizializzo il dizionario
	for(j = 0; j < coloriPalette; j++) {

		//Scrivo i simboli che già conosco nel dizionario
		dizionario[j] = (int*) malloc(2 * sizeof(int));
		dizionario[j][0] = j;
		dizionario[j][1] = -1;
	}

	//Scorro tutti i pixelPPM dell'immagine
	for(i = 0; i < larghezza * altezza; i++) {

		//Scrivo in forma di stringa il simbolo attuale
		temp[0] = pixelGif[i];
		temp[1] = -1;
		c = 1;

		//Cerco se la stringa attuale è presente nel dizionario
		do {
			prec = pos;
			pos = cercaDizionario(temp, dizionario, fineDizionario);

			//Se temp è presente nel dizionario devo aggiungere a temp un simbolo
			if(pos != -1) {
				temp[c] = pixelGif[i + c];
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
					pixelLzw[indiceLzw++] = prec;
				else {
					pixelLzw[indiceLzw++] = prec >> 8;
					pixelLzw[indiceLzw++] = prec & 255;
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
void decompressoreLZW(int indiceLzw) {

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
		if(fineDizionario < 256 - 1) {
			pos = pixelLzw[i++];
		} else {
			pos = (pixelLzw[i] << 8) + pixelLzw[i + 1];
			i += 2;
		}

		//Aggiorno il dizionario aggiungendo il nuovo codice a quello precedente
		if(fineDizionario != coloriPalette) {
			dizionario[fineDizionario - 1][numCifre[fineDizionario - 1]] = dizionario[pos][0];
			numCifre[fineDizionario -1]++;
		}

		//Scrivo l'output in base al codice letto attualmente
		for(j = 0; j < numCifre[pos]; j++) {
			pixelGif[indiceGif] = (unsigned char) dizionario[pos][j];
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
}

//-----------------------------------------------------------------------------------------------------
/* Funzione che legge un file in formato xpm e compila le strutture dati di conseguenza
 */
int LetturaFileXPM(char nomeFile[]) {

	FILE *fin;
	char *line, *str, hexString[2], simboloConf[bitColore];
	int i, cline=0, k, c=0, j=0, t, o=0, flag = 0, pos = 0;

	//Apro il file per la lettura controllando che non ci siano problemi
	fin = fopen(nomeFile,"r+");
	if(fin == NULL){
		fclose(fin);
		return -1;
	}

	//Dichiaro uno spazio iniziale per leggere le varie righe del file
	line = (char*) malloc (256 * sizeof(char));

	//Scorro tutto il file
	while(!(feof(fin))){

		//Leggo la linea contando quante e aggiorno il contatore delle linee lette
		fscanf(fin, " %[^\n]",line);
		cline++;

		//Lettura della linea giusta in cui trovare le informazioni sull'immagine
		if(cline == 3 && line[0] != '/'){

			//leggo il primo valore (larghezza) eliminando il carattere " iniziale
			str = strtok(line, " ");
			for(i = 0; i < strlen(str); i++)
				str[i] = str[i + 1];
			larghezza = atoi(str);

			//Leggo altezza e numero colori dalla linea
			altezza = atoi(strtok(line, " "));
			numColori = atoi(strtok(NULL, " "));

			//Leggo i bit del colore eliminando il carattere " finale
			str = strtok(NULL, "\0");
			str[strlen(str) - 1] = str[strlen(str)];
			bitColore = atoi(str);

			//Aumento la dimensione di line in base alla larghezza del file
			line = (char*) malloc (larghezza * bitColore * sizeof(char) + 6);

			//Alloco nella dimensione esatta lo spazio per le altre strutture
			pixelXpm = (char*) malloc (altezza * larghezza * bitColore * sizeof(char));
			pixelGif = (unsigned char*) malloc (altezza * larghezza * sizeof(unsigned char));

		//Nel caso in cui in linea 3 sia presente un commento
		} else if(cline == 3 && line[0] == '/') {
			cline--;

		//Lettura della lista dei colori
		} else if((cline >= 4) && (cline <= numColori + 3)){

			//Scrivo il simbolo del colore all'interno della struttura dati
			for(i = 0; i < bitColore; i++){
				colori[cline - 4].simbolo[i] = line[i + 1];
			}
			colori[cline - 4].simbolo[bitColore] = '\0';
			colori[cline - 4].occorrenza = 0;

			//Salvo il valore del colore esadecimale all'interno di str
			t = 0;
			str = (char*) malloc (6 * sizeof(char));
			for(i = bitColore + 5; i < strlen(line) - 3; i++){
				str[t++] = line[i];
			}

			//converto le tre componenti RGB e le porto in decimale
			hexString[0] = str[0];
			hexString[1] = str[1];
			colori[cline - 4].R = strtol(hexString, NULL, 16);
			hexString[0] = str[2];
			hexString[1] = str[3];
			colori[cline - 4].G = strtol(hexString,NULL,16);
			hexString[0] = str[4];
			hexString[1] = str[5];
			colori[cline - 4].B = strtol(hexString,NULL,16);

			//sono in ordine dal piu piccolo al piu grande, il colore successivo, ha almeno
			//due componenti maggiori di quello precedente
		}

		//Leggo i simboli che compongono i pixel dell'immagine
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

					/*for(k=0;(k<numColori && flag == 0);k++){
						//è una ricerca per simbolo non per valore non posso fare la dicotomica
						if(strcmp(simboloConf,colori[k].simbolo) == 0){
							colori[k].occorrenze++;
							flag = 1;
						}
					}*/

					flag = 0;
					pos = 0;
					//ricerca ottimizzata
					//da controllare se lo riesco a fare bene spacca
					for(k=bitColore-1;k>=0;k--){
						flag = 0;
						for(o=pos;(o<numColori && flag == 0);o += pow(92,k)){
							if(simboloConf[k] == colori[o].simbolo[k]){
								flag = 1;
								pos = o;
							}
						}
						if(k == 0){
							colori[pos].occorrenza++;
						}
					}
				}
			}
		}
		else{
				//controllo i casi che non rientrano in nessun if
		}
	}

	//stampa occorrenze
	//FILE *occo=fopen("prova1.txt","w+");
	//int somma = 0;
	//for(i=0;i<numColori;i++){
	//	fprintf(occo,"%s: %d\n",colori[i].simbolo,colori[i].occorrenze);
	//	somma += colori[i].occorrenze;
	//}
	//fprintf(occo,"%d\n",somma);
	printf("lettura del file XPM completata\n");
	free(line);
	getch();
	fclose(fin);
	return 0;
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

		//Nella quarta riga del file ricavo il numero dei colori
		} else if (cline == 4) {

			//Alloco la struttura dati contenente la lista di colori dell'immagine originale
			colori = (colorVet*) malloc (larghezza * altezza * sizeof(colorVet));
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
	int cline = 0, cpixel = 0, ccolori = 0, i;
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

		//Leggo le informazioni di ogni pixelPPM
		if(cline > coloriPalette + 1) {
			if(lzw == 0)
				pixelGif[cpixel++] = (unsigned char) atoi(line);
			else
				pixelLzw[cpixel++]  = (unsigned char) atoi(line);

		//Leggo le informazioni delle palette
		} else if(cline > 1 && cline <= coloriPalette + 1){
			//printf("%s\n", line);
			palette[ccolori].R = atoi(strtok(line, " "));
			palette[ccolori].G = atoi(strtok(NULL, " "));
			palette[ccolori++].B = atoi(strtok(NULL, "\0"));

		//Ricavo le informazioni sull'immagine
		} else {
			larghezza = atoi(strtok(line, " "));
			altezza = atoi(strtok(NULL, " "));
			coloriPalette = atoi(strtok(NULL, " "));
			lzw = atoi(strtok(NULL, "\0"));
			palette = (pixelInfo*) malloc(coloriPalette * sizeof(pixelInfo));
			pixelGif = (unsigned char*) malloc(larghezza * altezza * sizeof(unsigned char));
			pixelLzw = (unsigned char*) malloc(larghezza * altezza * sizeof(unsigned char));
		}
	}

	//Decomprimo i pixelPPM se sono stati compressi
	if (lzw == 1)
		decompressoreLZW(cpixel - 1);

	//Alloco la struttura dati che contiene tutti i dati dei pixelPPM
	pixelPPM = (pixelInfo*) malloc (larghezza * altezza * sizeof(pixelInfo));

	//Associo ad ogni pixelPPM il valore ricavato dalla palette
	for(i = 0; i < larghezza * altezza; i++) {
		pixelPPM[i].R = palette[(int) pixelGif[i]].R;
		pixelPPM[i].G = palette[(int) pixelGif[i]].G;
		pixelPPM[i].B = palette[(int) pixelGif[i]].B;
	}

	//Chiudo il file e ritorno un valore di default
	fclose(fin);
	return 0;
}

/* Funzione che conta le occorrenze dei colori nei pixelPPM originali della foto riodinandoli in modo
 * decrescente, infine crea le palette e trova il nuovo colore dei pixelPPM associando l'indice corretto
 */
void creaDatiGifDaPPM(int coloriPalette) {

	/*
	 * Blocco per la conta delle occorrenze e il riodino dei colori
	 */
	int i, j, pos, min, somma;
	colorVet temp;

	//Azzero le occorrenze del vettore dei colori
	for(i = 0; i < larghezza * altezza; i++)
		colori[i].occorrenza = 0;

	//Scorro tutti i pixelPPM presenti nell'immagine
	for(pos = 0; pos < larghezza * altezza; pos++) {

		//Scorro la lista dei colori per trovare se il colore Ã¨ giÃ  presente o meno
		i = 0;
		while(i < coloriPalette && colori[i].occorrenza != 0 && !(pixelPPM[pos].R == colori[i].R &&
				pixelPPM[pos].G == colori[i].G && pixelPPM[pos].B == colori[i].B))
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
			colori[i].R = pixelPPM[pos].R;
			colori[i].G = pixelPPM[pos].G;
			colori[i].B = pixelPPM[pos].B;
			colori[i].occorrenza++;
		}
	}

	/*
	 * Blocco la ricerca del nuovo colore di ogni pixelPPM
	 */

	//Inizializzo le strutture dati
	palette = (pixelInfo*) malloc(coloriPalette * sizeof(colorVet));
	pixelGif = (unsigned char*) malloc(larghezza * altezza * sizeof(unsigned char));

	//Prendo i primi grandezza colori dal vettore colori
	for(i = 0; i < coloriPalette; i++) {
		palette[i].R = colori[i].R;
		palette[i].G = colori[i].G;
		palette[i].B = colori[i].B;
	}

	//Trovo il nuovo colore dei pixelPPM
	for(i = 0; i < larghezza * altezza; i++) {

		//Imposto la ricerca del minimo
		min = 160000000;

		//Scansiono tutta la palette
		for(j = 0; j < coloriPalette; j++) {

			//Cerco la distanza minima tra il colore originale e quelli nella palette
			somma = abs(pixelPPM[i].R - palette[j].R) + abs(pixelPPM[i].G - palette[j].G) + abs(pixelPPM[i].B - palette[j].B);
			if(somma < min) {
				min = somma;
				pixelGif[i] = (unsigned char) j;
			}
		}
	}
}

//-----------------------------------------------------------------------------------------------------
/* Funzione utilizzata per scrivere un file in formato .PPM dai dati elaborati
 */
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

	//Scrivo larghezza, altezza, numero colori e un flag lzw
	fprintf(fout, "%d %d %d %d\n", larghezza, altezza, coloriPalette, lzw);

	//Scrivo tutta la palette
	for(i = 0; i < coloriPalette; i++) {
		fprintf(fout, "#%d %d %d\n", palette[i].R, palette[i].G, palette[i].B);
		//printf("%d %d %d\n", palette[i].R, palette[i].G, palette[i].B);
	}
	//Scrivo il contenuto dei pixelPPM
	for(i = 0; i < grandezza; i++)
		fprintf(fout, "%u\n", daScrivere[i]);

	//Chiudo il file e ritorno un valore di default
	fclose(fout);
	return 0;
}

int main(){

	/* ****************************************************************** *
	 * Lettura del file                                                   *
	 * ****************************************************************** */

	//Alcune variabili locali utili alla gestione dell'applicativo
	int i, j = 2, grandezzaLzw, input;
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
		if(LetturaFileXPM(nomeFile) == 0)
			printf("Lettura immagine completata\n");

	/* ****************************************************************** *
	 * Elaborazione di un .ppm                                            *
	 * ****************************************************************** */
	} else if(strcmp(estensione,"ppm") == 0) {

		//Eseguo la lettura da file .ppm
		if(LetturaFilePPM(nomeFile) == 0)
			printf("Lettura immagine completata\n");

		//Scrivo la lista dei colori nel file
		//if(scriviColori() == 0)
		//	printf("Scrittura colori completata\n");

		//Creo la palette globale
		printf("Creo dati gif\n");
		creaDatiGifDaPPM(256);

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
		free(pixelPPM);
		free(pixelGif);


	/* ****************************************************************** *
	 * Elaborazione di un .gif                                            *
	 * ****************************************************************** */
	} else if(strcmp(estensione, "gif") == 0) {

		//Leggo l'immagine gif
		LetturaFileGIF(nomeFile);
		printf("Lettura immagine completata\n");

		//Stampo il menù a video
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
