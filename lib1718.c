#include "lib1718.h"

bool executeQuery(char*query) {
	int i;
	int check = 0;

	correct_query(query);

	//TROVO UNA CORRISPONDENZA CON SELECT
	if (memcmp(query, "SELECT", 6) == 0)
	{
		if (strstr(query, "FROM") == NULL)
		{
			printf("\n\n\tLa query e' sintatticamente errata.\n");
			return false;
		}
		if (query[7] == '*')
			check = 1; //E' STATA ABILITATA LA STAMPA DI TUTTA LA TABELLA

		for (i = 7; i < (signed)strlen(query); i++)
		{
			if (memcmp(query + i, "WHERE", 5) == 0)
				return(select_where(query + 7));

			else if (memcmp(query + i, "ORDER BY", 8) == 0)
				return(select_order_by(query + 7));

			else if (memcmp(query + i, "GROUP BY", 8) == 0)
			{
				if (check == 1) //PICCOLO CONTROLLO DI ERRORE
				{
					printf("\n\n Attenzione! Non puoi utilizzare il filtro GROUP BY stampando tutta la tabella.");
					printf("\n\n\tLa query e' sintatticamente errata.\n");
					return false;
				}
				return(select_group_by(query + 7));
			}
		}
		//Se non ho trovato nessuna delle precedenti corrispondenze:
		// - applico la select_columns se check non è attivo
		// - applico select_all se check è attivo
		if (check == 1)
			return(select_all(query + 7));
		else
			return(select_columns(query + 7));

	}

	else if (memcmp(query, "CREATE TABLE", 12) == 0)
		return(create_table(query + 13));

	else if (memcmp(query, "INSERT INTO", 11) == 0)
		return(insert_into(query + 12));

	printf("\n\n\tLa query e' sintatticamente errata.\n");
	return false;
}

/*La funzione crea una tabella del database.
Input: stringa query
Output:
- true: Esecuzione andata a buon fine
- false: Esecuzione non andata a buon fine*/
bool create_table(char* query) {
	FILE *file;
	int i;

	char input[STRING_LEN];				// Nome del file
	char nome_tabella[STRING_LEN];		// Nome della tabella

										// Memorizzo il nome della tabella
	query = copy_table_name(input, query);
	if (query == NULL)
		return false;
	strcpy(nome_tabella, input);
	nome_tabella[strlen(input)] = '\0';

	// Aggiungo estensione al nome del file
	strcat(input, ".txt");

	// Controllo che non esista una tabella omonima
	file = fopen(input, "r");
	if (file != NULL)
	{
		fclose(file);
		printf("\n\tLa tabella richiesta e' già esistente.\n");
		return false;
	}

	file = fopen(input, "a");
	// Verifico l'effettiva apertura del file
	if (file == NULL)
	{
		printf("\n\n\tErrore con l'apertura del file.\n");
		return false;
	}

	// Stampo intestazione
	fprintf(file, "TABLE %s COLUMNS ", nome_tabella);

	// Inserisco i nomi delle colonne all'interno del file (rimuovendo eventuali caratteri in eccesso)
	for (i = 0; query[i] != '\0' && query[i] != ')'; i++)
		if (*(query + i) != '(' && *(query + i) != ')' && *(query + i) != ' ')
			fputc(*(query + i), file);

	// Aggiungo il terminatore di riga
	fputc(';', file);

	fclose(file);
	printf("\n");
	return true;
}

/*La funziona inserisce un record all'interno della tabella
Input: stringa query
Output:
- true: Esecuzione andata a buon fine
- false: Esecuzione non andata a buon fine*/
bool insert_into(char* query)
{
	FILE* file;
	int i;

	char * colonne_file;    // Puntatore a lista nomi colonne del file
	char * colonne_input;	// Puntatore a lista nomi colonne della query
	char * valori;			// Puntatore a lista valori contenuti nella query

	int numero_colonne = 0;
	int numero_valori = 0;

	char input[STRING_LEN];
	char buff[255];

	/// Apro la tabella:
	// Memorizzo il nome della tabella
	query = copy_table_name(input, query);
	if (query == NULL)
		return false;
	// Aggiungo estensione al nome del file
	strcat(input, ".txt");
	// Controllo che la tabella richiesta esista
	file = fopen(input, "r");
	if (file == NULL)
	{
		printf("\n\tLa tabella richiesta e' inesistente.\n");
		return false;
	}

	/// Leggo prima riga del file (intestazione della tabella)
	fgets(buff, 255, file);

	/// Verifico che tutte le colonne della query corrispondano con quelle della tabella:
	// Restituisce il puntatore alla prima occorrenza del carattere '('
	colonne_input = strchr(query, '(');
	colonne_input++;
	// Restituisce il puntatore all'ultima occorrenza del carattere ' '
	colonne_file = strrchr(buff, ' ');
	colonne_file++;
	// Conto il numero di colonne
	for (i = 0; colonne_file[i] != ';'; i++)
		if (colonne_file[i] == ',')
			numero_colonne++;

	// Confronta le colonne
	for (i = 0; colonne_file[i] == colonne_input[i] && colonne_file[i] != ';' && colonne_input[i] != ')'; i++);
	// Verifico la corrispondenza
	fclose(file);
	if (!(colonne_file[i] == ';' && colonne_input[i] == ')'))
	{
		printf("\n\n\tLe colonne non corrispondono a quelle in tabella.\n");
		return false;
	}
	query++;

	/// Inserisco i valori all'interno della tabella:
	// Preparo il file alla scrittura
	file = fopen(input, "a");
	// Restituisce il puntatore all'ultima occorrenza del carattere '(' della query
	valori = strrchr(query, '(');

	//controllo che strrchr abbia avuto buon fine
	if (valori == NULL)
	{
		printf("\n\n\tNon sono stati scritti i valori da inserire.\n");
		return false;
	}
	valori++;
	// Conto il numero di valori
	if (valori[0] == ',')
	{
		printf("\n\n\tPer inserire un campo vuoto non e' valido lo spazio: \n\t\tbisogna scrivere NULL!\n");
		fclose(file);
		return false;
	}

	for (i = 0; valori[i] != '\0'; i++)
		if (valori[i] == ',' && valori[i + 1] != ',')
			numero_valori++;
		else if ((valori[i] == ',') && (valori[i + 1] == ',' || valori[i + 1] == ')'))
		{
			printf("\n\n\tPer inserire un campo vuoto non e' valido lo spazio: \n\t\tbisogna scrivere NULL!\n");
			fclose(file);
			return false;
		}

	if (numero_valori != numero_colonne)
	{
		printf("\n\n\tLe colonne non corrispondono a quelle in tabella.\n");
		fclose(file);
		return false;
	}
	// Inserisco la ROW all'interno del file
	fprintf(file, "\nROW ");
	for (i = 0; valori[i] != ')'; i++)
		fprintf(file, "%c", valori[i]);
	fprintf(file, ";");

	fclose(file);
	printf("\n");
	return true;
}

