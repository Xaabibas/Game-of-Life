#include <ncurses.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

enum {
	success_code = 0,
	failure_code = 1,
	key_escape = 27,
	delay_duration = 200
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
	refresh();	
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
	refresh();
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

void draw(int **generation, int width, int height) {
	int x, y;
	for (x = 0; x < width; x++) {
		for (y = 0; y < height; y++) {
			color(x, y, generation[x][y]);
		}
	}
	refresh();
}

int main() 
{
	int height, width, x, y, key;
	int **generation, **counter;

	initscr();
	getmaxyx(stdscr, height, width);
	
	if (allocate_array(&generation, width, height) || allocate_array(&counter, width, height)) {
		fprintf(stderr, "Something went wrong\n");
		endwin();
		return 1;
	}

	cbreak();
	keypad(stdscr, 1);
	noecho();

	x = width / 2;
	y = height / 2;

	move(y, x);
	refresh();
	
	while ((key = getch()) != key_escape) {
		switch (key) {
			case KEY_UP:
				move_cursor(&x, &y, 0, -1, width, height);
				break;
			case KEY_DOWN:
				move_cursor(&x, &y, 0, 1, width, height);
				break;
			case KEY_RIGHT:
				move_cursor(&x, &y, 1, 0, width, height);
				break;
			case KEY_LEFT:
				move_cursor(&x, &y, -1, 0, width, height);
				break;
			case ' ':
				invert_field(generation, x, y);
		}
	}

	timeout(0);
	curs_set(0);
	while ((key = getch()) != key_escape) {
		draw(generation, width, height);
		calculate_next_generation(generation, counter, width, height);
		napms(delay_duration);
	}

	free_array(&generation, width);
	free_array(&counter, width);
	
	endwin();
	return 0;

}
