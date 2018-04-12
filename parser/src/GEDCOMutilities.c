#include <stdio.h>
#include <ctype.h>
#include <strings.h>

#include "GEDCOMutilities.h" 
#include "GEDCOMparser.h"
#include "LinkedListAPI.h"


GEDCOMerror parserDistributor (GEDCOMobject * obj, container allInformation, List * sendList, List * recieveList) {
	tagList * tagInformation = allInformation.t;
	int * i = malloc(sizeof(int));
	
	for (*i = 0; *i < allInformation.size;) {
		if (strcmp(tagInformation[*i].tag, "HEAD") == 0) {
			(*i)++;
			headParser(obj, tagInformation, i, sendList, recieveList);
		}
		else if (strcmp(tagInformation[*i].tag, "SUBM") == 0) {
			insertBack(recieveList, createAddress(NULL, &(obj->submitter), NULL , tagInformation[*i].recieverAddress, NULL, (*i) + 1));
			(*i)++;
			submitterParser(obj, tagInformation, i, sendList, recieveList);
			//call submitter parser
		}
		else if (strcmp(tagInformation[*i].tag, "INDI") == 0) {
			insertBack(&(obj->individuals), initializeIndividual());
			insertBack(recieveList, createAddress(NULL, NULL, NULL, tagInformation[*i].recieverAddress, getFromBack(obj->individuals), (*i) + 1));
			(*i)++;
			individualParser(obj, tagInformation, i, sendList, recieveList);
			//call individual parser
		}
		else if (strcmp(tagInformation[*i].tag, "FAM") == 0) {
			insertBack(&(obj->families), initializeFamily());
			insertBack(recieveList, createAddress(NULL, NULL, NULL, tagInformation[*i].recieverAddress, getFromBack(obj->families), (*i) + 1));
			(*i)++;
			familyParser(obj, tagInformation, i, sendList, recieveList);
			//call Family parser
		}
		else {
			(*i)++;
		}
	}
	
	free(i);
	return createError(OK, -1);
}

GEDCOMerror headParser (GEDCOMobject * obj, tagList * tagInformation, int * currentLocation, List * sendList, List * recieveList) {
	bool gedcVersion = false;
	Header * h = obj->header;
		
	while (strcmp(tagInformation[*currentLocation].level, "0") != 0) {
		
		if (strcmp(tagInformation[*currentLocation].tag, "SOUR") == 0) strcpy(h->source, tagInformation[*currentLocation].value);
		
		else if (strcmp(tagInformation[*currentLocation].tag, "GEDC") == 0) gedcVersion = true;
		
		else if (strcmp(tagInformation[*currentLocation].tag, "CHAR") == 0) {
			
			if (strcmp(tagInformation[*currentLocation].value, "ANSEL") == 0) {
				h->encoding = ANSEL;
			}
			else if (strcmp(tagInformation[*currentLocation].value, "UTF-8") == 0) {
				h->encoding = UTF8;
			}
			else if (strcmp(tagInformation[*currentLocation].value, "UNICODE") == 0) {
				h->encoding = UNICODE;
			}
			else if (strcmp(tagInformation[*currentLocation].value, "ASCII") == 0) {
				h->encoding = ASCII;
			}
			
		}
		else if (gedcVersion == true) {
			if (strcmp(tagInformation[*currentLocation].tag, "VERS") == 0) {
				h->gedcVersion = (atof(tagInformation[*currentLocation].value));
			}
			gedcVersion = false;
		}
		else if (strcmp(tagInformation[*currentLocation].tag, "SUBM") == 0) {
			
			insertBack(sendList, createAddress(NULL, &(h->submitter), NULL, tagInformation[*currentLocation].senderAddress, NULL, (*currentLocation) + 1));
			//ADD Linker afterwards!
		}
		else {
			//Otherfields
			Field * f = initializeField();
			f->tag = calloc(strlen(tagInformation[*currentLocation].tag) + 1, sizeof(char));
			f->value = calloc(strlen(tagInformation[*currentLocation].value) + 1, sizeof(char));
			
			strcpy(f->tag, tagInformation[*currentLocation].tag);
			strcpy(f->value, tagInformation[*currentLocation].value);
			
			insertBack(&(h->otherFields), f);
		}
		
		(*currentLocation)++;
	}
	
	return createError(OK, -1);
}

GEDCOMerror submitterParser (GEDCOMobject * obj, tagList * tagInformation, int * currentLocation, List * sendList, List * recieveList) {
	
	Submitter * s = obj->submitter;
	
	while (strcmp(tagInformation[*currentLocation].level, "0") != 0) {
		if (strcmp(tagInformation[*currentLocation].tag, "NAME") == 0) strcpy(s->submitterName, tagInformation[*currentLocation].value);
		
		else if (strcmp(tagInformation[*currentLocation].tag, "ADDR") == 0) strcpy(s->address, tagInformation[*currentLocation].value);
		
		else {
			Field * f = initializeField();
			f->tag = calloc(strlen(tagInformation[*currentLocation].tag) + 1, sizeof(char));
			f->value = calloc(strlen(tagInformation[*currentLocation].value) + 1, sizeof(char));
			
			strcpy(f->tag, tagInformation[*currentLocation].tag);
			strcpy(f->value, tagInformation[*currentLocation].value);
			
			insertBack(&(s->otherFields), f);
		}
		
		(*currentLocation)++;
	}
	
	return createError(OK, -1);
}

