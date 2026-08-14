#include <ncurses.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

enum {
	success_code = 0,
	failure_code = 1,
	key_escape = 27,
	default_delay_duration = 200
};

void free_array(int ***array, int width)
{
	int i;

	for (i = 0; i < width; i++) {
		free(*(*array + i));
	}
	free(*array);
}

int allocate_array(int ***array, int width, int height) 
{
	int i;
	*array = calloc(width, sizeof(int *));
	if (!*array) {
		return failure_code;
	}
	for (i = 0; i < width; i++) {
		*(*array + i) = calloc(height, sizeof(int));
		if (!*(*array + i)) {
			free_array(array, width);
			return failure_code;		
		}
	}
	return success_code;

}

void clear_array(int **array, int width, int height) 
{
	int i, j;
	for (i = 0; i < width; i++) {
		for (j = 0; j < height; j++) {
			array[i][j] = 0;
		}
	}
}

void move_cursor(int *x, int *y, int dx, int dy, int max_x, int max_y)
{
	*x += dx + max_x;
	*x %= max_x;
	*y += dy + max_y;
	*y %= max_y;
	
	move(*y, *x);
}

void color(int x, int y, int is_alive)
{
	move(y, x);
	if (is_alive) {
		addch(' ' | A_REVERSE);
	} else {
		addch(' ');
	}
	move(y, x);
}

void invert_field(int **generation, int x, int y)
{
	int field = generation[x][y];
	generation[x][y] = field ? 0 :1;
	color(x, y, field ? 0 : 1);
}

void increase(int **counter, int x, int y, int width, int height)
{
	int i, j;
	for (i = -1; i < 2; i++) {
		for (j = -1; j < 2; j++) {
			if (i || j) {
				counter[(width + x + i) % width][(height + y + j) % height]++;
			}
		}
	}
}

void calculate_next_generation(int **generation, int **counter, int width, int height)
{
	int x, y;
	for (x = 0; x < width; x++) {
		for (y = 0; y < height; y++) {
			if (generation[x][y]) {
				increase(counter, x, y, width, height);	
			}
		}
	}
	
	for (x = 0; x < width; x++) {
		for (y = 0; y < height; y++) {
			int tmp = counter[x][y];
			if (generation[x][y]) {
				generation[x][y] =  tmp == 2 || tmp == 3 ? 1 : 0;
			} else {
				generation[x][y] = tmp == 3 ? 1 : 0;
			}
		}
	}
	clear_array(counter, width, height);
}

void draw(int **generation, int width, int height) 
{
	int x, y;
	for (x = 0; x < width; x++) {
		for (y = 0; y < height; y++) {
			color(x, y, generation[x][y]);
		}
	}
}

void random_fill(int **generation, int width, int height, int prob) 
{
	int x, y;
	for (x = 0; x < width; x++) {
		for (y = 0; y < height; y++) {
			if (rand() % 100 < prob) {
				generation[x][y] = 1;
			} else {
				generation[x][y] = 0;
			}
		}
	}
}

int string_to_positive_int(char *str) 
{
	int acc = 0, prev = 0;

	while (*str) {
		if (*str < '0' || *str > '9') {
			return -1;
		}
		prev = acc;
		acc *= 10;
		acc += *str - '0';
		if (acc < 0 || prev > acc) {
			return -1;
		}
		str++;
	}
	return acc;
}

int main(int argc, char **argv) 
{
	int height, width, x, y, key, visual_mode = 0;
	int delay = default_delay_duration;
	int **generation, **counter;
	
	srand(time(NULL));

	initscr();
	getmaxyx(stdscr, height, width);
	
	if (allocate_array(&generation, width, height) || allocate_array(&counter, width, height)) {
		fprintf(stderr, "Something went wrong\n");
		endwin();
		return 1;
	}


	argv++;
	while (*argv) {
		if (0 == strcmp(*argv, "-h") || 0 == strcmp(*argv, "--help")) {
			
		} else if (0 == strcmp(*argv, "-r") || 0 == strcmp(*argv, "--rand")) {
			int prob; 
			argv++;
			prob = string_to_positive_int(*argv);
			if (prob < 0 || prob > 100) {
				fprintf(stderr, "Probability must be not negative integer less than 100\n");
				endwin();
				return 1;	
			}
			random_fill(generation, width, height, prob);
			draw(generation, width, height);
		} else if (0 == strcmp(*argv, "-d") || 0 == strcmp(*argv, "--delay")) {
		
		} else {
		
		}
		argv++;
	}

	cbreak();
	keypad(stdscr, 1);
	noecho();
	
	x = width / 2;
	y = height / 2;

	move(y, x);
	refresh();

	while (1) {
		key = getch();
		if (key == key_escape) {
			break;
		}
		
		if (key == '\n' || key == KEY_ENTER) {
			visual_mode = ~visual_mode + 2;
			if (visual_mode) {
				timeout(0);
				curs_set(0);
			} else {
				timeout(-1);
				curs_set(1);
				move(y, x);
			}
		}

		if (visual_mode) {
			draw(generation, width, height);
			refresh();
			calculate_next_generation(generation, counter, width, height);
			napms(delay);
			continue;
		}
		switch (key) {
			case KEY_UP:
			case 'k':
				move_cursor(&x, &y, 0, -1, width, height);
				break;
			case KEY_DOWN:
			case 'j':
				move_cursor(&x, &y, 0, 1, width, height);
				break;
			case KEY_RIGHT:
			case 'l':
				move_cursor(&x, &y, 1, 0, width, height);
				break;
			case KEY_LEFT:
			case 'h':
				move_cursor(&x, &y, -1, 0, width, height);
				break;
			case ' ':
				invert_field(generation, x, y);
		}
		refresh();
	}

	free_array(&generation, width);
	free_array(&counter, width);
	
	endwin();
	return 0;

}
