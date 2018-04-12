#ifndef GEDCOMUTILITIES_H
#define GEDCOMUTILITIES_H

#include "LinkedListAPI.h"
#include "GEDCOMparser.h"

typedef struct arrayList {
	char ** allLines;
	int size;
} arrayList;

typedef struct famList {
	char address[5];
	Family * reference;
} famList;

typedef struct indiList {
	char address[5];
	Individual * reference;
} indiList;

typedef struct tagList {
	char * level;
	char * tag;
	char * senderAddress;
	char * recieverAddress;
	char * value;
	int location;
	int length;
	int format;
} tagList;

typedef struct container {
	tagList * t;
	int size;
} container;

typedef struct {
	char ** tokenizedString;
	int size;
} stringInformation;

typedef struct {
	char * address;
	List * listPointer;
	Individual ** spousePointer;
	Submitter ** submitterPointer;
	void * initializedPointer;
	int type;
	int count;
} addressInformation;

GEDCOMobject ** initObject (GEDCOMobject ** obj);

bool fileExist (const char * fileName);

GEDCOMerror createError (ErrorCode type, int line);

arrayList initializeArray(FILE * inFile);

tagList * initializeTags(arrayList a, int * arraySize);

stringInformation stringTokenizer(char * input);

char * combineString (char ** string, int first, int second);

void freeTaglist(tagList * t, int size);

void freeStringArray(char ** stringArray, int size);


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
char * myfgets(char *dst, int max, FILE *fp);

GEDCOMerror checkForError(tagList * arr, int size);

void recursiveDescendant(List * descendants, Individual * i);

bool checkIfSpouse (Family * f, Individual * i);

bool checkIfExists(List * l, Individual * i);

bool compareFindPerson(const void* first,const void* second);

Individual * createCopy(Individual * input);

Header * initializeHeader ();

Submitter * initializeSubmitter ();

Individual * initializeIndividual ();

Family * initializeFamily ();

GEDCOMerror parserDistributor (GEDCOMobject * obj, container allInformation, List * sendList, List * recieveList);

GEDCOMerror headParser (GEDCOMobject * obj, tagList * tagInformation, int * currentLocation, List * sendList, List * recieveList);

GEDCOMerror submitterParser (GEDCOMobject * obj, tagList * tagInformation, int * currentLocation, List * sendList, List * recieveList);

GEDCOMerror individualParser (GEDCOMobject * obj, tagList * tagInformation, int * currentLocation, List * sendList, List * recieveList);

GEDCOMerror familyParser (GEDCOMobject * obj, tagList * tagInformation, int * currentLocation, List * sendList, List * recieveList);

Field * initializeField ();

void dummyDelete(void* toBeDeleted);

int dummyCompare(const void* first,const void* second);

char* dummyPrint(void* toBePrinted);

int compareAddress(const void* first,const void* second);

addressInformation * createAddress (List * listPointer, Submitter ** submitterPointer, Individual ** spousePointer, char * address, void * initializedPointer, int count);

bool checkIndividualEvent (char * tag);

bool checkFamilyEvent (char * tag);

Event * initializeEvent ();

bool findAndLink (List addressList, void * data);

void freeLists (List addressList, List recieveList);

char * writeHeader (Header * h);

char * writeSubmitter (Submitter * s);

char * writeIndividual (List individuals, List familyList);

char * writeFamily (List families, List individualList);

List createListFam (List individuals);

List createListIndi (List families);

void deleteVoid (void * data);

bool compareIndiShallow (const void* first,const void* second);

void recursiveNDescendant(List * descendants, Individual * i, unsigned int max, unsigned int * current);

void recursiveNAscendant(List * descendants, Individual * i, unsigned int max, unsigned int * current);

bool checkInList(Individual * i, List * originalList);

bool checkIfChild (Family * f, Individual * i);

bool checkIfExistsInN(List * l, Individual * i);

void deleteNList (void * data);

int comparePointer(const void* first,const void* second);

void deleteCopy (void* toBeDeleted);
#endif
