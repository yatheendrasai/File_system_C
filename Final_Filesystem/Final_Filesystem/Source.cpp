#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include<string.h>
/*
FIle size --> 104857600 ->100MB
BLOCK SIZE 128 bytes
FILE BLOCK SIZE 2048
Master block 1 block
Bit vector 1280 blocks 128 * 1280 = 163,840 bytes
Users data 1281 block num to 7sb complete next start from 1297th block

Meta message 1297 block no. start total 8192 + 8192 blocks reserved
Message data 17681 block no. start total 8192 + 8192 blocks reserved

File metadata 34065 block no start total	// 128 -> 2kb(2048bytes) block size // total 7082 blocks leaving 112 bytes atlast

*/
#define file_block_size 2048
#define block_size 128 
#define usermeta_start 6401 // users meta data 16 records 
#define messmeta_start 6417 //message metadata
#define mess_data_start 47377 // message block start number
#define filemeta_start 88337 // file meta starting block number 
//

#define file_data_start 170257//file data start block number


struct message{// actual message 
	char message_data[128];

};
struct mess_meta{//message metadata
	char username[16];
	char filename[20];
	long long int offset;

	char pad[84];
};
struct file_meta{//file metadata
	char username[16];
	char filename[20];
	long long int size;
	long long int extra;
	long long int blocks;
	long long int head_offset;
	char pad[60];

};
struct file_data{//file DATA total 2048
	char data[2040];
	long long int next_offset;


};
struct users{
	char username[16];
	char password[16];
	long long int first_meta_messageid;
	long long int last_meta_messageid;
	int pad[20];
};
struct master_block{//master metadata
	//users linked list metadata
	long long int usermeta_head;   // points to the first user
	long long int bitvector_head; // points to the bit vector
	long long int metamess_head; // points to the metamsg
	long long int filemeta_head;   // number count address
	long long int messmeta_count;
	long long int messdata_count;
	long long int user_count;
	long long int file_count;
	int pad[16];

	//files linked list metadata
};
struct bitvector{
	unsigned char bitvector_pos[8];
};
void * allocate(){
	return calloc(1, block_size);
}
void * allocate_file(){
	return calloc(1, file_block_size);
}
//writes into 1 block of memory from given offset using file pointer
void write_block(FILE *fp, int offset, void *memory){
	fseek(fp, offset*block_size, SEEK_SET);
	fwrite(memory, block_size, 1, fp);
}

void write_file_block(FILE *fp, int offset, void *memory){
	int off = (offset - file_data_start)*file_block_size;
	fseek(fp, off + file_data_start*block_size, SEEK_SET);
	fwrite(memory, file_block_size, 1, fp);
}


//returns 1 block of data from given offset using file pointer
void* read_block(FILE *fp, int offset){
	fseek(fp, offset*block_size, SEEK_SET);
	void *memory = allocate();
	fread(memory, block_size, 1, fp);
	return memory;
}

void* read_file_block(FILE *fp, int offset){
	int off = (offset - file_data_start)*file_block_size;
	fseek(fp, off + file_data_start*block_size, SEEK_SET);
	void *memory = allocate_file();
	fread(memory, file_block_size, 1, fp);
	return memory;
}

