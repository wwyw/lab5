#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int show_openlist();
void directory();
void show_help();
void show_ldisk();

void read_block(int,char *);
void write_block(int,char *);
void Init();
int create(char *);
int destroy(char *);
int open(char *);
int close(int);
int read(int,int,int);
int write(int,int,int);
int write_buffer(int,int);
int lseek(int,int);
void Init_block(char,int);

void read_block(int i,char *p)
{
	char * temp = (char *)malloc(sizeof(char));
	temp = p;
	for(int a = 0; a < B;)
	{
		*temp = ldisk[i][a];
		a++;
		temp++;
	}
}

void write_block(int i,char *p)
{
	char * temp = (char *)malloc(sizeof(char));
	temp = p;
	for(int a = 0; a < B;)
	{
		ldisk[i][a] = *temp;
		a++;
		temp++;
	}
}

void Init_block(char *temp,int length)
{
	int i;
	for(i = 0; i < length; i++)
	{
		temp[i] = '\0';
	}
}

int write_buffer(int index,int list)
{

	int i;
	int j;
	int freed;
	char temp[B];

	int buffer_length = BUFFER_LENGTH;
	for(i = 0; i < BUFFER_LENGTH ; i ++)
	{
		if(open_list[list].buffer[i] == '\0')
		{
			buffer_length = i;					//��������Ч���ȡ�
			break;
		}
	}

	int x = open_list[list].pointer[0];
	int y = open_list[list].pointer[1];
	int z = B-y;									//��ǰ��������
//	printf("W:buffer_length: %d  x:%d y:%d z:%d\n",buffer_length,x,y,z);

	if( buffer_length < z )					//��������д�뻺����������Ҫ�ٷ���
	{
		read_block(x,temp);
		strncat(temp+y,open_list[list].buffer,buffer_length);
		//printf("temp[0]: %c\n",temp[0]);
		write_block(x,temp);

		read_block(index+FILE_SIGN_AREA,temp);	//�����ļ����ȡ�
		//printf("temp[1] = %d\n",temp[1]);
		temp[1] += buffer_length;
		//printf("temp[1] = %d\n",temp[1]);
		write_block(index+FILE_SIGN_AREA,temp);

		open_list[list].pointer[0] = x;
		open_list[list].pointer[1] = y + buffer_length ;					//�����ļ���дָ��
	}
	else
	{
		read_block(index+FILE_SIGN_AREA,temp);
		if(temp[2]+(buffer_length - z)/B+1 > FILE_BLOCK_LENGTH)
		{
			printf("�ļ��������鲻������\n");
			return ERROR;
		}

		read_block(x,temp);
		strncat(temp+y,open_list[list].buffer,z);
		write_block(x,temp);

		for(i = 0; i < (buffer_length - z)/B ; i ++ )
		{
			for(j = K + FILE_NUM; j < L ; j++)
			{
				read_block((j-K)/B,temp);
				if(temp[(j-K)%B] == FREE)
				{
					freed = j;
		//			printf("freed= : %d\n",freed);
					break;
				}
			}
			if(j == L)
			{
				printf("������������ʧ��\n");
				return ERROR;
			}

			Init_block(temp,B);
			strncpy(temp,(open_list[list].buffer+z+(i*B)),B);
			write_block(freed,temp);

			read_block((freed-K)/B,temp);
			temp[(freed-K)%B] = BUSY;
			write_block((freed-K)/B,temp);

			read_block(index+FILE_SIGN_AREA,temp);
			temp[2] ++;
			temp[2+temp[2]] = freed;
			write_block(index+FILE_SIGN_AREA,temp);
		}
		for(j = K + FILE_NUM; j < L ; j++)
		{
			read_block((j-K)/B,temp);
			if(temp[(j-K)%B] == FREE)
			{
				freed = j;
				break;
			}
		}
		if(j == L)
		{
			printf("������������ʧ��\n");
			return ERROR;
		}
		Init_block(temp,B);
	//	printf("(buffer_length - z)%B = %d\n",(buffer_length - z)%B);
		strncpy(temp,(open_list[list].buffer+z+(i*B)),(buffer_length - z)%B);
		write_block(freed,temp);

		read_block((freed-K)/B,temp);
		temp[(freed-K)%B] = BUSY;
		write_block((freed-K)/B,temp);

		read_block(index+FILE_SIGN_AREA,temp);
		temp[2] ++;
		temp[2+temp[2]] = freed;
		write_block(index+FILE_SIGN_AREA,temp);


		read_block(index+FILE_SIGN_AREA,temp);
		temp[1] += buffer_length;
		write_block(index+FILE_SIGN_AREA,temp);

		open_list[list].pointer[0] = freed;
		open_list[list].pointer[1] = (buffer_length - z)%B ;
	}
//	printf("X = %d, Y = %d\n",open_list[list].pointer[0],open_list[list].pointer[1]);
}
int lseek(int index,int pos)
{
	int i;
	int list = -1;
	char temp[B];
	int pos_i = pos / B;
	int pos_j = pos % B;
	for(i = 0; i < FILE_NUM ; i++)
	{
		if(open_list[i].filesignnum == index)
		{
			list = i;
			break;
		}
	}

	if(list == -1)
	{
		printf("û�ҵ���ǰ�������ļ�������ʧ��\n");
		return ERROR;
	}
	if(open_list[list].flag != BUSY)
	{
		printf("��������������󣬲���ʧ��\n");
		return ERROR;
	}
	read_block(open_list[list].filesignnum + FILE_SIGN_AREA,temp);
	if(pos_i > temp[2]-1)
	{
		printf("�쳣Խ�磬��λʧ��\n");
		return ERROR;
	}

	open_list[list].pointer[0] = temp[3+pos_i];
	open_list[list].pointer[1] = pos_j;


	return OK;


}


