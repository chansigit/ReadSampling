#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include <pthread.h>
#include <unistd.h>
using namespace std;
#define READ_NUM 16625077
#define MAX_READ_LEN 90
#define MAX_ID_LEN 50
//Parameter
char inputFile1  [1024]={0};
char inputFile2  [1024]={0};
char outputDir   [1024]={0};
char outputPrefix[1024]={0};
float ratio=0.0;
int sampleSize=0;
int get_parameter(int argc, char** argv){
	char ch;
	//int opterr=0; //set opterr to ZERO to disable error msg
	while ( (ch=getopt(argc,argv,"i:I:d:p:r:n:")) !=-1  ){
		switch(ch){
			case 'i':
				strcpy(inputFile1,optarg);//strcpy(dest,src)
				break;
			case 'I':
				strcpy(inputFile2,optarg);//strcpy(dest,src)
				break;
			case 'd':
				strcpy(outputDir,optarg);//strcpy(dest,src)
				break;
			case 'p':
				strcpy(outputPrefix,optarg);//strcpy(dest,src)
				break;
			case 'r':
				ratio=atof(optarg);
				break;
			case 'n':
				sampleSize=atoi(optarg);
				break;
			default:
				printf("invalid option: %c\n",ch);
		}
	}
	if (optopt=='?')
		printf("option not found.\n");
	if (*inputFile1==0 || *inputFile2==0 || *outputDir==0 || *outputPrefix==0 ||
		ratio<0 || ratio>1 ){
		return 2;
	}
	//fix outputDir ending:  fix /usr/bin/ to /usr/bin
	int dirlen=strlen(outputDir);
	if (outputDir[dirlen-1]=='/') 
		outputDir[dirlen-1]='\0';
	return 0;
}

void print_parameter(){
	puts(inputFile1);
	puts(inputFile2);
	puts(outputDir);
	puts(outputPrefix);
	printf("ratio=%f sampleSize=%d\n",ratio,sampleSize);
}


//Container
char id1 [READ_NUM][MAX_ID_LEN]  ={0};
char seq1[READ_NUM][MAX_READ_LEN]={0};
char id2 [READ_NUM][MAX_ID_LEN]  ={0};
char seq2[READ_NUM][MAX_READ_LEN]={0};
int load_data(){
	FILE* fp;
	char id_buf  [MAX_ID_LEN]  ={0};
	char read_buf[MAX_READ_LEN]={0};
	size_t idx1,idx2=0;
	//----------load file 1-----------
	if ( (fp=fopen(inputFile1,"r"))==NULL ){
		printf("File1 Open Failed!\n");
		return -2;
	}
	idx1=0;
	while (!feof(fp)){
		fgets(id_buf,  MAX_ID_LEN,  fp);
		fgets(read_buf,MAX_READ_LEN,fp);
		strcpy(id1[idx1],    id_buf);
		strcpy(seq1[idx1++], read_buf);
	}
	fclose(fp);
	//----------load file 2-----------
	if ( (fp=fopen(inputFile2,"r"))==NULL ){
		printf("File2 Open Failed!\n");
		return -2;
	}
	idx2=0;
	while (!feof(fp)){
		fgets(id_buf,  MAX_ID_LEN,  fp);
		fgets(read_buf,MAX_READ_LEN,fp);
		strcpy(id2[idx2],    id_buf);
		strcpy(seq2[idx2++], read_buf);
	}
	fclose(fp);

	if (idx1 != idx2){
		printf("File1 and File2 linecnt not match!\n");
		return -1;
	}
	return idx1;
}
void print_container(int containerNbr, int cnt){
	if (containerNbr==1){
		for (int i=0; i!=cnt; ++i){
			if (id1[i][0]==0){
				printf("End Encountered! Terminated!\n");
				return;
			}
			printf("%s",id1[i]);
			printf("%s",seq1[i]);
		}
	}else if (containerNbr==2){
		for (int i=0; i!=cnt; ++i){
			if (id2[i][0]==0){
				printf("End Encountered! Terminated!\n");
				return;
			}
			printf("%s",id2[i]);
			printf("%s",seq2[i]);
		}
	}
}

int N=0,M=0;
int sampleID=0,readID=0;
time_t rawtime;
struct tm * timeinfo;
char tbuffer [128];

void* printProgress(void* arg){
	char progPath[1024]={0};
	sprintf(progPath,"%s/%sreport.log", outputDir,outputPrefix);
	FILE* progressLog=fopen(progPath, "w");

	while (1){
		sleep(5);
		time (&rawtime);
		timeinfo = localtime (&rawtime);
		strftime (tbuffer,sizeof(tbuffer),"%Y/%m/%d %H:%M:%S",timeinfo);
		float percent= float(sampleID*M+readID)/float(N*M)* 100.0;

		fprintf(progressLog, "%s\nSample.%d Read.%d\n%.3f in Total.\n",tbuffer,sampleID,readID, percent);
		fflush(progressLog);
	}
}

int main(int argc, char** argv){
	if (get_parameter(argc,argv)!=0){
		printf("Invalid Option!\n");
	}
	print_parameter();
	N=load_data();
	if (N<0){
		printf("Data Loading Failed!\n");
	}
	print_container(1,10);
	print_container(2,10);
	M=int(N*ratio);
	//----Allocate memory storing acquired idx
	int* offset= (int*)malloc(M*sizeof(int));
	if (!offset){
		printf("Memory allocate failed!\n");
	}

	srand(time(NULL));
	pthread_t progressTID;
	pthread_create(&progressTID, NULL, printProgress, NULL);
	for (sampleID=1; sampleID<=sampleSize; ++sampleID){
		//generate offsetlist
		//make file name
		//joint dir and filename
		//print id1[ *offset[i] ],seq1xxx id2xxx seq2xxx

		//----Sampling idx
		int itr=0,n=N,m=M;
		for (int i=0; i<n; ++i){
			if (rand() % (n-i) < m){
				offset[itr++]=i;
				--m;
			}
		}
		//----open output file
		char filepath[1024]={0};
		sprintf(filepath,"%s/%s%03d.fa", outputDir,outputPrefix,sampleID);
		freopen(filepath,"w",stdout);
		//----write result
		for (readID=0;readID<M;++readID){
			int pos=offset[readID];
			printf("%s",id1[pos]);
			printf("%s",seq1[pos]);
			printf("%s",id2[pos]);
			printf("%s",seq2[pos]);
		}
	}
	free(offset);

	return 0;
}

