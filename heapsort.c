#include <stdio.h>
#include "person.h"
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

//�ʿ��� ��� ��� ���ϰ� �Լ��� �߰��� �� ����

// ���� ������� �����ϴ� ����� ���� �ٸ� �� ������ �ణ�� ������ �Ӵϴ�.
// ���ڵ� ������ ������ ������ ���� �����Ǳ� ������ ����� ���α׷����� ���ڵ� ���Ϸκ��� �����͸� �а� �� ����
// ������ ������ ����մϴ�. ���� �Ʒ��� �� �Լ��� �ʿ��մϴ�.
// 1. readPage(): �־��� ������ ��ȣ�� ������ �����͸� ���α׷� ������ �о�ͼ� pagebuf�� �����Ѵ�
// 2. writePage(): ���α׷� ���� pagebuf�� �����͸� �־��� ������ ��ȣ�� �����Ѵ�
// ���ڵ� ���Ͽ��� ������ ���ڵ带 �аų� ���ο� ���ڵ带 �� ����
// ��� I/O�� ���� �� �Լ��� ���� ȣ���ؾ� �մϴ�. ��, ������ ������ �аų� ��� �մϴ�.

//
// ������ ��ȣ�� �ش��ϴ� �������� �־��� ������ ���ۿ� �о �����Ѵ�. ������ ���۴� �ݵ�� ������ ũ��� ��ġ�ؾ� �Ѵ�.
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
// ������ ������ �����͸� �־��� ������ ��ȣ�� �ش��ϴ� ��ġ�� �����Ѵ�. ������ ���۴� �ݵ�� ������ ũ��� ��ġ�ؾ� �Ѵ�.
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
// �־��� ���ڵ� ���Ͽ��� ���ڵ带 �о� heap�� ����� ������. Heap�� �迭�� �̿��Ͽ� ����Ǹ�, 
// heap�� ������ Chap9���� ������ �˰����� ������. ���ڵ带 ���� �� ������ ������ ����Ѵٴ� �Ϳ� �����ؾ� �Ѵ�.
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
// �ϼ��� heap�� �̿��Ͽ� �ֹι�ȣ�� �������� ������������ ���ڵ带 �����Ͽ� ���ο� ���ڵ� ���Ͽ� �����Ѵ�.
// Heap�� �̿��� ������ Chap9���� ������ �˰����� �̿��Ѵ�.
// ���ڵ带 ������� ������ ���� ������ ������ ����Ѵ�.
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
	FILE *inputfp;	// �Է� ���ڵ� ������ ���� ������
	FILE *outputfp;	// ���ĵ� ���ڵ� ������ ���� ������
	FILE *fp;
	char *headerbuf=malloc(PAGE_SIZE);
	char *pf;
	char **heaparray;

	switch(argv[1][0]){

		case 's':
			if(argc<4){
				fprintf(stderr,"heap sort �ɼǿ� �ش��ϴ� ������� ������ �����մϴ�.\n");
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
