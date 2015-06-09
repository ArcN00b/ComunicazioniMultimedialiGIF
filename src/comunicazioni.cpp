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

typedef struct pixelInfo{
	int R;
	int G;
	int B;
}pixelInfo;

typedef struct{
	char *simbolo;
}generazioneSimboli;

colorVet *colori;
pixelInfo *pixel, *palette;
paletteSimboli *paletteGlobale;
generazioneSimboli *simboli;
char *matriceImmagine;
unsigned char *matriceImmagineConvertita;
int larghezza,altezza,numColori,bitColore,coloriPalette, bitPrecisione;
char hexadecimalNumber[100];

//-----------------------------------------------------------------------------------------------------
/*Al termine di questa funzione abbiamo una lista di colori ordinata con il rispettivo simbolo e occorrenza
 * in più abbiamo una matrice dell'immagine scritta tutta su un vettore
 * e le informazioni riguardanti l'immagine
 */
int LetturaFileXPM(char nomeFile[]){

	FILE *fin;
	char *line,hexString[2],simboloConf[bitColore];
	int cline=0,i,k,c=0,j=0,t=0,o=0,flag = 0;

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
		int t = 0,pos = 0;
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

			//leggo la palette
			colori[cline - 4].simbolo = (char*) malloc (bitColore*sizeof(char));
			//simbolo colore
			for(i=0;i<bitColore;i++){
				colori[cline - 4].simbolo[i] = line[i+1];
			}
			colori[cline - 4].simbolo[bitColore] = '\0';
			colori[cline - 4].occorrenze = 0;

			str = (char*) malloc (6*sizeof(char));
			for(i=0;i<strlen(line);i++){
				//printf("%d\n",line[i]);
				if((i > (bitColore + 4)) && (i < (bitColore + 4 + 7))){
					str[t] = line[i];
					t++;
				}
			}

			//getch();
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
							colori[pos].occorrenze++;
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
	//getch();
	fclose(fin);
	return 0;
}

//---------------------------------------------------------------------------------------
/*
 * in questa funzione andiamo a costruire la palette globale dell'immagine basandoci sulle occorrenze dei vari colori
 * all'interno dell'immagine, e basandoci sul numero di colori con cui ci viene richiesta la palette
 * coloriPalette: è il numero di colori con il quale voglio costruire la palette
 */
int CostruzionePaletteGlobale(){

	int i,k,j,t,c = 0,indiceColore = 0,flag = 0,numColoriAssegnati = 0;
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
			if(colori[j].occorrenze > colori[i].occorrenze){
				scambio = colori[i];
				colori[i] = colori[j];
				colori[j] = scambio;
			}
		}
	}

	for(i=0;i<coloriPalette;i++){
		printf("R: %d G: %d B: %d\n",colori[i].R,colori[i].G,colori[i].B);
	}

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
					//break;
					flag = 1;
				}
				if(j == indiceColore){
					//devo scrivere nelle successive posizioni di j
					paletteGlobale[j+1].R = verifica[0]; //R
					paletteGlobale[j+1].G = verifica[1]; //G
					paletteGlobale[j+1].B = verifica[2]; //B
					paletteGlobale[j+1].simbolo = (char*) malloc (bitColore * sizeof(char));
					strcpy(paletteGlobale[j+1].simbolo,colori[k].simbolo);
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
					strcpy(paletteGlobale[indiceColore+1].simbolo,colori[k].simbolo);
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
	//getch();
	return 0;
}