/*La funzione restituisce tutto il contenuto della tabella.
Input: stringa query
Output:
- true: Esecuzione andata a buon fine
- false: Esecuzione non andata a buon fine*/
bool select_all(char* query)
{
	FILE* file;
	FILE * results = NULL;

	char input[STRING_LEN];
	char buff[255];

	// Apro il file query_results
	results = put_query_results(query, results);
	if (results == NULL)
	{
		printf("\n\n\tErrore con l'apertura del file.\n");
		return false;
	}

	query = query + 2;

	file = extract_table(query, input);
	if (file == NULL)
	{
		printf("\n\n\tErrore con l'apertura del file.\n");
		fprintf(results, "\n");
		fclose(results);
		return false;
	}

	// Visualizzo il contenuto della tabella
	printf("\n");
	while (feof(file) != 1)
	{
		fgets(buff, 255, file);
		printf("%s", buff);
		fprintf(results, "%s", buff);
	}
	printf("\n\n");
	fclose(file);
	fprintf(results, "\n\n");
	fclose(results);
	return true;
}

/*La funzione restituisce il contenuto delle colonne selezionate.
Input: Stringa query
Output:
- true: Esecuzione andata a buon fine
- false: Esecuzione non andata a buon fine*/
bool select_columns(char*query)
{
	FILE *file;
	FILE *results = NULL;

	char *** table;
	char ** columns;
	char input[STRING_LEN];

	int numero_colonne, numero_row;

	results = put_query_results(query, results); // Inserisco la query richiesta in query_results.txt
	if (results == NULL)
	{
		printf("\n\n\tErrore con l'apertura del file.\n");
		return false;
	}

	file = extract_table(query, input);
	if (file == NULL)
	{
		printf("\n\n\tErrore con l'apertura del file.\n");
		fprintf(results, "\n");
		fclose(results);
		return false;
	}

	table = make_structure(file, &columns, &numero_colonne, &numero_row);
	if (table == NULL)
	{
		printf("\n\n\tErrore con l'allocazione della memoria.\n");
		fprintf(results, "\n");
		fclose(results);
		fclose(file);
		return false;
	}

	/* Qui puoi vedere la stampa: semplice e veloce!
	for (int i = 0; i<numero_colonne; i++)
	printf("%s\n", columns[i]);

	for (int k = 0; k< numero_row; k++)
	for (int i = 0; i<numero_colonne; i++)
	printf("%s\n", table[k][i]);
	*/

	//Visualizzo le colonne richieste
	view(table, columns, numero_colonne, numero_row, query, input, &results);
	if (results == NULL) //Se ho chiuso il puntatore al file query_results.txt allora significa che non è andata a buon fine la stampa
	{
		fclose(file);
		free_structure(table, columns, numero_colonne, numero_row);
		return false;
	}

	fclose(file);
	fprintf(results, "\n");
	fclose(results);
	free_structure(table, columns, numero_colonne, numero_row);
	return true;
}

/*La funzione restituisce il contenuto delle colonne selezionate.
Input: Stringa query
Output:
- true: Esecuzione andata a buon fine
- false: Esecuzione non andata a buon fine*/
bool select_where(char*query) {
	FILE *file;
	FILE *results = NULL;

	char *** table;
	char ** columns;
	char*ptr; //Puntatore che uso per scorrere la riga di query

	char input[STRING_LEN];	// Salvo il nome della tabella senza estensione del file
	char condition[STRING_LEN]; // Salvo la condizione che mi richiede la query

	int numero_colonne, numero_row;

	results = put_query_results(query, results); // Inserisco la query richiesta in query_results.txt
	if (results == NULL)
	{
		printf("\n\n\tErrore con l'apertura del file.\n");
		return false;
	}

	// Spezzo la query e la where, in modo da poter chiamare la extract_table
	ptr = query;

	while (*ptr != '\0')
	{
		if (strncmp(ptr, "WHERE", 5) == 0)
		{
			strcpy(condition, ptr);

			//Modifico la query in modo da poter sfruttare la funzione extract_table
			ptr--;
			*ptr = '\0'; //Aggiungo l'invio
			ptr--; // Torno indietro di una posizione in modo che esca dal while
		}
		ptr++;
	}
	// Aggiungo alla query il terminatore di stringa
	ptr++;
	*ptr = '\0';

	// Apertura della tabella richiesta
	file = extract_table(query, input);
	if (file == NULL)
	{
		printf("\n\n\tErrore con l'apertura del file.\n");
		fprintf(results, "\n");
		fclose(results);
		return false;
	}

	table = make_structure(file, &columns, &numero_colonne, &numero_row);
	if (table == NULL)
	{
		printf("\n\n\tErrore con l'allocazione della memoria.\n");
		fclose(file);
		fprintf(results, "\n");
		fclose(results);
		return false;
	}

	//Applico la funzione che mi scarta le righe della struttura che non soddisfano la where
	clean_structure(table, columns, numero_colonne, &numero_row, condition);
	//Controllo che non siano stati commessi errori durante la clean structure
	if (numero_row == -1)
	{
		fclose(file);
		fprintf(results, "\n");
		fclose(results);
		return false;
	}

	//Visualizzo le colonne richieste
	view(table, columns, numero_colonne, numero_row, query, input, &results);
	if (results == NULL) //Se ho chiuso il puntatore al file query_results.txt allora significa che non è andata a buon fine la stampa
	{
		fclose(file);
		free_structure(table, columns, numero_colonne, numero_row);
		return false;
	}

	fclose(file);
	fprintf(results, "\n");
	fclose(results);
	free_structure(table, columns, numero_colonne, numero_row);
	return true;
}

/*La funzione restituisce il contenuto delle colonne selezionate.
Input: Stringa query
Output:
- true: Esecuzione andata a buon fine
- false: Esecuzione non andata a buon fine*/
bool select_order_by(char*query)
{
	FILE *file, *results = NULL;

	char *** table;
	char ** columns;
	char * tmp;
	char input[STRING_LEN], colonna[COLUMN_LENGHT];

	char condition[STRING_LEN];

	int numero_colonne, numero_row;
	int i = 0;
	int order = -1;

	results = put_query_results(query, results); // Inserisco la query richiesta in query_results.txt
	if (results == NULL)
	{
		//free(colonna);
		printf("\n\n\tErrore con l'apertura del file.\n");
		return false;
	}

	if (strstr(query, "ASC") != NULL)
		order = 1; // Ordinamento crescente
	if (strstr(query, "DES") != NULL)
		order = 0; // Ordinamento crescente

				   // Query non presenta il tipo di ordinamento
	if (order == -1)
	{
		printf("\n\n\tLa query e' sintatticamente errata: Mancanza di ASC o DESC.\n");
		fprintf(results, "\n");
		fclose(results);
		return false;
	}

	// Identifico la colonna da ordinare
	tmp = strstr(query, "ORDER BY");
	strcpy(condition, tmp);
	tmp += 9;
	while (*(tmp + i) != ' ')
		i++;
	memcpy(colonna, tmp, i);
	colonna[i] = '\0';

	// Extract table modificata
	{
		char* ptr;
		int i;

		//Prelevo l'ultima corripondenza del carattere ' '
		ptr = strchr(query, ' ');
		ptr++;
		ptr = strchr(ptr, ' ');

		//Vado alla posizione successiva del carattere ' '
		ptr++; //Ora ptr punta al nome del file

			   //Aggiungo l'estensione al nome del file per poterci accedere

			   //Cerco la corrispondenza del primo spazio
		for (i = 0; ptr[i] != ' '; i++);
		//Sostituisco il carattere con '\0'
		ptr[i] = '\0';
		//Copio il nome della tabella nella variabile dedicata
		strcpy(input, ptr);
		//Aggiungo l'estensione al puntatore
		strcat(ptr, ".txt");

		//Apertura in lettura del file

		file = fopen(ptr, "r");
	}

	//file = extract_table(query, input);
	if (file == NULL)
	{
		fclose(results);
		//free(colonna);
		printf("\n\n\tErrore con l'apertura del file.\n");
		fprintf(results, "\n");
		fclose(results);
		return false;
	}

	table = make_structure(file, &columns, &numero_colonne, &numero_row);
	if (table == NULL)
	{
		fclose(file);
		fprintf(results, "\n");
		fclose(results);
		//free(colonna);
		printf("\n\n\tErrore con l'allocazione della memoria.\n");
		return false;
	}

	for (i = 0; i < numero_colonne && memcmp(columns[i], colonna, strlen(colonna) - 1); i++)
		continue;
	if (i >= numero_colonne)
	{
		fclose(file);
		fprintf(results, "\n");
		fclose(results);
		//free(colonna);
		free_structure(table, columns, numero_colonne, numero_row);
		printf("\n\n\tLa colonna richiesta per l'odrinamento non è valida.\n");
		return false;
	}
	mergeSort(table, 0, numero_row - 1, i, order, 2);

	//Stampo il contenuto della tabella
	view(table, columns, numero_colonne, numero_row, query, input, &results);
	if (results == NULL) //Se ho chiuso il puntatore al file query_results.txt allora significa che non è andata a buon fine la stampa
	{
		fclose(file);
		free_structure(table, columns, numero_colonne, numero_row);
		return false;
	}

	fprintf(results, "\n");

	//Preparo alla chiusura
	fclose(file);
	fclose(results);
	//free(colonna);
	free_structure(table, columns, numero_colonne, numero_row);
	return true;
}

