#include <stdio.h>
#include "person.h"
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

//필요한 경우 헤더 파일과 함수를 추가할 수 있음

// 과제 설명서대로 구현하는 방식은 각자 다를 수 있지만 약간의 제약을 둡니다.
// 레코드 파일이 페이지 단위로 저장 관리되기 때문에 사용자 프로그램에서 레코드 파일로부터 데이터를 읽고 쓸 때도
// 페이지 단위를 사용합니다. 따라서 아래의 두 함수가 필요합니다.
// 1. readPage(): 주어진 페이지 번호의 페이지 데이터를 프로그램 상으로 읽어와서 pagebuf에 저장한다
// 2. writePage(): 프로그램 상의 pagebuf의 데이터를 주어진 페이지 번호에 저장한다
// 레코드 파일에서 기존의 레코드를 읽거나 새로운 레코드를 쓸 때나
// 모든 I/O는 위의 두 함수를 먼저 호출해야 합니다. 즉, 페이지 단위로 읽거나 써야 합니다.

//
// 페이지 번호에 해당하는 페이지를 주어진 페이지 버퍼에 읽어서 저장한다. 페이지 버퍼는 반드시 페이지 크기와 일치해야 한다.
//

typedef struct _Header
{
	int entire_page;
	int entire_record;
} Header;

Header header;

long long *snarray;

void readPage(FILE *fp, char *pagebuf, int pagenum)
{
	fseek(fp, PAGE_SIZE*pagenum, SEEK_SET);
	fread(pagebuf, PAGE_SIZE, 1, fp);
}

//
// 페이지 버퍼의 데이터를 주어진 페이지 번호에 해당하는 위치에 저장한다. 페이지 버퍼는 반드시 페이지 크기와 일치해야 한다.
//
void writePage(FILE *fp, const char *pagebuf, int pagenum)
{
	fseek(fp, PAGE_SIZE*pagenum, SEEK_SET);
	fwrite(pagebuf, PAGE_SIZE, 1, fp);
}

void pack(char *recordbuf, const Person *p)
{

	sprintf(recordbuf,"%s#%s#%s#%s#%s#%s#", p->sn, p->name, p->age, p->addr, p->phone, p->email);

}


void unpack(const char *recordbuf, Person *p)
{
	char *pf = malloc(RECORD_SIZE);
	char *buf[6];
	char *tmp = malloc(RECORD_SIZE);

	for(int i=0; i<6; i++){
		buf[i]=malloc(30*sizeof(char));
	}

	strcpy(tmp, recordbuf);

	pf=strtok(tmp, "#");
	buf[0]=pf;
	strncpy(p->sn, buf[0], strlen(buf[0]));

	pf=strtok(NULL, "#");
	buf[1]=pf;
	strncpy(p->name, buf[1], strlen(buf[1]));

	pf=strtok(NULL, "#");
	buf[2]=pf;
	strncpy(p->age, buf[2], strlen(buf[2]));

	pf=strtok(NULL, "#");
	buf[3]=pf;
	strncpy(p->addr, buf[3], strlen(buf[3]));

	pf=strtok(NULL, "#");
	buf[4]=pf;
	strncpy(p->phone, buf[4], strlen(buf[4]));

	pf=strtok(NULL,"#");
	buf[5]=pf;
	strncpy(p->email, buf[5], strlen(buf[5]));

}

void swap(char *a, char *b){
	char *tmp;
	tmp=malloc(RECORD_SIZE);

	strcpy(tmp, a);
	strcpy(a, b);
	strcpy(b, tmp);
}

void num_swap(long long *a, long long *b){
	long long tmp;

	tmp=*a;
	*a=*b;
	*b=tmp;
}
//
// 주어진 레코드 파일에서 레코드를 읽어 heap을 만들어 나간다. Heap은 배열을 이용하여 저장되며, 
// heap의 생성은 Chap9에서 제시한 알고리즘을 따른다. 레코드를 읽을 때 페이지 단위를 사용한다는 것에 주의해야 한다.
//
void buildHeap(FILE *inputfp, char **heaparray)
{
	char *pagebuf;
	int pagenum=1;
	int recordnum=0;
	char *headerbuf;
	char *pf;
	char *recordbuf;
	int arraynum=0;
	long long smallest=0;

	Person p;
	memset(&p, 0, sizeof(Person));

	recordbuf=malloc(RECORD_SIZE);
	pagebuf=malloc(PAGE_SIZE);
	headerbuf=malloc(PAGE_SIZE);

	readPage(inputfp, headerbuf, 0);
	pf=headerbuf;

	memcpy(&header.entire_page, pf, sizeof(int));
	pf+=sizeof(int);
	memcpy(&header.entire_record, pf, sizeof(int));
	pf= headerbuf;

	snarray=malloc(sizeof(long long)*header.entire_record);

	memset(recordbuf, (char)0xFF, RECORD_SIZE);
	memset(pagebuf, (char)0xFF, PAGE_SIZE);

	while(pagenum<header.entire_page){
		recordnum=0;
		readPage(inputfp, pagebuf, pagenum);

		while(pagebuf[recordnum*RECORD_SIZE]!=(char)0xFF){
			memcpy(recordbuf , pagebuf+(recordnum*RECORD_SIZE), RECORD_SIZE);
			unpack(recordbuf, &p);

			memcpy(heaparray[arraynum],recordbuf,RECORD_SIZE);

			if(arraynum==1)
				smallest=atoll(p.sn);
			if(smallest>atoll(p.sn))
				smallest=atoll(p.sn);
			snarray[arraynum]=atoll(p.sn);

			if(arraynum>=1&&snarray[(arraynum-1)/2]>snarray[arraynum]){

				swap(heaparray[(arraynum-1)/2], heaparray[arraynum]);
				num_swap(&snarray[(arraynum-1)/2], &snarray[arraynum]);
			}

			recordnum++;
			arraynum++;
		}
		pagenum++;
	}

}