void Init()
{
	int i;
	char temp[B];


	for(i = 0; i < L; i++)
	{
		Init_block(temp,B);
		write_block(i,temp);
	}

	for(i = K; i < L; i ++)
	{
		read_block((i-K)/B,temp);
		temp[(i-K)%B] = FREE;
		write_block((i-K)%B,temp);
	}
	filesign temp_cnt_sign;
	temp_cnt_sign.filesign_flag =1;
	temp_cnt_sign.file_length = 0;
	temp_cnt_sign.file_block = FILE_BLOCK_LENGTH;

	Init_block(temp,B);
	temp[0] = temp_cnt_sign.filesign_flag;
	temp[1] = temp_cnt_sign.file_length;
	temp[2] = temp_cnt_sign.file_block;

	for(i = 0; i < FILE_BLOCK_LENGTH ; i++ )
	{
		temp[i+3] = K+i;
	}
	write_block(FILE_SIGN_AREA,temp);
	read_block(0,temp);
	for(i = 0; i < FILE_NUM ; i++ )
	{
		temp[i] = FREE;
	}
	write_block(0,temp);
}


int create(char filename[])
{
	int i;
	int frees;
	int	freed;
	int freed2;
	char temps[B];
	char tempc[B];
	char temp[B];
	for(i = K ; i < K+FILE_NUM ; i++)
	{
		read_block((i-K)/B,temp);
		if(temp[(i-K)%B] == BUSY)
		{
			read_block(i,temp);
			if(strncmp(temp+1,filename,FILE_NAME_LENGTH) == 0)
			{
				printf("��Ŀ¼�Ѿ������ļ���Ϊ%s���ļ�\n",filename);
				return ERROR;
			}
		}
	}
	for(i = FILE_SIGN_AREA ; i < K; i ++)
	{
		read_block(i,temp);
		if(temp[0] == FREE)
		{
			frees = i;
			break;
		}
	}
	if(i == K)
	{
		printf("û�п��е��ļ�������\n");
		return ERROR;
	}
	for(i = K ; i < K+FILE_NUM ; i++)
	{
		read_block((i-K)/B,temp);
		if(temp[(i-K)%B] == FREE)
		{
			freed = i;
			break;
		}
	}
	if(i == K+FILE_NUM)
	{
		printf("�ļ������Ѵ�����\n");
		return ERROR;
	}
	for(i = K + FILE_NUM; i < L ; i++)
	{
		read_block((i-K)/B,temp);
		if(temp[(i-K)%B] == FREE)
		{
			freed2 = i;
			break;
		}
	}
	if(i == L)
	{
		printf("��������������ʧ��\n");
		return ERROR;
	}
	filesign temp_filesign;
	contents temp_contents;

	temp_filesign.filesign_flag = 1;
	temp_filesign.file_length = 0;
	temp_filesign.file_block = 1;


	Init_block(temps,B);
	temps[0] = temp_filesign.filesign_flag;
	temps[1] = temp_filesign.file_length;
	temps[2] = temp_filesign.file_block;
	temps[3] = freed2;
	for(i = 4 ; i < FILE_BLOCK_LENGTH ; i++)
	{
		temps[i] = '\0';
	}
	write_block(frees,temps);

	temp_contents.filesignnum = frees - FILE_SIGN_AREA;
	strncpy(temp_contents.filename,filename,FILE_NAME_LENGTH);

	Init_block(tempc,B);
	tempc[0] = temp_contents.filesignnum;
	tempc[1] = '\0';
	strcat(tempc,temp_contents.filename);
	write_block(freed,tempc);

	read_block((freed-K)/B,temp);
	temp[(freed-K)%B] = BUSY;
	write_block((freed-K)/B,temp);

	read_block((freed2-K)/B,temp);
	temp[(freed2-K)%B] = BUSY;
	write_block((freed2-K)/B,temp);

	read_block(FILE_SIGN_AREA,temp);
	temp[1]++;
	write_block(FILE_SIGN_AREA,temp);

	return OK;


}