GEDCOMerror individualParser (GEDCOMobject * obj, tagList * tagInformation, int * currentLocation, List * sendList, List * recieveList) {
	
	Individual * i = getFromBack(obj->individuals);
	
	bool isEvent = false;
	
	while (strcmp(tagInformation[*currentLocation].level, "0") != 0) {
		
		if (strcmp(tagInformation[*currentLocation].level, "1") == 0) {
			//checks if Level 1 is an event!
			isEvent = false;
		}
		
		if (isEvent == true) {
			
			Event * e = getFromBack(i->events);
			
			if (strcmp(tagInformation[*currentLocation].tag, "DATE") == 0) {
				e->date = calloc(strlen(tagInformation[*currentLocation].value) + 1, sizeof(char));
				strcpy(e->date, tagInformation[*currentLocation].value);
			}
			else if (strcmp(tagInformation[*currentLocation].tag, "PLAC") == 0) {
				e->place = calloc(strlen(tagInformation[*currentLocation].value) + 1, sizeof(char));
				strcpy(e->place, tagInformation[*currentLocation].value);
			}
			else {
				Field * f = initializeField();
				f->tag = calloc(strlen(tagInformation[*currentLocation].tag) + 1, sizeof(char));
				f->value = calloc(strlen(tagInformation[*currentLocation].value) + 1, sizeof(char));
			
				strcpy(f->tag, tagInformation[*currentLocation].tag);
				strcpy(f->value, tagInformation[*currentLocation].value);
			
				insertBack(&(e->otherFields), f);
			}
		}
		else if (strcmp(tagInformation[*currentLocation].tag, "NAME") == 0) {
			char * givenName = NULL;
			char * surName = NULL;
			
			stringInformation fullName = stringTokenizer(tagInformation[*currentLocation].value);
			
			if (fullName.size == 2 && strcmp(i->givenName, "") == 0 && strcmp(i->surname, "") == 0) {
				free(i->givenName);
				free(i->surname);
				givenName = fullName.tokenizedString[0];
				surName = fullName.tokenizedString[1];
				
				free(fullName.tokenizedString);
				
				i->givenName = givenName;
				i->surname = surName;
				
			}
			else if (fullName.size == 1 && strcmp(i->givenName, "") == 0) {
				//TEST
				if (tagInformation[*currentLocation].location > 1) {
					free(i->givenName);
					givenName = fullName.tokenizedString[0];
					i->givenName = givenName;
					free(fullName.tokenizedString);
				}
				else {
					free(i->surname);
					surName = fullName.tokenizedString[0];
					i->surname = surName;
					free(fullName.tokenizedString);
				}
			}
			else if (fullName.size > 2) {
				free(i->givenName);
				i->givenName = calloc(strlen(tagInformation[*currentLocation].value) + 10, sizeof(char));
				strcat(i->givenName, fullName.tokenizedString[0]);
				
				for (int x = 1; x < (tagInformation[*currentLocation].location - 1); x++) {
					strcat(i->givenName, " ");
					strcat(i->givenName, fullName.tokenizedString[x]);
				}
				
				free(i->surname);
				i->surname = calloc(strlen(tagInformation[*currentLocation].value) + 10, sizeof(char));
				strcat(i->surname, fullName.tokenizedString[tagInformation[*currentLocation].location - 1]);
				
				for (int x = (tagInformation[*currentLocation].location); x < fullName.size; x++) {
					strcat(i->surname, " ");
					strcat(i->surname, fullName.tokenizedString[x]);
				}
				
				for (int i = 0; i < fullName.size; i++) {
					free(fullName.tokenizedString[i]);
				}
				free(fullName.tokenizedString);
				
				//let GIVN and SURN handle it
			}
			
		}
		else if (strcmp(tagInformation[*currentLocation].tag, "GIVN") == 0) {
			free(i->givenName);
			i->givenName = calloc(strlen(tagInformation[*currentLocation].value) + 1, sizeof(char));
			strcpy(i->givenName, tagInformation[*currentLocation].value);
		}
		else if (strcmp(tagInformation[*currentLocation].tag, "SURN") == 0) {
			free(i->surname);
			i->surname = calloc(strlen(tagInformation[*currentLocation].value) + 1, sizeof(char));
			strcpy(i->surname, tagInformation[*currentLocation].value);
		}
		else if (strcmp(tagInformation[*currentLocation].tag, "FAMC") == 0 || strcmp(tagInformation[*currentLocation].tag, "FAMS") == 0) {
			insertBack(sendList, createAddress((&(i->families)), NULL, NULL, tagInformation[*currentLocation].senderAddress, NULL, (*currentLocation) + 1));
		}
		else if (checkIndividualEvent(tagInformation[*currentLocation].tag) && isEvent == false) {
			Event * e = initializeEvent();
			strcpy(e->type, tagInformation[*currentLocation].tag);
			
			insertBack(&(i->events), e);
			isEvent = true;
			//Continue from inserting event!
		}
		else {
			Field * f = initializeField();
			f->tag = calloc(strlen(tagInformation[*currentLocation].tag) + 1, sizeof(char));
			f->value = calloc(strlen(tagInformation[*currentLocation].value) + 1, sizeof(char));
			
			strcpy(f->tag, tagInformation[*currentLocation].tag);
			strcpy(f->value, tagInformation[*currentLocation].value);
			
			insertBack(&(i->otherFields), f);
		}
		(*currentLocation)++;
	}
	
	return createError(OK, -1);
}

GEDCOMerror familyParser (GEDCOMobject * obj, tagList * tagInformation, int * currentLocation, List * sendList, List * recieveList) {
	
	Family * f = getFromBack(obj->families);
	
	bool isEvent = false;
	
	while (strcmp(tagInformation[*currentLocation].level, "0") != 0) {
		
		if (strcmp(tagInformation[*currentLocation].level, "1") == 0) {
			//checks if Level 1 is an event!
			isEvent = false;
		}
		
		if (checkFamilyEvent(tagInformation[*currentLocation].tag) && isEvent == false) {
			Event * e = initializeEvent();
			strcpy(e->type, tagInformation[*currentLocation].tag);
			
			insertBack(&(f->events), e);
			isEvent = true;
			//Continue from inserting event!
		}
		else if (isEvent == true) {
			
			Event * e = getFromBack(f->events);
			
			if (strcmp(tagInformation[*currentLocation].tag, "DATE") == 0) {
				e->date = calloc(strlen(tagInformation[*currentLocation].value) + 1, sizeof(char));
				strcpy(e->date, tagInformation[*currentLocation].value);
			}
			else if (strcmp(tagInformation[*currentLocation].tag, "PLAC") == 0) {
				e->place = calloc(strlen(tagInformation[*currentLocation].value) + 1, sizeof(char));
				strcpy(e->place, tagInformation[*currentLocation].value);
			}
			else {
				Field * f = initializeField();
				f->tag = calloc(strlen(tagInformation[*currentLocation].tag) + 1, sizeof(char));
				f->value = calloc(strlen(tagInformation[*currentLocation].value) + 1, sizeof(char));
			
				strcpy(f->tag, tagInformation[*currentLocation].tag);
				strcpy(f->value, tagInformation[*currentLocation].value);
			
				insertBack(&(e->otherFields), f);
			}
		}
		else if (strcmp(tagInformation[*currentLocation].tag, "CHIL") == 0) {
			insertBack(sendList, createAddress(&(f->children), NULL, NULL, tagInformation[*currentLocation].senderAddress, NULL, (*currentLocation) + 1));
		}
		else if (strcmp(tagInformation[*currentLocation].tag, "HUSB") == 0) {
			insertBack(sendList, createAddress(NULL, NULL, &(f->husband), tagInformation[*currentLocation].senderAddress, NULL, (*currentLocation) + 1));
		}
		else if (strcmp(tagInformation[*currentLocation].tag, "WIFE") == 0) {
			insertBack(sendList, createAddress(NULL, NULL, &(f->wife), tagInformation[*currentLocation].senderAddress, NULL, (*currentLocation) + 1));
		}
		else {
			Field * t = initializeField();
			t->tag = calloc(strlen(tagInformation[*currentLocation].tag) + 1, sizeof(char));
			t->value = calloc(strlen(tagInformation[*currentLocation].value) + 1, sizeof(char));
			
			strcpy(t->tag, tagInformation[*currentLocation].tag);
			strcpy(t->value, tagInformation[*currentLocation].value);
			
			insertBack(&(f->otherFields), t);
		}
		
		(*currentLocation)++;
	}
	
	return createError(OK, -1);
}