/*La funzione restituisce il raggruppamento della colonna richiesta.
Input: Stringa query
Output:
- true: Esecuzione andata a buon fine
- false: Esecuzione non andata a buon fine*/
bool select_group_by(char*query)
{
	FILE *file, *results = NULL;

	char *** table;
	char ** columns;
	char *tmp, *ptr;
	char input[STRING_LEN], colonna[COLUMN_LENGHT], assist[COLUMN_LENGHT];

	int numero_colonne, numero_row;
	int i = 0, k, j;

	// Apertura file di OUTPUT
	results = put_query_results(query, results);
	if (results == NULL)
	{
		printf("\n\n\tErrore con l'apertura del file.\n");
		return false;
	}

	// Identifico la colonna da ordinare
	tmp = strstr(query, "GROUP BY");
	tmp += 9;
	while (*(tmp + i) != '\0')
		i++;

	strncpy(colonna, tmp, i);
	colonna[i] = '\0';

	// Verifico che la colonna della select sia la stessa del group by
	j = 0;
	while (*(query + j) != ' ')
		j++;
	if (j > COLUMN_LENGHT)
	{
		fprintf(results, "\n");
		fclose(results);
		printf("\n\n\tLa query e' sintatticamente errata.\n");
		return false;
	}

	memcpy(assist, query, j);
	assist[j] = '\0';
	if (strlen(colonna) != strlen(assist) || strncmp(assist, colonna, strlen(assist)) != 0)
	{
		fprintf(results, "\n");
		fclose(results);
		printf("\n\n\tLa query e' sintatticamente errata.\n");
		return false;
	}

	/// Extract table modificata
	//Prelevo l'ultima corripondenza del carattere ' '
	ptr = strchr(query, ' ');
	ptr++;
	ptr = strchr(ptr, ' ');
	//Vado alla posizione successiva del carattere ' '
	ptr++; //Ora ptr punta al nome del file
		   //Cerco la corrispondenza dello spazio
	for (k = 0; ptr[k] != ' '; k++);
	//Sostituisco il carattere con '\0'
	ptr[k] = '\0';
	//Copio il nome della tabella nella variabile dedicata
	strcpy(input, ptr);
	//Aggiungo l'estensione al puntatore
	strcat(ptr, ".txt");

	//Apertura in lettura del file
	file = fopen(ptr, "r");

	if (file == NULL)
	{
		fclose(results);
		printf("\n\n\tErrore con l'apertura del file.\n");
		return false;
	}

	table = make_structure(file, &columns, &numero_colonne, &numero_row);
	if (table == NULL)
	{
		fclose(file);
		fprintf(results, "\n");
		fclose(results);
		printf("\n\n\tErrore con l'allocazione della memoria.\n");
		return false;
	}

	// Ricerco l'indice della colonna su cui effettuare il group by
	for (i = 0; i <= numero_colonne - 1 && memcmp(columns[i], colonna, strlen(colonna)); i++)
		continue;
	if (i > numero_colonne-1)
		return false;

	// Verifico che la colonna esista
	if (i >= numero_colonne)
	{
		// Preparo alla chiusura
		fclose(file);
		fprintf(results, "\n");
		fclose(results);
		free_structure(table, columns, numero_colonne, numero_row);
		printf("\n\n\tLa query e' sintatticamente errata.\n");
		return false;
	}

	mergeSort(table, 0, numero_row - 1, i, 1, 2);

	///VIEW MODIFICATA
	// Stampo intestazione
	fprintf(results, "TABLE %s COLUMNS %s,COUNT;\n", colonna, colonna);
	printf("TABLE %s COLUMNS %s,COUNT;\n", colonna, colonna);

	// Stampo la funzione group by
	j = 0;
	char * colonna_attuale = NULL;
	for (int k = 0; k < numero_row; k++)
	{
		if (colonna_attuale == NULL)
		{
			colonna_attuale = table[k][i];
			j++;
			continue;
		}


		if (strncmp(table[k][i], colonna_attuale, strlen(colonna_attuale)) == 0)
			j++;
		else
		{
			fprintf(results, "ROW %s,%d;\n", colonna_attuale, j);
			printf("ROW %s,%d;\n", colonna_attuale, j);
			colonna_attuale = table[k][i];
			j = 1;
		}

	}
	fprintf(results, "ROW %s,%d;\n\n", colonna_attuale, j);
	printf("ROW %s,%d;\n\n", colonna_attuale, j);

	// Preparo alla chiusura
	fclose(results);
	fclose(file);
	free_structure(table, columns, numero_colonne, numero_row);
	return true;
}

char* copy_table_name(char *input, char* query)
{
	int i;

	// Estraggo il nome della tabella
	for (i = 0; query[i + 1] != '(' && query[i] != '\0'; i++)
	{
		if (query[i] == ' ')
		{
			printf("\n\n\tIl nome della tabella non puo' contenere spazi.\n");
			return NULL;
		}
		input[i] = *(query + i);
	}
	input[i] = '\0';

	// Indicizzo la definizione delle colonne
	query = query + (i + 1);

	return query;
}

FILE* extract_table(char*query, char *input)
{
	FILE *file;

	char* ptr;

	//Prelevo l'ultima corripondenza del carattere ' '
	ptr = strrchr(query, ' ');
	//Vado alla posizione successiva del carattere ' '
	ptr++; //Ora ptr punta al nome del file

		   //Aggiungo l'estensione al nome del file per poterci accedere

		   //Copio il nome della tabella nella variabile dedicata
	strcpy(input, ptr);
	//Aggiungo l'estensione al puntatore
	strcat(ptr, ".txt");

	//Apertura in lettura del file

	file = fopen(ptr, "r");
	// Controllo che la tabella richiesta sia esistente
	if (file == NULL)
		return NULL;
	else
	{
		//Rimuovo il .txt 
		ptr = strrchr(query, '.');
		*ptr = ' ';
		ptr[1] = '\0';
		return file;
	}
}

