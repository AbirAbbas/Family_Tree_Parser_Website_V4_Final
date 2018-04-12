#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <ctype.h>
#include "assert.h"
#include <unistd.h>

#include "LinkedListAPI.h"
#include "GEDCOMutilities.h"
#include "GEDCOMparser.h"

GEDCOMerror createGEDCOM(char* fileName, GEDCOMobject** obj) {
	
	bool filePass = false;
	
	//fileValidator : checks if file exists, if fileName is NULL, and if fileName ends with .ged
	if (fileName != NULL) {
			
		if ((strlen(fileName) - 3) > 0) {
			if (fileName[strlen(fileName) - 4] == '.' && fileName[strlen(fileName) - 3] == 'g' && fileName[strlen(fileName) - 2] == 'e' && fileName[strlen(fileName) - 1] == 'd') {
				if (fileExist(fileName)) {
					filePass = true;
				}
			}
		}
	}
	//Returns error code if failed!
	if (filePass == false) {
		obj = NULL;
		return createError((ErrorCode)INV_FILE, -1);
	}
	
	FILE * inFile = fopen(fileName, "r");
	
	arrayList gedFile = initializeArray(inFile);
	
	int * size = malloc(sizeof(int));
	
	tagList * allTags = initializeTags(gedFile, size);
	
	container c;
	c.t = allTags;
	c.size = *size;
		
	GEDCOMerror e = checkForError(c.t, c.size);
	
	if (e.type == OK) {
		
	}
	else {
		return e;
	}
	
	List * recieveList = malloc(sizeof(List));
	List * sendList = malloc(sizeof(List));
	
	*recieveList = initializeList(dummyPrint, dummyDelete, compareAddress);
	*sendList = initializeList(dummyPrint, dummyDelete, compareAddress);
	
	obj = initObject(obj);	
	e = parserDistributor(*obj, c, sendList, recieveList);
	
	if (e.type == OK) {
		
	}
	else {
		return e;
	}
	
	Node * n = sendList->head;
		
	while (n!=NULL) {
		
		if (!findAndLink(*recieveList, n->data)) {
			//deleteGEDCOM(*obj);
			//freeLists(addressList, recieveList);
			//freeStringArray(splitString);
			//fclose(inFile);
			//free(currentRecord);
			//free(subRecord);
			//free(checkHeader); 
			return createError(INV_GEDCOM, -1);
		}

		n = n->next;
	}	
		
		
		
		
	//after it is done being used
	freeTaglist(allTags, *size);
	free(size);
	
	freeLists(*sendList, *recieveList);
	
	free(sendList);
	free(recieveList);
	
	//temporary!
	fclose(inFile);

	return createError((ErrorCode)OK, -1);
	
}


char* printGEDCOM(const GEDCOMobject* obj) {
	
	if (obj == NULL) {
		return "";
	}
	
	char * temp = calloc(1024, sizeof(char));
	char * tempHolder = NULL;
	
	strcat(temp, "Header Information : \n");
	
	if (obj->header != NULL) {
		Header * h = obj->header;
		if (h->source != NULL) {
			strcat(temp, h->source);
			strcat(temp, "\n");
		}
		char * num = malloc(sizeof(char) * 32);
		if (h->gedcVersion >= 0) {
			strcat(temp, "GEDC Version : ");
			sprintf(num, "%f", h->gedcVersion);
			if (num != NULL) {
				strcat(temp, num);
				free(num);
			}
			strcat(temp, "\n");
		}
	}
	
	if (obj->submitter != NULL) {
		Submitter * s = obj->submitter;
		
		if (s->submitterName != NULL) {
			strcat(temp, "Submitter Name : ");
			strcat(temp, s->submitterName);
			strcat(temp, "\n");
		}
		
		if (s->address != NULL) {
			strcat(temp, "Submitter Address : ");
			strcat(temp, s->address);
			strcat(temp, "\n");
		}
		
		tempHolder = toString(s->otherFields);
		
		if (tempHolder != NULL) {
			temp = realloc(temp, (strlen(temp) + strlen(tempHolder) + 50) * sizeof(char));
			strcat(temp, "Header, OtherFields : \n");
			strcat(temp, tempHolder);
			free(tempHolder);
			strcat(temp, "\n");
		}
	}
	
	temp = realloc(temp, (strlen(temp)+ 1024) * sizeof(char));
	
	strcat(temp, "Header information completed\n");
	
	char * allOtherFields = toString(obj->families);
	
	if (allOtherFields != NULL) {
		temp = realloc(temp, (strlen(temp) + strlen(allOtherFields) + 1024) * sizeof(char));
		strcat(temp, allOtherFields);
		free(allOtherFields);
	}
	
	strcat(temp, "All information has been completely printed!\n");

	return temp;
}

void deleteGEDCOM(GEDCOMobject * obj) {
	
	if (obj == NULL) {
		return;
	}
	
	Header * h = obj->header;
	Submitter * sub = obj->submitter;
	
	//Header started
	
	if (h != NULL) {
		clearList(&(h->otherFields));
		free(h);
	//Header deleted
	}
		
	//printf("TESSST\n");
	//Families started
	if (obj->families.length > 0) {
		clearList(&(obj->families));
	}	
	//FamiliesDeleted
	
	//Individuals started
	if (obj->individuals.length > 0) {
		clearList(&(obj->individuals));
	}
	//Invidiaul deleted
	
	//Submitter->otherFields started
	if (sub!= NULL) {
		clearList(&(sub->otherFields));
		free(sub);
	}
	
	free(obj);
	//Submitter->otherFields deleted
}