//initializes bitvector with all zeros
void init_bitv(FILE *fp){
	fseek(fp, block_size, SEEK_SET);
	void *bit = allocate();
	fwrite(bit, block_size, block_size * 10, fp);
}
//returns the empty block number for a given type(ex.msgmeta,msgdata,filemeta,filedata)
int getblock(FILE* fp, int range_offset){

	if (range_offset == messmeta_start){
		int i = 144;
		fseek(fp, i, SEEK_SET);
		unsigned char s;

		fread(&s, 1, 1, fp);

		while (s == 1 && i<41104){

			fread(&s, 1, 1, fp);
			i++;
		}
		unsigned char set = 1;
		fseek(fp, -1, SEEK_CUR);

		fwrite(&set, 1, 1, fp);
		fseek(fp, -1, SEEK_CUR);
		char sp = fgetc(fp);
		return i - 144 + messmeta_start;
	}
	else if (range_offset == mess_data_start){
		int i = 41104;
		fseek(fp, i, SEEK_SET);
		unsigned char s;
		fread(&s, 1, 1, fp);

		while (s == 1 && i<82064){
			fread(&s, 1, 1, fp);
			i++;
		}
		unsigned char set = 1;

		fseek(fp, -1, SEEK_CUR);

		fwrite(&set, sizeof(unsigned char), 1, fp);
		fseek(fp, -1, SEEK_CUR);
		char sp = fgetc(fp);
		return i - 41104 + mess_data_start;
	}
	else if (range_offset == usermeta_start){
		int i = 128;
		fseek(fp, i, SEEK_SET);
		unsigned char s;
		fread(&s, 1, 1, fp);

		while (s == 1 && i<144){
			fread(&s, 1, 1, fp);
			i++;
		}
		unsigned char set = 1;

		fseek(fp, -1, SEEK_CUR);

		fwrite(&set, sizeof(unsigned char), 1, fp);
		fseek(fp, -1, SEEK_CUR);
		char sp = fgetc(fp);
		return i - 128 + usermeta_start;
	}
	else if (range_offset == filemeta_start){
		int i = 82064;
		fseek(fp, i, SEEK_SET);
		unsigned char s;
		fread(&s, 1, 1, fp);

		while (s == 1 && i<163984){
			fread(&s, 1, 1, fp);
			i++;
		}
		unsigned char set = 1;

		fseek(fp, -1, SEEK_CUR);

		fwrite(&set, sizeof(unsigned char), 1, fp);
		fseek(fp, -1, SEEK_CUR);
		char sp = fgetc(fp);
		return i - 82064 + filemeta_start;
	}
	else if (range_offset == file_data_start){
		int i = 163984;
		fseek(fp, i, SEEK_SET);
		unsigned char s;
		fread(&s, 1, 1, fp);

		while (s == 1 && i<204543){  //less or equal??
			fread(&s, 1, 1, fp);
			i++;
		}
		unsigned char set = 1;

		fseek(fp, -1, SEEK_CUR);

		fwrite(&set, sizeof(unsigned char), 1, fp);
		fseek(fp, -1, SEEK_CUR);
		char sp = fgetc(fp);
		return i - 163984 + file_data_start;
	}
}
//reads n writes the user's message into the file and increements count in master block
void create_message(FILE *fp, struct master_block* mastr){
	char username[10], filename[10], message_data[128];
	printf("Enter your username : \n");
	scanf("%s", username);
	printf("Enter your file name : \n");
	scanf("%s", filename);
	printf("Enter your  message : \n");
	scanf("%s", message_data);
	struct mess_meta *mess_temp = (struct mess_meta *)allocate();
	strcpy(mess_temp->filename, filename);
	strcpy(mess_temp->username, username);
	int offset = (getblock(fp, messmeta_start));
	mess_temp->offset = (getblock(fp, mess_data_start));
	write_block(fp, offset, (void*)mess_temp);
	struct message *mess = (struct message*)allocate();
	strcpy(mess->message_data, message_data);
	write_block(fp, mess_temp->offset, (void*)mess);
	mastr->messmeta_count++;
	mastr->messdata_count++;


}
int bit_occupied(FILE* fp, int type){ //type 1: messmeta 2:filemeta
	if (type == 1){
		int a = ftell(fp) / 128;
		int b = messmeta_start;
		a = (a - b) + 144;
		fseek(fp, a, SEEK_SET);
		//printf("\n---------->%d\n", ftell(fp));
		unsigned char s;
		fread(&s, 1, 1, fp);
		if (s == '0'){
			return 0;
		}
		else{
			return 1;
		}
	}
	else 	if (type == 2){
		int a = ftell(fp) / 128;
		int b = filemeta_start;
		a = (a - b) + 82064;
		fseek(fp, a, SEEK_SET);
		//printf("\n---------->%d\n", ftell(fp));
		unsigned char s;
		fread(&s, 1, 1, fp);
		if (s == '0'){
			return 0;
		}
		else{
			return 1;
		}
	}
}
//takes filename as input and prints message
void read_message(FILE* fp, struct master_block* mastr){
	char filename[16], message_data[128], username[20];
	printf("Enter file name : \n");
	scanf("%s", filename);
	printf("Enter file's user name : \n");
	scanf("%s", username);
	int i = mastr->messmeta_count, flag = 0;
	struct mess_meta *mess_temp = (struct mess_meta *)allocate();
	struct message *mess = (struct message*)allocate();
	fseek(fp, messmeta_start*block_size, SEEK_SET);
	while (i){
		if (bit_occupied(fp, 1) == 1){ //for messages type=1
			//printf("\n -before ftell:%d\n", ftell(fp));
			mess_temp = (struct mess_meta*)read_block(fp, messmeta_start + (mastr->messmeta_count - i));
			//printf("\n -after ftell:%d\n", ftell(fp));
			if (
				!(strcmp(mess_temp->filename, filename))
				&&
				!(strcmp(mess_temp->username, username))
				)
			{
				flag++;
				printf("your message is:\n");
				mess = (struct message*)read_block(fp, mess_temp->offset);
				printf("%s\n", mess->message_data);
				//break;
			}
			i--;
		}
		else{

			fseek(fp, 128, SEEK_CUR);
		}
	}
	if (flag == 0){
		printf("\nno message found :-(\n");
	}
}