GEDCOMobject ** initObject (GEDCOMobject ** obj) {	
	//creates object
	(*obj) = calloc(1, sizeof(GEDCOMobject));
	//initializes List
	(*obj)->individuals = initializeList(printIndividual, deleteIndividual, compareIndividuals);
	(*obj)->families = initializeList(printFamily, deleteFamily, compareFamilies);
	
	(*obj)->header = initializeHeader();
	(*obj)->submitter = initializeSubmitter();

	return obj;
}

Event * initializeEvent () {
	Event * e = calloc(1, sizeof(Event));
	e->date = NULL;
	e->place = NULL;

	e->otherFields = initializeList(printField, deleteField, compareFields);
	
	return e;
}

Header * initializeHeader () {
	Header * h = calloc(1, sizeof(Header));
	h->gedcVersion = -1;
	strcpy(h->source, "");
	h->otherFields = initializeList(printField, deleteField, compareFields);
	
	return h;
}

Submitter * initializeSubmitter () {
	Submitter * s = calloc(1, sizeof(Submitter) + 1024);
	strcpy(s->submitterName, "");
	s->otherFields = initializeList(printField, deleteField, compareFields);
	
	return s;
}

Individual * initializeIndividual () {
	Individual * i = calloc(1, sizeof(Individual));
	i->givenName = calloc(1, sizeof(" "));
	i->surname = calloc(1, sizeof(" "));
	
	strcpy(i->givenName, "");
	strcpy(i->surname, "");
	
	i->events = initializeList(printEvent, deleteEvent, compareEvents);
	i->otherFields = initializeList(printField, deleteField, compareFields);
	i->families = initializeList (printFamily, deleteFamily, compareFamilies);
	
	return i;
}

Family * initializeFamily () {
	Family * f = calloc(1, sizeof(Family));
	
	f->husband = NULL;
	f->wife = NULL;
	
	f->events = initializeList(printEvent, deleteEvent, compareEvents);
	f->children = initializeList(printIndividual, deleteIndividual, compareIndividuals);
	f->otherFields = initializeList(printField, deleteField, compareFields);
	
	return f;
}

Field * initializeField () {
	Field * f = malloc(sizeof(Field));
	f->tag = NULL;
	f->value = NULL;
	
	return f;
}

bool fileExist (const char * fileName) {
	
	FILE * f = NULL;
	if ((f = fopen(fileName, "r"))) {
		fclose(f);
		return true;
	}
	
	return false;
}

GEDCOMerror createError (ErrorCode type, int line) {
	GEDCOMerror errorContainer;
	errorContainer.type = type;
	errorContainer.line = line;
	
	return errorContainer;
}

arrayList initializeArray(FILE * inFile) {
	arrayList a;
	char temp[1024];
	int size = 0;
	
	if (inFile == NULL) {
		return a;
	}
	
	a.allLines = calloc(1, sizeof(char*));
	
	while (myfgets(temp, 257, inFile) != NULL) {
		a.allLines[size] = calloc(strlen(temp) + 1, sizeof(char));
		strcpy(a.allLines[size], temp);
		size++;
		a.allLines = realloc(a.allLines, sizeof(char*) * (size + 1));
	}
		
	a.size = size;
	
	return a;
}

tagList * initializeTags(arrayList a, int * arraySize) {
	
	if (a.allLines == NULL) {
		return NULL;
	}
	
	tagList * t = calloc(a.size, sizeof(tagList));
	char temp[1024];
	char ** array = a.allLines;
	bool isAddress = false;
	
	char * level = NULL;
	char * tag = NULL;
	char * value = NULL;
	char * address = NULL;
	char terminator[2];
	
	for (int i = 0, counter = 0; counter < a.size ; counter++) {
		isAddress = false;
		level = NULL;
		tag = NULL;
		value = NULL;
		
		strcpy(temp, array[counter]);
		t[i].length = strlen(temp);
		if(temp[0] == ' '|| temp[0] == '\n' || temp[0] == '\t') {
			t[i].format = 1;
		}
		else {
			t[i].format = 0;
		}
		
		if (temp[strlen(temp) - 1] == '\r' && temp[strlen(temp)] == '\n') {
			strcpy(terminator, "\r\n");
		}
		else if (temp[strlen(temp) - 1] == '\n') {
			strcpy(terminator, "\n");
		}
		else if (temp[strlen(temp) - 1] == '\r') {
			strcpy(terminator, "\r");
		}
		
		temp[strcspn(temp, "\r\n")] = '\0';
		
		
		t[i].location = -1;
		
		for (int x = 0; x < strlen(temp); x++) {
			if (temp[x] == ' ' && temp[x + 1] != ' ') {
				t[i].location++;
			}
			else if (temp[x] == '/') {
				break;
			}
		}
		
		stringInformation lines = stringTokenizer(temp);
		char ** tokenizedString = lines.tokenizedString;
		
		if (lines.size > 2 && (strcmp(tokenizedString[1], "CONT") == 0 || strcmp(tokenizedString[1], "CONC") == 0)) {
			
			if (tokenizedString[2] != NULL && strcmp(tokenizedString[1], "CONC") == 0) {
				if (i-1 != 0 && t[i-1].value != NULL) {
					
					t[i-1].value = realloc(t[i-1].value, (strlen(t[i-1].value) + 1024) * sizeof(char));
					char * combined = combineString(tokenizedString, 3, lines.size);
					strcat(t[i-1].value, combined);
					free(combined);
				}
				else if (i-1 != 0){
					t[i-1].value = combineString(tokenizedString, 3, lines.size);
				}
			}
			else if (tokenizedString[2] != NULL && strcmp(tokenizedString[1], "CONT") == 0) {
				if (i-1 != 0 && t[i-1].value != NULL) {
					
					t[i-1].value = realloc(t[i-1].value, (strlen(t[i-1].value) + 1024) * sizeof(char));
					char * combined = combineString(tokenizedString, 3, lines.size);
					strcat(t[i-1].value, terminator);
					strcat(t[i-1].value, combined);
					free(combined);
				}
				else if (i-1 != 0){
					t[i-1].value = combineString(tokenizedString, 3, lines.size);
				}
			}
			
			
		}
		else {
			
			if (tokenizedString[0] != NULL) {
				level = calloc(2, strlen(tokenizedString[0]) * sizeof(char));
				strcpy(level, tokenizedString[0]);
				t[i].level = level;
			}
			if (tokenizedString[1] != NULL) {
				if (tokenizedString[1][0] != '@') {
					tag = calloc(2, strlen(tokenizedString[1]) * sizeof(char));
					strcpy(tag, tokenizedString[1]);
					t[i].tag = tag;
				}
				else {
					address = calloc(2, strlen(tokenizedString[1]) * sizeof(char));
					strcpy(address, tokenizedString[1]);
					t[i].recieverAddress = address;
					isAddress = true;
				}
			}
			if (tokenizedString[2] != NULL) {
				if (tokenizedString[2][0] == '@') {
					address = calloc(2, strlen(tokenizedString[2]) * sizeof(char));
					strcpy(address, tokenizedString[2]);
					t[i].senderAddress = address;
					if (tokenizedString[3] != NULL) {
						value = combineString(tokenizedString, 4, lines.size);
					t[i].value = value;
					}
				}
				else if (isAddress == true) {
					tag = calloc(2, strlen(tokenizedString[2]) * sizeof(char));
					strcpy(tag, tokenizedString[2]);
					t[i].tag = tag;
					if (tokenizedString[3] != NULL) {
						value = combineString(tokenizedString, 4, lines.size);
						t[i].value = value;
					}
				}
				else {
					value = combineString(tokenizedString, 3, lines.size);
					t[i].value = value;
				}
			}		
			i++;
			*arraySize = i;
		}	//for added if
	
	freeStringArray(tokenizedString, lines.size);
	}
	
	
	
	for (int i = 0; i < a.size; i++) {
		free(array[i]);
	}
	free(array);
	
	return t;
}