void view(char*** table, char ** columns, int num_columns, int num_row, char*query, char* input, FILE **results)
{
	//Indici per scorrere la struttura
	int i = 0;
	int j = 0;
	//Variabile d'appoggio richiesta query
	char value[STRING_LEN];

	int indici[STRING_LEN];
	int num_indici;
	int found = 0; //Variabile di controllo se e' stata trovata la colonna inserita

	if (table_is_empty(table, num_row) == false)
	{
		if (*query != '*')
		{

			//Conto quante colonne la query vuole stampare e cerco le occorrenze nella tabella columns
			for (num_indici = 0; *query != ' '; query++)
			{
				if (*query != ',')
					value[i] = *query;
				else
				{
					value[i] = '\0';
					found = 0;
					for (j = 0; j < num_columns; j++)
						if (strcmp(columns[j], value) == 0)
						{
							indici[num_indici] = j;
							num_indici++;
							i = -1;
							found = 1;
						}
					if (found == 0)
					{
						printf("\n\tAlmeno una delle colonne inserite non esiste in tabella.\n");
						fprintf(*results, "\n");
						fclose(*results);
						*results = NULL;
						return;
					}
				}
				i++;
			}
			value[i] = '\0';
			found = 0;
			for (i = 0; i < num_columns; i++)
				if (strcmp(columns[i], value) == 0)
				{
					indici[num_indici] = i;
					num_indici++;
					found = 1;
				}
			if (found == 0)
			{
				printf("\n\tAlmeno una delle colonne inserite non esiste in tabella.\n");
				fprintf(*results, "\n");
				fclose(*results);
				*results = NULL;
				return;
			}

			//Stampo l'intestazione della tabella
			printf("TABLE %s COLUMNS ", input);
			fprintf(*results, "TABLE %s COLUMNS ", input);
			for (j = 0; j < num_indici; j++)
			{
				printf("%s", columns[indici[j]]);
				fprintf(*results, "%s", columns[indici[j]]);
				if ((j + 1) < num_indici)
				{
					printf(",");
					fprintf(*results, ",");
				}
			}
			printf(";\n");
			fprintf(*results, ";\n");

			// Stampo le colonne richieste
			for (i = 0; i < num_row; i++)
			{
				if (table[i] != NULL)
				{
					printf("ROW ");
					fprintf(*results, "ROW ");
					for (j = 0; j < num_indici; j++)
					{
						printf("%s", table[i][indici[j]]);
						fprintf(*results, "%s", table[i][indici[j]]);
						if ((j + 1) < num_indici)
						{
							printf(", ");
							fprintf(*results, ", ");
						}
					}
					printf(";\n");
					fprintf(*results, ";\n");
				}

			}
			printf("\n");
			return;
		}
		else
		{
			//Stampo tutte le colonne di intestazione
			printf("TABLE %s COLUMNS ", input);
			fprintf(*results, "TABLE %s COLUMNS ", input);
			for (j = 0; j < num_columns; j++)
			{
				printf("%s", columns[j]);
				fprintf(*results, "%s", columns[j]);
				if ((j + 1) < num_columns)
				{
					printf(", ");
					fprintf(*results, ", ");
				}
			}
			printf(";\n");
			fprintf(*results, ";\n");

			//Stampo tutto il contenuto della tabella
			for (i = 0; i < num_row; i++)
			{
				if (table[i] != NULL)
				{
					printf("ROW ");
					fprintf(*results, "ROW ");
					for (j = 0; j < num_columns; j++)
					{
						printf("%s", table[i][j]);
						fprintf(*results, "%s", table[i][j]);
						if ((j + 1) < num_columns)
						{
							printf(", ");
							fprintf(*results, ", ");
						}
					}
					printf(";\n");
					fprintf(*results, ";\n");
				}
			}
			printf("\n");
			return;
		}
	}
	else
		printf("\n\n\tNessuna riga corrisponde ai criteri di ricerca inseriti.\n");

}