char* printError(GEDCOMerror err) {
	char * error = calloc(1024, sizeof(char));
	if (err.type == OK) {
		strcpy(error, "No errors reported\n");
	}
	else if (err.type == INV_FILE) {
		strcpy(error, "Error while opening file of format .ged!\n");
	}
	else if (err.type == INV_GEDCOM) {
		strcpy(error, "Invalid Gedcom object, Missing header or TRLR character!\n");
		char * num = malloc(sizeof(char) * 32);
		sprintf(num, "%d", err.line);
		if (num != NULL) {
			strcat(error, num);
			free(num);
		}
		strcat(error, "\n");
	}
	else if (err.type == INV_HEADER) {
		strcpy(error, "Invalid Header Object, at line ");
		char * num = malloc(sizeof(char) * 32);
		sprintf(num, "%d", err.line);
		if (num != NULL) {
			strcat(error, num);
			free(num);
		}
		strcat(error, "\n");
	}
	else if (err.type == INV_RECORD) {
		strcpy(error, "Invalid Record Object, at line ");
		char * num = malloc(sizeof(char) * 32);
		sprintf(num, "%d", err.line);
		if (num != NULL) {
			strcat(error, num);
			free(num);
		}
		strcat(error, "\n");
	}
	else if (err.type == OTHER_ERROR) {
		strcpy(error, "System out of memory!\n");
	}
	
	return error;
}

Individual* findPerson(const GEDCOMobject* familyRecord, bool (*compare)(const void* first, const void* second), const void* person) {
	
	if (familyRecord == NULL || person == NULL) {
		return NULL;
	}
	
	if (familyRecord->individuals.length == 0) {
		return NULL;
	}
	
	Node * n = familyRecord->individuals.head;
	
	while (n!= NULL) {
		if (compare(person, n->data) == true) { // Found
			Individual * data = n->data;
			
			return data;
		}
		n = n->next;
	}
	
	// Not Found
	return NULL;
}


List getDescendants(const GEDCOMobject* familyRecord, const Individual* person) {
	
	List descendants = initializeList(printIndividual, deleteCopy, compareIndividuals);
	
	if (familyRecord == NULL || person == NULL) {
		return descendants;
	}

	recursiveDescendant(&descendants, (Individual*)person); 
	
	return descendants;
}

// ******************* HELPER FUNCTIONS ***********************
//EVENTS
void deleteEvent(void* toBeDeleted) {
	
	if (toBeDeleted == NULL) {
		return;
	}
	
	Event * e = ((Event*)toBeDeleted);
	
	if (e->date != NULL) free(e->date);
	if (e->place != NULL) free(e->place);
	
	clearList(&(e->otherFields));
	
	free(e);
}

int compareEvents(const void* first,const void* second) {
	return 0;
}

char* printEvent(void* toBePrinted) {
	if (toBePrinted == NULL) {
		return "";
	}
	
	Event * e = toBePrinted;
	
	char * printEvent = calloc(2, sizeof(char));
	char * tempHolder = calloc(2048, sizeof(char));
	strcpy(printEvent, "");
	strcpy(tempHolder, "");
	strcat(tempHolder, "		[Event]\n");
	if (e->type != NULL) { 
		strcat(tempHolder, "			Type : ");
		strcat(tempHolder, e->type);
		strcat(tempHolder, "\n");
	}
	
	if (e->date != NULL) { 
		strcat(tempHolder, "			Date : ");
		strcat(tempHolder, e->date);
		strcat(tempHolder, "\n");
	}
	
	if (e->place != NULL) { 
		strcat(tempHolder, "			Place : ");
		strcat(tempHolder, e->place);
		strcat(tempHolder, "\n");
	}
	
	if (tempHolder != NULL) {
		printEvent = (char*)realloc(printEvent, (strlen(printEvent) + strlen(tempHolder) + 50) * sizeof(char));
		strcpy(printEvent, tempHolder);
		free(tempHolder);
	}
	
	tempHolder = toString(e->otherFields);
	
	if (tempHolder != NULL) {
		printEvent = (char*)realloc(printEvent, (strlen(printEvent) + strlen(tempHolder) + 150) * sizeof(char));	
		strcat(printEvent, tempHolder);
		free(tempHolder);
	}
	
	strcat(printEvent, "		} End of event\n");
	
	return printEvent;
}


//INDIVIDUAL
void deleteIndividual(void* toBeDeleted) {
	
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
	
	if (i->events.length > 0) {
		clearList(&(i->events));
	}
	if (i->otherFields.length > 0) {
		clearList(&(i->otherFields));
	}
	
	Node * n = i->families.head;
	Node * delete;
	
	while (n != NULL) {
		delete = n;
		n = n->next;
		free(delete);
	}
	
	free(i);
	
}

int compareIndividuals(const void* first,const void* second) {
	
	Individual * i1 = (Individual*)first;
	Individual * i2 = (Individual*)second;
	
	if (i1 == NULL || i2 == NULL) {
		return -1;
	}
	
	if (strcmp(i1->surname, i2->surname) > 0) {
		return 1;
	}
	else if (strcmp(i1->surname, i2->surname) < 0) {
		return -1;
	}
	
	if (strcmp(i1->givenName, i2->givenName) > 0) {
		return 1;
	}
	else if (strcmp(i1->givenName, i2->givenName) < 0) {
		return -1;
	}
	
	return 0;
}


char* printIndividual(void* toBePrinted) {
	
	if (toBePrinted == NULL) {
		return "";
	}
	
	Individual * e = toBePrinted;
	
	char * printField = calloc(1,sizeof(char));
	char * tempHolder = calloc(1,sizeof(char) * 2048);
	
	strcpy(tempHolder, "	[Individual]\n");
	
	if (e->givenName != NULL) { 
		strcat(tempHolder, "		Given Name : ");
		strcat(tempHolder, e->givenName);
		strcat(tempHolder, "\n");
	}
	
	if (e->surname != NULL) { 
		strcat(tempHolder, "		Surname : ");
		strcat(tempHolder, e->surname);
		strcat(tempHolder, "\n");
	}

	
	if (tempHolder != NULL) {
		printField = (char*)realloc(printField, (strlen(printField) + strlen(tempHolder) + 50) * sizeof(char));	
		strcpy(printField, tempHolder);
		free(tempHolder);
	}
	
	tempHolder = toString(e->events);
	
	if (tempHolder != NULL) {
		printField = (char*)realloc(printField, (strlen(printField) + strlen(tempHolder) + 150) * sizeof(char));	
		strcat(printField, tempHolder);
		free(tempHolder);
	}
	
	tempHolder = toString(e->otherFields);
	
	if (tempHolder != NULL) {
		printField = (char*)realloc(printField, (strlen(printField) + strlen(tempHolder) + 150) * sizeof(char));	
		strcat(printField, tempHolder);
		free(tempHolder);
	}
	
	strcat(printField, "	[End of individual]\n");

	return printField;
}



