#include "../../includes/win32def.h"
#include "2DAImp.h"
#include "../Core/FileStream.h"

p2DAImp::p2DAImp(void)
{
	str = NULL;
	autoFree = false;
}

p2DAImp::~p2DAImp(void)
{
	if(str && autoFree)
		delete(str);
	for(int i = 0; i < ptrs.size(); i++) {
		free(ptrs[i]);
	}
}

bool p2DAImp::Open(DataStream * stream, bool autoFree)
{
	if(stream == NULL)
		return false;
	if(str && this->autoFree)
		delete(str);
	str = stream;
	this->autoFree = autoFree;
	char Signature[20];
	str->CheckEncrypted();
	str->ReadLine(Signature,20);
	char * strp = Signature;
	while(*strp == ' ')
		strp++;
	if(strncmp(strp, "2DA V1.0", 8) != 0) {
		printf("Bad signature!\n");
		return false;
	}
	defVal[0] = 0;
	str->ReadLine(defVal, 32);
	bool colHead = true;
	int row = 0;
	while(true) {
		char * line = (char*)malloc(1024);
		int len = str->ReadLine(line, 1023);
		if(len <=0) {
			free(line);
			break;
		}
		if(len < 1024)
			line = (char*)realloc(line, len+1);
		ptrs.push_back(line);
		if(colHead) {
			colHead = false;
			char * str = strtok(line, " ");
			while(str != NULL) {
				colNames.push_back(str);
				str = strtok(NULL, " ");
			}
		}
		else {
			char * str = strtok(line, " ");
			if(str==NULL)
				continue;
			rowNames.push_back(str);
			RowEntry r;
			rows.push_back(r);
			while( (str=strtok(NULL, " ")) != NULL) {
				rows[row].push_back(str);
			}
			row++;
		}
	}
	return true;
}