GEDCOMerror checkForError(tagList * arr, int size) {
	
	//All header requirements!
	bool headerCheck = true;
	bool submitterCheck = false;
	bool sourceCheck = false;
	bool charCheck = false;
	bool versCheck = false;
	bool gedcCheck = false;
	bool mainSubmitter = false;
	
	int endOfHeader = 0;
	
	int tempLevel = 0;
	int currentLevel = 0;
	
	
	if (arr == NULL) {
		return createError(INV_GEDCOM, -1);
	}
	
	if (strcmp(arr[0].tag, "HEAD") != 0) {
		return createError(INV_GEDCOM, -1);
	}
	
	for (int i = 0; i < size; i ++) {
					
		if (arr[i].length > 255) {
			//add check if error is in header or not!
			return createError(INV_RECORD, i + 1);
		}
		
		if (arr[i].format == 1) {
			return createError(INV_RECORD, i + 1);
		}
		
		if (arr[i].tag == NULL) {
			return createError(INV_RECORD, i + 1);
		}
		
		if (arr[i].level == NULL) {
			return createError(INV_RECORD, i + 1);
		}
		
		if (strlen(arr[i].tag) > 4) {
			return createError(INV_RECORD, i + 1);
		}
		
		if (arr[i].senderAddress != NULL && strlen(arr[i].senderAddress) > 22) {
			return createError(INV_RECORD, i + 1);
		}
		if (arr[i].recieverAddress != NULL && strlen(arr[i].recieverAddress) > 22) {
			return createError(INV_RECORD, i + 1);
		}
		
		for (int x = 0; x < strlen(arr[i].tag); x++) {
			if (!isupper(arr[i].tag[x])) {
				return createError(INV_RECORD, i + 1);
			}
		}
		
		for (int x = 0; x < strlen(arr[i].level); x++) {
			if (isalpha(arr[i].level[x])) {
				return createError(INV_RECORD, i + 1);
			}
		}
		
		if (strtol(arr[i].level, NULL, 10) < 0 && strtol(arr[i].level, NULL, 10) > 99) {
			return createError(INV_RECORD, i + 1);
		}
		
		if ((strcmp(arr[i].tag, "INDI") == 0	|| strcmp(arr[i].tag, "FAM") == 0 || strcmp(arr[i].tag, "TRLR") == 0) && strtol(arr[i].level, NULL, 10) != 0) {
			return createError(INV_RECORD, i + 1);
		}
		
		if ((strcmp(arr[i].tag, "HEAD") == 0) && strtol(arr[i].level, NULL, 10) != 0) {
			return createError(INV_HEADER, i + 1);
		}
		
		if (strcmp(arr[i].tag, "HEAD") != 0 && strtol(arr[i].level, NULL, 10) == 0 && headerCheck == true) {
			headerCheck = false;
			endOfHeader = i;
		}
		
		if (headerCheck == true) {
			if (strcmp(arr[i].tag, "GEDC") == 0) {
				gedcCheck = true;
			}
			
			if (gedcCheck == true) {
				if (strcmp(arr[i].tag, "VERS") == 0 && arr[i].value != NULL) {
					versCheck = true;
				}
			}
			
			if (strcmp(arr[i].tag, "CHAR") == 0 && arr[i].value != NULL) {
				charCheck = true;
			}
			if (strcmp(arr[i].tag, "SOUR") == 0 && arr[i].value != NULL) {
				sourceCheck = true;
			}
			if (strcmp(arr[i].tag, "SUBM") == 0 && (arr[i].value != NULL || arr[i].senderAddress != NULL)) {
				submitterCheck = true;
			}
		}
		
		if (strcmp(arr[i].tag, "SUBM") == 0 && strtol(arr[i].level, NULL, 10) == 0) {
			mainSubmitter = true;
		}
		
		currentLevel = strtol(arr[i].level, NULL, 10);
		
		if ((currentLevel - tempLevel) > 1) {
			return createError(INV_RECORD, i + 1);
		}
		
		tempLevel = currentLevel;
	}
	
	if (versCheck == false || charCheck == false || sourceCheck == false || submitterCheck == false) {	
		return createError(INV_HEADER, endOfHeader + 1);
	}
	
	if (mainSubmitter == false) {
		return createError(INV_GEDCOM, -1);
	}
	
	if (strcmp(arr[size-1].tag, "TRLR") != 0) {
		return createError(INV_GEDCOM, -1);
	}
	
	return createError(OK, -1);
}

void freeTaglist(tagList * t, int size) {
	
	if (t == NULL) {
		return;
	}
	
	char * value;
	char * level;
	char * Saddress;
	char * Raddress;
	char * tag;
	
	for (int i = 0; i < size; i ++) {
		value = t[i].value;
		tag = t[i].tag;
		Saddress = t[i].senderAddress;
		Raddress = t[i].recieverAddress;
		level = t[i].level;
		
		if (value != NULL) {
			free(value);
		}
		if (level != NULL) {
			free(level);
		}
		if (Saddress != NULL) {
			free(Saddress);
		}
		if (Raddress != NULL) {
			free(Raddress);
		}
		if (tag != NULL) {
			free(tag);
		}
	}
	
	free(t);
}

void freeStringArray(char ** stringArray, int size) {
	
	if (stringArray == NULL) {
		return;
	}
	
	for (int i = 0; i < size; i ++) {
		free(stringArray[i]);
	}
	free(stringArray);
}