//FAMILY
void deleteFamily(void* toBeDeleted) {
	
	if (toBeDeleted == NULL) {
		return;
	}
	
	Family * f = (Family*)toBeDeleted;
	
	clearList(&(f->otherFields));
	
	clearList(&(f->events));
	
	Node * n = f->children.head;
	Node * delete;
	
	while (n != NULL) {
		delete = n;
		n = n->next;
		free(delete);
	}
	
	f->wife = NULL;
	f->husband = NULL;
	
	free(f);
}

int compareFamilies(const void* first,const void* second) {
	
	if (first == second) {
		return 1;
	}
	
	if (first == NULL || second == NULL) {
		return 0;
	}
	
	Family * fam1 = (Family*)first;
	Family * fam2 = (Family*)second;
	
	if (!compareFindPerson(fam1->husband, fam2->husband)) {
		return 0;
	}
	
	if (!compareFindPerson(fam1->wife, fam2->wife)) {
		return 0;
	}
	
	int counter1 = 0;
	int counter2 = 0;
	
	Node * n = fam1->events.head;
	
	while (n!= NULL) {
		counter1++;
		n=n->next;
	}
	
	n = fam1->children.head;
	
	while (n!= NULL) {
		counter1++;
		n=n->next;
	}
	
	n = fam1->otherFields.head;
	
	while (n!= NULL) {
		counter1++;
		n=n->next;
	}
	
	n = fam2->events.head;
	
	while (n!= NULL) {
		counter2++;
		n=n->next;
	}
	
	n = fam2->children.head;
	
	while (n!= NULL) {
		counter2++;
		n=n->next;
	}
	
	n = fam2->otherFields.head;
	
	while (n!= NULL) {
		counter2++;
		n=n->next;
	}
	
	if (counter1 != counter2) {
		return 0;
	}
	
	return 1;
}


char* printFamily(void* toBePrinted) {
	if (toBePrinted == NULL) {
		return "";
	}
	
	Family * e = toBePrinted;
	
	char * printField = calloc(1,sizeof(char));
	char * tempHolder = calloc(1,sizeof(char) * 2048);
	
	
	strcpy(tempHolder, "");
	strcat(tempHolder, "[Family]\n");
	
	if (e->husband != NULL) { 
		strcat(tempHolder, "	Husband : \n");
		char * husband = printIndividual(e->husband);
		if (husband != NULL) {
			strcat(tempHolder, husband);
			free(husband);
		}
		strcat(tempHolder, "\n");
	}
	
	if (e->wife != NULL) { 
		strcat(tempHolder, "	Wife : \n");
		char * wife = printIndividual(e->wife);
		if (wife != NULL) {
			strcat(tempHolder, wife);
			free(wife);
		}
		strcat(tempHolder, "\n");
	}
	
	if (tempHolder != NULL) {
		printField = (char*)realloc(printField, (strlen(tempHolder) + 50) * sizeof(char));	
		strcpy(printField, tempHolder);
		free(tempHolder);
	}
	
	tempHolder = toString(e->events);
	
	if (tempHolder != NULL) {
		printField = (char*)realloc(printField, (strlen(printField) + strlen(tempHolder) + 150	) * sizeof(char));
		strcat(printField, "		[Family Events] : \n");
		strcat(printField, tempHolder);
		free(tempHolder);
		strcat(printField, "\n		[End of Family Events]\n");
	}
	
	tempHolder = toString(e->otherFields);
	
	if (tempHolder != NULL) {
		printField = (char*)realloc(printField, (strlen(printField) + strlen(tempHolder) + 150	) * sizeof(char));
		strcat(printField, "		[Other fields] : \n");
		strcat(printField, tempHolder);
		strcat(printField, "\n		[End of othersField]\n");
		free(tempHolder);
	}
	
	Node * n = e->children.head;
	
	strcat(printField, "	[Children] :\n");
	
	while (n!=NULL) {
		printField = (char*)realloc(printField, (strlen(printField) + 1024) * sizeof(char));
		strcat(printField, "	CHILD :\n");
		tempHolder = printIndividual(n->data);
		if (tempHolder != NULL) {
			strcat(printField, tempHolder);
		} 
		n=n->next;
	}
	
	strcat(printField, "	[End of Children] :\n");
	
	strcat(printField, "[End of Family]\n");
	
		
	return printField;
}


//FIELD
void deleteField(void* toBeDeleted) {
	
	Field * f = (Field*)toBeDeleted;
	
	if (f->tag != NULL) {
		free(f->tag);
	}
	if(f->value != NULL) {
		free(f->value);
	}
	
	free(f);
	
}

int compareFields(const void* first,const void* second) {
	
	Field * temp1 = (Field*)first;
	Field * temp2 = (Field*)first;
	
	if (temp1 == NULL || temp2 == NULL) {
		return 0;
	}
	
	if (temp1 == temp2) {
		return 1;
	}
	
	if (temp1->tag == NULL || temp2->tag == NULL || strcmp(temp1->tag, temp2->tag) != 0) {
		return 0;
	}
	
	if (temp1->value == NULL || temp2->value == NULL || strcmp(temp1->value, temp2->value) != 0) {
		return 0;
	}
	
	return 1;
}