void clean_structure(char*** table, char ** columns, int num_columns, int *num_row, char*condition)
{
	int f = 0;
	//Indici
	int i = 0;
	int j = 0;
	int k = 0;
	//int atoi
	int val_condizione = 0;
	int val_struttura = 0;
	//controlli
	char control[STRING_LEN];
	//int check;

	int column = -1; //Colonna della condizione
	int len = 0; //Lunghezza della colonna condizione

				 //Cancello la parte iniziale della condizione
	condition = condition + 6;

	//Copio la colonna
	//Conto la lunghezza della colonna scritta
	while (condition[i] != '<' && condition[i] != '>' && condition[i] != '=')
	{
		len++;
		i++;
	}

	//Cerco una corrispondenza con l'intestazione del file e salvo il relativo indice;
	for (i = 0; i < num_columns && column == -1; i++)
	{
		if (strncmp(columns[i], condition, len) == 0)
			column = i;
	}

	//Se non viene trovata la colonna in struttura allora e' stato commesso un errore
	if (column == -1)
	{
		printf("\n\tLa colonna inserita nella condizione non esiste in tabella.\n");
		*num_row = -1; //Passo al programma principale un numero di righe che mi consente di trovare la presenza di un errore
		return;
	}
	//Identifico l'operatore e in base a questo effettuo la pulizia della tabella
	condition = condition + len;

	//Ciclo tutte le righe e faccio i controlli in base all'operatore: ALLA FINE VIENE COMPIUTO UNO SOLO DI QUESTI FRAMMENTI.

	if (strncmp(condition, ">=", 2) == 0) //Cerco la corrispondenza con l'operatore
	{
		condition = condition + 2; //Elimino l'operatore
								   //tento di convertire la condizione in intero
		val_condizione = atoi(condition);
		//converto l'intero in stringa per controllare che atoi() sia corretto
		_itoa(val_condizione, control, 10);
		//valuto se la conversione effettuata e' uguale alla condizione, se non lo è significa che ho a che fare con un tipo di dato misto
		if (strcmp(control, condition) != 0)
		{
			//CASO IN CUI LA CONDIZIONE E' UNA STRINGA
			for (i = 0; i < *num_row; i++)
			{
				val_struttura = atoi(table[i][column]);
				//converto l'intero in stringa per controllare che atoi() sia corretto
				_itoa(val_struttura, control, 10);
				//valuto se la conversione effettuata e' uguale alla condizione, se non lo è significa che ho a che fare con un tipo di dato misto
				if (strcmp(control, table[i][column]) == 0)
				{
					printf("\n\tIl contenuto della colonna richiesta non e' una stringa.\n");
					*num_row = -1;//Restituisco un valore che mi permetta di riconoscere che e'avvenuto un errore
					return;
				}
				if (strcmp(condition, table[i][column]) > 0)
				{	//Se non è verificata la condizione elimino la riga
					//Libero il contenuto delle celle della riga
					for (int j = 0; j < num_columns; j++)
						free(table[i][j]);
					//Dico che la riga è vuota
					table[i] = NULL;
				}
			}
		}
		else
		{
			//CASO IN CUI LA CONDIZIONE E' UN INTERO
			for (i = 0; i < *num_row; i++)
			{
				val_struttura = atoi(table[i][column]);
				//converto l'intero in stringa per controllare che atoi() sia corretto
				_itoa(val_struttura, control, 10);
				//valuto se la conversione effettuata e' uguale alla condizione, se non lo è significa che ho a che fare con un tipo di dato misto(stringa)
				if (strcmp(control, table[i][column]) != 0)
				{
					printf("\n\tIl contenuto della colonna richiesta non e' un intero.\n");
					*num_row = -1;//Restituisco un valore che mi permetta di riconoscere che e'avvenuto un errore
					return;
				}
				if (val_struttura < val_condizione)
				{	//Se non è verificata la condizione elimino la riga
					//Libero il contenuto delle celle della riga
					for (int j = 0; j < num_columns; j++)
						free(table[i][j]);
					//Dico che la riga è vuota
					table[i] = NULL;
				}
			}
		}
		return;
	}
	else if (strncmp(condition, "<=", 2) == 0)
	{
		condition = condition + 2; //Elimino l'operatore
								   //tento di convertire la condizione in intero
		val_condizione = atoi(condition);
		//converto l'intero in stringa per controllare che atoi() sia corretto
		_itoa(val_condizione, control, 10);
		//valuto se la conversione effettuata e' uguale alla condizione, se non lo è significa che ho a che fare con un tipo di dato misto
		if (strcmp(control, condition) != 0)
		{
			//CASO IN CUI LA CONDIZIONE E' UNA STRINGA
			for (i = 0; i < *num_row; i++)
			{
				val_struttura = atoi(table[i][column]);
				//converto l'intero in stringa per controllare che atoi() sia corretto
				_itoa(val_struttura, control, 10);
				//valuto se la conversione effettuata e' uguale alla condizione, se non lo è significa che ho a che fare con un tipo di dato misto
				if (strcmp(control, table[i][column]) == 0)
				{
					printf("\n\tIl contenuto della colonna richiesta non e' una stringa.\n");
					*num_row = -1;//Restituisco un valore che mi permetta di riconoscere che e'avvenuto un errore
					return;
				}
				if (strcmp(condition, table[i][column]) < 0)
				{	//Se non è verificata la condizione elimino la riga
					//Libero il contenuto delle celle della riga
					for (int j = 0; j < num_columns; j++)
						free(table[i][j]);
					//Dico che la riga è vuota
					table[i] = NULL;
				}
			}
		}
		else {
			//CASO IN CUI LA CONDIZIONE E' UN INTERO
			for (i = 0; i < *num_row; i++)
			{
				val_struttura = atoi(table[i][column]);
				//converto l'intero in stringa per controllare che atoi() sia corretto
				_itoa(val_struttura, control, 10);
				//valuto se la conversione effettuata e' uguale alla condizione, se non lo è significa che ho a che fare con un tipo di dato misto(stringa)
				if (strcmp(control, table[i][column]) != 0)
				{
					printf("\n\tIl contenuto della colonna richiesta non e' un intero.\n");
					*num_row = -1;//Restituisco un valore che mi permetta di riconoscere che e'avvenuto un errore
					return;
				}

				if (val_struttura > val_condizione)
				{	//Se non è verificata la condizione elimino la riga
					//Libero il contenuto delle celle della riga
					for (int j = 0; j < num_columns; j++)
						free(table[i][j]);
					//Dico che la riga è vuota
					table[i] = NULL;
				}
			}
		}
		return;
	}
	else if (strncmp(condition, "==", 2) == 0)
	{
		condition = condition + 2; //Elimino l'operatore
								   //tento di convertire la condizione in intero
		val_condizione = atoi(condition);
		//converto l'intero in stringa per controllare che atoi() sia corretto
		_itoa(val_condizione, control, 10);
		//valuto se la conversione effettuata e' uguale alla condizione, se non lo è significa che ho a che fare con un tipo di dato misto
		if (strcmp(control, condition) != 0)
		{
			//CASO IN CUI LA CONDIZIONE E' UNA STRINGA
			for (i = 0; i < *num_row; i++)
			{
				val_struttura = atoi(table[i][column]);
				//converto l'intero in stringa per controllare che atoi() sia corretto
				_itoa(val_struttura, control, 10);
				//valuto se la conversione effettuata e' uguale alla condizione, se non lo è significa che ho a che fare con un tipo di dato misto
				if (strcmp(control, table[i][column]) == 0)
				{
					printf("\n\tIl contenuto della colonna richiesta non e' una stringa.\n");
					*num_row = -1;//Restituisco un valore che mi permetta di riconoscere che e'avvenuto un errore
					return;
				}
				if (strcmp(condition, table[i][column]) != 0)
				{	//Se non è verificata la condizione elimino la riga
					//Libero il contenuto delle celle della riga
					for (int j = 0; j < num_columns; j++)
						free(table[i][j]);
					//Dico che la riga è vuota
					table[i] = NULL;
				}
			}
		}
		else
		{
			//CASO IN CUI LA CONDIZIONE E' UN INTERO
			for (i = 0; i < *num_row; i++)
			{
				val_struttura = atoi(table[i][column]);
				//converto l'intero in stringa per controllare che atoi() sia corretto
				_itoa(val_struttura, control, 10);
				//valuto se la conversione effettuata e' uguale alla condizione, se non lo è significa che ho a che fare con un tipo di dato misto(stringa)
				if (strcmp(control, table[i][column]) != 0)
				{
					printf("\n\tIl contenuto della colonna richiesta non e' un intero.\n");
					*num_row = -1;//Restituisco un valore che mi permetta di riconoscere che e'avvenuto un errore
					return;
				}

				if (val_struttura != val_condizione)
				{	//Se non è verificata la condizione elimino la riga
					//Libero il contenuto delle celle della riga
					for (int j = 0; j < num_columns; j++)
						free(table[i][j]);
					//Dico che la riga è vuota
					table[i] = NULL;
				}
			}
		}
		return;
	}
	else if (strncmp(condition, ">", 1) == 0)
	{
		condition = condition + 1; //Elimino l'operatore
								   //tento di convertire la condizione in intero
		val_condizione = atoi(condition);
		//converto l'intero in stringa per controllare che atoi() sia corretto
		_itoa(val_condizione, control, 10);
		//valuto se la conversione effettuata e' uguale alla condizione, se non lo è significa che ho a che fare con un tipo di dato misto
		if (strcmp(control, condition) != 0)
		{
			//CASO IN CUI LA CONDIZIONE E' UNA STRINGA
			if ((strncmp(condition, ">", 1) != 0) && (strncmp(condition, "<", 1) != 0))
			{
				for (i = 0; i < *num_row; i++)
				{
					val_struttura = atoi(table[i][column]);
					//converto l'intero in stringa per controllare che atoi() sia corretto
					_itoa(val_struttura, control, 10);
					//valuto se la conversione effettuata e' uguale alla condizione, se non lo è significa che ho a che fare con un tipo di dato misto
					if (strcmp(control, table[i][column]) == 0)
					{
						printf("\n\tIl contenuto della colonna richiesta non e' una stringa.\n");
						*num_row = -1;//Restituisco un valore che mi permetta di riconoscere che e'avvenuto un errore
						return;
					}
					if (strcmp(condition, table[i][column]) > 0 || strcmp(condition, table[i][column]) == 0)
					{	//Se non è verificata la condizione elimino la riga
						//Libero il contenuto delle celle della riga
						for (int j = 0; j < num_columns; j++)
							free(table[i][j]);
						//Dico che la riga è vuota
						table[i] = NULL;
					}
				}
			}
			else
			{
				//Non viene riconosciuto l'operatore inserito.
				printf("\n\tL'operatore inserito nella condizione non e' valido.\n");
				*num_row = -1;//Restituisco un valore che mi permetta di riconoscere che e'avvenuto un errore
				return;
			}
		}
		else
		{
			//CASO IN CUI LA CONDIZIONE E' UN INTERO
			if ((strncmp(condition, ">", 1) != 0) && (strncmp(condition, "<", 1) != 0))
			{
				for (i = 0; i < *num_row; i++)
				{
					val_struttura = atoi(table[i][column]);
					//converto l'intero in stringa per controllare che atoi() sia corretto
					_itoa(val_struttura, control, 10);
					//valuto se la conversione effettuata e' uguale alla condizione, se non lo è significa che ho a che fare con un tipo di dato misto(stringa)
					if (strcmp(control, table[i][column]) != 0)
					{
						printf("\n\tIl contenuto della colonna richiesta non e' un intero.\n");
						*num_row = -1;//Restituisco un valore che mi permetta di riconoscere che e'avvenuto un errore
						return;
					}
					if (val_struttura < val_condizione || val_struttura == val_condizione)
					{	//Se non è verificata la condizione elimino la riga
						//Libero il contenuto delle celle della riga
						for (int j = 0; j < num_columns; j++)
							free(table[i][j]);
						//Dico che la riga è vuota
						table[i] = NULL;
					}
				}
			}
			else
			{
				//Non viene riconosciuto l'operatore inserito.
				printf("\n\tL'operatore inserito nella condizione non e' valido.\n");
				*num_row = -1;//Restituisco un valore che mi permetta di riconoscere che e'avvenuto un errore
				return;
			}
		}
		return;
	}
	else if (strncmp(condition, "<", 1) == 0)
	{
		condition = condition + 1; //Elimino l'operatore
								   //tento di convertire la condizione in intero
		val_condizione = atoi(condition);
		//converto l'intero in stringa per controllare che atoi() sia corretto
		_itoa(val_condizione, control, 10);
		//valuto se la conversione effettuata e' uguale alla condizione, se non lo è significa che ho a che fare con un tipo di dato misto
		if (strcmp(control, condition) != 0)
		{
			//CASO IN CUI LA CONDIZIONE E' UNA STRINGA
			if ((strncmp(condition, ">", 1) != 0) && (strncmp(condition, "<", 1) != 0))
			{
				for (i = 0; i < *num_row; i++)
				{
					val_struttura = atoi(table[i][column]);
					//converto l'intero in stringa per controllare che atoi() sia corretto
					_itoa(val_struttura, control, 10);
					//valuto se la conversione effettuata e' uguale alla condizione, se non lo è significa che ho a che fare con un tipo di dato misto
					if (strcmp(control, table[i][column]) == 0)
					{
						printf("\n\tIl contenuto della colonna richiesta non e' una stringa.\n");
						*num_row = -1;//Restituisco un valore che mi permetta di riconoscere che e'avvenuto un errore
						return;
					}
					if (strcmp(condition, table[i][column]) < 0 || strcmp(condition, table[i][column]) == 0)
					{	//Se non è verificata la condizione elimino la riga
						//Libero il contenuto delle celle della riga
						for (int j = 0; j < num_columns; j++)
							free(table[i][j]);
						//Dico che la riga è vuota
						table[i] = NULL;
					}
				}
			}
			else {
				//Non viene riconosciuto l'operatore inserito.
				printf("\n\tL'operatore inserito nella condizione non e' valido.\n");
				*num_row = -1;//Restituisco un valore che mi permetta di riconoscere che e'avvenuto un errore
				return;
			}
		}
		else
		{
			//CASO IN CUI LA CONDIZIONE E' UN INTERO
			if ((strncmp(condition, ">", 1) != 0) && (strncmp(condition, "<", 1) != 0))
			{
				for (i = 0; i < *num_row; i++)
				{
					val_struttura = atoi(table[i][column]);
					//converto l'intero in stringa per controllare che atoi() sia corretto
					_itoa(val_struttura, control, 10);
					//valuto se la conversione effettuata e' uguale alla condizione, se non lo è significa che ho a che fare con un tipo di dato misto(stringa)
					if (strcmp(control, table[i][column]) != 0)
					{
						printf("\n\tIl contenuto della colonna richiesta non e' un intero.\n");
						*num_row = -1;//Restituisco un valore che mi permetta di riconoscere che e'avvenuto un errore
						return;
					}
					if (strcmp(condition, table[i][column]) < 0 || strcmp(condition, table[i][column]) == 0)
					{	//Se non è verificata la condizione elimino la riga
						//Libero il contenuto delle celle della riga
						for (int j = 0; j < num_columns; j++)
							free(table[i][j]);
						//Dico che la riga è vuota
						table[i] = NULL;
					}
				}
			}
			else {
				//Non viene riconosciuto l'operatore inserito.
				printf("\n\tL'operatore inserito nella condizione non e' valido.\n");
				*num_row = -1;//Restituisco un valore che mi permetta di riconoscere che e'avvenuto un errore
				return;
			}
		}
		return;
	}
	else {
		//Caso base, non viene riconosciuto l'operatore inserito.
		printf("\n\tL'operatore inserito nella condizione non e' valido.\n");
		*num_row = -1;//Restituisco un valore che mi permetta di riconoscere che e'avvenuto un errore
		return;
	}
}

