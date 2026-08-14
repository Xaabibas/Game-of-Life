#include <limits.h>
#include <ncurses.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

const char *help_message = "LAUNCH:\n\r"
                           "\t./game_life [-h | --help] [-r | --rand <value>] [-d | --delay <value>]\n\r"
                           "\n\r"
			   "DESCRIPTION:\n\r"
			   "\tThis program is a simple implementation of Conway's Game of Life on a torus\n\r"
			   "\tLearn more: https://en.wikipedia.org/wiki/Conway's_Game_of_Life\n\r"
			   "\n\r"
                           "OPTIONS:\n\r"
                           "\t-h, --help\n\r"
                           "\t\tPrint reference information about program\n\r"
			   "\n\r"
                           "\t-r <value>, --rand <value>\n\r"
                           "\t\tFill the torus with randomly placed live cells\n\r"
			   "\t\t<value> specifies the probability of a cell being alive\n\r"
			   "\t\t<value> must be a non-negative integer no greater than 100\n\r"
			   "\n\r"
                           "\t-d <value>, --delay <value>\n\r"
                           "\t\tSet the delay between generations in milliseconds\n\r"
			   "\t\t<value> specifies the delay duration\n\r"
			   "\t\t<value> must be a positive integer\n\r"
			   "\n\r"
			   "USAGE:\n\r"
			   "\tThis program has two modes:\n\r"
			   "\t  1. Visual mode - run the simulation\n\r"
			   "\t  2. Edit mode - edit the field and place live cells\n\r"
			   "\t     Move the cursor with the arrow key or \"h\", \"j\", \"k\", \"l\"\n\r"
			   "\t     Press Space to toggle a cell\n\r"
			   "\tPress Enter to switch between modes\n\r"
			   "\tPress Escape to exit the program\n\r";

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

void update_cursor_position(int *x, int *y, int dx, int dy, int max_x, int max_y)
{
	*x += dx + max_x;
	*x %= max_x;
	*y += dy + max_y;
	*y %= max_y;
	
	move(*y, *x);
}

void draw_cell(int x, int y, int is_alive)
{
	move(y, x);
	if (is_alive) {
		addch(' ' | A_REVERSE);
	} else {
		addch(' ');
	}
	move(y, x);
}

void invert_cell(int **generation, int x, int y)
{
	int cell = generation[x][y];
	generation[x][y] = cell ? 0 : 1;
	draw_cell(x, y, cell ? 0 : 1);
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
			draw_cell(x, y, generation[x][y]);
		}
	}
}

void random_fill(int **generation, int width, int height, int prob) 
{
	int x, y;
	for (x = 0; x < width; x++) {
		for (y = 0; y < height; y++) {
			if ((int)(100.0 * rand() / (RAND_MAX + 1.0)) < prob) {
				generation[x][y] = 1;
			} else {
				generation[x][y] = 0;
			}
		}
	}
}

int parse_int(char *str) 
{
	int acc = 0, digit;
	
	if (*str == '\0') {
		return -1;
	}

	while (*str) {
		if (*str < '0' || *str > '9') {
			return -1;
		}
		digit = *str - '0';
		
		if (acc > INT_MAX / 10) {
			return -1;
		}
		acc *= 10;
		if (acc > INT_MAX - digit) {
			return -1;
		}
		acc += digit;
		str++;
	}
	return acc;
}

int main(int argc, char **argv) 
{
	int height, width, x, y, key, visual_mode = 0;
	int delay = default_delay_duration;
	int r_count = 0, d_count = 0;
	int **generation, **counter;
	
	srand(time(NULL));

	initscr();
	getmaxyx(stdscr, height, width);
	
	if (allocate_array(&generation, width, height)) {
		fprintf(stderr, "Something went wrong\n");
		endwin();
		return 1;
	}
	if (allocate_array(&counter, width, height)) {
		free_array(&generation, width);
		fprintf(stderr, "Something went wrong\n");
		endwin();
		return 1;
	}


	argv++;
	while (*argv) {
		if (0 == strcmp(*argv, "-h") || 0 == strcmp(*argv, "--help")) {
			fprintf(stdout, "%s", help_message);
			endwin();
			return 0;
		} else if (0 == strcmp(*argv, "-r") || 0 == strcmp(*argv, "--rand")) {
			int prob;
			char *option = *argv;
			argv++;
			
			if (!*argv) {
				fprintf(stderr, "ERROR - option \"%s\" requires an argument\n\r", option);
				endwin();
				return 1;
			}
			if (r_count++) {
				fprintf(stderr, "ERROR - you can't fill randomly twice\n\r");
				endwin();
				return 1;
			}

			prob = parse_int(*argv);
			if (prob < 0 || prob > 100) {
				fprintf(stderr, "ERROR - probability must be not negative integer " 
						"no more than 100: \"%s\"\n\r", *argv);
				fprintf(stderr, "You can use \"--help\" to get more information\n");
				endwin();
				return 1;	
			}
			random_fill(generation, width, height, prob);
			draw(generation, width, height);
		} else if (0 == strcmp(*argv, "-d") || 0 == strcmp(*argv, "--delay")) {
			char *option = *argv;
			argv++;
				
			if (!*argv) {
				fprintf(stderr, "ERROR - option \"%s\" requires an argument\n\r", option);
				endwin();
				return 1;
			}
			if (d_count++) {
				fprintf(stderr, "ERROR - you can't set delay duration twice\n\r");
				endwin();
				return 1;
			}

			delay = parse_int(*argv);
			if (delay < 1) {
				fprintf(stderr, "ERROR - delay duration must be positive "
						"integer: \"%s\"\n\r", *argv);
				fprintf(stderr, "You can use \"--help\" to get more information\n");
				endwin();
				return 1;
			}
		} else {
			fprintf(stderr, "ERROR - invalid argument: \"%s\"\n\r", *argv);
			fprintf(stderr, "You can use \"--help\" to get more information\n");
			endwin();
			return 1;
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
			visual_mode = !visual_mode;
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
				update_cursor_position(&x, &y, 0, -1, width, height);
				break;
			case KEY_DOWN:
			case 'j':
				update_cursor_position(&x, &y, 0, 1, width, height);
				break;
			case KEY_RIGHT:
			case 'l':
				update_cursor_position(&x, &y, 1, 0, width, height);
				break;
			case KEY_LEFT:
			case 'h':
				update_cursor_position(&x, &y, -1, 0, width, height);
				break;
			case ' ':
				invert_cell(generation, x, y);
		}
		refresh();
	}

	free_array(&generation, width);
	free_array(&counter, width);
	
	endwin();
	return 0;

}