//-------------------------------------------------------------------------------------
int CostruzioneMatriceImmagineGIF(){

	int i,c=0,j,flag=0,min,sommaDiff,t=0;
	int componenteRossa,componentVerde,componenteBlu;
	char *simbolo;
	unsigned char pos;

	simbolo = (char*) malloc ((bitColore+1) * sizeof(char));

	//leggo tutti i caratteri all'interno della matrice dell'immagine XPM
	for(i=1;i<=strlen(matriceImmagine);i++){
		//ciclo che prende ogni simbolo dell'immagine e verifica se si trova all'interno della palette

		simbolo[c] = matriceImmagine[i-1];
		c++;

		if((i % bitColore) == 0){
			//a questo punto io ho un simbolo e devo andare a recuperare il valore delle sue componenti
			c = 0;
			simbolo[bitColore] = '\0';
			for(j=0;(j<numColori && flag == 0);j++){
				if(strcmp(simbolo,colori[j].simbolo) == 0){
					flag = 1;
					componenteRossa = colori[j].R;
					componentVerde = colori[j].G;
					componenteBlu = colori[j].B;
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
				sommaDiff = abs((componenteRossa >> bitPrecisione) - (paletteGlobale[j].R >> bitPrecisione)) +
							abs((componentVerde >> bitPrecisione) - (paletteGlobale[j].G >> bitPrecisione)) +
							abs((componenteBlu >> bitPrecisione) - (paletteGlobale[j].B >> bitPrecisione));

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
		}
	}

	printf("costruzione matrice immagine GIF completata\n");
	//getch();
	return 0;
}
//-------------------------------------------------------------------------------------

int LetturaFilePPM(char nomeFile[]){

	return 0;
}

//-----------------------------------------------------------------------------------------

int ScriviGIF(char nomeFile[]) {

	//Dichiaro alcune variabili locali utili
	int i;

	//Cambio estensione al nome del file
	//nomeFile[strlen(nomeFile) - 3] = 'g';
	//nomeFile[strlen(nomeFile) - 2] = 'i';
	//nomeFile[strlen(nomeFile) - 1] = 'f';

	//Apro il file per scrivere i colori
	FILE *fout = fopen(nomeFile, "w");
	if(fout == NULL) {
		fclose(fout);
		return -1;
	}

	//Scrivo larghezza, altezza e numero colori
	fprintf(fout, "%d %d %d\n", larghezza, altezza, coloriPalette);

	//Scrivo tutta la palette
	//mettiamo un segno distitivo nella palette
	for(i = 0; i < coloriPalette; i++) {
		fprintf(fout, "#%d %d %d\n", paletteGlobale[i].R, paletteGlobale[i].G, paletteGlobale[i].B);
	}

	//Scrivo il contenuto dei pixel
	for(i = 0; i < larghezza * altezza; i++){
		fprintf(fout, "%u\n", matriceImmagineConvertita[i]);
	}
	//Chiudo il file e ritorno un valore di default
	fclose(fout);
	return 0;
}

//-----------------------------------------------------------------------------------------

int LetturaFileGIF(char nomeFile[]) {

	//Dichiaro alcune variabili utili per la lettura da file
	FILE *fin;
	char line[256],*str;
	int cline = 0, cpixel = 0, ccolori = 0, i, c = 0;

	//Apro il file per la lettura controllando che non ci siano problemi
	fin = fopen(nomeFile,"r+");
	if(fin == NULL) {
		fclose(fin);
		return -1;
	}
	else{
		printf("file aperto correttamente\n");
	}

	str = (char*) malloc (256 * sizeof(char));

	//leggo quanti colori contiene la palette
	while(line[0] == '#' || coloriPalette <= 1){

		fscanf(fin, " %[^\n]", line);
		coloriPalette++;

	}

	//torno all'inizio del file
	fseek(fin, 0, SEEK_SET);

	//leggo completamente il file
	while (!(feof(fin))){

		//leggo una linea dal file
		fscanf(fin," %[^\n]", line);
		cline++;
		//andiamo a leggere le informazioni dell'immagine
		if(cline == 1){

			larghezza = atoi(strtok(line," "));
			altezza = atoi(strtok(NULL," "));
			numColori = atoi(strtok(NULL,"\0"));

			//inizializzo la matrice dell'immagine e la palette
			palette = (pixelInfo*) malloc((numColori) * sizeof(pixelInfo));
			matriceImmagineConvertita = (unsigned char*) malloc(larghezza * altezza * sizeof(unsigned char));
		}
		else if(line[0] == '#') {
			//Leggo le informazioni di ogni pixel
			str = strtok(line,"#");
			palette[ccolori].R = atoi(strtok(str, " "));
			palette[ccolori].G = atoi(strtok(NULL, " "));
			palette[ccolori].B = atoi(strtok(NULL, "\0"));
			ccolori++;
		}
		else if(line[0] != '#'){
			//Leggo le informazioni delle palette
			matriceImmagineConvertita[cpixel] = (unsigned char) atoi(line);
			cpixel++;
		}
		else {
			//controllo i casi che non entrano in nulla
			printf("dentro3\n");
		}
	}

	//Alloco la struttura dati che contiene tutti i dati dei pixel
	pixel = (pixelInfo*) malloc (larghezza * altezza * sizeof(pixelInfo));

	//Associo ad ogni pixel il valore ricavato dalla palette
	for(i = 0; i < larghezza * altezza; i++) {
		pixel[i].R = palette[(int) matriceImmagineConvertita[i]].R;
		pixel[i].G = palette[(int) matriceImmagineConvertita[i]].G;
		pixel[i].B = palette[(int) matriceImmagineConvertita[i]].B;
	}

	//Chiudo il file e ritorno un valore di default
	fclose(fin);
	printf("completata lettura file GIF\n");
	getch();
	return 0;
}

//-----------------------------------------------------------------------------------------
/*
int ScriviPPM(char nomeFile[]) {

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
*/
//-----------------------------------------------------------------------------------------
int DecToHex(long decimalNumber){

	long int remainder,quotient;
	int i=1,j,temp;

	quotient = decimalNumber;

	while(quotient != 0){
		 temp = quotient % 16;

	  if( temp < 10)
		   temp =temp + 48;
	  else
		 temp = temp + 55;

	  hexadecimalNumber[i]= temp;
	  i++;
	  quotient = trunc(quotient / 16);
	}

	return 0;
}

//-----------------------------------------------------------------------------------------
int ScriviXPM(char nomeFile[]){

	char *simbolo;
	int i,j,k,numSimbolo,pos;

	//Cambio estensione al nome del file
	nomeFile[strlen(nomeFile) - 3] = 'x';
	nomeFile[strlen(nomeFile) - 2] = 'f';
	nomeFile[strlen(nomeFile) - 1] = 'm';

	//Apro il file per scrivere i colori
	FILE *fout = fopen(nomeFile, "w");
	if(fout == NULL) {
		fclose(fout);
		return -1;
	}

	fprintf(fout, "/* XPM */\n");
	fprintf(fout, "static char *");
	for(i=0;i<=strlen(nomeFile)-5;i++){
		fprintf(fout,"%c",nomeFile[i]);
	}
	fprintf(fout,"[] = {\n");

	//calcolo di quanti caratteri vengono utilizzati per rappresentare i colori
	//91 è il numero di simboli che abbiamo disponibili nell'alfabeto
	if(((log((double)numColori))/(log((double)91))) <= 2){
		bitColore = 2;
	}
	else if(((log((double)numColori))/(log((double)91))) < abs(((log((double)numColori))/(log((double)91))))+1){
		bitColore = abs(((log((double)numColori))/(log((double)91))))+1;
	}

	//stampo le caratteristiche dell'immagine
	fprintf(fout, "\"%d %d %d %d \",\n",larghezza,altezza,numColori,bitColore);

	//alloco lo spazio per andare a salvare i simboli
	simboli = (generazioneSimboli*) malloc (numColori * sizeof(generazioneSimboli));

	//stampo la palette
	//il simbolo iniziale da cui partiamo è il 032 lo spazio
	numSimbolo = 32;
	k = bitColore - 1;

	//alloco lo spazio generale per ogni simbolo
	for(i=0;i<numColori;i++){
		simboli[i].simbolo = (char*) malloc ((bitColore+1) * sizeof(char));
	}
	for(i=bitColore;i>0;i--){
		numSimbolo = 32;
		for(j=0;j<numColori;j++){

			simboli[j].simbolo[i-1] = (char)numSimbolo;
			if(numSimbolo == 124){
				numSimbolo = 32;
			}
			if((j > 1) || (i == bitColore-1)){
				if((j % ((int)pow(91,k))) == 0){
					numSimbolo++;
					if(numSimbolo == 34){
						numSimbolo++;
					}
				}
			}

		}
		k--;
	}
	for(i=0;i<numColori;i++){
		simboli[i].simbolo[bitColore] = '\0';
		//printf("%s\n",simboli[i].simbolo);
		fprintf(fout, "\"%s c #",simboli[i].simbolo);
		if(palette[i].R == 0){
			hexadecimalNumber[2] = 48;
			hexadecimalNumber[1] = 48;
		}
		else{
			DecToHex(palette[i].R);
		}
		fprintf(fout,"%c%c",hexadecimalNumber[2],hexadecimalNumber[1]);
		if(palette[i].G == 0){
			hexadecimalNumber[2] = 48;
			hexadecimalNumber[1] = 48;
		}
		else{
			DecToHex(palette[i].G);
		}
		fprintf(fout,"%c%c",hexadecimalNumber[2],hexadecimalNumber[1]);
		if(palette[i].B == 0){
			hexadecimalNumber[2] = 48;
			hexadecimalNumber[1] = 48;
		}
		else{
			DecToHex(palette[i].B);
		}
		fprintf(fout,"%c%c\",\n",hexadecimalNumber[2],hexadecimalNumber[1]);
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
		for(j=0;(j<numColori && k == 0);j++){
			if((pixel[i].R == palette[j].R) && (pixel[i].G == palette[j].G) && (pixel[i].B == palette[j].B)){
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
	getch();
	fclose(fout);
	return 0;
}

//-----------------------------------------------------------------------------------------

int main(){

	int res, i,k=2,tipoPalette=0,c=0;
	char *nomeFile,ext[3];

	//di sicuro ci sono modi piu efficaci
	res = strlen("256.gif");
	nomeFile = (char*) malloc (res*sizeof(char));
	strcpy(nomeFile,"256.gif");

	//ciclo che prende l'estensione del file
	for(i=res-1;i>=res-3;i--){
		ext[k] = nomeFile[i];
		k--;
	}

	if(strcmp(ext,"xpm")==0){
		res |= LetturaFileXPM(nomeFile);

		/*while((tipoPalette != 1) && (tipoPalette != 2)){
			printf("Per la coversione vuoi fare usare una palette globale o una palette locale ?\n");
			printf("1)palette globale\n");
			printf("2)palette locale\n");
			scanf("%d",&tipoPalette);
		}*/
		tipoPalette = 1;

		if(tipoPalette == 1){
			//costruzione palette globale

			/*while((c != 1) && ((c != 2)) && ((c != 3)) && ((c != 4)) && ((c != 5))){
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
			}*/

			for(i=255;i<256;i++){
				coloriPalette = i+1;
				res |= CostruzionePaletteGlobale();
				res |= CostruzioneMatriceImmagineGIF();

				char str[10];
				itoa(coloriPalette,str,10);
				printf("%s\n",str);
				res |= ScriviGIF(str);
			}
		}
		else if(tipoPalette == 2){
			//costruzione palette locale
		}
		else{
			//controllo i casi che non entrano in nessuna delle possibilita
		}
	}
	else if(strcmp(ext,"gif")==0){
		res |= LetturaFileGIF(nomeFile);
		res |= ScriviXPM(nomeFile);
	}

	if(res < 0){
		printf("ci sono degli errori\n");
	}

	free(matriceImmagineConvertita);
	free(nomeFile);
	free(colori);
	return 0;
	//ricorda di togliere tutti i return 0 e di far ritornare la cosa giusta
}