//
// 완성한 heap을 이용하여 주민번호를 기준으로 오름차순으로 레코드를 정렬하여 새로운 레코드 파일에 저장한다.
// Heap을 이용한 정렬은 Chap9에서 제시한 알고리즘을 이용한다.
// 레코드를 순서대로 저장할 때도 페이지 단위를 사용한다.
//
void makeSortedFile(FILE *outputfp, char **heaparray)
{
	char *temp=malloc(RECORD_SIZE);
	long long tmp;
	int root=0;
	int c=1;
	char *pagebuf=malloc(PAGE_SIZE);
	char *recordbuf=malloc(RECORD_SIZE);

	memset(recordbuf, (char)0xFF, RECORD_SIZE);
	memset(pagebuf, (char)0xFF, PAGE_SIZE);


	for(int j=header.entire_record-1; j>=0; j--){
		strcpy(temp,heaparray[0]);
		strcpy(heaparray[0], heaparray[j]);
		strcpy(heaparray[j], temp);

		tmp=snarray[0];
		snarray[0]=snarray[j];
		snarray[j]=tmp;


		root=0;
		c=1;
		do{
			c=2*root+1;
			if(c<j-1&&snarray[c]>snarray[c+1]){
				c++;
			}
			if(c<j&&snarray[root]>snarray[c]){
				tmp=snarray[root];
				snarray[root]=snarray[c];
				snarray[c]=tmp;
				strcpy(temp,heaparray[root]);
				strcpy(heaparray[root], heaparray[c]);
				strcpy(heaparray[c], temp);

			}
			root = c;
		} while(c<j);

	}

	for(int n=0;n<header.entire_record/2;n++){
		strcpy(temp,heaparray[n]);
		strcpy(heaparray[n],heaparray[(header.entire_record-1)-n]);
		strcpy(heaparray[(header.entire_record-1)-n],temp);
	}

	int l=0;

	for(int k=1;k<header.entire_page;k++){
		memset(pagebuf, (char)0xFF, PAGE_SIZE);
		for(int m=0;l<header.entire_record;l++){
			if(m==2)
				break;
			memcpy(pagebuf+m++*RECORD_SIZE,heaparray[l],strlen(heaparray[l]));
		}
		writePage(outputfp, pagebuf, k);
	}

}

int main(int argc, char *argv[])
{
	FILE *inputfp;	// 입력 레코드 파일의 파일 포인터
	FILE *outputfp;	// 정렬된 레코드 파일의 파일 포인터
	FILE *fp;
	char *headerbuf=malloc(PAGE_SIZE);
	char *pf;
	char **heaparray;

	switch(argv[1][0]){

		case 's':
			if(argc<4){
				fprintf(stderr,"heap sort 옵션에 해당하는 명령인자 갯수가 부족합니다.\n");
				exit(1);
			}
			outputfp=fopen(argv[3], "w");
			inputfp=fopen(argv[2], "r");

			readPage(inputfp, headerbuf, 0);
			pf=headerbuf;

			writePage(outputfp, headerbuf, 0);

			memcpy(&header.entire_page, pf, sizeof(int));
			pf+=sizeof(int);
			memcpy(&header.entire_record, pf, sizeof(int));
			pf= headerbuf;

			heaparray=(char**)malloc(sizeof(*heaparray)*header.entire_record);
			for(int i=0; i<header.entire_record; i++){
				heaparray[i]=malloc(RECORD_SIZE);
			}

			buildHeap(inputfp, heaparray);
			makeSortedFile(outputfp, heaparray);
			break;
		default:
			break;
	}

	for(int i=0; i<header.entire_record; i++){
		free(heaparray[i]);
	}
	free(heaparray);

	fclose(inputfp);
	fclose(outputfp);

	exit(0);
}