char* printField(void* toBePrinted) {
	if (toBePrinted == NULL) {
		return "";
	}
	
	Field * e = toBePrinted;
	
	char * printField = calloc(1, sizeof(char) * 2);
	char * tempHolder = calloc(2048, sizeof(char));
	
	strcpy(printField, "");
	strcpy(tempHolder, "");
	strcat(tempHolder, "		Field { \n");
	
	if (e->tag != NULL) { 
		strcat(tempHolder, "			Tag : ");
		strcat(tempHolder, e->tag);
		strcat(tempHolder, "\n");
	}
	
	if (e->value != NULL) { 
		strcat(tempHolder, "			Value : ");
		strcat(tempHolder, e->value);
		strcat(tempHolder, "\n");
	}
	
	strcat(tempHolder, "		} End of Field\n");
	
	if (tempHolder != NULL) {
		printField = (char*)realloc(printField, (strlen(printField) + strlen(tempHolder) + 50) * sizeof(char));
	
		strcpy(printField, tempHolder);
	
		free(tempHolder);
	}
	
	return printField;
}


// ****************************** A2 functions ******************************

/** Function to writing a GEDCOMobject into a file in GEDCOM format.
 *@pre GEDCOMobject object exists, is not null, and is valid
 *@post GEDCOMobject has not been modified in any way, and a file representing the
 GEDCOMobject contents in GEDCOM format has been created
 *@return the error code indicating success or the error encountered when parsing the calendar
 *@param obj - a pointer to a GEDCOMobject struct
 **/
GEDCOMerror writeGEDCOM(char* fileName, const GEDCOMobject* obj) {
	
	char * writtenGedcom = calloc(1, sizeof(char));
	
	FILE * inFile;
	
	if (obj == NULL || fileName == NULL) {
		return createError(WRITE_ERROR, -1);
	}
	
	if (fileName != NULL) {
			
		if ((strlen(fileName) - 3) > 0) {
			if (fileName[strlen(fileName) - 4] == '.' && fileName[strlen(fileName) - 3] == 'g' && fileName[strlen(fileName) - 2] == 'e' && fileName[strlen(fileName) - 1] == 'd') {
				inFile = fopen(fileName, "w+");
			}
			else {
				return createError(INV_FILE, -1);
			}
		}
		else {
			
			return createError(INV_FILE, -1);
		}
	}
	else {
		return createError(INV_FILE, -1);
	}

	
	if (inFile == NULL) {
		return createError(INV_FILE, -1);
	}
	
	List familyList = createListFam(obj->families);
	List individualList = createListIndi(obj->individuals);
	
		
	//write header
	
	char * head = writeHeader(obj->header);
	
	writtenGedcom = realloc(writtenGedcom, (strlen(head) + 1) * sizeof(char));
	writtenGedcom = strcpy(writtenGedcom, head);
	
	free(head);
	
	
	//write submitter
	
	char * submitter = writeSubmitter(obj->submitter);
	
	writtenGedcom = realloc(writtenGedcom, ((strlen(submitter) + 1 + strlen(writtenGedcom)) * sizeof(char)));
	writtenGedcom = strcat(writtenGedcom, submitter);
	
	free(submitter);
	
	//write individual
	
	char * individual = writeIndividual(obj->individuals, familyList);
	
	writtenGedcom = realloc(writtenGedcom, ((strlen(individual) + 1 + strlen(writtenGedcom)) * sizeof(char)));
	writtenGedcom = strcat(writtenGedcom, individual);
	
	free(individual);	
	
	//write family
	
	char * family = writeFamily(obj->families, individualList);
	
	writtenGedcom = realloc(writtenGedcom, ((strlen(family) + 100 + strlen(writtenGedcom)) * sizeof(char)));
	writtenGedcom = strcat(writtenGedcom, family);
	
	free(family);	
	
	//write TRLR
	
	char * trailer = calloc(1, sizeof("0 TRLR\n"));
	strcpy(trailer, "0 TRLR\n");
	writtenGedcom = strcat(writtenGedcom, trailer);
	free(trailer);
		
	fputs(writtenGedcom, inFile);
	fclose(inFile);
	free(writtenGedcom);
	clearList(&familyList);
	clearList(&individualList);
	
	return createError(OK, -1);
}

/** Function for validating an existing GEDCOM object
 *@pre GEDCOM object exists and is not null
 *@post GEDCOM object has not been modified in any way
 *@return the error code indicating success or the error encountered when validating the GEDCOM
 *@param obj - a pointer to a GEDCOMobject struct
 **/
ErrorCode validateGEDCOM(const GEDCOMobject* obj) {
	
	if (obj == NULL) {
		return INV_GEDCOM;
	}
	
	//GEDCOM check
	if (obj->header == NULL) {
		return INV_GEDCOM;
	}
	
	if (obj->submitter == NULL) {
		return INV_GEDCOM;
	}
	//Header
	if (strcmp(obj->header->source, "") == 0) {
		return INV_HEADER;
	}
	
	if (obj->header->gedcVersion < 0) {
		return INV_HEADER;
	}
	
	if (obj->header->submitter == NULL) {
		return INV_HEADER;
	}
	//Submitter
	if(strcmp(obj->submitter->submitterName, "") == 0) {
		return INV_RECORD;
	}
	
	//Individuals
	Node * n = obj->individuals.head;
	
	while (n!=NULL) {
		if (n->data == NULL) {
			return INV_RECORD;
		}
		
		Individual * i = n->data;
		
		Node * e = i->families.head;
		
		while (e!=NULL) {
			
			if (e->data == NULL) {
				return INV_RECORD;
			}
			
			e=e->next;
		}
		
		e = i->otherFields.head;
		
		while (e!=NULL) {
			
			Field * f = e->data;
			
			if (f->value == NULL || strlen(f->value) > 200) {
				return INV_RECORD;
			}
			
			e=e->next;
		}
		
		n=n->next;
	}
	
	//Families
	
	n = obj->families.head;
	
	while (n != NULL) {
		if (n->data == NULL) {
			return INV_RECORD;
		}
		
		Family * f = n->data;
		
		Node * e = f->children.head;
		
		while (e!=NULL) {
			
			if (e->data == NULL) {
				return INV_RECORD;
			}
			
			e=e->next;
		}
		
		e = f->otherFields.head;
		
		while (e!=NULL) {
			
			Field * f = e->data;
			
			if (f->value == NULL || strlen(f->value) > 200) {
				return INV_RECORD;
			}
			
			e=e->next;
		}
		
		n=n->next;
	}
	
	
	
	return OK;
}

