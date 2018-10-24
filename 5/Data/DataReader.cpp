/*
 * Copyright (c) 2017 Technical University of Košice (author: Martin Lojka)
 *
 * This file is part of EAR-TUKE.
 *
 * EAR-TUKE is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * EAR-TUKE is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with EAR-TUKE. If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <string.h>
#include <limits.h>

#include "DataReader.h"
#include "Utils.h"

using namespace std;
using namespace Ear;

CDataHolder::CDataHolder()
{
	mapWords.iSize = 0;
	fst.iSize = 0;
	fst.pNet = NULL;
	am.Pdfs = NULL;
	am.States = NULL;
	mapWords.ppszWords = NULL;
}

CDataHolder::~CDataHolder()
{
	if(mapWords.ppszWords){

		for(unsigned int i = 0; i<mapWords.iSize; i++) {  delete[] mapWords.ppszWords[i]; }
		delete[] mapWords.ppszWords;
	}

	if(am.States){

		for(unsigned int i = 0; i<am.iNumberOfStates; i++) {
			delete[] am.States[i];
			//delete[] am.Active[i];
		}
		delete[] am.States;
		//delete[] am.Active;
	}
	
	if(am.Pdfs){
		for(unsigned int i = 0; i<am.iNumberOfPdfs; i++) {
			delete[] am.Pdfs[i].fVar;
			delete[] am.Pdfs[i].fMean;
		}
		delete[] am.Pdfs;
	}

	if(fst.pNet) delete[] fst.pNet;

	mapStates.clear();
}


unsigned int CDataHolder::load(const char *_szFileName, const char *_szIndexName)
{
	FILE *pf = NULL;
	char szbuf[5000];
	unsigned int ubuf = 0;
	unsigned int i;
	map<unsigned int, unsigned int>::iterator it;

	//read index file
	pf = fopen(_szIndexName, "r");
	if(pf == NULL) return EAR_FAIL;

	//scan whole file
	while(fscanf(pf, "%s\t%u\n", szbuf, &ubuf) == 2)
	{if(ubuf > mapWords.iSize) mapWords.iSize = ubuf;}
	mapWords.iSize++; //because we have words from 0 index, thus one more to add into array

	//seek to the beginning and read all data
	fseek(pf, 0, SEEK_SET);
	mapWords.ppszWords = new char*[mapWords.iSize];
	while(fscanf(pf, "%s\t%u\n", szbuf, &ubuf) == 2)	{mapWords.ppszWords[ubuf] = cloneString(szbuf);}

	fclose(pf);

	//read binary file with acoustic model and transducer
	pf = NULL;
	pf = fopen(_szFileName, "rb");
	if(pf == NULL) return EAR_FAIL;

	//am = new EAR_AM_Info;

	if(fread(&am.iVectorSize, sizeof(unsigned short), 1, pf) != 1) return EAR_FAIL;
	if(fread(&am.iNumberOfStates, sizeof(unsigned int), 1, pf) != 1) return EAR_FAIL;
	if(fread(&am.iNumberOfPdfs, sizeof(unsigned int), 1, pf) != 1) return EAR_FAIL;
	if(fread(&am.iPdfsOnState, sizeof(unsigned int), 1, pf) != 1) return EAR_FAIL;

	am.States = new unsigned int*[am.iNumberOfStates];
	//am.Active = new unsigned int*[am.iNumberOfStates];
	for(i=0;i<am.iNumberOfStates;i++)
	{
		am.States[i] = new unsigned int[am.iPdfsOnState];
		//am.Active[i] = new unsigned int[am.iPdfsOnState];
		if(fread(am.States[i], sizeof(unsigned int), am.iPdfsOnState, pf) != am.iPdfsOnState) return EAR_FAIL;
		//memset(am.Active[i], 0, am.iPdfsOnState * sizeof(unsigned int));
	}

	am.Pdfs = new EAR_AM_Pdf[am.iNumberOfPdfs];
	for(i=0; i<am.iNumberOfPdfs; i++)
	{
		//allocate space for var and mean
		am.Pdfs[i].fVar = new float[am.iVectorSize];
		am.Pdfs[i].fMean = new float[am.iVectorSize];

		if(fread(am.Pdfs[i].fVar, sizeof(float), am.iVectorSize, pf) != am.iVectorSize) return EAR_FAIL;
		if(fread(am.Pdfs[i].fMean, sizeof(float), am.iVectorSize, pf) != am.iVectorSize) return EAR_FAIL;
		if(fread(&am.Pdfs[i].fgconst, sizeof(float), 1, pf) != 1) return EAR_FAIL;
		if(fread(&am.Pdfs[i].fWeight, sizeof(float), 1, pf) != 1) return EAR_FAIL;
	}

	//read fst
	if(fread(&fst.iSize, sizeof(unsigned int), 1, pf) != 1) return EAR_FAIL;
	fst.pNet = new EAR_FST_Trn[fst.iSize];

	for(i=0; i<fst.iSize; i++)
	{
		if(fread(&fst.pNet[i].iStart, sizeof(unsigned int), 1, pf) != 1) return EAR_FAIL;
		if(fread(&fst.pNet[i].iEnd, sizeof(unsigned int), 1, pf) != 1) return EAR_FAIL;
		if(fread(&fst.pNet[i].iIn, sizeof(unsigned int), 1, pf) != 1) return EAR_FAIL;
		if(fread(&fst.pNet[i].iOut, sizeof(unsigned int), 1, pf) != 1) return EAR_FAIL;
		if(fread(&fst.pNet[i].fWeight, sizeof(float), 1, pf) != 1) return EAR_FAIL;
	}

	fclose(pf);

	//reindex iEnd number to array positions
	ubuf = 0; mapStates[ubuf] = 0;
	for(i=0; i<fst.iSize; i++)
	{
		if(fst.pNet[i].iStart != ubuf) {ubuf = fst.pNet[i].iStart; mapStates[ubuf] = i;}
	}

	for(i=0; i<fst.iSize; i++)
	{
		if(fst.pNet[i].iEnd == END_STATE) continue;

		it = mapStates.begin();
		it = mapStates.find(fst.pNet[i].iEnd);
		if(it == mapStates.end()) return EAR_FAIL;
		fst.pNet[i].iEnd = it->second;
	}

	//erase mapStates map
	mapStates.clear();

	return EAR_SUCCESS;
}

EAR_AM_Info *CDataHolder::getAcousticData()
{
	return &am;
}

EAR_FST_Net *CDataHolder::getFSTData()
{
	return &fst;
}

EAR_Dict *CDataHolder::getDict()
{
	return &mapWords;
}