stringInformation stringTokenizer(char * input) {
	stringInformation tempInfo;
	char **tokenizedString = NULL;
	int countDelim = 0;
	char * temp = NULL;
	
	temp = calloc(strlen(input) + 1, sizeof(char));
	
	strcpy(temp, input);
	
	for (char * counter = strtok(temp, " /"); counter != NULL; counter = strtok(NULL, " /")) {
		countDelim++;
	}
	
	tokenizedString = calloc(countDelim + 1, sizeof(char*));
	//tokenizedString = calloc(1,(countDelim + 1));
	strcpy(temp, input);
	
	int size = countDelim;
	
	countDelim = 0;
	
	for (char * counter = strtok(temp, " /"); counter != NULL; counter = strtok(NULL, " /")) {
		tokenizedString[countDelim] = calloc(strlen(counter) + 1, sizeof(char));
		//tokenizedString[countDelim] = calloc(1,sizeof(char) * strlen(counter) + 1);
		strcpy(tokenizedString[countDelim], counter);
		countDelim++;
	}
	
	free(temp);
		
	tempInfo.tokenizedString = tokenizedString;
	tempInfo.size = size;
	
	return tempInfo;
}

char * combineString (char ** string, int first, int second) {
	
	int initial = first - 1; //2
	int final = second; //4
	int size = 0;
	
	if (string[initial] == NULL || string[final - 1] == NULL || first < 1) {
		return NULL;
	}
	//used to combine tokenized strings back to their original form!
	if (first == second) {
		char * combinedString = calloc(1, sizeof(char) * (strlen(string[first-1]) + 255));
		strcpy(combinedString, string[first-1]);
		return combinedString;
	}
	
	char * combinedString = NULL;
	
	for (int i = initial; i < final; i++) {
		size += (sizeof(string[i]) + 2);
	}
	
	combinedString = calloc(size + 255, sizeof(char));
	
	strcpy(combinedString, "");
	
	for (int i = initial; i < final; i++) {
		strcat(combinedString, string[i]);
		strcat(combinedString, " ");
	}
	
	if (combinedString[strlen(combinedString) - 1] == ' ') {
		combinedString[strlen(combinedString) - 1] = '\0';
	}
	
	return combinedString;
	
}


/* $Id: fgets.c,v 1.1.1.1 2006/08/23 17:03:06 pefo Exp $ */

/*
 * Copyright (c) 2000-2002 Opsycon AB  (www.opsycon.se)
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by Opsycon AB.
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

/*  char *fgets(dst,max,fp) -- get string from stream */

//I downloaded fgets sourcecode and modified to accept \r's and 'n's
char * myfgets(char *dst, int max, FILE *fp)
{
	int c;
	int temp;
	char *p;
	long int position = 0;

	/* get max bytes or upto a newline */

	for (p = dst, max--; max > 0; max--) {
		
		if ((c = fgetc (fp)) == EOF)
			break;
		*p++ = c;
		
		position = ftell(fp);
		temp = fgetc(fp);
		if (temp == EOF) {
			break;
		}
		if (c == '\n' && temp != '\r') {
			fseek(fp, position, SEEK_SET);
			break;
		}
		else if (c == '\r' && temp != '\n') {
			fseek(fp, position, SEEK_SET);
			break;
		}
		
		fseek(fp, position, SEEK_SET);
	}
	
	*p = 0;
	if (p == dst || c == EOF)
		return NULL;
	return (p);
}

void recursiveDescendant(List * descendants, Individual * i) {
	if (i == NULL) {
		return;
	}
	
	if (i->families.length == 0) {
		return;
	}
	
	Node * n = i->families.head;
	
	while (n != NULL) {
		if (checkIfSpouse(n->data, i)) {
			Node * e = ((Family*)(n->data))->children.head;
			while (e!= NULL) {
				if (!compareFindPerson(i, e->data)) {
					recursiveDescendant(descendants, e->data);
					if (!checkIfExists(descendants, e->data)) {
						insertBack(descendants, createCopy(e->data));
					}
				}
				e=e->next;
			}
		}
		n = n->next;
	}
}

void recursiveNDescendant(List * descendants, Individual * i, unsigned int max, unsigned int * currentLevel) {
		
	if (i == NULL) {
		return;
	}
	
	Node * n = i->families.head;
	
	while (n != NULL) {
		if (checkIfSpouse(n->data, i)) {
			Node * e = ((Family*)(n->data))->children.head;
			while (e!= NULL) {
				unsigned int pLevel = *currentLevel;
				(*currentLevel)++;
				if (!compareFindPerson(i, e->data)) {
					recursiveNDescendant(descendants, e->data, max, currentLevel);
					if (!checkIfExistsInN(descendants, e->data) && *currentLevel <= max) {
						Node * x = descendants->head;
						int i = 1;
						
						while (i < *currentLevel) {
							x = x->next;
							i++;
						}
						insertSorted(x->data, createCopy(e->data));
					}
				}
				(*currentLevel) = pLevel;
				e=e->next;
			}
		}
		n = n->next;
	}
}

void recursiveNAscendant(List * descendants, Individual * i, unsigned int max, unsigned int * currentLevel) {
		
	if (i == NULL || i->families.length == 0) {
		return;
	}
		
	Node * n = i->families.head;
	
	while (n != NULL) {
		unsigned int temp = *currentLevel;
		if (checkIfChild(n->data, i)) {
			
			Individual * husb = ((Family*)(n->data))->husband;
			Individual * wife = ((Family*)(n->data))->wife;
			
			(*currentLevel)++;
			recursiveNAscendant(descendants, husb, max, currentLevel);
			recursiveNAscendant(descendants, wife, max, currentLevel);
			
			if (husb != NULL && !checkIfExistsInN(descendants, husb) && *currentLevel <= max) {
				Node * x = descendants->head;
				int i = 1;
						
				while (i < *currentLevel) {
					x = x->next;
					i++;
				}
				insertSorted(x->data, createCopy(husb));
			}
			if (wife != NULL && !checkIfExistsInN(descendants, wife) && *currentLevel <= max) {
				Node * x = descendants->head;
				int i = 1;
						
				while (i < *currentLevel) {
					x = x->next;
					i++;
				}
				insertSorted(x->data, createCopy(wife));
			}
		}
		*currentLevel = temp;
		n = n->next;
	}
}

bool checkIfChild (Family * f, Individual * i) {
	
	if (f->children.length == 0) {
		return false;
	}
	
	for (Node * n = f->children.head; n != NULL; n = n->next) {
		if (compareFindPerson(n->data, i)) {
			return true;
		}
	}
	
	return false;
}

bool checkInList(Individual * i, List * originalList) {
	
	for (Node * n = (*originalList).head; n != NULL; n = n->next) {
		if (compareFindPerson(i, n->data)) {
			return true;
		}
	}
	
	return false;
}

bool checkIfSpouse (Family * f, Individual * i) {
	
	if (f==NULL || i == NULL) {
		return false;
	}
	
	if (f->husband == i) {
		return true;
	}
	
	else if (f->wife == i) {
		return true;
	}
	else {
		return false;
	}
}

bool checkIfExists(List * l, Individual * i) {
	
	if (l == NULL || i == NULL) {
		return false;
	}
	
	Node * n = l->head;
	
	while (n!=NULL) {
		if (compareFindPerson(n->data, i)) {
			return true;
		}
		n = n->next;
	}
	return false;	
}

//for N Lists