/** Function to return a list of up to N generations of descendants of an individual in a GEDCOM
 *@pre GEDCOM object exists, is not null, and is valid
 *@post GEDCOM object has not been modified in any way, and a list of descendants has been created
 *@return a list of descendants.  The list may be empty.  All list members must be of type List.    *@param familyRecord - a pointer to a GEDCOMobject struct
 *@param person - the Individual record whose descendants we want
 *@param maxGen - maximum number of generations to examine (must be >= 1)
 **/
List getDescendantListN(const GEDCOMobject* familyRecord, const Individual* person, unsigned int maxGen) {
	
	List descendants = initializeList(dummyPrint, deleteNList, comparePointer);
	
	if (maxGen == 0) {
		maxGen = 100;
	}
	
	if (maxGen > 100) {
		maxGen = 100;
	}
	
	if (familyRecord == NULL || person == NULL) {
		return descendants;
	}
	
	for (int i = 0; i < maxGen; i ++) {
		List * depthList = malloc(sizeof(List));
		*depthList = initializeList(printIndividual, deleteCopy, compareIndividuals);
		insertBack(&descendants, depthList);
	}
	
	unsigned int * current = malloc(sizeof(unsigned int));
	*current = 0;
	
	if (familyRecord == NULL || person == NULL) {
		return descendants;
	}
			
	recursiveNDescendant(&descendants, (Individual*)person, maxGen, current); 
	
	free(current);
	
	Node * n = descendants.head;
	
	while (n!=NULL) {
		List * x = n->data;
		
		if (x->length == 0) {
			deleteDataFromList(&descendants, x);
			n=descendants.head;
			free(x);
		}
		else {
			n = n->next;
		}
	}
	
	return descendants;
}

/** Function to return a list of up to N generations of ancestors of an individual in a GEDCOM
 *@pre GEDCOM object exists, is not null, and is valid
 *@post GEDCOM object has not been modified in any way, and a list of ancestors has been created
 *@return a list of ancestors.  The list may be empty.
 *@param familyRecord - a pointer to a GEDCOMobject struct
 *@param person - the Individual record whose descendants we want
 *@param maxGen - maximum number of generations to examine (must be >= 1)
 **/
List getAncestorListN(const GEDCOMobject* familyRecord, const Individual* person, int maxGen) {
	
	List descendants = initializeList(dummyPrint, deleteNList, comparePointer);
	
	if (maxGen == 0) {
		maxGen = 100;
	}
	
	if (maxGen > 100) {
		maxGen = 100;
	}
	
	if (maxGen < 0) {
		return descendants;
	}
	
	if (familyRecord == NULL || person == NULL || familyRecord->individuals.length == 0) {
		return descendants;
	}
	
	for (int i = 0; i < maxGen; i ++) {
		List * depthList = malloc(sizeof(List));
		*depthList = initializeList(printIndividual, deleteCopy, compareIndividuals);
		insertBack(&descendants, depthList);
	}
	
	unsigned int * current = malloc(sizeof(unsigned int));
	*current = 0;
	
	if (familyRecord == NULL || person == NULL) {
		return descendants;
	}
		
	recursiveNAscendant(&descendants, (Individual*)person, maxGen, current); 
	
	free(current);
	
	Node * n = descendants.head;
	
	while (n!=NULL) {
		List * x = n->data;
		
		if (x->length == 0) {
			deleteDataFromList(&descendants, x);
			n=descendants.head;
			free(x);
		}
		else {
			n = n->next;
		}
	}
	
	return descendants;
}

/** Function for converting an Individual struct into a JSON string
 *@pre Individual exists, is not null, and is valid
 *@post Individual has not been modified in any way, and a JSON string has been created
 *@return newly allocated JSON string.  May be NULL.
 *@param ind - a pointer to an Individual struct
 **/
char* indToJSON(const Individual* ind) {
	
	if (ind == NULL) {
		return "";
	}
	
	char * returnedString = calloc(2048, sizeof(char));
	strcat(returnedString, "{\"givenName\":\"");
	
	if (ind->givenName == NULL || strlen(ind->givenName) == 0) {
		strcat(returnedString, "\",");
	}
	else {
		strcat(returnedString, ind->givenName);
		strcat(returnedString, "\",");
	}
	
	strcat(returnedString, "\"surname\":\"");
	
	if (ind->surname == NULL || strlen(ind->surname) == 0) {
		strcat(returnedString, "\",");
	}
	else {
		strcat(returnedString, ind->surname);
		strcat(returnedString, "\",");
	}
	
	strcat(returnedString, "\"sex\":\"");
	
	Node * n = ((Individual*)ind)->otherFields.head;
	
	bool foundSex = false;
	
	while (n!=NULL) {
		Field * f = n->data;
		if (strcmp(f->tag, "SEX") == 0) {
			strcat(returnedString, f->value);
			strcat(returnedString, "\",");
			foundSex = true;
			break;
		}
		n=n->next;
	}
	
	if (foundSex == false) {
		strcat(returnedString, "Unknown\",");
	}
	
	strcat(returnedString, "\"familysize\":\"");
	
	int counter = 1;
	
	n = ind->families.head;
	
	for (int i = 0; i < ind->families.length; i ++) {
		if (checkIfSpouse(n->data, (Individual*)ind)) {
			Family * f = n->data;
			if (f->husband!=NULL && f->wife!=NULL) {
				counter++;
			}
			counter += f->children.length;
		}
		n = n->next;
	}
	
	sprintf(returnedString + strlen(returnedString), "%d", counter);
	strcat(returnedString, "\"");
	
	strcat(returnedString, "}");
	
	
	
	return returnedString;
}