int destroy(char * filename)
{
	int i;
	int dtys;
	int dtyd;
	int use_block;
	int index;
	char temp[B];
	char tempd[B];

	for(i = K ; i < K+FILE_NUM ; i++)
	{
		read_block((i-K)/B,temp);
		if(temp[(i-K)%B] == BUSY)
		{
			read_block(i,temp);
			if(strncmp(temp+1,filename,FILE_NAME_LENGTH) == 0)
			{
				dtyd = i;
				dtys = temp[0] + FILE_SIGN_AREA;
				index = temp[0];
				break;
			}
		}
	}
	if(i == K+FILE_NUM)
	{
		printf("û���ҵ����ļ�\n");
		return ERROR;
	}

	int list = -1;
	for(i = 0; i < FILE_NUM ; i++)
	{
		if(open_list[i].filesignnum == index)
		{
			list = i;
			break;
		}
	}
	if(open_list[list].flag == BUSY && list != -1)
	{
		printf("���ļ��Ѿ����򿪣���Ҫ�رղ���ɾ��\n");
		return ERROR;
	}

	read_block(dtys,temp);
	use_block = temp[2];
	for(i = 0 ; i < use_block ; i++)
	{
		read_block((temp[i+3]-K)/B,tempd);
		tempd[(temp[i+3]-K)%B] = FREE;
		write_block((temp[i+3]-K)/B,tempd);
	}
	Init_block(temp,B);
	write_block(dtys,temp);


	Init_block(temp,B);
	write_block(dtyd,temp);


	read_block((dtyd-K)/B,temp);
	temp[(dtyd-K)%B] = FREE;
	write_block((dtyd-K)/B,temp);

	read_block(FILE_SIGN_AREA,temp);
	temp[1]--;
	write_block(FILE_SIGN_AREA,temp);


	return OK;

}
int open(char * filename)
{
	int i;
	int opd;
	int ops;
	int list;
	char temp[B];
	int index;

	for(i = K ; i < K+FILE_NUM ; i++)
	{
		read_block((i-K)/B,temp);
		if(temp[(i-K)%B] == BUSY)
		{
			read_block(i,temp);
			if(strncmp(temp+1,filename,FILE_NAME_LENGTH) == 0)
			{
				opd = i;
				ops = temp[0] ;
			//	printf("opd: %d,ops: %d\n",opd,ops);
				break;
			}
		}
	}
	if(i == K+FILE_NUM)
	{
		printf("û���ҵ����ļ�\n");
		return ERROR;
	}

	for(i = 0 ; i < FILE_NUM ; i++)
	{
		if(open_list[i].filesignnum == ops && open_list[i].flag == BUSY)
		{
			printf("���ļ��Ѿ�����\n");
			return ERROR;
		}
	}


	for(i = 0 ; i < FILE_NUM ; i++)
	{
		if(open_list[i].flag != BUSY)
		{
			list = i;
			break;
		}
	}


	open_list[list].filesignnum = ops;
	open_list[list].flag = BUSY;

	index = open_list[list].filesignnum;
	lseek(index,0);

	Init_block(open_list[list].buffer,BUFFER_LENGTH);
	read_block(open_list[list].pointer[0],temp);
	strncpy(open_list[list].buffer,temp,BUFFER_LENGTH);

	return OK;

}

