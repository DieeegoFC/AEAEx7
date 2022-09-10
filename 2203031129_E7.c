// @author: Diego Flores Chávez | 2203031129.

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>

#define DEPARTMENT_REFERENCE 3
#define DATE_REFERENCE 4
#define STRING_SIZE 51
#define CODE_SIZE 12
#define CREATE_FILES_CASE 1
#define ADD_REGISTER_CASE 2
#define DELETE_REGISTER_CASE 3
#define UPDATE_REGISTER_CASE 4
#define ECONOMIC_NUMBER_CASE 1
#define NAME_CASE 2
#define DEPARTMENT_CASE 3
#define DATE_CASE 4
#define BULLET 254
#define FALSE 0
#define TRUE 1
#define EXIT_CODE 5

struct Register {
	char economic_number[STRING_SIZE];
	char name[STRING_SIZE];
	char department[STRING_SIZE];
	char date[STRING_SIZE];
};

struct Index {
	char code[CODE_SIZE];
	int index;
};

struct Register global_register;
struct Index global_index;

void DeleteRegister(void);
void UpdateRegister(void);
void AddRegister(void);
void CreateFiles(void);
void UpdateIndex(FILE *index, FILE* out, char* code);
void ExtractRegistrationCode(void);
void CleanGlobalRegister(void);
void UpperCase(char* str);
void ReadRegister(void);
void Trim(char* str);
void RunSystem(void);

int main(int argc, char** argv) {
	RunSystem();
	return EXIT_SUCCESS;
}