/** Function for creating an Individual struct from an JSON string
 *@pre String is not null, and is valid
 *@post String has not been modified in any way, and an Individual struct has been created
 *@return a newly allocated Individual struct.  May be NULL.
 *@param str - a pointer to a JSON string
 **/
Individual* JSONtoInd(const char* str) {
	
	if (str == NULL || strlen(str) == 0) {
		return NULL;
	}
	
	if (strlen(str) < 29) {
		return NULL;
	} 
	
	if (strstr(str, "givenName") == NULL || strstr(str, "surname") == NULL) {
		return NULL;
	} 
	
	Individual * i = initializeIndividual();
	
	int currentTag = 0;
	
	char temp[1024];
	
	strcpy(temp, str);
	
	char * token = strtok(temp, "\",:{}");
	
	while (token != NULL) {
		
		if (currentTag == 0) {
			if (token == NULL || strcmp(token, "givenName") != 0) {
				free(i);
				return NULL;
			}
		}
		else if (currentTag == 1) {
			if (strcmp(token, "surname") != 0) {
				free(i->givenName);
				i->givenName = calloc(1024, sizeof(char));
				strcpy(i->givenName, token);
			}
			else {
				free(i->givenName);
				i->givenName = calloc(1024, sizeof(char));
				strcpy(i->givenName, "");
			}
		}
		else if (currentTag == 2) {
			if (strcmp(token, "surname") != 0) {
				free(i->surname);
				i->surname = calloc(1024, sizeof(char));
				strcpy(i->surname, token);
			}
		}
		else if (currentTag == 3) {
			free(i->surname);
			i->surname = calloc(1024, sizeof(char));
			strcpy(i->surname, token);
		}

		token = strtok(NULL, "\",:{}");
		currentTag++;
	}
	
		
	return i;
}

/** Function for creating a GEDCOMobject struct from an JSON string
 *@pre String is not null, and is valid
 *@post String has not been modified in any way, and a GEDCOMobject struct has been created
 *@return a newly allocated GEDCOMobject struct.  May be NULL.
 *@param str - a pointer to a JSON string
 **/
GEDCOMobject* JSONtoGEDCOM(const char* str) {
	
	if (str == NULL || strlen(str) == 0 || strstr(str, "source") == NULL || strstr(str, "gedcVersion") == NULL || strstr(str, "encoding") == NULL || strstr(str, "subName") == NULL || strstr(str, "subAddress") == NULL) {
		return NULL;
	}
	
	GEDCOMobject * obj = malloc(sizeof(GEDCOMobject));
	
	obj->header = malloc(sizeof(Header));
	obj->header->otherFields = initializeList(printField, deleteField, compareFields);
	obj->submitter = calloc(1, sizeof(Submitter) + 300);
	obj->submitter->otherFields = initializeList(printField, deleteField, compareFields);
	obj->header->submitter = obj->submitter;
	obj->individuals = initializeList(printIndividual, deleteIndividual, compareIndividuals);
	obj->families = initializeList(printFamily, deleteFamily, compareFamilies);
	
	char temp[1024];
	
	strcpy(temp, str);
	
	char * token = strtok(temp, "\",:{}");
	
	while (token != NULL) {
		
		if (strcmp(token, "subName") == 0) {
			token = strtok(NULL, "\",:{}");
			if (token == NULL || strcmp(token, "source") == 0 || strcmp(token, "subName") == 0 || strcmp(token, "subAddress") == 0 || strcmp(token, "gedcVersion") == 0 || strcmp(token, "encoding") == 0) {
				free(obj->header);
				free(obj->submitter);
				free(obj);
				return NULL;
			}
			else {
				if (strcmp(token, "source") == 0 || strcmp(token, "subName") == 0 || strcmp(token, "subAddress") == 0 || strcmp(token, "gedcVersion") == 0 || strcmp(token, "encoding") == 0) {
					free(obj->header);
					free(obj->submitter);
					free(obj);
					return NULL;
				} 
				else if (strlen(token) > 61) {
					free(obj->header);
					free(obj->submitter);
					free(obj);
					return NULL;
				}
				else {
					strcpy(obj->submitter->submitterName, token);
				}
			}
		}
		else if (strcmp(token, "source") == 0) {
			token = strtok(NULL, "\",:{}");
			
			if (token == NULL || strcmp(token, "source") == 0 || strcmp(token, "subName") == 0 || strcmp(token, "subAddress") == 0 || strcmp(token, "gedcVersion") == 0 || strcmp(token, "encoding") == 0) {
					free(obj->header);
					free(obj->submitter);
					free(obj);
					return NULL;
			} 
			else {
				if (strlen(token) > 248) {
					free(obj->header);
					free(obj->submitter);
					free(obj);
					return NULL;
				}
				else {
					strcpy(obj->header->source, token);
				}
			}
		}
		else if (strcmp(token, "gedcVersion") == 0) {
			token = strtok(NULL, "\",:{}");
			
			if (token == NULL || strcmp(token, "source") == 0 || strcmp(token, "subName") == 0 || strcmp(token, "subAddress") == 0 || strcmp(token, "gedcVersion") == 0 || strcmp(token, "encoding") == 0) {
					free(obj->header);
					free(obj->submitter);
					free(obj);
					return NULL;
			} 
			else {
				if (atof(token) == 0) {
					free(obj->header);
				free(obj->submitter);
				free(obj);
					return NULL;
				}
				else {
					obj->header->gedcVersion = atof(token);
				}
			}
		}
		if (strcmp(token, "encoding") == 0) {
			token = strtok(NULL, "\",:{}");
			
			if (token == NULL || strcmp(token, "source") == 0 || strcmp(token, "subName") == 0 || strcmp(token, "subAddress") == 0 || strcmp(token, "gedcVersion") == 0 || strcmp(token, "encoding") == 0) {
					free(obj->header);
					free(obj->submitter);
					free(obj);
					return NULL;
			} 
			else {
				if (strlen(token) > 255) {
					free(obj->header);
					free(obj->submitter);
					free(obj);
					return NULL;
				}
				else {
					if (strcmp(token, "ANSEL") == 0) {
						obj->header->encoding = ANSEL;
					}
					else if (strcmp(token, "UTF-8") == 0) {
						obj->header->encoding = UTF8;
					}
					else if (strcmp(token, "UNICODE") == 0) {
						obj->header->encoding = UNICODE;
					}
					else if (strcmp(token, "ASCII") == 0) {
						obj->header->encoding = ASCII;
					}
					else {
						free(obj->header);
						free(obj->submitter);
						free(obj);
						return NULL;
					}
				}
			}
		}
		if (strcmp(token, "subAddress") == 0) {
			token = strtok(NULL, "\",:{}");
			
			if (token == NULL) {
				strcpy(obj->submitter->address, "");
			}
			else if (strcmp(token, "source") == 0 || strcmp(token, "subName") == 0 || strcmp(token, "subAddress") == 0 || strcmp(token, "gedcVersion") == 0 || strcmp(token, "encoding") == 0) {
				free(obj->header);
				free(obj->submitter);
				free(obj);
				return NULL;
			} 
			else {
				if (strlen(token) > 255) {
					free(obj->header);
					free(obj->submitter);
					free(obj);
					return NULL;
				}
				else {
					strcpy(obj->submitter->address, token);
				}
			}
		}
		
		token = strtok(NULL, "\",:{}");
	}
	
	
	
	return obj;
}

