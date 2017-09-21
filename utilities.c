
void itoa(long long num, char **snum)
{
	int i = 0;
	int j = 0;
	int temp = 0;
	snum[0][i] = '\0';
	while (num > 0) {
		snum[0][i] = num % 10 + '0';
		num /= 10;
		i++;
	}

	while (j < i / 2) {
		temp = snum[0][j];
		snum[0][j] = snum[0][i - j - 1];
		snum[0][i - j - 1] = temp;
		j++;
	}
	snum[0][i] = '\0';
	return;
}

int atoint(char *snum)
{
	int num = 0;
	int i = 0;
	while (snum[i] != '\0') {
		num = num * 10 + (snum[i] - '0');
		i++;
	}
	return num;
}
