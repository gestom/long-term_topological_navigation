#include <iostream>
#include <fstream>	
#include <cstdlib>	
#include "CFrelement.h"
#include "CPerGaM.h"
#include "CTimeAdaptiveHist.h"
#include "CTimeHist.h"
#include "CTimeNone.h"
#include "CTimeMean.h"
#include "CTemporal.h"
#include "CNew.h"
#include "CTimer.h"
#define MAX_SIGNAL_LENGTH 1000000

CTemporal *temporalModel;

int trainingTimes[MAX_SIGNAL_LENGTH];
unsigned char trainingStates[MAX_SIGNAL_LENGTH];
int testingTime;
float predictions[MAX_SIGNAL_LENGTH];

int testingLength = 0;
int trainingLength = 0;
int dummyState = 0;
float dummyFloat;
int dummyTime = 0;
int lastTime = 0; 
int lastState = 0; 
int numPaths = 0;
FILE *file,*pathFile;

typedef struct
{
	CTemporal* segmentModel[100];
	int numSegments;
}SPath;

SPath pathArray[100];

int main(int argc,char *argv[])
{
	char pathFileName[1000];
	char segmentFileName[1000];
	char segmentName[1000];

	/*read path info*/
	bool pathAvail = true;
	while (pathAvail){
		sprintf(pathFileName,"%s/path_%i.txt",argv[1],numPaths);
		pathFile=fopen(pathFileName,"r");
		if (pathFile != NULL){
			fprintf(stdout,"Loaded %s\n",pathFileName);
			pathArray[numPaths].numSegments=0;
			while (feof(pathFile)==0){
				fscanf(pathFile,"%s\n",segmentName);
				sprintf(segmentFileName,"%s/%s",argv[1],segmentName);

				trainingLength=0;
				/*read path segments data*/
				file=fopen(segmentFileName,"r");
				lastTime = 0;
				while (feof(file)==0)
				{
					fscanf(file,"%i %f %i\n",&dummyTime,&dummyFloat,&dummyState);
					if (dummyState != lastState || dummyTime > lastTime+60)
					{
						trainingTimes[trainingLength] = dummyTime;
						trainingStates[trainingLength] = dummyState;
						lastState = dummyState; 
						lastTime = dummyTime; 
						trainingLength++;
					}
				}
				fclose(file);


				/*traning the model*/
				if (argv[3][0] == 'I') temporalModel = new CTimeHist(0);
				else if (argv[3][0] == 'A') temporalModel = new CTimeAdaptiveHist(0);
				else if (argv[3][0] == 'F') temporalModel = new CFrelement(0);
				else if (argv[3][0] == 'M') temporalModel = new CTimeMean(0);
				else if (argv[3][0] == 'G') temporalModel = new CPerGaM(0);
				else if (argv[3][0] == 'N') temporalModel = new CNew(0);
				else temporalModel = new CTimeNone(0);

				temporalModel->init(86400,atoi(argv[4]),1);
				for (int i = 0;i<trainingLength;i++){
					temporalModel->add(trainingTimes[i],trainingStates[i]);
				}
				temporalModel->update(atoi(argv[4]));
				temporalModel->print(true);
				pathArray[numPaths].segmentModel[pathArray[numPaths].numSegments++] = temporalModel;
			}
			fclose(pathFile);

			//printf("Path %i contains %i segments\n",numPaths,pathArray[numPaths].numSegments);
			testingLength = 0;
			/*read testing timestamps and make predictions*/
			file=fopen(argv[2],"r");
			while (feof(file)==0){
				fscanf(file,"%i\n",&testingTime);
				predictions[testingLength] = pathArray[numPaths].segmentModel[0]->predict(testingTime);
				for (int i = 1;i<pathArray[numPaths].numSegments;i++) predictions[testingLength]=predictions[testingLength]*pathArray[numPaths].segmentModel[i]->predict(testingTime);
				testingLength++; 
			}
			fclose(file);

			sprintf(pathFileName,"predictions_%i.txt",numPaths);
			file=fopen(pathFileName,"w");
			for (int i =0;i<testingLength;i++) fprintf(file,"%.3f\n",predictions[i]);
			fclose(file);
			numPaths++;
		}else{
			pathAvail = false;
		}
	}
	return 0;
}