void DeleteRegister(void) {
	fprintf(stdout, "\n----------> DELETE REGISTER <----------\n\n");

	FILE *index = fopen("indices.txt", "r+"), *data = fopen("datos.txt", "r+");

	if (index == NULL || data == NULL) {
		fprintf(stderr, "\nERROR: %s.\n\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	char target[CODE_SIZE];

	fprintf(stdout, "Type the index of the register you want to delete: ");
	while (getchar() != '\n');
	fscanf(stdin, "%11[^\n]", target);

	FILE *out = fopen("out.txt", "w");
	int register_exists = FALSE;
	char line[STRING_SIZE];

	while (fgets(line, STRING_SIZE, index)) {
		Trim(line);

		char *token = strtok(line, " ");
		int offset = atoi(strtok(NULL, " "));

		if (!strcmp(target, token)) {
			fseek(data, offset, SEEK_SET);
			fread(&global_register, sizeof(struct Register), 1, data);

			fprintf(
				stdout                          ,
				"\n\t%c Economic number: %s.\n"
				"\t%c Name: %s.\n"
				"\t%c Department: %s.\n"
				"\t%c Date: %s.\n\n"            ,
				BULLET,
				global_register.economic_number ,
				BULLET                          ,
				global_register.name            ,
				BULLET                          ,
				global_register.department      ,
				BULLET                          ,
				global_register.date
			);

			char control;

			fprintf(stdout, "Are you sure? [Y]Yes/[N]No: ");
			while (getchar() != '\n');
			fscanf(stdin, "%c", &control);
			if (control == 'n' || control == 'N') {
				fclose(index);
				fclose(data);
				fclose(out);

				remove("out.txt");

				fprintf(stdout, "\nCancelled.\n\n");

				return;
			}

			register_exists = TRUE;

			CleanGlobalRegister();
			fseek(data, offset, SEEK_SET);
			fwrite(&global_register, sizeof(struct Register), 1, data);
		}
		else
			fprintf(out, "%s %d\n", line, offset);
	}

	if (register_exists)
		fprintf(stdout, "\nSuccessful elimination!\n\n");
	else
		fprintf(stdout, "\nRegistration not found.\n\n");

	fclose(index);
	fclose(data);
	fclose(out);

	remove("indices.txt");
	rename("out.txt", "indices.txt");
}

void UpdateRegister(void) {
	fprintf(stdout, "\n----------> UPDATE REGISTER <----------\n\n");

	FILE *index = fopen("indices.txt", "r+"), *data = fopen("datos.txt", "r+");

	if (index == NULL || data == NULL) {
		fprintf(stderr, "\nERROR: %s.\n\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	char target[CODE_SIZE];

	fprintf(stdout, "Type the index of the register you want to update: ");
	while (getchar() != '\n');
	fscanf(stdin, "%11[^\n]", target);

	int register_exists = FALSE, offset;
	char line[STRING_SIZE];

	while (fgets(line, STRING_SIZE, index)) {
		Trim(line);

		char *token = strtok(line, " ");

		if (!strcmp(target, token)) {
			offset = atoi(strtok(NULL, " "));

			fseek(data, offset, SEEK_SET);
			fread(&global_register, sizeof(struct Register), 1, data);

			register_exists = TRUE;
		}
	}

	if (!register_exists) {
		fprintf(stdout, "\nRegistration not found.\n\n");
		fclose(index);
		fclose(data);

		return;
	}

	int control;

	fprintf(
		stdout                                              ,
		"\nFOUND REGISTER: \n"
			"\t1.- Economic number: %s.\n"
			"\t2.- Name: %s.\n"
			"\t3.- Department: %s.\n"
			"\t4.- Date: %s.\n"
		"Type the number of the field you want to update: " ,
		global_register.economic_number                     ,
		global_register.name                                ,
		global_register.department                          ,
		global_register.date
	);
	while (getchar() != '\n');
	fscanf(stdin, "%d", &control);

	FILE *out = fopen("out.txt", "w");
	
	switch (control) {

		case ECONOMIC_NUMBER_CASE:
			ExtractRegistrationCode();
			UpdateIndex(index, out, global_index.code);

			fprintf(stdout, "\nNEW ECONOMIC NUMBER: ");
			while (getchar() != '\n');
			fscanf(stdin, "%4[^\n]", global_register.economic_number);

			fseek(data, offset, SEEK_SET);
			fwrite(&global_register, sizeof(struct Register), 1, data);
			fclose(data);

			ExtractRegistrationCode();
			global_index.index = offset;

			fprintf(stdout, "\nNEW CODE: %s | OFFSET: %d\n\n", global_index.code, global_index.index);

			fprintf(out, "%s %d\n", global_index.code, global_index.index);
			fclose(index);
			fclose(out);
			remove("indices.txt");
			rename("out.txt", "indices.txt");

			break;

		case NAME_CASE:
			fprintf(stdout, "\nNEW NAME: ");
			while (getchar() != '\n');
			fscanf(stdin, "%25[^\n]", global_register.name);
			UpperCase(global_register.name);

			fseek(data, offset, SEEK_SET);
			fwrite(&global_register, sizeof(struct Register), 1, data);

			ExtractRegistrationCode();
			global_index.index = offset;

			fprintf(stdout, "\nNEW CODE: %s | OFFSET: %d\n\n", global_index.code, global_index.index);

			fclose(index);
			fclose(data);
			fclose(out);
			remove("out.txt");

			break;

		case DEPARTMENT_CASE:
			ExtractRegistrationCode();
			UpdateIndex(index, out, global_index.code);

			fprintf(stdout, "\nNEW DEPARTMENT: ");
			while (getchar() != '\n');
			fscanf(stdin, "%10[^\n]", global_register.department);
			UpperCase(global_register.department);

			fseek(data, offset, SEEK_SET);
			fwrite(&global_register, sizeof(struct Register), 1, data);
			fclose(data);

			ExtractRegistrationCode();
			global_index.index = offset;

			fprintf(stdout, "\nNEW CODE: %s | OFFSET: %d\n\n", global_index.code, global_index.index);

			fprintf(out, "%s %d\n", global_index.code, global_index.index);
			fclose(index);
			fclose(out);
			remove("indices.txt");
			rename("out.txt", "indices.txt");

			break;
		
		case DATE_CASE:
			ExtractRegistrationCode();
			UpdateIndex(index, out, global_index.code);

			fprintf(stdout, "\nNEW DATE: ");
			while (getchar() != '\n');
			fscanf(stdin, "%20[^\n]", global_register.date);
			UpperCase(global_register.date);

			fseek(data, offset, SEEK_SET);
			fwrite(&global_register, sizeof(struct Register), 1, data);
			fclose(data);

			ExtractRegistrationCode();
			global_index.index = offset;

			fprintf(stdout, "\nNEW CODE: %s | OFFSET: %d\n\n", global_index.code, global_index.index);
			fprintf(out, "%s %d\n", global_index.code, global_index.index);
			fclose(index);
			fclose(out);
			remove("indices.txt");
			rename("out.txt", "indices.txt");

			break;

		default:
			fprintf(stdout, "\nWARNING: Invalid field.\n\n");
			fclose(index);
			fclose(data);
			fclose(out);
			remove("out.txt");
			
			return;

	}
}

void AddRegister(void) {
	fprintf(stdout, "\n----------> ADD REGISTER <----------\n\n");
	
	FILE *index = fopen("indices.txt", "a"), *data = fopen("datos.txt", "a");
	
	CleanGlobalRegister();
	ReadRegister();
	
	fseek(data, 0L, SEEK_END);
	global_index.index = (int)ftell(data);
	
	fwrite(&global_register, sizeof(struct Register), 1, data);
	fclose(data);
	
	ExtractRegistrationCode();
	
	fprintf(index, "%s %d\n", global_index.code, global_index.index);
	fclose(index);
	
	fprintf(stdout, "\nADDED CODE: %s | OFFSET: %d\n\n", global_index.code, global_index.index);
}

void CreateFiles(void) {
	fprintf(stdout, "\n----------> CREATE FILES <----------\n\n");
	
	FILE *a = fopen("indices.txt", "w"), *b = fopen("datos.txt", "w");
	
	fclose(a);
	fclose(b);
	
	fprintf(stdout, "Successful creation!\n\n");
}

void UpdateIndex(FILE *index, FILE* out, char* code) {
	char line[STRING_SIZE];

	*(code + (strlen(code) - 2)) = '\0';

	rewind(index);
	while (fgets(line, STRING_SIZE, index)) {
		Trim(line);

		char *token = strtok(line, " ");
		int offset = atoi(strtok(NULL, " "));

		if (strcmp(code, token) != 0)
			fprintf(out, "%s %d\n", token, offset);
	}
}

void ExtractRegistrationCode(void) {
	int size = (int)strlen(global_register.date);
	char date_token[STRING_SIZE];
	
	global_register.department[DEPARTMENT_REFERENCE] = '\0';
	strcpy(global_index.code, global_register.economic_number);
	strcat(global_index.code, global_register.department);
	strncpy(date_token, global_register.date + (size - DATE_REFERENCE), DATE_REFERENCE);
	strcat(global_index.code, date_token);
}

void CleanGlobalRegister(void) {
	memset(global_register.economic_number , 0 , STRING_SIZE - 1);
	memset(global_register.name            , 0 , STRING_SIZE - 1);
	memset(global_register.department      , 0 , STRING_SIZE - 1);
	memset(global_register.date            , 0 , STRING_SIZE - 1);
}

void UpperCase(char* str) {
	int position = 0;

	while (*(str + position) != '\0')
		*(str + position++) = toupper(*(str + position));
}

void ReadRegister(void) {
	fprintf(stdout, "\t%c Economic number: ", BULLET);
	while (getchar() != '\n');
	fscanf(stdin, "%4[^\n]", global_register.economic_number);
	
	fprintf(stdout, "\t%c Name: ", BULLET);
	while (getchar() != '\n');
	fscanf(stdin, "%25[^\n]", global_register.name);
	UpperCase(global_register.name);
	
	fprintf(stdout, "\t%c Department: ", BULLET);
	while (getchar() != '\n');
	fscanf(stdin, "%10[^\n]", global_register.department);
	UpperCase(global_register.department);
	
	fprintf(stdout, "\t%c Date: ", BULLET);
	while (getchar() != '\n');
	fscanf(stdin, "%20[^\n]", global_register.date);
	UpperCase(global_register.date);
}

void Trim(char* str) {
	int position = 0;

	while (*(str + position++) != '\n');
	*(str + --position) = '\0';
}

void RunSystem(void) {
	int control;
	
	fprintf(stdout, "\n==========[ FILE SYSTEM ]==========\n\n");
	do {
		fprintf(
			stdout,
			"MAIN MENU:\n"
				"\t1.- Create files.\n"
				"\t2.- Add register.\n"
				"\t3.- Delete register.\n"
				"\t4.- Update register.\n"
				"\t5.- Exit.\n"
			"Type the number of the option you want to choose: "
		);
		fscanf(stdin, "%d", &control);
		
		switch (control) {
			
			case CREATE_FILES_CASE:
				CreateFiles();
				break;
				
			case ADD_REGISTER_CASE:
				AddRegister();
				break;
				
			case DELETE_REGISTER_CASE:
				DeleteRegister();
				break;

			case UPDATE_REGISTER_CASE:
				UpdateRegister();
				break;

			case EXIT_CODE:
				fprintf(stdout, "\nSee you soon!\n\n");
				break;
				
			default:
				fprintf(stdout, "\nWARNING: Nonexistent option.\n\n");
				break;
			
		}
	} while (control != EXIT_CODE);
}