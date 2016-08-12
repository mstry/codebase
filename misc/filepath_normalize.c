#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_NAME_LEN 256

/**
 * strip both-ends blank spaces and tail '/'
 * contiguous '/' or intervaled with blank spaces
 * will be treated as one '/'
 * '.' and '..' will not be treated specially as relative path
 */
static int __fs_path_strip(char *path)
{
	int i, j, k, flag;
	char *p;
	char name[MAX_NAME_LEN] = {0};

	enum path_flag {
		P_FLG_NOP,
		P_FLG_SEP,  // seperator '/'
		P_FLG_VAL,  // valid character
	};

	p = path;
	j = 0, k = 0;
	flag = P_FLG_NOP;
	for (i = 0; path[i] != '\0'; i++) {
		switch (path[i]) {
		case '/':
			if (P_FLG_VAL == flag) {
				strncpy(p, name, k);
				p += k;
			}
			j = 0;
			name[j++] = path[i];
			flag = P_FLG_SEP;
			break;
		case ' ':
			if (P_FLG_VAL == flag) {
				name[j++] = path[i];
			}
			break;
		default:
			name[j++] = path[i];
			k = j;
			flag = P_FLG_VAL;
			break;
		}
	}
	if (P_FLG_VAL == flag) {
		strncpy(p, name, k);
		p += k;
	}
	*p = '\0';

	return 0;
}


int main(int argc, char *argv[])
{
	char path[] = "   /home/peter/   ///hello world/   path   //  //world   //  ";
	printf("org-path: %s\n", path);
	__fs_path_strip(path);
	printf("reg-path: %s\n", path);
	return 0;
}