void upload_file(FILE *fp, struct master_block* mastr){
	char username[10], filename[10], message_data[128];
	printf("Enter your username : \n");
	scanf("%s", username);
	//fflush(stdin);
	printf("Enter your file name : \n");
	scanf("%s", filename);
	//file path also
	struct file_meta *file1 = (struct file_meta *)allocate();
	FILE *inp_fp = fopen(filename, "rb");
	int block_num = 0, byte_count = 2048;
	unsigned char byte;
	if (inp_fp == NULL){
		printf("\nFile not found.\n");
	}
	else{
		strcpy(file1->filename, filename);
		strcpy(file1->username, username);
		block_num = getblock(fp, file_data_start);
		file1->head_offset = block_num;
		fseek(inp_fp, 0, SEEK_END);
		file1->size = ftell(inp_fp);
		int blocks = (file1->size / byte_count);
		int extra = (file1->size % byte_count);
		if (extra != 0){
			blocks++;
		}
		file1->blocks = blocks;
		file1->extra = extra;
		if (extra != 0 && blocks != 1){
			blocks--;
		}
		write_block(fp, getblock(fp, filemeta_start), file1);
		struct file_data *data = (struct file_data *)allocate_file();
		fseek(fp, block_num*block_size, SEEK_SET);
		fseek(inp_fp, 0, SEEK_SET);
		//unsigned char s;
		while (blocks){
			//printf("\n%d\n",ftell(inp_fp));
			//printf("\nafter while:%d\n", ftell(fp));
			for (int i = 0; i < byte_count; i++){
				//fread(&byte, 1, 1, inp_fp);
				byte=fgetc(inp_fp);
				//printf("%c",byte);
				data->data[i] = byte;
			}
			//printf("\n");
			int prev_block = block_num;
			block_num = getblock(fp, file_data_start);
			data->next_offset = block_num;
			write_file_block(fp, prev_block, (void*)data);
			blocks--;
			//printf("\nafter after while:%d\n", ftell(fp));
		}
		if (extra != 0 && file1->blocks != 1){
			for (int i = 0; i < extra; i++){

				fread(&byte, 1, 1, inp_fp);
				data->data[i] = byte;

			}
			data->next_offset = -1;
			write_block(fp, block_num, (void*)data);
		}
		mastr->file_count++;
		printf("FILE UPLOADED SUCCESFULLY!!...");
	}

}
void view_files(FILE *fp, struct master_block* mastr){
	char username[10], filename[10];
	printf("Enter your username : \n");
	scanf("%s", username);
	fseek(fp, mastr->filemeta_head*block_size, SEEK_SET);
	struct file_meta *temp_meta = (struct file_meta *)allocate();
	int count = mastr->file_count, flag = 0;
	while (count)
	{
		if (bit_occupied(fp, 2) == 1){
			temp_meta = (struct file_meta *)read_block(fp, filemeta_start + (mastr->file_count - count));
			if (!(strcmp(temp_meta->username, username)))
			{
				flag++;
				printf("\nfilename:  ");

				printf("%s\n", temp_meta->filename);

			}
			count--;
		}
		else{

			fseek(fp, 128, SEEK_CUR);

		}


	}


}
void download_file(FILE *fp, struct master_block* mastr){
	char username[10], filename[10], message_data[128];
	//printf("Enter your username : \n");
	//scanf("%s", username);
	//fflush(stdin);
	printf("Enter your file name to download: \n");
	scanf("%s", filename);
	FILE *fp_write = fopen("output.txt", "w");

	fseek(fp, mastr->filemeta_head*block_size, SEEK_SET);
	int count = mastr->file_count;
	struct file_meta* temp_meta = (struct file_meta*)allocate();
	struct file_data* file_data = (struct file_data*)allocate_file();
	
	temp_meta = (struct file_meta*)read_block(fp, filemeta_start);
	int meta_num = filemeta_start;
	fseek(fp,170257*block_size, SEEK_SET);
	unsigned char s;
	while (count){
		if (strcmp(temp_meta->filename, filename) == 0){
			int block = temp_meta->blocks;
			int extra = temp_meta->extra;
			char * ext = (char*)malloc(extra);
			int block_num = temp_meta->head_offset;
			if (temp_meta->extra != 0){

				block--;
			}
			while (block--){
				//printf("\nfp_write-->%d\n", ftell(fp_write));
				//printf("\n%d\n",ftell(fp_write));
				//file_data = (struct file_data*)read_file_block(fp, block_num);
				//fwrite(file_data->data, 2040, 1, fp_write);
				//printf("\nfp_write-->%d\n", ftell(fp_write));
				for (int i = 0; i < 2040; i++){
					s = fgetc(fp);
					//fwrite(&s, 1, 1, fp_write);
					fputc(s, fp_write);
					//printf("\n-->%d %d\n",i, ftell(fp_write));
				}
				//printf("\nfp_write-->%d\n", ftell(fp_write));
				fseek(fp_write, 8, SEEK_CUR);
			}
			unsigned char byte;
			while (extra--){
				//printf("\nfp-->%d\n", ftell(fp));
				//printf("\nfp_write-->%d\n", ftell(fp_write));
				s = fgetc(fp);
				if (s != '\r')
					fwrite(&s, 1, 1, fp_write);
			}
			break;

		}
		meta_num = meta_num + block_size;
		temp_meta = (struct file_meta*)read_block(fp, meta_num);
		count--;
	}
	if (count<0)
		printf("\nfile not Downloaded\n");
	else{
		printf("\nDownloaded\n");
	}
}
//type 1-->msg 2-->file
void nullifybit(FILE *fp, void* meta, int type){
	if (type == 1){
		mess_meta* mess_temp = (mess_meta*)meta;
		int a = ftell(fp) / 128;
		int b = messmeta_start;
		a = (a - b) + 144;
		//printf("\na is %d\n", a);
		fseek(fp, a, SEEK_SET);
		unsigned char s = 0;
		fwrite(&s, 1, 1, fp); // changed messmetabit vector to empty again

		a = mess_temp->offset;
		a = (a - mess_data_start) + 41104;
		fseek(fp, a, SEEK_SET);
		fwrite(&s, 1, 1, fp);
	}
	else if (type == 2){
		//for files....
		struct file_meta* temp_meta = (struct file_meta*)meta;
		int a = ftell(fp) / 128;
		int b = filemeta_start;
		a = (a - b) + 82064;
		//printf("\na is %d\n", a);
		fseek(fp, a, SEEK_SET);
		unsigned char s = 0;
		fwrite(&s, 1, 1, fp); // changed filemeta bit vector to empty again

		/*int blocks = temp_meta->blocks,next;
		//file data bits
		a = temp_meta->head_offset;
		a = (a - file_data_start) + 49296;
		fseek(fp, a, SEEK_SET);
		while (blocks--){

		fwrite(&s, 1, 1, fp);
		next=
		fseek(fp, a, SEEK_SET);
		}

		*/
	}
}
void delete_file(FILE *fp, struct master_block* mastr){
	char username[10], filename[10];
	printf("Enter your username to del: \n");
	scanf("%s", username);
	printf("Enter your file name to del: \n");
	scanf("%s", filename);
	int i = mastr->file_count, flag = 0, offset = 0;
	struct file_meta *temp_meta = (struct file_meta *)allocate();
	struct file_data *data = (struct file_data *)allocate_file();
	while (i){

		//printf("\n -before ftell:%d\n", ftell(fp));
		temp_meta = (struct file_meta*)read_block(fp, filemeta_start + (mastr->file_count - i));
		//printf("\n -after ftell:%d\n", ftell(fp));
		if (
			!(strcmp(temp_meta->filename, filename))
			&&
			!(strcmp(temp_meta->username, username))
			){
			int blocks = temp_meta->blocks;
			//data = (struct file_data*)read_block(fp, mess_temp->offset);
			//fseek(fp, offset, SEEK_SET);
			while (blocks--){
				flag++;
				fseek(fp, -block_size, SEEK_CUR);
				nullifybit(fp, temp_meta, 2);//type 2 for files

			}

			mastr->file_count--;
			//mastr->messdata_count--;
			//fseek(fp, offset, SEEK_SET);
			break;

		}
		i--;
	}
	if (flag == 0){
		printf("\nno file to delete\n");
	}
	else
	{
		printf("\n %d File Deleted!\n", flag);
	}
}