/** Function for adding an Individual to a GEDCCOMobject
 *@pre both arguments are not NULL and valid
 *@post Individual has not been modified in any way, and its address had been added to GEDCOMobject's individuals list
 *@return void
 *@param obj - a pointer to a GEDCOMobject struct
 *@param toBeAdded - a pointer to an Individual struct
**/
void addIndividual(GEDCOMobject* obj, const Individual* toBeAdded) {
	
	if (obj == NULL || toBeAdded == NULL) {
		return;
	}
	
	insertBack(&(obj->individuals), (Individual*)toBeAdded);
	
}

/** Function for converting a list of Individual structs into a JSON string
 *@pre List exists, is not null, and has been initialized
 *@post List has not been modified in any way, and a JSON string has been created
 *@return newly allocated JSON string.  May be NULL.
 *@param iList - a pointer to a list of Individual structs
 **/
char* iListToJSON(List iList) {
	
	
	char * returnedString = calloc(1024, sizeof(char));
	strcpy(returnedString, "[");
	
	if (iList.length == 0) {
		strcat(returnedString, "]");
		return returnedString;
	}
	
	Node * n = iList.head;
	
	char * temp = indToJSON(n->data);
	
	if (temp != NULL) {
		returnedString = realloc(returnedString, (strlen(returnedString) + strlen(temp) + 2) * sizeof(char));
		strcat(returnedString, temp);
		n = n->next;
		free(temp);
	}
	
	for (; n != NULL; n = n -> next) {
		char * temp = indToJSON(n->data);
		if (temp != NULL) {
			strcat(returnedString, ",");
			returnedString = realloc(returnedString, (strlen(returnedString) + strlen(temp) + 2) * sizeof(char));
			strcat(returnedString, temp);
			free(temp);
		}
		
	}
	
	strcat(returnedString, "]");
		
	return returnedString;
}

/** Function for converting a list of lists of Individual structs into a JSON string
 *@pre List exists, is not null, and has been initialized
 *@post List has not been modified in any way, and a JSON string has been created
 *@return newly allocated JSON string.  May be NULL.
 *@param gList - a pointer to a list of lists of Individual structs
 **/
char* gListToJSON(List gList) {
	
	
	Node * n = gList.head;
	
	char * returnedString = calloc(1024, sizeof(char));
	
	strcpy(returnedString, "[");
	
	if (gList.length == 0) {
		strcat(returnedString, "]");
		return returnedString;
	}
	
	char * temp = iListToJSON(*((List*)(n->data)));

	returnedString = realloc(returnedString, ((strlen(returnedString) + 2 + strlen(temp)) * sizeof(char)));
	strcat(returnedString, temp);
	free(temp);
	n = n->next;
	
	while (n != NULL) {
		char * temp = iListToJSON(*((List*)(n->data)));
		if (temp != NULL) {
			strcat(returnedString, ",");
			returnedString = realloc(returnedString, ((strlen(returnedString) + 2 + strlen(temp)) * sizeof(char)));
			strcat(returnedString, temp);
			free(temp);
		}
		
		n = n->next;
	}
	
	strcat(returnedString, "]");
	
	
	return returnedString;
}

void deleteGeneration(void* toBeDeleted) {
	
	if (toBeDeleted == NULL) {
		return;
	}
	
	clearList(toBeDeleted);
	free(toBeDeleted);
}

int compareGenerations(const void* first,const void* second) {
	
	List * one = (List*)first;
	List * two = (List*)second;
	
	if (first == NULL || second == NULL) {
		return -1;
	}
	
	if (one->length > two->length) {
		return 1;
	}
	else if (one->length < two->length) {
		return -1;
	}
	else {
		return 0;
	}
	
}

char* printGeneration(void* toBePrinted) {
	if (toBePrinted == NULL) {
		return "";
	}
	
	char * test = "test\n";
	
	return test;
	
}