bool checkIfExistsInN(List * l, Individual * i) {
	
	if (l == NULL || i == NULL) {
		return false;
	}
	
	for (Node * x = l->head; x!=NULL; x=x->next) {
		Node * n = ((List*)(x)->data)->head;
	
		while (n!=NULL) {
			if (compareFindPerson(n->data, i)) {
				return true;
			}
			n = n->next;
		}
	}
	
	return false;	
}

bool compareFindPerson(const void* first,const void* second) {
	
	if (first == NULL || second == NULL) {
		return false;
	}
	
	char temp[1024];
	
	if (((Individual*)first)->givenName != NULL) strcpy(temp, ((Individual*)first)->givenName);
	strcat(temp, ", ");
	if (((Individual*)first)->surname != NULL) strcat(temp, ((Individual*)first)->surname);
	
	char secondTemp[1024];
	
	if (((Individual*)second)->givenName != NULL) strcpy(secondTemp, ((Individual*)second)->givenName);
	strcat(secondTemp, ", ");
	if (((Individual*)second)->surname != NULL) strcat(secondTemp, ((Individual*)second)->surname);
	
	if (strcmp(temp, secondTemp) != 0) {
		return false;
	}
	
	int eventCounter1 = 0;
	int eventCounter2 = 0;
	
	int otherCounter1 = 0;
	int otherCounter2 = 0;
	
	
	int counter1 = 0;
	int counter2 = 0;
	
	Node * n = ((Individual*)first)->events.head;
	
	while (n!= NULL) {
		counter1++;
		eventCounter1++;
		n=n->next;
	}
	
	n = ((Individual*)first)->families.head;
	
	while (n!= NULL) {
		counter1++;
		n=n->next;
	}
	
	n = ((Individual*)first)->otherFields.head;
	
	while (n!= NULL) {
		otherCounter1++;
		counter1++;
		n=n->next;
	}
	
	n = ((Individual*)second)->events.head;
	
	while (n!= NULL) {
		eventCounter2++;
		counter2++;
		n=n->next;
	}
	
	n = ((Individual*)second)->families.head;
	
	while (n!= NULL) {
		counter2++;
		n=n->next;
	}
	
	n = ((Individual*)second)->otherFields.head;
	
	while (n!= NULL) {
		otherCounter2++;
		counter2++;
		n=n->next;
	}
	
	if (counter1 != counter2) {
		return false;
	}
	else {
		if (eventCounter1 == eventCounter2) {
			Node * ptr1 = ((Individual*)first)->events.head;
			Node * ptr2 = ((Individual*)second)->events.head;
			
			while (ptr1!=NULL && ptr2!=NULL) {
				if ((((Event*)(ptr1->data))->date != NULL && ((Event*)(ptr2->data))->date != NULL) && strcmp(((Event*)(ptr1->data))->date, ((Event*)(ptr2->data))->date) != 0) {
					return false;
				}
				
				if ((((Event*)(ptr1->data))->place != NULL && ((Event*)(ptr2->data))->place != NULL) && strcmp(((Event*)(ptr1->data))->place, ((Event*)(ptr2->data))->place) != 0) {
					return false;
				}
				
				ptr1=ptr1->next;
				ptr2=ptr2->next;
			}
			
		}
		
		if (otherCounter1 == otherCounter2) {
			
			Node * ptr1 = ((Individual*)first)->otherFields.head;
			Node * ptr2 = ((Individual*)second)->otherFields.head;
			
			while (ptr1!=NULL && ptr2!=NULL) {
				if (strcmp(((Field*)(ptr1->data))->tag, ((Field*)(ptr2->data))->tag) != 0) {
					return false;
				}
				
				if (strcmp(((Field*)(ptr1->data))->value, ((Field*)(ptr2->data))->value) != 0) {
					return false;
				}
				
				ptr1=ptr1->next;
				ptr2=ptr2->next;
			}
			
		}
	}
	
	return true;

}

bool compareIndiShallow (const void* first,const void* second) {
	
	if (first == NULL || second == NULL) {
		return false;
	}
	
	char temp[1024];
	
	if (((Individual*)first)->givenName != NULL) strcpy(temp, ((Individual*)first)->givenName);
	strcat(temp, ", ");
	if (((Individual*)first)->surname != NULL) strcat(temp, ((Individual*)first)->surname);
	
	char secondTemp[1024];
	
	if (((Individual*)second)->givenName != NULL) strcpy(secondTemp, ((Individual*)second)->givenName);
	strcat(secondTemp, ", ");
	if (((Individual*)second)->surname != NULL) strcat(secondTemp, ((Individual*)second)->surname);
	
	if (strcmp(temp, secondTemp) != 0) {
		return false;
	}
	
	return true;

}

Individual * createCopy(Individual * input) {
	if (input == NULL) {
		return NULL;
	}
	
	Individual * i = calloc(1, sizeof(Individual));
	if (input->givenName != NULL) {
		i->givenName = calloc(1, strlen(input->givenName) + 2);
		strcpy(i->givenName, input->givenName);
	}
	
	if (input->surname != NULL) {
		i->surname = calloc(1, strlen(input->surname) + 2);
		strcpy(i->surname, input->surname);
	}
	
	i->otherFields = input->otherFields;
	i->events = input->events;
	i->families = input->families;
	
	return i;
	
}

void dummyDelete(void* toBeDeleted) {
	
}

int dummyCompare(const void* first,const void* second) {
	return 0;
}

char* dummyPrint(void* toBePrinted) {
	return "";
}

int compareAddress(const void* first,const void* second) {
	return strcmp(((char*)first), ((char*)second));
}

addressInformation * createAddress (List * listPointer, Submitter ** submitterPointer, Individual ** spousePointer, char * address, void * initializedPointer, int count) {
	addressInformation * a = calloc(1,sizeof(addressInformation));
	a->address = calloc(1,(strlen(address) + 1) * sizeof(char));
	
	if (listPointer != NULL) {
		a->listPointer = listPointer;
		a->type = 0;
	}
	if (submitterPointer != NULL) {
		a->submitterPointer = submitterPointer;
		a->type = 1;
	}
	if (spousePointer != NULL) {
		a->spousePointer = spousePointer;
		a->type = 2;
	}
	if (initializedPointer != NULL) {
		a->initializedPointer = initializedPointer;
		a->type = 3;
	}
	
	a->count = count;

	strcpy(a->address, address);
	
	return a;
}

bool checkIndividualEvent (char * tag) {
	
	char * eventTags[] = { "BIRT", "CHR", "ADOP", "DEAT", "BURI", "CREM",
	"BAPM", "BARM", "BASM", "BLES", "CHRA", "CONF", "FCOM", "ORDN", "NATU", "EMIG",
	"EMIG", "IMMI", "CENS", "PROB", "WILL", "GRAD", "RETI", "EVEN" };
	
	if (tag == NULL) {
		return false;
	}
	
	for (int i = 0; i < 24; i ++) {
		if (strcmp(tag, eventTags[i]) == 0) {
			return true;
		}
	}
	
	return false;
}