int close(int index)
{
	int i;
	int list = -1;
	char temp[B];

	for(i = 0; i < FILE_NUM ; i++)
	{
		if(open_list[i].filesignnum == index)
		{
			list = i;
			break;
		}
	}
	if(list == -1)
	{
		printf("û�ҵ���ǰ�������ļ�������ʧ��\n");
		return ERROR;
	}
	if(open_list[list].flag != BUSY)
	{
		printf("��������������󣬲���ʧ��\n");
		return ERROR;
	}


	write_buffer(index,list);


	Init_block(open_list[list].buffer,BUFFER_LENGTH);
	open_list[list].filesignnum = FREE;
	open_list[list].flag = FREE;
	open_list[list].pointer[0] = NULL;
	open_list[list].pointer[1] = NULL;
	return OK;
}
int read(int index, int mem_area, int count)
{
	int i;
	int list = -1;
	char temp[B];
	for(i = 0; i < FILE_NUM ; i++)
	{
		if(open_list[i].filesignnum == index)
		{
			list = i;
			break;
		}
	}
	if(list == -1)
	{
		printf("û�ҵ���ǰ�������ļ�������ʧ��\n");
		return ERROR;
	}
	if(open_list[list].flag != BUSY)
	{
		printf("��������������󣬲���ʧ��\n");
		return ERROR;
	}

	char temp_output[OUTPUT_LENGTH];
	Init_block(temp_output,OUTPUT_LENGTH);
	char output[OUTPUT_LENGTH];
	Init_block(output,OUTPUT_LENGTH);

	read_block(FILE_SIGN_AREA+index,temp);
	int file_length = temp[1];
	int file_block = temp[2];
	int file_area;

	for(i = 0; i < file_block - 1 ; i++)
	{
		read_block(FILE_SIGN_AREA+index,temp);
		read_block(temp[3+i],temp);
		strncpy(temp_output+i*B,temp,B);
	}
	read_block(FILE_SIGN_AREA+index,temp);
	read_block(temp[3+i],temp);
	strncpy(temp_output+i*B,temp,B);

	int x = open_list[list].pointer[0];
	int y = open_list[list].pointer[1];

	for(i = 0 ; i < file_block ; i++)
	{
		read_block(FILE_SIGN_AREA+index,temp);
		if(temp[3+i] == x)
		{
			break;
		}
	}
	file_area = i * B + y;

	for(i = 0 ;  i < count ; i++)
	{
		output[i+mem_area] = temp_output[i+file_area];
	}

	printf("%s\n",output+mem_area);
	return OK;
}
int write(int index,int mem_area,int count)
{
	int i;
	int list = -1;
	int input_length;
	char temp[B];

	for(i = 0; i < FILE_NUM ; i++)
	{
		if(open_list[i].filesignnum == index)
		{
			list = i;
			break;
		}
	}
	if(list == -1)
	{
		printf("û�ҵ���ǰ�������ļ�������ʧ��\n");
		return ERROR;
	}
	if(open_list[list].flag != BUSY)
	{
		printf("��������������󣬲���ʧ��\n");
		return ERROR;
	}

	char input[INPUT_LENGTH];
	Init_block(input,INPUT_LENGTH);
	i = 0;
	fflush(stdin);
	while(scanf("%c",&input[i]))
	{
		if(input[i] == '\n')
		{
			input[i] == '\0';
			break;
		}
		i++;
	}
	input_length = i;

	if(count <= BUFFER_LENGTH )
	{
		strncat(open_list[list].buffer,input+mem_area,count);
	}
	else
	{
		int rest;
		for(i = 0; i < BUFFER_LENGTH ; i++)
		{
			if(open_list[list].buffer[i] == FREE)
			{
				rest = BUFFER_LENGTH - i;
				break;
			}
		}
		strncat(open_list[list].buffer+BUFFER_LENGTH-rest,input + mem_area,rest);
		write_buffer(index,list);
		Init_block(open_list[list].buffer,BUFFER_LENGTH);
		for(i = 0; i < (count/BUFFER_LENGTH)-1 ; i++)
		{
			strncpy(open_list[list].buffer,(input+mem_area)+rest+i*BUFFER_LENGTH,BUFFER_LENGTH);

			write_buffer(index,list);
			Init_block(open_list[list].buffer,BUFFER_LENGTH);
		}

		Init_block(open_list[list].buffer,BUFFER_LENGTH);
		strncpy(open_list[list].buffer,(input+mem_area)+rest+i*BUFFER_LENGTH,count%BUFFER_LENGTH);
		int buffer_start;
	}
	return OK;
}