void delete_msg(FILE* fp, struct master_block* mastr){
	char username[10], filename[10], message_data[128];
	printf("Enter your username to del: \n");
	scanf("%s", username);
	printf("Enter your file name to del: \n");
	scanf("%s", filename);
	printf("Enter your  message to del: \n");
	scanf("%s", message_data);
	int i = mastr->messmeta_count, flag = 0, offset = 0;
	struct mess_meta *mess_temp = (struct mess_meta *)allocate();
	struct message *mess = (struct message*)allocate();
	while (i){
		//printf("\n -before ftell:%d\n", ftell(fp));
		mess_temp = (struct mess_meta*)read_block(fp, messmeta_start + (mastr->messmeta_count - i));
		//printf("\n -after ftell:%d\n", ftell(fp));
		if (
			!(strcmp(mess_temp->filename, filename))
			&&
			!(strcmp(mess_temp->username, username))
			)
		{
			int offset = ftell(fp);
			mess = (struct message*)read_block(fp, mess_temp->offset);
			fseek(fp, offset, SEEK_SET);
			if (!(strcmp(mess->message_data, message_data))){
				flag++;
				fseek(fp, -block_size, SEEK_CUR);
				nullifybit(fp, mess_temp, 1);
				mastr->messmeta_count--;
				mastr->messdata_count--;
				//fseek(fp, offset, SEEK_SET);
				break;
			}
			//printf("\nmessage deleted!\n");
		}
		i--;
	}
	if (flag == 0){
		printf("\nno message to delete\n");
	}
	else
	{
		printf("\n %d messages deleted\n", flag);
	}
}
void menu(FILE  *fp, struct master_block* mstr){
	int flag = 0, choice = 0;
	while (1){
		printf("enter your choice for messages:\n");

		printf("\n1: Upload File\n2:View Files\n3:Download File\n4:Delete File\n");
		printf("5: create message\n6:Read message\n7:delete message\n8:Exit\n");
		scanf("%d", &choice);
		switch (choice){
		case 1:
			upload_file(fp, mstr);
			break;
		case 2:
			view_files(fp, mstr);

			break;
		case 3:
			download_file(fp, mstr);
			break;
		case 4:
			delete_file(fp, mstr);
			break;
		case 5:
			create_message(fp, mstr);

			break;
		case 6:
			read_message(fp, mstr);

			break;
		case 7:
			delete_msg(fp, mstr);
			break;
		case 8:
			fclose(fp);
			flag = 1;
			printf("------------------------THANK YOU-----------------------------");
			break;
		}
		if (flag == 1)
			break;
	}
}
void sign_up(FILE* fp, struct master_block* mstr){
	char password[16], username[16];
	printf("Enter User name : \n");
	scanf("%s", username);
	printf("Enter password : \n");
	scanf("%s", password);
	struct users *user = (struct users*)malloc(sizeof(struct users));
	strcpy(user->username, username);
	strcpy(user->password, password);
	int empty = (getblock(fp, usermeta_start));

	write_block(fp, empty, (void*)user);
	mstr->user_count++;
	printf("\nSign IN Sucessfull!\n");
}
void sign_in(FILE* fp, struct master_block* mstr){
	char password[16], username[16];
	printf("Enter login User name : \n");
	scanf("%s", username);
	printf("Enter login password : \n");
	scanf("%s", password);
	struct users *user = (struct users*)allocate();
	//fseek(fp, mstr->usermeta_head*block_size, SEEK_SET);
	user = (struct users*)read_block(fp, mstr->usermeta_head);
	int i = mstr->user_count, flag = 0;
	while (i){
		if (strcmp(user->username, username) == 0 && strcmp(user->password, password) == 0){
			flag = 1;

			printf("\n Login Sucessful\n");
			menu(fp, mstr);
			break;
		}
		fseek(fp, block_size, SEEK_CUR);
		i--;
	}
	printf("\n%d", flag);
	if (flag == 0){
		printf("\n User not found please Sign up\n");
	}
}
void login_menu(FILE *fp, struct master_block* mstr){
	int choice = 0, flag = 0;
	while (1){

		printf("\nChoose 1:sign up  2:login 3:Exit\n ");
		scanf("%d", &choice);
		switch (choice){
		case 1:
			sign_up(fp, mstr);
			break;
		case 2:
			sign_in(fp, mstr);
			break;
		case 3:
			fclose(fp);
			flag = 1;
			printf("------------------------THANK YOU-----------------------------");
			break;
		}
		if (flag == 1){
			break;
		}
	}
}
int main(int argc, char *argv[]) {
	int i, choice;
	char username[20], filename[20];

	FILE *fp = fopen("data.dat", "rb+");
	if (fp == NULL){
		perror("\nfile issue\n");
	}
	struct master_block* mstr = (struct master_block*)allocate();
	fread(mstr, 128, 1, fp);
	fseek(fp, 0, SEEK_SET);
	//fseek(fp, 0, SEEK_END);
	
	int p = ftell(fp), flag = 0;
	if (mstr->usermeta_head != 6401)
	{
		init_bitv(fp);
		mstr->bitvector_head = block_size;
		mstr->usermeta_head = 6401;
		mstr->metamess_head = 6417;
		mstr->filemeta_head = 88337;
		mstr->messdata_count = 0;
		mstr->messmeta_count = 0;
		mstr->user_count = 0;
		mstr->file_count = 0;
	}
	login_menu(fp, mstr);
	return 0;
}