bool checkFamilyEvent (char * tag) {
	
	char * feventTags[] = { "ANUL", "CENS", "DIV", "DIVF", "ENGA", "MARB", "MARC", "MARR", "MARL", "MARS" };
	
	if (tag == NULL) {
		return false;
	}
	
	for (int i = 0; i < 10; i ++) {
		if (strcmp(tag, feventTags[i]) == 0) {
			return true;
		}
	}
	
	return false;
}

bool findAndLink (List addressList, void * data) {
	
	addressInformation * a = data;
	
	char * address = a->address;
	
	
	Node * n = addressList.head;
	
	while (n!=NULL) {
		addressInformation * b = n->data;
		if (compareAddress(b->address, address) == 0) {
			//printf("%d\n", ((addressInformation*)(n->data))->type);
			
			if (a->type == 0) { // List *
				
				if (b->type == 3) { // Void * 
					List * l = a->listPointer;
					insertBack(l, b->initializedPointer);
					return true;
				}
				
			}
			else if (a->type == 1) { // Submitter *
				
				if (b->type == 1) { // Submitter *
					
					*(a->submitterPointer) = *(b->submitterPointer);
					return true;
				}				
			}
			else if (a->type == 2) { // Individual *
				
				if (b->type == 3) { // Void * 
					*(a->spousePointer) = b->initializedPointer;
					return true;
				}
			}
		}
		n=n->next;
	}
	
	return false;
}

void freeLists (List addressList, List recieveList) {
	Node * n = addressList.head;
	Node * delete = NULL;
	
	while (n!=NULL) {
		addressInformation * a = n->data;
		
		free(a->address);
		free(a);
		delete = n;
		n = n->next;
		free(delete);
	}
	
	n = recieveList.head;
	
	while (n!=NULL) {
		addressInformation * a = n->data;
		
		free(a->address);
		free(a);
		delete = n;
		n = n->next;
		free(delete);
	}
	
	
}

char * writeHeader (Header * h) {
	
	if (h == NULL) {
		return "";
	}
	
	char * returnedString = calloc(1024, sizeof(char));
	
	strcpy(returnedString, "");
	
	strcat(returnedString, "0 HEAD\n");
	
	strcat(returnedString, "1 SOUR ");
	strcat(returnedString, h->source);
	strcat(returnedString, "\n");
	
	strcat(returnedString, "1 GEDC\n");
	
	strcat(returnedString, "2 VERS ");
	sprintf(returnedString + strlen(returnedString), "%.2f", h->gedcVersion);
	strcat(returnedString, "\n");
	
	strcat(returnedString, "2 FORM LINEAGE-LINKED\n");
	
	strcat(returnedString, "1 CHAR ");
	
	if (h->encoding == ANSEL) {
		strcat(returnedString, "ANSEL\n");
	}
	else if (h->encoding == UTF8) {
		strcat(returnedString, "UTF-8\n");
	}
	else if (h->encoding == UNICODE) {
		strcat(returnedString, "UNICODE\n");
	}
	else if (h->encoding == ASCII) {
		strcat(returnedString, "ASCII\n");
	}
	
	strcat(returnedString, "1 SUBM @SUB1@\n");
	
	return returnedString;
}

char * writeSubmitter (Submitter * s) {
	
	if (s == NULL) {
		return "";
	}
		
	char * returnedString = calloc(1024, sizeof(char));
	
	strcpy(returnedString, "");
	
	strcat(returnedString, "0 @SUB1@ SUBM\n");
	
	strcat(returnedString, "1 NAME ");
	strcat(returnedString, s->submitterName);
	strcat(returnedString, "\n");
	
	if (strcmp(s->address, "") != 0) {
		strcat(returnedString, "1 ADDR ");
		strcat(returnedString, s->address);
		strcat(returnedString, "\n");
	}
	
	return returnedString;
}

char * writeIndividual(List individuals, List familyList) {
	
	int reference = 1;
	
	Node * n = individuals.head;
	
	char * returnedString = calloc(2048, sizeof(char));
	
	strcpy(returnedString, "");
	
	while (n!= NULL) {
		
	strcat(returnedString, "0 @I");
	sprintf(returnedString + strlen(returnedString), "%d", reference);
	strcat(returnedString,"@ INDI\n");
		
	Individual * i = n->data;
	
	if (strcmp(i->givenName, " ") != 0) {
		strcat(returnedString, "1 NAME ");
		strcat(returnedString, i->givenName);
		strcat(returnedString, " /");
	}
	else {
		strcat(returnedString, "1 NAME /");

	}
	
	if (strcmp(i->surname, " ") != 0) {
		strcat(returnedString, i->surname);
		strcat(returnedString, "/\n");
	}
	else {
		strcat(returnedString, "/\n");
	}
	
	Node * e = i->otherFields.head;
	
	//start reallocing frmo here
	
	while (e!=NULL) {
		returnedString = realloc(returnedString, ((strlen(returnedString) + 256) * sizeof(char)));
		Field * f = e->data;
		
		if (strcmp(f->tag, "GIVN") == 0) {
			returnedString = realloc(returnedString, (strlen(returnedString) + 10 + strlen(f->value)) * sizeof(char));
			strcat(returnedString, "2 GIVN ");
			strcat(returnedString, f->value);
			strcat(returnedString, "\n");
		}
		else if (strcmp(f->tag, "SURN") == 0) {
			returnedString = realloc(returnedString, (strlen(returnedString) + 10 + strlen(f->value)) * sizeof(char));
			strcat(returnedString, "2 SURN ");
			strcat(returnedString, f->value);
			strcat(returnedString, "\n");
		}
		else {
			returnedString = realloc(returnedString, (strlen(returnedString) + 100 + strlen(f->value) + strlen(f->tag)) * sizeof(char));
			strcat(returnedString, "1 ");
			strcat(returnedString, f->tag);
			strcat(returnedString, " ");
			strcat(returnedString, f->value);
			strcat(returnedString, "\n");
		}
		
		e=e->next;
	}
	
	e = i->events.head;
	
	while (e!=NULL) {
		Event * event = e->data;
		
		if (event->place != NULL && event->date != NULL) {
			returnedString = realloc(returnedString, (strlen(returnedString) + 256 + strlen(event->place) + strlen(event->date)) * sizeof(char));
		}
		else if (event->place != NULL) {
			returnedString = realloc(returnedString, (strlen(returnedString) + 256 + strlen(event->place)) * sizeof(char));
		}
		else if (event->date != NULL) {
			returnedString = realloc(returnedString, (strlen(returnedString) + 256 + strlen(event->date)) * sizeof(char));
		}
		else {
			returnedString = realloc(returnedString, (strlen(returnedString) + 256) * sizeof(char));
		}
		
		strcat(returnedString, "1 ");
		strcat(returnedString, event->type);
		strcat(returnedString, "\n");
		
		if (event->date != NULL) {
			strcat(returnedString, "2 DATE ");
			strcat(returnedString, event->date);
			strcat(returnedString, "\n");
		}
		
		if (event->place != NULL) {
			strcat(returnedString, "2 PLAC ");
			strcat(returnedString, event->place);
			strcat(returnedString, "\n");
		}
		
		e = e->next;	
	}
	
	e = i->families.head;
	
	while (e != NULL) {
		returnedString = realloc(returnedString, ((strlen(returnedString) + 256) * sizeof(char)));
		Family * f = e->data;
		
		if (checkIfSpouse(f, i)) {
			strcat(returnedString, "1 FAMS @");
			
			Node * temp = familyList.head;
			
			while (temp != NULL) {
				//HERE
				if (compareFamilies(f, ((famList*)(temp->data))->reference) == 1) {
					returnedString = realloc(returnedString, (strlen(returnedString) + 100) * sizeof(char));
					strcat(returnedString, ((famList*)(temp->data))->address);
					strcat(returnedString, "@\n");
					break;
				}
				temp = temp->next;
			}
			
		}
		else {
			strcat(returnedString, "1 FAMC @");
			
			Node * temp = familyList.head;
			
			while (temp != NULL) {
				//HERE
				if (compareFamilies(f, ((famList*)(temp->data))->reference) == 1) {
					returnedString = realloc(returnedString, (strlen(returnedString) + 100) * sizeof(char));
					strcat(returnedString, ((famList*)(temp->data))->address);
					strcat(returnedString, "@\n");
					break;
				}
				temp = temp->next;
			}
		}
		
		e = e->next;
	}
	
	
	
	n = n->next;
	reference++;	
	}
	
	return returnedString;
}