void directory()
{
	int i;
	int filenum;
	int filelength;
	char filename[FILE_NAME_LENGTH];
	char temp[B];
	char tempd[B];
	char temps[B];
	read_block(FILE_SIGN_AREA,temp);
	filenum = temp[1];
	printf("\n");
	if(filenum == 0 )
	{
		printf("\t\t\t\t��Ŀ¼��û���ļ�\n");
	}

	for(i = 0; i < FILE_NUM; i++)
	{
		read_block(temp[3+i],tempd);
		if(tempd[0] != 0)
		{
			read_block(tempd[0]+FILE_SIGN_AREA,temps);
			if(temps[0] == BUSY && tempd[0] != 0)
			{
				filelength = temps[1];
				strcpy(filename,tempd+1);
				printf("%.10s\t\t%.2d�ֽ�\n",filename,filelength);
			}
		}
	}

	if(filenum != 0 )
	{
		printf("\t\t\t\t��%d���ļ�\n",filenum);
	}
}

int show_openlist()
{

	int i,j;
	int openfile = 0;
	char temp[B];
	int index;
	printf("\n������\t\t��С\t\t�ļ���\n");
	for(i = 0 ; i < FILE_NUM ; i ++)
	{
		if(open_list[i].flag == BUSY)
		{
			index = open_list[i].filesignnum;
			printf("  %-2d",index);
			openfile++;
			read_block(FILE_SIGN_AREA+index,temp);
			printf("\t\t %-2d",temp[1]);
			for(j = K; j < K+FILE_NUM ; j++)
			{
				read_block(j,temp);
				if(temp[0] == index)
				{
					printf("\t\t%-10s\n",temp+1);
				}
			}
		}
	}
	return openfile;

}
void show_help()
{
	printf("**----------------------------------------------------------------------**\n");
	printf("**    ����                   ˵��                                       **\n");
	printf("** dir                  ��ʾĿ¼����                                    **\n");
	printf("**                                                                      **\n");
	printf("** create+filename      �½���filenameΪ�ļ������ļ�                    **\n");
	printf("**                                                                      **\n");
	printf("** delete+filename      ɾ����filenameΪ�ļ������ļ�                    **\n");
	printf("**                                                                      **\n");
	printf("** open+filename        ����filenameΪ�ļ������ļ�                    **\n");
	printf("**                                                                      **\n");
	printf("** close                �ر�indexΪ�����ŵ��ļ�                         **\n");
	printf("**                                                                      **\n");
	printf("** read                 �������ʾ������Ҫ�������������Լ���ȡ���Ƚ��ж�**\n");
	printf("**                      ȡ�ļ�����                                      **\n");
	printf("**                                                                      **\n");
	printf("** write                �������ʾ������Ҫ�������������Լ�д�볤�Ƚ���д**\n");
	printf("**                      ���ļ�����                                      **\n");
	printf("**                                                                      **\n");
	printf("** lseek                �������ʾ������Ҫ������һ���������ļ����ȵ�����**\n");
	printf("**                      ���ڶ�λ��дָ��                                **\n");
	printf("**                                                                      **\n");
	printf("** help                 ����                                            **\n");
	printf("**                                                                      **\n");
	printf("** exit                 �˳��ļ�ϵͳ                                    **\n");
	printf("**----------------------------------------------------------------------**\n");
}



void show_ldisk()
{
	int a,b;
	for(a = 0; a < K+30 ;a++)
	{
		printf("%-3d :",a);
		for(b = 0; b< B ;b++)
		{
			printf("%-3d ",ldisk[a][b]);
		}
		printf("\n");
	}
}