void correct_query(char*query)
{
	int i, j;
	char* ptr;
	//Prima di tutto rimuovo eventuali spazi all'inizio della query
	while (query[0] == ' ')
		for (i = 0; query[i] != '\0'; i++)
			query[i] = query[i + 1];

	//Piccolo controllo che i comandi dati siamo maiuscoli
	if ((ptr = strstr(query, "select")) != NULL || (ptr = strstr(query, "Select")) != NULL)
	{
		*ptr = 'S';
		ptr[1] = 'E';
		ptr[2] = 'L';
		ptr[3] = 'E';
		ptr[4] = 'C';
		ptr[5] = 'T';
	}

	if ((ptr = strstr(query, "insert into")) != NULL || (ptr = strstr(query, "Insert Into")) != NULL || (ptr = strstr(query, "Insert into")) != NULL || (ptr = strstr(query, "insert Into")) != NULL)
	{
		*ptr = 'I';
		ptr[1] = 'N';
		ptr[2] = 'S';
		ptr[3] = 'E';
		ptr[4] = 'R';
		ptr[5] = 'T';
		ptr[6] = ' ';
		ptr[7] = 'I';
		ptr[8] = 'N';
		ptr[9] = 'T';
		ptr[10] = 'O';
	}

	if ((ptr = strstr(query, "create table")) != NULL || (ptr = strstr(query, "Create Table")) != NULL || (ptr = strstr(query, "Create table")) != NULL || (ptr = strstr(query, "create Table")) != NULL)
	{
		*ptr = 'C';
		ptr[1] = 'R';
		ptr[2] = 'E';
		ptr[3] = 'A';
		ptr[4] = 'T';
		ptr[5] = 'E';
		ptr[6] = ' ';
		ptr[7] = 'T';
		ptr[8] = 'A';
		ptr[9] = 'B';
		ptr[10] = 'L';
		ptr[11] = 'E';
	}

	if ((ptr = strstr(query, "group by")) != NULL || (ptr = strstr(query, "Group By")) != NULL || (ptr = strstr(query, "Group by")) != NULL || (ptr = strstr(query, "group By")) != NULL)
	{
		*ptr = 'G';
		ptr[1] = 'R';
		ptr[2] = 'O';
		ptr[3] = 'U';
		ptr[4] = 'P';
		ptr[5] = ' ';
		ptr[6] = 'B';
		ptr[7] = 'Y';
	}
	if ((ptr = strstr(query, "order by")) != NULL || (ptr = strstr(query, "Order By")) != NULL || (ptr = strstr(query, "Order by")) != NULL || (ptr = strstr(query, "order By")) != NULL)
	{
		*ptr = 'O';
		ptr[1] = 'R';
		ptr[2] = 'D';
		ptr[3] = 'E';
		ptr[4] = 'R';
		ptr[5] = ' ';
		ptr[6] = 'B';
		ptr[7] = 'Y';
	}
	if ((ptr = strstr(query, "from")) != NULL || (ptr = strstr(query, "From")) != NULL)
	{
		*ptr = 'F';
		ptr[1] = 'R';
		ptr[2] = 'O';
		ptr[3] = 'M';
	}
	if ((ptr = strstr(query, "values")) != NULL || (ptr = strstr(query, "Values")) != NULL)
	{
		*ptr = 'V';
		ptr[1] = 'A';
		ptr[2] = 'L';
		ptr[3] = 'U';
		ptr[4] = 'E';
		ptr[5] = 'S';
	}
	if ((ptr = strstr(query, "where")) != NULL || (ptr = strstr(query, "Where")) != NULL)
	{
		*ptr = 'W';
		ptr[1] = 'H';
		ptr[2] = 'E';
		ptr[3] = 'R';
		ptr[4] = 'E';
	}
	if ((ptr = strstr(query, " asc")) != NULL || (ptr = strstr(query, " Asc")) != NULL)
	{
		ptr++;
		*ptr = 'A';
		ptr[1] = 'S';
		ptr[2] = 'C';
	}
	if ((ptr = strstr(query, " desc")) != NULL || (ptr = strstr(query, " Desc")) != NULL)
	{
		ptr++;
		*ptr = 'D';
		ptr[1] = 'E';
		ptr[2] = 'S';
		ptr[3] = 'C';
	}

	//Vado a controllare la sintassi della query
	for (i = 0; query[i] != '\0'; i++)
	{
		if (query[i] == ' '&& query[i + 1] == ' ')
		{
			for (j = (i + 1); query[j] != '\0'; j++)
				query[j] = query[j + 1];
			i--;
		}
		if (query[i] == ',' && query[i + 1] == ' ')
		{
			for (j = (i + 1); query[j] != '\0'; j++)
				query[j] = query[j + 1];
			i--;
		}
		if (query[i] == ' ' && query[i + 1] == ',')
		{
			for (j = i; query[j] != '\0'; j++)
				query[j] = query[j + 1];
			i--;
		}
		if (query[i] == ' ' && query[i + 1] == '\0')
		{
			for (j = i; query[j] != '\0'; j++)
				query[j] = query[j + 1];
			i--;
		}
		if (query[i] == ' ' && query[i + 1] == '\n')
		{
			for (j = i; query[j] != '\0'; j++)
				query[j] = query[j + 1];
			i--;
		}
		if (query[i] == '(' && query[i + 1] == ' ')
		{
			for (j = (i + 1); query[j] != '\0'; j++)
				query[j] = query[j + 1];
			i--;
		}
		if (query[i] == ' ' && query[i + 1] == ')')
		{
			for (j = i; query[j] != '\0'; j++)
				query[j] = query[j + 1];
			i--;
		}
		if (query[i] == ' ' && query[i + 1] == '=')
		{
			for (j = i; query[j] != '\0'; j++)
				query[j] = query[j + 1];
			i--;
		}
		if (query[i] == ' ' && query[i + 1] == '<')
		{
			for (j = i; query[j] != '\0'; j++)
				query[j] = query[j + 1];
			i--;
		}
		if (query[i] == ' ' && query[i + 1] == '>')
		{
			for (j = i; query[j] != '\0'; j++)
				query[j] = query[j + 1];
			i--;
		}
		if (query[i] == '>' && query[i + 1] == ' ')
		{
			for (j = (i + 1); query[j] != '\0'; j++)
				query[j] = query[j + 1];
			i--;
		}
		if (query[i] == '<' && query[i + 1] == ' ')
		{
			for (j = (i + 1); query[j] != '\0'; j++)
				query[j] = query[j + 1];
			i--;
		}
		if (query[i] == '=' && query[i + 1] == ' ')
		{
			for (j = (i + 1); query[j] != '\0'; j++)
				query[j] = query[j + 1];
			i--;
		}
		if (query[i] == '\n')
			query[i] = '\0';

	}

}

