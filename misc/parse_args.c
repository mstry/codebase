static int __parse_args(const char *args, char **argv, int *argc, int argl)
{
	int i = 0, j = 0;
	const char *p = args;
	char *_argv = (char *)argv;
	enum flag_args {
		F_ARG_NOP,
		F_ARG_SPC,
		F_ARG_VAL
	} flag;

	flag = F_ARG_NOP;
	while ('\0' != *p) {
		if (' ' == *p) {
			if (F_ARG_VAL == flag) {
				*(_argv + argl * i + j) = '\0';
				i++;
				if (i >= *argc) break;
				j = 0;
				flag = F_ARG_SPC;
			}
		} else {
			*(_argv + argl * i + j) = *p;
			j++;
			if (j >= argl) break;
			flag = F_ARG_VAL;
		}
		p++;
	}
	if (0 == i && 0 == j)
		*argc = 0;
	else
		*argc = i + 1;
	return 0;
}
