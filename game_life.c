#include <ncurses.h>
#include <unistd.h>
#include <stdlib.h>


enum {
	key_escape = 27,
	delay_duration = 200
};

void allocate_array(int ***generation, int row, int col) 
{
	int i;
	*generation = malloc(sizeof(int *) * row);
	
	for (i = 0; i < row; i++) {
		*(*generation + i) = calloc(col, sizeof(int));
	}

}

void free_array(int ***generation, int row)
{
	int i;

	for (i = 0; i < row; i++) {
		free(*(*generation + i));
	}
	free(*generation);
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

int get(int **arr, int x, int y) 
{
	return *(*(arr + y) + x);
}

void set(int **arr, int x, int y, int val)
{
	*(*(arr + y) + x) = val;
}

void inc(int **arr, int x, int y)
{
	*(*(arr + y) + x) += 1;
}

void invert_field(int **generation, int x, int y)
{
	int field = get(generation, x, y);
	set(generation, x, y, field ? 0 :1);
	color(x, y, field ? 0 : 1);
	refresh();
}

void increase(int **counter, int x, int y, int col, int row)
{
	int i, j;
	for (i = -1; i < 2; i++) {
		for (j = -1; j < 2; j++) {
			if (i || j) {
				inc(counter, (col + x + i) % col, (row + y + j) % row);
			}
		}
	}
}

void calculate_next_generation(int **generation, int col, int row)
{
	int i, j;
	int **counter;
	allocate_array(&counter, row, col);
	for (i = 0; i < col; i++) {
		for (j = 0; j < row; j++) {
			if (get(generation, i, j)) {
				increase(counter, i, j, col, row);	
			}
		}
	}
	
	for (i = 0; i < col; i++) {
		for (j = 0; j < row; j++) {
			int tmp = get(counter, i, j);
			if (get(generation, i, j)) {
				set(generation, i, j, tmp == 2 || tmp == 3 ? 1 : 0);
			} else {
				set(generation, i, j, tmp == 3 ? 1 : 0);
			}
		}
	}

	free_array(&counter, row);
}

void draw(int **generation, int col, int row) {
	int x, y;
	for (x = 0; x < col; x++) {
		for (y = 0; y < row; y++) {
			color(x, y, get(generation, x, y));
		}
	}
	refresh();
}

int main() 
{
	int row, col, x, y, key;
	int **generation;

	initscr();
	cbreak();
	keypad(stdscr, 1);
	noecho();

	getmaxyx(stdscr, row, col);
	allocate_array(&generation, row, col);

	x = col / 2;
	y = row / 2;

	move(y, x);
	refresh();
	
	while ((key = getch()) != key_escape) {
		switch (key) {
			case KEY_UP:
				move_cursor(&x, &y, 0, -1, col, row);
				break;
			case KEY_DOWN:
				move_cursor(&x, &y, 0, 1, col, row);
				break;
			case KEY_RIGHT:
				move_cursor(&x, &y, 1, 0, col, row);
				break;
			case KEY_LEFT:
				move_cursor(&x, &y, -1, 0, col, row);
				break;
			case ' ':
				invert_field(generation, x, y);
		}
	}

	timeout(0);
	curs_set(0);
	while ((key = getch()) != key_escape) {
		draw(generation, col, row);
		calculate_next_generation(generation, col, row);
		napms(delay_duration);
	}

	free_array(&generation, row);
	endwin();
	return 0;

}
