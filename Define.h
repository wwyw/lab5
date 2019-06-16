#define B		10
#define L		500
#define K		100
#define BUSY	1
#define FREE	0
#define OK		1
#define ERROR	0
#define FILE_BLOCK_LENGTH		(B-3)
#define FILE_NAME_LENGTH		(B-1)
#define FILE_SIGN_AREA			((L-1-K)/B+1)
#define FILE_NUM				FILE_BLOCK_LENGTH
#define BUFFER_LENGTH			25
#define INPUT_LENGTH			100
#define OUTPUT_LENGTH			100



struct filesign{
	int file_length;
	int filesign_flag;
	int file_block;
	int file_block_ary[FILE_BLOCK_LENGTH];
};

struct contents{
	char filename[FILE_NAME_LENGTH];
	int	 filesignnum;
};

struct openfilelist{
	char buffer[BUFFER_LENGTH];
	int pointer[2];
	int filesignnum;
	int flag;
};

char ldisk[L][B];
openfilelist open_list[FILE_NUM];