char * writeFamily (List families, List individualList) {
	
	int counter = 1;
	
	char * returnedString = calloc(1024, sizeof(char));
	
	Node * n = families.head;
	
	while (n != NULL) {
		returnedString = realloc(returnedString, ((strlen(returnedString) + 256) * sizeof(char)));
		strcat(returnedString, "0 @F");
		sprintf(returnedString + strlen(returnedString), "%d", counter);
		strcat(returnedString,"@ FAM\n");
		
		Family * f = n->data;
		
		if (f->husband != NULL) {
			returnedString = realloc(returnedString, ((strlen(returnedString) + 256) * sizeof(char)));
			strcat(returnedString, "1 HUSB @");
			Individual * i = f->husband;
			
			Node * temp = individualList.head;
			
			while (temp != NULL) {
				//HERE
				if (compareFindPerson(i, ((indiList*)(temp->data))->reference)) {
					returnedString = realloc(returnedString, (strlen(returnedString) + 256) * sizeof(char));
					strcat(returnedString, ((indiList*)(temp->data))->address);
					strcat(returnedString, "@\n");
					break;
				}
				temp = temp->next;
			}
		}
		
		if (f->wife != NULL) {
			returnedString = realloc(returnedString, ((strlen(returnedString) + 256) * sizeof(char)));
			strcat(returnedString, "1 WIFE @");
			Individual * i = f->wife;
			
			Node * temp = individualList.head;
			
			while (temp != NULL) {
				//HERE
				if (compareFindPerson(i, ((indiList*)(temp->data))->reference)) {
					returnedString = realloc(returnedString, (strlen(returnedString) + 256) * sizeof(char));
					strcat(returnedString, ((indiList*)(temp->data))->address);
					strcat(returnedString, "@\n");
					break;
				}
				temp = temp->next;
			}
		}
		
		Node * e = f->events.head;
	
		while (e!=NULL) {
			Event * event = e->data;
			if (event->place != NULL && event->date != NULL) {
				returnedString = realloc(returnedString, (strlen(returnedString) + 256 + strlen(event->place) + strlen(event->date)) * sizeof(char));
			}
			else if (event->place != NULL) {
				returnedString = realloc(returnedString, (strlen(returnedString) + 256 + strlen(event->place)) * sizeof(char));
			}
			else if (event->date != NULL) {
				returnedString = realloc(returnedString, (strlen(returnedString) + 256 + strlen(event->date)) * sizeof(char));
			}
			else {
				returnedString = realloc(returnedString, (strlen(returnedString) + 256) * sizeof(char));
			}
		
			strcat(returnedString, "1 ");
			strcat(returnedString, event->type);
			strcat(returnedString, "\n");
		
			if (event->date != NULL) {
				strcat(returnedString, "2 DATE ");
				strcat(returnedString, event->date);
					strcat(returnedString, "\n");
			}
				
			if (event->place != NULL && strcmp(event->place, "") != 0) {
				strcat(returnedString, "2 PLAC ");
				strcat(returnedString, event->place);
				strcat(returnedString, "\n");
			}
		
			e = e->next;	
		}
		
		e = f->children.head;
		
		while (e!=NULL) {
			returnedString = realloc(returnedString, ((strlen(returnedString) + 256) * sizeof(char)));
			strcat(returnedString, "1 CHIL @");
			Node * temp = individualList.head;
			
			while (temp != NULL) {
				
				//HERE
				if (compareFindPerson(e->data, ((indiList*)(temp->data))->reference)) {
					returnedString = realloc(returnedString, (strlen(returnedString) + 256) * sizeof(char));
					strcat(returnedString, ((indiList*)(temp->data))->address);
					strcat(returnedString, "@\n");
					break;
				}
				temp = temp->next;
			}
			
			e = e->next;
		}
			
		n = n->next;
		counter++;
	}
	
	return returnedString;
}

List createListFam (List families) {
	List familyList = initializeList(dummyPrint, deleteVoid, dummyCompare);
	
	Node * n = families.head;
	
	int counter = 1;
	
	while (n!=NULL) {
		famList * f = malloc(sizeof(famList));
		strcpy(f->address, "F");
		sprintf(f->address + strlen(f->address), "%d", counter);
		f->reference = n->data;
		insertBack(&familyList, f);
				
		n=n->next;
		counter++;
	}

	return familyList;
}

List createListIndi (List individuals) {
	List individualList = initializeList(dummyPrint, deleteVoid, dummyCompare);
	
	Node * n = individuals.head;
	
	int counter = 1;
	
	while (n!=NULL) {
		indiList * f = malloc(sizeof(famList));
		strcpy(f->address, "I");
		sprintf(f->address + strlen(f->address), "%d", counter);
		f->reference = n->data;
		insertBack(&individualList, f);
				
		n=n->next;
		counter++;
	}
	
	return individualList;
}

void deleteVoid (void * data) {
	free(data);
}

void deleteNList (void * data) {
	List * l = data;
	
	clearList(l);
	
	free(data);
}

int comparePointer(const void* first,const void* second) {
	if (first == second) {
		return 0;
	}
	
	return 1;
}

void deleteCopy (void* toBeDeleted) {
	
	if (toBeDeleted == NULL) {
		return;
	}
	
	Individual * i = (Individual*)toBeDeleted;
	
	if (i->givenName!=NULL) {
		free(i->givenName);
	}
	
	if (i->surname != NULL) {
		free(i->surname);
	}
	
	free(i);
	
}
