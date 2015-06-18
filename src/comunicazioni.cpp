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
#include<math.h>

//Struttura in cui vengono inseriti i valori dei pixelPPM trovati
typedef struct pixelInfo{
	int R;
	int G;
	int B;
}pixelInfo;

//Struttura in cui vengono inseriti i colori trovati
typedef struct colorVet{
	char *simbolo;
	int R;
	int G;
	int B;
	int occorrenza;
}colorVet;

//Struttura in cui vengono inseriti i colori che compongono la paletteXPM
typedef struct paletteSimboli{
	unsigned char R;
	unsigned char G;
	unsigned char B;
	char *simbolo;
}paletteSimboli;

typedef struct paletteXPM{
	char R[3];
	char G[3];
	char B[3];
}paletteXPM;

typedef struct generazioneSimboli{
	char *simbolo;
}generazioneSimboli;
//-----------------------------------------------------------------------------------------------------
//Dichiaro le variabili globali utilizzate in futuro
pixelInfo *pixelPPM, *palette[16], *palXPMint;
colorVet **colori,*listaColoriXPM;
paletteSimboli *paletteGlobale, **paletteLocale;
paletteXPM *palXPMchar;
generazioneSimboli *simboli;
unsigned char *pixelGif[16], *pixelLzw[16], *matriceImmagineConvertita, **sottoMatriciImmagineConvertita;
int larghezza, altezza, coloriPalette, bitColore, bitPrecisione, numPalette, lzw, numColori, numeroSottoMatriciImmagine, altezzaMatrici, larghezzaMatrici;
char *pixelXpm, **sottoMatriciImmagini, hexadecimalNumber[3];
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

	int i, j, c, pos = 0, prec, indiceLzw = 0;
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

/*Al termine di questa funzione abbiamo una lista di colori ordinata con il rispettivo simbolo e occorrenza
 * in più abbiamo una matrice dell'immagine scritta tutta su un vettore
 * e le informazioni riguardanti l'immagine
 */