char* GEDCOMtoIndiJSON (char * fileName) {
	
	GEDCOMobject * obj = NULL;
	GEDCOMerror err = createGEDCOM(fileName, &obj);
	
	char * temp;
	
	if (err.type != OK) {
		temp = calloc(5, sizeof(char));
		strcpy(temp, "[]");
		return temp;
	}
	
	temp = iListToJSON(obj->individuals);
	
	deleteGEDCOM(obj);
	
	return temp;
}

char* GEDCOMtoObjJSON (char * fileName) {
	
	GEDCOMobject * obj = NULL;
	
	GEDCOMerror err = createGEDCOM(fileName, &obj);
	
	char * returnedString = calloc(1024, sizeof(char));
	strcpy(returnedString, "[");
	
	if (obj == NULL || err.type != OK || obj->submitter == NULL || obj->header == NULL) {
		strcat(returnedString, "]");
		return returnedString;
	}
	
	strcat(returnedString, "{\"source\":\"");
	
	if (strlen(obj->header->source) == 0) {
		strcat(returnedString, "\",");
	}
	else {
		strcat(returnedString, obj->header->source);
		strcat(returnedString, "\",");
	}
	
	strcat(returnedString, "\"gedc\":\"");
	
	sprintf(returnedString + strlen(returnedString), "%.2f", obj->header->gedcVersion);
	strcat(returnedString, "\",");
	
	
	strcat(returnedString, "\"encoding\":\"");
	
	if (obj->header->encoding == ANSEL) {
		strcat(returnedString, "ANSEL");
		strcat(returnedString, "\",");
	}
	else if (obj->header->encoding == UTF8) {
		strcat(returnedString, "UTF-8");
		strcat(returnedString, "\",");
	}
	else if (obj->header->encoding == UNICODE) {
		strcat(returnedString, "UNICODE");
		strcat(returnedString, "\",");
	}
	else if (obj->header->encoding == ASCII) {
		strcat(returnedString, "ASCII");
		strcat(returnedString, "\",");
	}
	else {
		strcat(returnedString, "Unknown");
		strcat(returnedString, "\",");
	}
	
	strcat(returnedString, "\"submittername\":\"");
	
	if (strlen(obj->submitter->submitterName) == 0) {
		strcat(returnedString, "\",");
	}
	else {
		strcat(returnedString, obj->submitter->submitterName);
		strcat(returnedString, "\",");
	}
	
	strcat(returnedString, "\"submitteraddress\":\"");
	
	if (obj->submitter->address == NULL || strlen(obj->submitter->address) == 0) {
		strcat(returnedString, "\",");
	}
	else {
		strcat(returnedString, obj->submitter->address );
		strcat(returnedString, "\",");
	}
	
	strcat(returnedString, "\"indilength\":\"");
	sprintf(returnedString + strlen(returnedString), "%d", obj->individuals.length);
	strcat(returnedString, "\",");
	
	strcat(returnedString, "\"famlength\":\"");
	sprintf(returnedString + strlen(returnedString), "%d", obj->families.length);
	strcat(returnedString, "\"");
	
	strcat(returnedString, "}");
	
	strcat(returnedString, "]");
	
	deleteGEDCOM(obj);
		
	return returnedString;
}

char * addIndi (char * jsonString, char * fileName) {
	
	//add a check to see if it already exists!
	
	Individual * i = JSONtoInd(jsonString);
	
	if (i == NULL) {
		return "FAIL at part 1";
	}
	
	GEDCOMobject * obj = NULL;
	
	GEDCOMerror err = createGEDCOM(fileName, &obj);
	
	if (i == NULL) {
		return "FAIL at part 2";
	}
	
	if (err.type != OK) {
		return "FAIL at part 3";
	}
	
	addIndividual(obj, i);
	
	err = writeGEDCOM(fileName, obj);
	
	if (err.type != OK) {
		return "FAIL at part 4";
	}
	
	deleteGEDCOM(obj);

	return "OK";
}

char * createAndWriteGEDCOM ( char * jsonString, char * fileName) {
	
	GEDCOMobject * obj = JSONtoGEDCOM(jsonString);
	
	if (obj == NULL) {
		return "FAIL 1";
	}
	if (validateGEDCOM(obj) == OK) {
		GEDCOMerror err = writeGEDCOM(fileName, obj);
		
		if (err.type != OK) {
			return "FAIL 2";
		}
	}
	else {
		return "FAIL 3";
	}
	
	deleteGEDCOM(obj);
	
	return "OK";
}

char * getDescJson ( char * fileName, char * givenName, char * surname, char * maxDepth ) {
	
	Individual * i = malloc(sizeof(Individual));
	i->givenName = calloc(strlen(givenName) + 1, sizeof(char));
	i->surname = calloc(strlen(surname) + 1, sizeof(char));
	
	strcpy(i->givenName, givenName);
	strcpy(i->surname, surname);
	
	
	GEDCOMobject * obj = NULL;
		
	GEDCOMerror err = createGEDCOM(fileName, &obj);

	int max = atoi(maxDepth);
	
	
	Individual * indi = findPerson(obj, compareIndiShallow, i);
	
	List descendants = getDescendantListN(obj, indi, max);
	
	char * returnL = gListToJSON(descendants);
	deleteGEDCOM(obj);
	
	return returnL;
}

char * getAncesJson ( char * fileName, char * givenName, char * surname, char * maxDepth ) {
	
	Individual * i = malloc(sizeof(Individual));
	i->givenName = calloc(strlen(givenName) + 1, sizeof(char));
	i->surname = calloc(strlen(surname) + 1, sizeof(char));
	
	strcpy(i->givenName, givenName);
	strcpy(i->surname, surname);
	
	
	GEDCOMobject * obj = NULL;
	
	GEDCOMerror err = createGEDCOM(fileName, &obj);
		
	int max = atoi(maxDepth);
	
	
	Individual * indi = findPerson(obj, compareIndiShallow, i);
	
	List descendants = getAncestorListN(obj, indi, max);
	
	char * returnL = gListToJSON(descendants);
	
	deleteGEDCOM(obj);
	
	return returnL;
}









