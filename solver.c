#include <alloca.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


static void     die        (char *fmt, ...);
static void     display    (uint8_t **pos);
static uint16_t row_options(uint8_t **pos, int idx);
static uint16_t col_options(uint8_t **pos, int idx);
static uint16_t sqr_options(uint8_t **pos, int x, int y);
static uint16_t all_options(uint8_t **pos, int x, int y);
static void     bit_print(uint16_t v);


#define BETWEEN(x,a,b) ((a < x) && (x < b))


void die(char *fmt, ...) {
	va_list ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);

	if (fmt[0] && fmt[strlen(fmt)-1] == ':') {
		fputc(' ', stderr);
		perror(NULL);
	} else {
		fputc('\n', stderr);
	}

	exit(1);
}


void bit_print(uint16_t v) {
	for (int i = 15; i >= 0; --i)
		printf("%u", (v >> i) & 1);
	printf("\n");
}


uint8_t **parse(char *position_str) {
	uint8_t **pos = malloc(9 * sizeof(uint8_t*));
	for (int i = 0; i < 9; i++){
		pos[i] = malloc(9);
		memset(pos[i], 0, 9);
	}

	for (char *c = position_str; *c != '\0'; c++) {
		if (*c != ' ' && c != position_str) 
			continue;

		char *tmp;
		int idx = strtoul(c, &tmp, 10);
		int num = strtoul(++tmp, NULL, 10);
		
		if (!BETWEEN(idx, 0, 82) || !BETWEEN(num, 0, 10))
			die("invalid position string: idx=%d, num=%d", idx, num);

		idx -= 1;
		int x = idx % 9;
		int y = idx / 9;
		pos[x][y] = num;
	}

	/* check if position is valid */
	// for (int i = 0; i<FIELD_SIZE; i++)
	// 	all_options(pos, i);

	return pos;
}


void display(uint8_t **pos) {
	for (int i = 0; i<9; i++) {
		for (int j = 0; j<9; j++)
			printf("|%d", pos[j][i]);
		printf("\n");
	}	
}


uint16_t row_options(uint8_t **pos, int idx) {
	uint16_t opt = 0;
	for (int i = 0; i<9; i++) {
		if (pos[i][idx] == 0)
			continue;
		if ((opt & (1 << pos[i][idx])) != 0) 
			die("invalid position, row");
		opt |= 1 << pos[i][idx];
	}

	return opt;
}


uint16_t col_options(uint8_t **pos, int idx) {
	uint16_t opt = 0;
	for (int i = 0; i<9; i++) {
		if (!pos[idx][i])
			continue;
		if ((opt & (1 << pos[idx][i])) != 0) 
			die("invalid position, col");
		opt |= 1 << pos[idx][i];
	}

	return opt;
}


uint16_t sqr_options(uint8_t **pos, int x, int y) {
	int l_cornerx = x / 3;
	int l_cornery = y / 3;
	uint16_t opt = 0;

	for (int i = 0; i<3; i++)
		for (int j = 0; j<3; j++) {
			int e = pos[l_cornerx*3 + i][l_cornery*3 + j];
			if (!e)
				continue;
			if ((opt & (1 << e)) != 0) 
				die("invalid position, sqr");
			opt |= 1 << e;
	}
	return opt;
}


uint16_t all_options(uint8_t **pos, int x, int y) {
	return ((
		(~sqr_options(pos, x, y)) & (~row_options(pos, y)) & (~col_options(pos, x))
	));
}


void help(void) {
	printf("-s <position string>\n");
	printf("-d -- show initial position\n\n");
	printf("position string: \"<square>,<number> <square>,<number>...\"\n");
}

int solve(uint8_t **pos) {
	int x = -1, y = -1;
	uint16_t opts;

	for (int i = 0; i<9; i++)
		for (int j = 0; j<9; j++) {
			if (pos[j][i] == 0){
				x = j; y = i;
				break;
			}
	}

	printf("start, x=%d, y=%d\n", x,y);
	if (x == -1 || y == -1)
		return 0;

	opts = all_options(pos, x, y);
	bit_print(opts);
	uint16_t mask = ~(((1 << 10) - 1) ^ ((1 << 1) - 1));

	if (opts == mask) {
		return 1;
	}

	for (int j = 9; j >= 1; --j) {
		int num = (opts >> j) & 1;
		if (num == 0) {
			bit_print(num);
			continue;
		}
		pos[x][y] = j;

		display(pos);
		printf("----------------------------\n");

		if (!solve(pos))
			return 0;
	}			

	pos[x][y] = 0;
	return 1;
}


int main(int argc, char* argv[]) {
	uint8_t **pos = NULL;
	int display_position = 0;

	if (argc < 2) {
		help();
		die("supply arguments");
	}

	for (int i = 0; i < argc; i++) {
		if (!strcmp(argv[i], "-s") && (i == argc-2))
			pos = parse(argv[i+1]);
		if (!strcmp(argv[i], "-d")) 
			display_position = 1;
		if (!strcmp(argv[i], "-h")) {
			help();
			exit(0);
		}
	}

	if (!pos) {
		help();
		die("supply position string");
	}

	if (display_position) {
		display(pos);
	}

	if (solve(pos))
		printf("unsolvable\n");
	display(pos);

	return 0;
}