FILE* put_query_results(char * query, FILE* results)
{
	// Apro il file query_results
	results = fopen("query_results.txt", "a");
	if (results == NULL)
		return NULL;

	fprintf(results, "SELECT %s;\n", query); //Stampo la query richiesta
	return results;
}

bool table_is_empty(char***table, int num_rows)
{
	int check = true;
	int i;

	for (i = 0; i < num_rows; i++)
		if (table[i] != NULL)
			check = false;

	return check;
}

char *** make_structure(FILE * file, char *** n_colonne, int * num_colonne, int * num_row)
{
	int k;							// Contatore ad uso generico
	char * ptr;						// Puntatore di appoggio
	char prima_riga[255];			// Variabile contenente la row in esame

									//Prelevo la prima riga della tabella contenente la sua intestazione
	fgets(prima_riga, 255, file);

	// Vado a puntare alle colonne del file
	char * colonne_intestazione = strrchr(prima_riga, ' ');
	colonne_intestazione++;

	// Conto il numero di colonne
	int numero_colonne;
	ptr = colonne_intestazione;
	for (numero_colonne = 0; *ptr != ';'; ptr++)
		if (*ptr == ',')
			numero_colonne++;
	numero_colonne++; // Il numero di virgole è inferiore al numero di colonne

					  // Creo un vettore di puntatori a char, di dimensione 'numero_colonne', che puntano ai nomi delle colonne sotto forma di stringa
	char ** nomi_colonne = (char **)malloc(sizeof(char *) * numero_colonne);
	if (nomi_colonne == NULL)
		return NULL; //Controllo errori di allocazione memoria

	for (int i = 0; i < numero_colonne; i++)
	{
		nomi_colonne[i] = (char *)malloc(sizeof(char) * COLUMN_LENGHT);
		if (nomi_colonne[i] == NULL)
		{
			for (int j = 0; j < i; j++)
				free(nomi_colonne[j]);
			free(nomi_colonne);
			return NULL; //Controllo errori di allocazione memoria
		}
	}
	// Inserisco i nomi delle colonne all'interno della struttura
	int n_columns;
	ptr = colonne_intestazione;
	for (k = 0, n_columns = 0; *ptr != ';'; ptr++)
	{
		if (*ptr == ',')	// Cambio colonna
		{
			*(nomi_colonne[n_columns] + k) = '\0';
			n_columns++;
			k = 0;
		}
		else                // Aggiungo carattere al nome 
		{
			*(nomi_colonne[n_columns] + k) = *ptr;
			k++;
		}
	}
	*(nomi_colonne[n_columns] + k) = '\0';

	/*
	Stampa nomi colonne:
	for (i = 0; i<numero_colonne; i++)
	printf("%s\n", nomi_colonne[i]);
	*/

	// Conto il numero di row
	int n_row = 0;
	while (feof(file) != 1)
	{
		n_row++;
		fgets(prima_riga, 255, file);
	}

	// Creo la struttura che dovrà contenere i valori delle row
	char *** valori_colonne = (char ***)malloc(sizeof(char **) * n_row);
	if (valori_colonne == NULL)
	{
		for (int i = 0; i < numero_colonne; i++)
			free(nomi_colonne[i]);
		free(nomi_colonne);
		return NULL; //Controllo errori di allocazione memoria
	}

	for (int i = 0; i < n_row; i++)
	{
		valori_colonne[i] = (char **)malloc(sizeof(char *) * (numero_colonne));
		if (valori_colonne[i] == NULL)
		{
			for (int j = 0; j < i; j++)
				free(valori_colonne[j]);
			for (int j = 0; j < numero_colonne; j++)
				free(nomi_colonne[j]);
			free(nomi_colonne);
			return NULL; //Controllo errori di allocazione memoria
		}
		for (k = 0; k < numero_colonne; k++)
		{
			valori_colonne[i][k] = (char *)malloc(sizeof(char) * COLUMN_LENGHT);
			if (valori_colonne[i][k] == NULL)
			{
				// Dealloco riga corrente
				for (int j = 0; j < k; j++)
					free(valori_colonne[i][j]);
				free(valori_colonne[i]);
				// Dealloco il resto delle righe
				for (int l = 0; l < i - 1; i++)
				{
					for (int j = 0; j < numero_colonne; j++)
						free(valori_colonne[l][j]);
					free(valori_colonne[l]);
				}
				for (int j = 0; j < numero_colonne; j++)
					free(nomi_colonne[j]);
				free(nomi_colonne);
				return NULL; //Controllo errori di allocazione memoria
			}
		}
	}

	// Riporto il puntatore a file alla prima riga
	fseek(file, 0, SEEK_SET);

	// Scarto l'intestazione della tabella
	fgets(prima_riga, 255, file);


	// Inserisco i valori delle ROW all'interno della struttura
	n_row = 0;
	while (feof(file) != 1)
	{
		fgets(prima_riga, 255, file);
		// Vado a puntare alle colonne del file. ptr l'ho già utilizzato prima, ma non mi serviva più una volta aperta la tabella.
		char * ptr = strchr(prima_riga, ' ');
		if (ptr == NULL)
			break;
		ptr = strchr(ptr, ' ');
		if (ptr == NULL)
			break;
		ptr++;



		for (k = 0, n_columns = 0; *ptr != ';'; ptr++)
		{
			if (*ptr == ',')	// Cambio colonna
			{
				*(valori_colonne[n_row][n_columns] + k) = '\0';
				n_columns++;
				k = 0;
			}
			else                // Aggiungo carattere al nome 
			{
				*(valori_colonne[n_row][n_columns] + k) = *ptr;
				k++;
			}
		}
		*(valori_colonne[n_row][n_columns] + k) = '\0';

		n_row++;
	}

	/*
	Stampa delle row
	for(k = 0; k<n_row; k++)
	for (i = 0; i<numero_colonne+1; i++)
	printf("%s\n", valori_colonne[k][i]);
	*/

	// Return
	*num_colonne = numero_colonne;
	*num_row = n_row;
	*n_colonne = nomi_colonne;
	return valori_colonne;
}