int LetturaFileXPM(char nomeFile[]){

	//Dichiaro alcune variabili utili
	FILE *fin;
	char *line, hexString[2], simboloConf[bitColore], *str;
	int cline = 0, i, k, c = 0, j = 0, t = 0, o = 0,flag = 0, pos = 0;

	//Apertura del file
	fin = fopen(nomeFile,"r+");
	if(fin == NULL){
		fclose(fin);
		return -1;
	}
	else{
		printf("file aperto correttamente\n");
	}

	//Alloco lo spazio della variabile line
	line = (char*) malloc (256 * sizeof(char));

	while(!(feof(fin))){

		t = 0;
		pos = 0;
		fscanf(fin, " %[^\n]",line);
		cline++;

		//queste variazioni agli indici vanno bene soltanto nel caso in cui il file sia stato convertito con gimp
		if(cline == 3){
			//andiamo a leggere tutti i parametri dell'immagine

			str = strtok(line," ");
			for(i = 0;i < int(strlen(str)-1); i++){
				str[i] = str[i + 1];
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
					listaColoriXPM = (colorVet*) malloc (numColori * sizeof(colorVet));
					break;
				case 2:
					for(i = 0;i < int(strlen(str)); i++){
						if(str[i] == '"'){
							str[i] = '\0';
							//condizione di chiusura for
							i = strlen(str);
						}
					}
					bitColore = atoi(str);
					line = (char*) malloc (larghezza * bitColore * sizeof(char));
					//ricordati di leggere bene questo vettore o è la fine
					pixelXpm = (char*) malloc (altezza * larghezza * bitColore * sizeof(char));
					matriceImmagineConvertita = (unsigned char*) malloc (altezza * larghezza * sizeof(unsigned char));
					break;
				}
				c++;
			}
		}

		else if((cline >= 4) && (cline <= numColori + 3)){

			//leggo la palette
			listaColoriXPM[cline - 4].simbolo = (char*) malloc ((bitColore + 1) * sizeof(char));
			//simbolo colore
			for(i=0;i<bitColore;i++){
				listaColoriXPM[cline - 4].simbolo[i] = line[i + 1];
			}
			listaColoriXPM[cline - 4].simbolo[bitColore] = '\0';
			listaColoriXPM[cline - 4].occorrenza = 0;

			str = (char*) malloc (6*sizeof(char));
			for(i = 0;i < int(strlen(line)); i++){
				if((i > (bitColore + 4)) && (i < (bitColore + 4 + 7))){
					str[t] = line[i];
					t++;
				}
			}

			//prendo le tre componenti RGB e le porto in decimale
			hexString[0] = str[0];
			hexString[1] = str[1];
			listaColoriXPM[cline - 4].R = strtol(hexString,NULL,16);
			hexString[0] = str[2];
			hexString[1] = str[3];
			listaColoriXPM[cline - 4].G = strtol(hexString,NULL,16);
			hexString[0] = str[4];
			hexString[1] = str[5];
			listaColoriXPM[cline - 4].B = strtol(hexString,NULL,16);

			//sono in ordine dal piu piccolo al piu grande, il colore successivo, ha almeno
			//due componenti maggiori di quello precedente
		}

		else if((cline > numColori + 3) && (cline < altezza + numColori + 4)){

			//il vettore che contiene tutta l'immagine è scritto riga per riga
			c = 0;
			for(i = 1;i < int(strlen(line) - 2); i++){

				//c è l'indice che mi dice quando sono stati salvati due simboli, quindi
				//quando posso prendere i due simboli per controllare l'occorrenza
				c++;
				//copio un carattere del simbolo all'interno della matrice
				pixelXpm[j] = line[i];
				//j è l'indice con cui vado a salvari simboli nella matrice
				j++;
				if(c == bitColore){
					c = 0;
					for(t = bitColore - 1;t >= 0; t--){
						simboloConf[c] = line[i - t];
						c++;
					}

					c = 0;
					simboloConf[bitColore] = '\0';
					flag = 0;
					pos = 0;

					//ricerca ottimizzata
					//da controllare se lo riesco a fare bene spacca
					for(k = (bitColore - 1);k >= 0; k--){

						flag = 0;

						for(o = pos;(o < numColori && flag == 0);o += pow(92,k)){
							if(simboloConf[k] == listaColoriXPM[o].simbolo[k]){
								flag = 1;
								pos = o;
							}
						}
						if(k == 0){
							listaColoriXPM[pos].occorrenza++;
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
	fclose(fin);
	return 0;
}
//--------------------------------------------------------------------------------------------------
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
	getch();
	return 0;
}

//-----------------------------------------------------------------------------------------------------
/* Funzione che legge un file in formato ASCII gif e compila le strutture dati di conseguenza
 */
int LetturaFileGIF(char nomeFile[]) {

	//Dichiaro alcune variabili utili per la lettura da file
	FILE *fin;
	char line[256];
	int cline = 0, ccolori = 0, i = 0, c[16], selettore = 0;
	coloriPalette = 256; // temporaneo

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
			palette[i][ccolori].B = atoi(strtok(NULL, "\0"));
			ccolori++;

		//Ricavo le informazioni sull'immagine
		} else {
			larghezza = atoi(strtok(line, " "));
			altezza = atoi(strtok(NULL, " "));
			coloriPalette = atoi(strtok(NULL, " "));
			numPalette = atoi(strtok(NULL, " "));
			lzw = atoi(strtok(NULL, "\0"));

			palXPMchar = (paletteXPM*) malloc((numPalette * numPalette * coloriPalette) * sizeof(paletteXPM));
			palXPMint = (pixelInfo*) malloc((numPalette * numPalette * coloriPalette) * sizeof(pixelInfo));
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
//---------------------------------------------------------------------------------------------------------------
int CostruzioneMatriceImmagineGIF(){

	int i,c=0,j,flag=0,min,sommaDiff,t=0;
	int componenteRossa = 0,componentVerde = 0,componenteBlu = 0;
	char *simbolo;
	unsigned char pos = '\0';

	simbolo = (char*) malloc ((bitColore + 1) * sizeof(char));

	//leggo tutti i caratteri all'interno della matrice dell'immagine XPM
	for(i = 1;i <= int(strlen(pixelXpm)); i++){
		//ciclo che prende ogni simbolo dell'immagine e verifica se si trova all'interno della palette

		simbolo[c] = pixelXpm[i-1];
		c++;

		if((i % bitColore) == 0){
			//a questo punto io ho un simbolo e devo andare a recuperare il valore delle sue componenti
			c = 0;
			simbolo[bitColore] = '\0';
			for(j=0;(j<numColori && flag == 0);j++){
				if(strcmp(simbolo,listaColoriXPM[j].simbolo) == 0){
					flag = 1;
					componenteRossa = listaColoriXPM[j].R;
					componentVerde = listaColoriXPM[j].G;
					componenteBlu = listaColoriXPM[j].B;
				}
			}
			if(flag == 0){
				printf("non trovato\n");
			}
			else{
				flag = 0;
			}

			//a questo punto abbiamo i valori numerici interi del colore e dobbiamo andare a fare la differenza con i colori all'interno
			//palette
			sommaDiff = 0;
			min = 10000000;
			//scorro la palette
			for(j=0;j<coloriPalette;j++){

				//è univoco questo conto ?
				//sommaDiff = abs((componenteRossa >> bitPrecisione) - (paletteGlobale[j].R >> bitPrecisione)) +
				//			abs((componentVerde >> bitPrecisione) - (paletteGlobale[j].G >> bitPrecisione)) +
				//			abs((componenteBlu >> bitPrecisione) - (paletteGlobale[j].B >> bitPrecisione));

				sommaDiff = abs((componenteRossa ) - (paletteGlobale[j].R )) +
							abs((componentVerde ) - (paletteGlobale[j].G )) +
							abs((componenteBlu ) - (paletteGlobale[j].B ));

				//piu la differenza è minore piu il colore si avvicina all'originale
				if(sommaDiff < min){
					min = sommaDiff;
					pos = (unsigned char)j;
				}

				if(min == 0){
					j += coloriPalette;
				}
			}

			//vado a copiare il valore della posizione del colore all'interno della matrice dell'immagine
			matriceImmagineConvertita[t] = pos;
			//printf("%d\n",int(matriceImmagineConvertita[t]));

			//contatore che mi tiene conto del numero di simboli che ho riconosciuto
			t++;

			//aggiorno il contatore di pixel per goni elemento all'interno della matrice
			lunghezzaPixel[0]++;
		}
	}

	printf("costruzione matrice immagine GIF completata\n");
	//getch();
	return 0;
}
//-----------------------------------------------------------------------------------------------------------
int CostruzioneMatriceImmagineLocaleGIF(){

	int i,c=0,j,flag=0,min,sommaDiff,t=0,k;
	int componenteRossa = 0,componentVerde = 0,componenteBlu = 0;
	char *simbolo;
	unsigned char pos = '\0';

	sottoMatriciImmagineConvertita = (unsigned char**) malloc (numeroSottoMatriciImmagine * sizeof(unsigned char*));
	for(i=0;i<numeroSottoMatriciImmagine;i++){
		sottoMatriciImmagineConvertita[i]  = (unsigned char*) malloc ((altezzaMatrici * larghezzaMatrici) * sizeof(unsigned char));
	}

	simbolo = (char*) malloc (bitColore * sizeof(char));

	//ciclo per prendere dentro tutte le sottomatrici
	for(k=0;k<numeroSottoMatriciImmagine;k++){

		t = 0;

		for(i=1;i<=(altezzaMatrici * larghezzaMatrici * bitColore);i++){
			//ciclo che prende ogni simbolo dell'immagine e verifica se si trova all'interno della palette

			simbolo[c] = sottoMatriciImmagini[k][i-1];
			c++;

			if((i % bitColore) == 0){
				//a questo punto io ho un simbolo e devo andare a recuperare il valore delle sue componenti
				c = 0;
				simbolo[bitColore] = '\0';
				for(j=0;(j<(altezzaMatrici * larghezzaMatrici) && flag == 0);j++){
					if(strcmp(simbolo,colori[k][j].simbolo) == 0){
						flag = 1;
						componenteRossa = colori[k][j].R;
						componentVerde = colori[k][j].G;
						componenteBlu = colori[k][j].B;
					}
				}
				if(flag == 0){
					printf("non trovato\n");
				}
				else{
					flag = 0;
				}

				//a questo punto abbiamo i valori numerici interi del colore e dobbiamo andare a fare la differenza con i colori all'interno
				//palette
				sommaDiff = 0;
				min = 10000000;
				//scorro la palette
				for(j=0;j<coloriPalette;j++){

					//è univoco questo conto ?
					//sommaDiff = abs((componenteRossa >> bitPrecisione) - (paletteLocale[k][j].R >> bitPrecisione)) +
					//			abs((componentVerde >> bitPrecisione) - (paletteLocale[k][j].G >> bitPrecisione)) +
					//			abs((componenteBlu >> bitPrecisione) - (paletteLocale[k][j].B >> bitPrecisione));

					sommaDiff = abs((componenteRossa ) - (paletteLocale[k][j].R)) +
								abs((componentVerde ) - (paletteLocale[k][j].G )) +
								abs((componenteBlu ) - (paletteLocale[k][j].B ));

					//piu la differenza è minore piu il colore si avvicina all'originale
					if(sommaDiff < min){
						min = sommaDiff;
						pos = (unsigned char)j;
					}

					if(min == 0){
						j += coloriPalette;
					}
				}

				//vado a copiare il valore della posizione del colore all'interno della matrice dell'immagine
				sottoMatriciImmagineConvertita[k][t] = pos;

				//contatore che mi tiene conto del numero di simboli che ho riconosciuto
				t++;

				lunghezzaPixel[k]++;
			}
		}
	}

	return 0;
}
//---------------------------------------------------------------------------------------
int OrdinamentoPaletteLocale(){

	int i,k,j,indiceColore = 0,flag = 0,numColoriAssegnati = 0,bitPrecisione;
	colorVet scambio;
	unsigned char verifica[3];

	//vado ad inizializzare il vettore che conterrà i simboli dei colori che si trovano dentro la palette
	paletteLocale = (paletteSimboli**) malloc (numeroSottoMatriciImmagine * sizeof(paletteSimboli*));
	for(i=0;i<numeroSottoMatriciImmagine;i++){
		paletteLocale[i]  = (paletteSimboli*) malloc (coloriPalette * sizeof(paletteSimboli));
	}

	//vado a settare i bit per controllare i colori simili
	bitPrecisione = 4;

	for(i=0;i<numeroSottoMatriciImmagine;i++){
		for(j=0;j<numColori-1;j++){
			if(colori[i][j].simbolo != NULL){
				for(k=j+1;k<numColori;k++){
					if(colori[i][k].simbolo != NULL){
						if(colori[i][k].occorrenza > colori[i][j].occorrenza){
							scambio = colori[i][j];
							colori[i][j] = colori[i][k];
							colori[i][k] = scambio;
						}
					}
				}
			}
		}
	}

	for(i=0;i<numeroSottoMatriciImmagine;i++){

		flag = 0;
		k = 0;
		j = 0;
		numColoriAssegnati = 0;
		indiceColore = 0;

		paletteLocale[i][k].R = (colori[i][k].R) & 0xFF;
		paletteLocale[i][k].G = (colori[i][k].G) & 0xFF;
		paletteLocale[i][k].B = (colori[i][k].B) & 0xFF;

		//vado a copiare all'interno il simbolo del colore che viene inserito nella palette
		paletteLocale[i][k].simbolo = (char*) malloc (bitColore * sizeof(char));
		strcpy(paletteLocale[i][k].simbolo,colori[i][k].simbolo);

		//aggiungo uno ai colori assegnati all'interno della palette
		numColoriAssegnati++;
		//indico l'ultima posizione scritta
		indiceColore = j;

		for(k=1;k<numColori;k++){
			if(colori[i][k].simbolo != NULL){
				if(numColoriAssegnati < coloriPalette){
					flag = 0;
					verifica[0] = colori[i][k].R & 0xFF;
					verifica[1] = colori[i][k].G & 0xFF;
					verifica[2] = colori[i][k].B & 0xFF;
					for(j=0;(j<coloriPalette && flag == 0);j++){
						if(((verifica[0] >> bitPrecisione) == (paletteLocale[i][j].R >> bitPrecisione)) && (((verifica[1] >> bitPrecisione) == (paletteLocale[i][j].G >> bitPrecisione))) && (((verifica[2] >> bitPrecisione) == (paletteLocale[i][j].B >> bitPrecisione)))){
							break;
						}
						if(j == indiceColore){
							//devo scrivere nelle successive posizioni di j
							paletteLocale[i][j+1].R = verifica[0]; //R
							paletteLocale[i][j+1].G = verifica[1]; //G
							paletteLocale[i][j+1].B = verifica[2]; //B
							paletteLocale[i][numColoriAssegnati].simbolo = (char*) malloc (bitColore * sizeof(char));
							strcpy(paletteLocale[i][numColoriAssegnati].simbolo,colori[i][k].simbolo);
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
		}

		//printf("numColoriAssegnati = %d\n",numColoriAssegnati);

		if(numColoriAssegnati < coloriPalette){
			for(k=1;k<numColori;k++){
				if(numColoriAssegnati < coloriPalette){
					flag = 1;
					verifica[0] = colori[i][k].R & 0xFF;
					verifica[1] = colori[i][k].G & 0xFF;
					verifica[2] = colori[i][k].B & 0xFF;
					for(j=0;(j<indiceColore && flag == 0);j++){
						//controllo se il colore l'ho gia inserito all'interno della palette
						//con questo indice dovrei arrivare all'ultimo valore inserito
						if((verifica[0] == paletteLocale[i][j].R) && ((verifica[1] == paletteLocale[i][j].G)) && ((verifica[2] == paletteLocale[i][j].B))){
							flag = 0;
							break;
						}
						else{
							flag = 1;
						}
					}
					if(flag == 1){
						//se non l'ho inserito lo inserisco alla prima posizione libera in teoria
						paletteLocale[i][indiceColore+1].R = verifica[0];
						paletteLocale[i][indiceColore+1].G = verifica[1];
						paletteLocale[i][indiceColore+1].B = verifica[2];
						paletteLocale[i][indiceColore+1].simbolo = (char*) malloc (bitColore * sizeof(char));
						strcpy(paletteLocale[i][indiceColore+1].simbolo,colori[i][k].simbolo);
						indiceColore++;
						numColoriAssegnati++;
					}
				}
				else{
					break;
				}
			}
		}
		//printf("numColoriAssegnati = %d\n",numColoriAssegnati);

	}
	//CostruzioneMatriceImmagineLocaleGIF();

	return 0;
}
//---------------------------------------------------------------------------------------------------------------

int CostruzionePaletteLocale(){

	int simboliInseriti,numeroSottoMatrici = -1;
	int i, c, t, j, k, flag = 0, pos = 0;
	char simboloConf[bitColore];

	numeroSottoMatriciImmagine = numPalette*numPalette;
	larghezzaMatrici = int(larghezza / numPalette);
	altezzaMatrici = int(altezza/numPalette);

	//creazione delle sottomatrici
	//allocazione in memoria
	sottoMatriciImmagini = (char**) malloc (numeroSottoMatriciImmagine * sizeof(char*));
	for(i=0;i<numeroSottoMatriciImmagine;i++){
		sottoMatriciImmagini[i]  = (char*) malloc ((altezzaMatrici * larghezzaMatrici * bitColore) * sizeof(char));
	}

	for(i = 0;i < numPalette; i++){
		for(j = 0;j < numPalette; j++){

			numeroSottoMatrici++;
			c = 0;

			for(k = 0;k < altezzaMatrici; k++){
				for(t = (bitColore * ((j * larghezzaMatrici) + (k * larghezza) + (i * altezzaMatrici * larghezza)));t < (bitColore * (((j + 1) * larghezzaMatrici) + (k * larghezza) + (i * altezzaMatrici * larghezza))); t++){
					sottoMatriciImmagini[numeroSottoMatrici][c] = pixelXpm[t];
					c++;
				}
			}
		}
	}

	//creazione delle palette
	//allocazione in memoria
	colori = (colorVet**) malloc (numeroSottoMatriciImmagine * sizeof(colorVet*));
	for(i=0;i<numeroSottoMatriciImmagine;i++){
		colori[i]  = (colorVet*) malloc (numColori * sizeof(colorVet));
	}

	for(i=0;i<numeroSottoMatriciImmagine;i++){
		c = 0;
		simboliInseriti = 0;
		for(j = 0;j < int(strlen(sottoMatriciImmagini[i])); j++){

			c++;
			if(c == bitColore){
				c = 0;
				for(t=bitColore-1;t>=0;t--){
					simboloConf[c] = sottoMatriciImmagini[i][j-t];
					c++;
				}
				c = 0;
				simboloConf[bitColore] = '\0';
				//printf("simbolo: %s\n",simboloConf);

				colori[i][simboliInseriti].simbolo = (char*) malloc (bitColore * sizeof(char));
				flag = 0;
				for(k=0;k<simboliInseriti && flag == 0;k++){
					if((strcmp(simboloConf,colori[i][k].simbolo)) == 0){
						//simbolo trovato
						flag = 1;
						pos = k;
					}
				}
				//simbolo non trovato all'interno della palette
				if(flag == 0){
					strcpy(colori[i][simboliInseriti].simbolo,simboloConf);
					simboliInseriti++;
				}
				//simbolo trovato all'interno della palette
				else if(flag == 1){
					flag = 0;
					colori[i][pos].occorrenza++;
				}
			}
		}
	}

	//andiamo a recuperare le informazioni sui colori
	for(i=0;i<numeroSottoMatriciImmagine;i++){
		for(c=0;c<numColori;c++){
			flag = 0;
			for(j=0;(j<numColori && flag == 0);j++){
				if(colori[i][c].simbolo != NULL){
					if(strcmp(colori[i][c].simbolo,listaColoriXPM[j].simbolo) == 0){
						colori[i][c].R = listaColoriXPM[j].R;
						colori[i][c].G = listaColoriXPM[j].G;
						colori[i][c].B = listaColoriXPM[j].B;
						flag = 1;
					}
				}
				else{

				}
			}
		}
	}

	return 0;
}
//---------------------------------------------------------------------------------------------------------------
/*
 * in questa funzione andiamo a costruire la palette globale dell'immagine basandoci sulle occorrenze dei vari colori
 * all'interno dell'immagine, e basandoci sul numero di colori con cui ci viene richiesta la palette
 * coloriPalette: è il numero di colori con il quale voglio costruire la palette
 */
int CostruzionePaletteGlobale(){

	int i, k, j, indiceColore = 0, flag = 0, numColoriAssegnati = 0;
	colorVet scambio;
	unsigned char verifica[3];

	//vado ad inizializzare il vettore che conterrà i simboli dei colori che si trovano dentro la palette
	paletteGlobale = (paletteSimboli*) malloc (coloriPalette * sizeof(paletteSimboli));

	//vado a settare i bit per controllare i colori simili
	bitPrecisione = 4;

	//ordino il vettore che contiene tutti i colori per occorrenze
	//questa operazione andrebbe fatta al momento dell'inserimento
	for(i=0;i<numColori-1;i++){
		for(j=i+1;j<numColori;j++){
			if(listaColoriXPM[j].occorrenza > listaColoriXPM[i].occorrenza){
				scambio = listaColoriXPM[i];
				listaColoriXPM[i] = listaColoriXPM[j];
				listaColoriXPM[j] = scambio;
			}
		}
	}

	//for(i=0;i<coloriPalette;i++){
	//	printf("R: %d G: %d B: %d\n",listaColoriXPM[i].R,listaColoriXPM[i].G,listaColoriXPM[i].B);
	//}

	//funzionaaaaa
	//con questa funzione se vogliamo differenziare ancora di piu la palette allora basta aumentare
	//il numero di bit che controlliamo che non siano uguali
	//vado a riempire la palette di colori
	flag = 0;
	k = 0;
	j = 0;

	paletteGlobale[k].R = (listaColoriXPM[k].R) & 0xFF;
	paletteGlobale[k].G = (listaColoriXPM[k].G) & 0xFF;
	paletteGlobale[k].B = (listaColoriXPM[k].B) & 0xFF;

	//vado a copiare all'interno il simbolo del colore che viene inserito nella palette
	paletteGlobale[k].simbolo = (char*) malloc ((bitColore + 1) * sizeof(char));
	strcpy(paletteGlobale[k].simbolo,listaColoriXPM[k].simbolo);

	//aggiungo uno ai colori assegnati all'interno della palette
	numColoriAssegnati++;
	//indico l'ultima posizione scritta
	indiceColore = j;

	for(k=1;k<numColori;k++){
		if(numColoriAssegnati < coloriPalette){
			flag = 0;
			verifica[0] = listaColoriXPM[k].R & 0xFF;
			verifica[1] = listaColoriXPM[k].G & 0xFF;
			verifica[2] = listaColoriXPM[k].B & 0xFF;
			for(j=0;(j<coloriPalette && flag == 0);j++){
				if(((verifica[0] >> bitPrecisione) == (paletteGlobale[j].R >> bitPrecisione)) && (((verifica[1] >> bitPrecisione) == (paletteGlobale[j].G >> bitPrecisione))) && (((verifica[2] >> bitPrecisione) == (paletteGlobale[j].B >> bitPrecisione)))){
					//break;
					flag = 1;
				}
				if(j == indiceColore){
					//devo scrivere nelle successive posizioni di j
					paletteGlobale[j+1].R = verifica[0]; //R
					paletteGlobale[j+1].G = verifica[1]; //G
					paletteGlobale[j+1].B = verifica[2]; //B
					paletteGlobale[j+1].simbolo = (char*) malloc (bitColore * sizeof(char));
					strcpy(paletteGlobale[j+1].simbolo,listaColoriXPM[k].simbolo);
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

	//printf("numColoriAssegnati = %d\n",numColoriAssegnati);

	//nel caso in cui i colori che non sono simili non bastano per riempire tutta la palette
	//faccio un ciclo in cui vado a reinserire tenendo sempre come ordinamento le occorrenze per questi ultimi
	//cioè dopo l'inserimento di tutti i colori non simili, gli ultimi colori saranno inseriti in base alle
	//occorrenze calcolate precedentemente.
	if(numColoriAssegnati < coloriPalette){
		for(k=1;k<numColori;k++){
			if(numColoriAssegnati < coloriPalette){
				flag = 1;
				verifica[0] = listaColoriXPM[k].R & 0xFF;
				verifica[1] = listaColoriXPM[k].G & 0xFF;
				verifica[2] = listaColoriXPM[k].B & 0xFF;
				for(j=0;(j<indiceColore && flag == 0);j++){
					//controllo se il colore l'ho gia inserito all'interno della palette
					//con questo indice dovrei arrivare all'ultimo valore inserito
					if((verifica[0] == paletteGlobale[j].R) && ((verifica[1] == paletteGlobale[j].G)) && ((verifica[2] == paletteGlobale[j].B))){
						flag = 0;
						//break;
					}
					else{
						flag = 1;
					}
				}
				if(flag == 1){
					//se non l'ho inserito lo inserisco alla prima posizione libera in teoria
					//provo a fare inserire il colore piu vicino
					paletteGlobale[indiceColore+1].R = verifica[0];
					paletteGlobale[indiceColore+1].G = verifica[1];
					paletteGlobale[indiceColore+1].B = verifica[2];
					paletteGlobale[indiceColore+1].simbolo = (char*) malloc (bitColore * sizeof(char));
					strcpy(paletteGlobale[indiceColore+1].simbolo,listaColoriXPM[k].simbolo);
					indiceColore++;
					numColoriAssegnati++;
				}
			}
			else{
				break;
			}
		}
	}
	//printf("numColoriAssegnati = %d\n",numColoriAssegnati);

	return 0;
}
//-------------------------------------------------------------------------------------------------------
/* Funzione che conta le occorrenze dei colori nei pixelPPM originali della foto riodinandoli in modo
 * decrescente, infine crea le palette e trova il nuovo colore dei pixelPPM associando l'indice corretto
 */
void creaDatiGifDaPPM() {

	/*
	 * Blocco per la conta delle occorrenze e il riodino dei colori
	 */
	int i, j, k, pos, min, somma, c[16], selettore = 0;
	colorVet temp;

	colori = (colorVet**) malloc (numPalette * numPalette * sizeof(colorVet*));

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
		palette[i] = (pixelInfo*) malloc(coloriPalette * sizeof(pixelInfo));
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
int ScriviPPM(char nomeFile[]) {

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
int ScriviGIFdaPPM(char nomeFile[], unsigned char *daScrivere[]) {

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
//-----------------------------------------------------------------------------------------
int ScriviGIFdaXPM(char nomeFile[], unsigned char *daScrivere[]) {

	//Dichiaro alcune variabili locali utili
	int i,j;

	//Cambio estensione al nome del file
	nomeFile[strlen(nomeFile) - 3] = 'g';
	nomeFile[strlen(nomeFile) - 2] = 'i';
	nomeFile[strlen(nomeFile) - 1] = 'f';

	//Apro il file per scrivere i colori
	FILE *fout = fopen(nomeFile, "w+");
	if(fout == NULL) {
		fclose(fout);
		return -1;
	}

	//caso in cui non sia presente la compressione lzw
	if(lzw == 2) lzw = 0;

	//Scrivo larghezza, altezza e numero colori
	fprintf(fout, "%d %d %d %d %d\n", larghezza, altezza, coloriPalette, numPalette, lzw);

	//palette globale
	if(numPalette == 1){

		if(lzw == 0){
			//Scrivo tutta la palette
			//mettiamo un segno distitivo nella palette
			for(i = 0; i < coloriPalette; i++) {
				fprintf(fout, "%d %d %d\n", paletteGlobale[i].R, paletteGlobale[i].G, paletteGlobale[i].B);
			}

			//Scrivo il contenuto dei pixel
			for(i = 0; i < larghezza * altezza; i++){
				fprintf(fout, "%u\n", matriceImmagineConvertita[i]);
			}
		}
		else{

			for(i = 0; i < coloriPalette; i++) {
				fprintf(fout, "%d %d %d\n", paletteGlobale[i].R, paletteGlobale[i].G, paletteGlobale[i].B);
			}

			printf("dentro2\n");
			//Scrivo il contenuto dei pixel
			for(i = 0; i < lunghezzaPixel[0]; i++){
				fprintf(fout, "%u\n", daScrivere[0][i]);
			}

		}

	}

	//palette locale
	else{
		for(j=0;j<numeroSottoMatriciImmagine;j++){
			for(i = 0; i < coloriPalette; i++) {
				fprintf(fout, "%d %d %d\n", paletteLocale[j][i].R, paletteLocale[j][i].G, paletteLocale[j][i].B);
			}

			//Scrivo il contenuto dei pixel
			for(i = 0; i < lunghezzaPixel[j]; i++){
				fprintf(fout, "%u\n", daScrivere[j][i]);
			}

			if(j < numeroSottoMatriciImmagine - 1){
				fprintf(fout,",\n");
			}
		}
	}


	fclose(fout);
	return 0;
}
//-----------------------------------------------------------------------------------------
int DecToHex(int decimalNumber){

	int quotient;
	int i=1,temp;

	quotient = decimalNumber;

	hexadecimalNumber[0] = 48;
	hexadecimalNumber[1] = 48;

	while(quotient != 0){
		 temp = quotient % 16;

	  if( temp < 10){
		   temp =temp + 48;
	  }
	  else{
		 temp = temp + 55;
	  }

	  hexadecimalNumber[i]= temp;
	  i--;
	  quotient = trunc(quotient / 16);
	}


	return 0;
}
//-----------------------------------------------------------------------------------------
int ScriviXPM(char nomeFile[]){

	int i,j,k,numSimbolo,pos,f = 0,cont = 0;
	paletteXPM confronto;

	//Cambio estensione al nome del file
	nomeFile[strlen(nomeFile) - 6] = 'p';
	nomeFile[strlen(nomeFile) - 5] = 'm';
	nomeFile[strlen(nomeFile) - 4] = '.';
	nomeFile[strlen(nomeFile) - 3] = 'x';
	nomeFile[strlen(nomeFile) - 2] = 'p';
	nomeFile[strlen(nomeFile) - 1] = 'm';

	//Apro il file per scrivere i colori
	FILE *fout = fopen(nomeFile, "w+");
	if(fout == NULL) {
		fclose(fout);
		return -1;
	}

	fprintf(fout, "/* XPM */\n");
	fprintf(fout, "static char *");

	for(i = 0;i <= int(strlen(nomeFile)-5); i++){
		fprintf(fout,"%c",nomeFile[i]);
	}
	fprintf(fout,"[] = {\n");

	numeroSottoMatriciImmagine = numPalette * numPalette;

	//calcolo di quanti caratteri vengono utilizzati per rappresentare i colori
	//91 è il numero di simboli che abbiamo disponibili nell'alfabeto
	if(((log((double)coloriPalette * numeroSottoMatriciImmagine))/(log((double)91))) <= 2){
		bitColore = 2;
	}
	else if(((log((double)coloriPalette * numeroSottoMatriciImmagine))/(log((double)91))) < abs(((log((double)coloriPalette * numeroSottoMatriciImmagine))/(log((double)91))))+1){
		bitColore = abs(((log((double)coloriPalette * numeroSottoMatriciImmagine))/(log((double)91))))+1;
	}

	for(j=0;j<numeroSottoMatriciImmagine;j++){
		for(i=0;i<coloriPalette;i++){

			f = 0;
			DecToHex(palette[j][i].R);
			confronto.R[0] = hexadecimalNumber[0];
			confronto.R[1] = hexadecimalNumber[1];
			confronto.R[2] = '\0';

			DecToHex(palette[j][i].G);
			confronto.G[0] = hexadecimalNumber[0];
			confronto.G[1] = hexadecimalNumber[1];
			confronto.G[2] = '\0';

			DecToHex(palette[j][i].B);
			confronto.B[0] = hexadecimalNumber[0];
			confronto.B[1] = hexadecimalNumber[1];
			confronto.B[2] = '\0';

			for(k = 0;((k < numeroSottoMatriciImmagine * coloriPalette) && f == 0); k++){
				if((strcmp(confronto.R,palXPMchar[k].R) == 0) && (strcmp(confronto.G,palXPMchar[k].G) == 0) && (strcmp(confronto.B,palXPMchar[k].B) == 0)){
					f = 1;
				}
			}

			if(f == 0){
				strcpy(palXPMchar[cont].R,confronto.R);
				strcpy(palXPMchar[cont].G,confronto.G);
				strcpy(palXPMchar[cont].B,confronto.B);
				palXPMint[cont].R = palette[j][i].R;
				palXPMint[cont].G = palette[j][i].G;
				palXPMint[cont].B = palette[j][i].B;
				cont++;
			}
		}
	}

	//stampo le caratteristiche dell'immagine
	fprintf(fout, "\"%d %d %d %d \",\n",larghezza,altezza,cont,bitColore);

	//alloco lo spazio per andare a salvare i simboli
	simboli = (generazioneSimboli*) malloc (cont * sizeof(generazioneSimboli));

	//stampo la palette
	//il simbolo iniziale da cui partiamo è il 032 lo spazio
	numSimbolo = 32;
	k = bitColore - 1;

	//alloco lo spazio generale per ogni simbolo
	for(i=0;i<cont;i++){
		simboli[i].simbolo = (char*) malloc ((bitColore + 1) * sizeof(char));
	}

	//calcolo dei simboli da mettere all'interno della palette
	for(i=bitColore;i>0;i--){
		numSimbolo = 32;
		for(j=0;j<cont;j++){

			simboli[j].simbolo[i-1] = (char)numSimbolo;
			if(numSimbolo == 123){
				numSimbolo = 32;
			}
			if((j > 1) || (i == bitColore-1)){
				if((j % ((int)pow(91,k))) == 0){
					numSimbolo++;
				}
			}

		}
		k--;
	}

	for(j=0;j<cont;j++){
		simboli[j].simbolo[bitColore] = '\0';
	}

	//scrivo la palette sul file
	for(j = 0;j < cont; j++){
		fprintf(fout, "\"%s c #",simboli[j].simbolo);
		fprintf(fout,"%c%c",palXPMchar[j].R[0],palXPMchar[j].R[1]);
		fprintf(fout,"%c%c",palXPMchar[j].G[0],palXPMchar[j].G[1]);
		fprintf(fout,"%c%c\",\n",palXPMchar[j].B[0],palXPMchar[j].B[1]);

	}


	//stampo la matrice che contiene i colori
	j = 0;
	fprintf(fout,"\"");
	for(i=0;i<larghezza*altezza;i++){

		k = 0;
		if(j == 1){
			fprintf(fout,"\"");
		}
		//cerco il colore all'interno della palette
		for(j=0;(j<cont && k == 0);j++){
			if((pixelPPM[i].R == palXPMint[j].R) && (pixelPPM[i].G == palXPMint[j].G) && (pixelPPM[i].B == palXPMint[j].B)){
				k = 1;
				pos = j;
			}
		}

		j = 0;
		//simbolo trovato
		if(k == 1){
			fprintf(fout,"%s",simboli[pos].simbolo);
		}
		else{
			printf("non trovato\n");
		}

		//scrivere il fine riga
		if((i+1) != 1){
			if(((i+1) % larghezza) == 0){
				if((i+1) == (altezza*larghezza)){
					fprintf(fout,"\"");
				}
				else{
					fprintf(fout,"\",\n");
					j = 1;
				}
			}
		}
	}
	fprintf(fout,"\n};");

	printf("ricostruzione immagine XPM completata\n");
	fclose(fout);
	return 0;
}
//-----------------------------------------------------------------------------------------

int MCD(int a, int b) {
	int resultA = 1, resultB = 1, i = 1;

    while (resultA != 0 || resultB != 0){
    	i++;
    	resultA = a % i;
    	resultB = b % i;
    }
    return i;
}

//-----------------------------------------------------------------------------------------------------
int main(){

	/* ****************************************************************** *
	 * Lettura del file                                                   *
	 * ****************************************************************** */

	//Alcune variabili locali utili alla gestione dell'applicativo
	int i, j = 2, input;
	char nomeFile[1024], estensione[4];

	//Chiedo all'utente il file da utilizzare
	printf("Inserire il nome del file da aprire\n");
	scanf("%s", nomeFile);

	//Controllo che sia stato selezionato un file
	while(strlen(nomeFile) == 0) {

		//Esecuzione errata del programma
		fprintf(stderr, "Specificare un file da utilizzare (trascinarlo in questa finestra è il modo più semplice)\n");
	}

	//Ricavo l'estensione del file
	for(i = 1; i <= 3; i++){
		estensione[j--] = nomeFile[strlen(nomeFile) - i];
	}
	estensione[3] = '\0';
	printf("estensione: %s\n",estensione);


	/* ****************************************************************** *
	 * Elaborazione di un .xpm                                            *
	 * ****************************************************************** */
	if(strcmp(estensione, "xpm") == 0) {

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
			printf("1)Matrice globale\n");
			printf("2)Matrici Locali\n");
			scanf("%d",&input);
		}

		//Controllo l'input appena inserito
		switch(input){
		case 1:
			numPalette = 1;
			break;
		case 2:
			//numero a caso diverso da 1
			numPalette = 2;
			break;
		}

		//Stampo il menù a video
		input = -1;
		while(input < 0 || input > 8){
			printf("Quanti bit di precisione uso nella scelta dei colori? (0 <= input <= 8)\n");
			scanf("%d",&input);
		}

		//Stampo il menù a video
		lzw = 0;
		while(lzw != 1 && lzw != 2){
			printf("Utilizzare compressione LZW?\n");
			printf("1)Si\n");
			printf("2)No\n");
			scanf("%d",&lzw);
		}

		//Assegno il numero inserito alla variabile globale bitPrecisione
		bitPrecisione = input;

		//Eseguo la lettura da file .xpm
		if(LetturaFileXPM(nomeFile) == 0){
			printf("Lettura immagine completata\n");
		}

		if(numPalette != 1){
			//numPalette = MCD(larghezza,altezza);
			numPalette = 4;
			printf("matrice %d x %d\n",numPalette,numPalette);
		}

		//caso della palette globale
		if(numPalette == 1){
			if(CostruzionePaletteGlobale() == 0){
				printf("Costruzione palette globale completata\n");

				if(CostruzioneMatriceImmagineGIF()==0){

					//inizializzo le altre posizioni del vettore
					for(i = 0;i < numPalette * numPalette; i++){
						pixelGif[i] = (unsigned char*) malloc(larghezza * altezza * sizeof(unsigned char));
					}
					for(i = 0;i < altezza * larghezza; i++){
						pixelGif[0][i] = matriceImmagineConvertita[i];
					}

					//Scrivo l'immagine gif
					if(lzw == 1) {

						for(i = 0;i < numPalette * numPalette; i++){
							printf("dentro\n");
							lunghezzaPixel[i] = compressoreLZW(i);
							printf("%d\n",lunghezzaPixel[i]);
						}

						//scrittura GIF utilizzando lzw
						if(ScriviGIFdaXPM(nomeFile, pixelLzw) == 0){
							printf("Scrittura gif lzw avvenuta con successo\n");
						}

					} else {
						//scrittura GIF non utilizzando lzw
						if(ScriviGIFdaXPM(nomeFile,pixelGif) == 0)
							printf("Scrittura gif avvenuta con successo\n");
					}

				}
			}
		}
		else{
			//nel caso in cui la nostra immagine deve essere composta da piu palette locali
			if(CostruzionePaletteLocale() == 0){
				printf("Costruzione palette locali completata\n");

				if(OrdinamentoPaletteLocale() == 0){
					printf("ordinamento palette locali completata\n");

					if(CostruzioneMatriceImmagineLocaleGIF() == 0){
						printf("costruzione matrice immagine GIF completata\n");

						//inizializzo le altre posizioni del vettore
						for(i = 0;i < numPalette * numPalette; i++){
							pixelGif[i] = (unsigned char*) malloc(larghezzaMatrici * altezzaMatrici * sizeof(unsigned char));
						}
						for(j = 0;j < numeroSottoMatriciImmagine; j++){
							for(i = 0;i < altezzaMatrici * larghezzaMatrici; i++){
								pixelGif[j][i] = sottoMatriciImmagineConvertita[j][i];
							}
						}

						//Scrivo l'immagine gif
						if(lzw == 1) {

							for(i = 0;i < numPalette * numPalette; i++){
								lunghezzaPixel[i] = compressoreLZW(i);
							}

							//scrittura GIF utilizzando lzw
							if(ScriviGIFdaXPM(nomeFile, pixelLzw) == 0){
								printf("Scrittura gif lzw avvenuta con successo\n");
							}

						} else {
							//scrittura GIF non utilizzando lzw
							if(ScriviGIFdaXPM(nomeFile,pixelGif) == 0)
								printf("Scrittura gif avvenuta con successo\n");
						}
					}

				}
			}

		}

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
			if(ScriviGIFdaPPM(nomeFile, pixelLzw) == 0)
				printf("Scrittura gif lzw avvenuta con successo\n");

		} else {
			if(ScriviGIFdaPPM(nomeFile, pixelGif) == 0)
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
			ScriviXPM(nomeFile);
			break;
		case 2:
			ScriviPPM(nomeFile);
			break;
		}

	//In caso di errori
	} else{
		printf("Formato non riconosciuto\n");
		getch();
	}

	return 0;
}
