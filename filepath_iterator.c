#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_NAME_LEN 256

static int __path_iterator(const char *path, char **pos, char *name)
{
	int i = 0;
	char *_pos = *pos;

	if ('\0' == *_pos) {
		name[0] = '\0';
		return 0;
	}

	if (path == _pos && '/' == *_pos) {
		name[i++] = '/';
		_pos++;
	} else {
		while('/' != *_pos && '\0' != *_pos) {
			name[i++] = *_pos;
			_pos++;
		}
		if ('/' == *_pos) _pos++;
	}
	*pos = _pos;
	name[i] = '\0';
	return i;
}

int main(int argc, char *argv[])
{

	char path[] = "/home/peter/path/world";
	char name[MAX_NAME_LEN] = {0};
	char *pos = path;

	while(__path_iterator(path, &pos, name))
	{
		printf("%s\n", name);
	}
	return 0;
}