void free_structure(char*** table, char ** columns, int num_columns, int num_row)
{
	for (int i = 0; i < num_columns; i++)
		free(columns[i]);

	for (int k = 0; k < num_row; k++)
	{
		if (table[k] != NULL)
		{
			for (int i = 0; i < num_columns; i++)
				free(table[k][i]);
			free(table[k]);
		}
	}
	free(columns);
	free(table);
}

// Merges two subarrays of arr[].
// First subarray is arr[l..m]
// Second subarray is arr[m+1..r]
void merge(char *** arr, int l, int m, int r, int order_colon, int order, int string_or_not)
{
	int i, j, k;
	int n1 = m - l + 1;
	int n2 = r - m;

	/* create temp arrays */
	char *** L, *** R;
	L = (char***)malloc(sizeof(char**)*n1);
	if (L == NULL)
		return;
	R = (char***)malloc(sizeof(char**)*n2);
	if (R == NULL)
	{
		free(L);
		return;
	}

	/* Copy data to temp arrays L[] and R[] */
	for (i = 0; i < n1; i++)
		L[i] = arr[l + i];
	for (j = 0; j < n2; j++)
		R[j] = arr[m + 1 + j];


	/* Merge the temp arrays back into arr[l..r]*/
	i = 0; // Initial index of first subarray
	j = 0; // Initial index of second subarray
	k = l; // Initial index of merged subarray
	if (string_or_not == 1)
	{
		while (i < n1 && j < n2)
		{
			if (order == 1)
			{
				if (memcmp(L[i][order_colon], R[j][order_colon], strlen(L[i][order_colon])) <= 0)
				{
					arr[k] = L[i];
					i++;
				}
				else
				{
					arr[k] = R[j];
					j++;
				}
			}
			else
			{
				if (memcmp(L[i][order_colon], R[j][order_colon], strlen(L[i][order_colon])) > 0)
				{
					arr[k] = L[i];
					i++;
				}
				else
				{
					arr[k] = R[j];
					j++;
				}
			}
			k++;
		}
	}
	else
	{
		while (i < n1 && j < n2)
		{
			if (order == 1)
			{
				if (atoi(L[i][order_colon]) <= atoi(R[j][order_colon]))
				{
					arr[k] = L[i];
					i++;
				}
				else
				{
					arr[k] = R[j];
					j++;
				}
			}
			else
			{
				if (atoi(L[i][order_colon]) > atoi(R[j][order_colon]))
				{
					arr[k] = L[i];
					i++;
				}
				else
				{
					arr[k] = R[j];
					j++;
				}
			}
			k++;
		}
	}

	/* Copy the remaining elements of L[], if there
	are any */
	while (i < n1)
	{
		arr[k] = L[i];
		i++;
		k++;
	}

	/* Copy the remaining elements of R[], if there
	are any */
	while (j < n2)
	{
		arr[k] = R[j];
		j++;
		k++;
	}

	free(L);
	free(R);
}

/* l is for left index and r is right index of the
sub-array of arr to be sorted */
void mergeSort(char *** arr, int l, int r, int order_colon, int order, int string_or_not)
{
	// Se è uguale a due non è ancora stato deciso se ordinare come interi o come stringhe
	if (string_or_not == 2)
	{
		char * tmp_string = (char*)malloc(sizeof(char)*STRING_LEN);
		if (tmp_string == NULL)
			return;

		int value;
		int is_int = true;
		for (int i = 0; i <= r && string_or_not == 2; i++)
		{
			/*
			value = atoi(arr[i][order_colon]);
			_itoa(value, tmp_string, 10);
			if (atoi(arr[i][order_colon]) == 0 || memcmp(arr[i][order_colon], tmp_string, strlen(arr[i][order_colon])) != 0)
			is_int = false;
			*/

			value = atoi(arr[i][order_colon]);
			if (value == 0)
				is_int = false;
		}

		// == 1: ordino per stringhe
		if (is_int == false)
			string_or_not = 1;
		// == -1: ordino per interi
		else
			string_or_not = -1;

		free(tmp_string);
	}


	if (l < r)
	{
		// Same as (l+r)/2, but avoids overflow for
		// large l and h
		int m = l + (r - l) / 2;

		// Sort first and second halves
		mergeSort(arr, l, m, order_colon, order, string_or_not);
		mergeSort(arr, m + 1, r, order_colon, order, string_or_not);

		merge(arr, l, m, r, order_colon, order, string_or_not);
	}
}