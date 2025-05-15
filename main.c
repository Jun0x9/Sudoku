#include "raylib.h" 
#include <stdbool.h>
#include <stdint.h>

#define CELL_WIDTH 64 
#define CELL_HEIGHT 64 
#define ROWS 9
#define COLS 9 
#define FPS 60 
#define PAD 10
#define FONTSIZE 24 
#define FONTSIZE_M 32 
#define MAX_HINTS 40 
#define FOREGROUND BLACK 
#define BACKGROUND WHITE 

typedef enum { MAIN, GAMEOVER, FINISH } GameState ;  
typedef enum { NONE, ERROR, HINT } CellState   ; 

const int WINDOW_WIDTH = CELL_WIDTH * COLS + PAD ;  
const int WINDOW_HEIGHT = CELL_HEIGHT * ROWS + PAD  ; 
const int THICKNESS = 1.0; //line thickness  
const int CELLS = ROWS * COLS; 

char* numbers[9] = {"1", "2", "3", "4", "5", "6", "7", "8", "9"};  
uint8_t number_sizes[9] ; 
uint8_t num_keys[10] ;

struct Board{ 
	uint8_t cells[ROWS][COLS]; 
	uint8_t solution[ROWS][COLS];

	uint8_t hints[ROWS][COLS]; 
	bool errors[COLS][ROWS]; 

	uint8_t states[ COLS ][ ROWS ]; 
	uint8_t cell_x, cell_y; //hovered cell	
	uint8_t cells_left; //cells left to uncover
}board; 

uint8_t InitBoard   (uint8_t hints ); 
bool    Solve	    (uint8_t row, uint8_t col ) ; 
uint8_t RenderBoard ( ); 
bool valid( uint8_t B[ROWS][COLS], uint8_t row, uint8_t col, uint8_t n);

int main( void ){ 
	InitWindow( WINDOW_WIDTH, WINDOW_HEIGHT, "SUDOKU"); 
	SetTargetFPS( 60 ); 

	Vector2 mouse_pos ; 
	uint8_t cell_x, cell_y;

	//KEY_N
	for(int i =0 ; i < 10; i++) 
		num_keys[i] = KEY_ZERO + i; 
	for(int i = 1; i < 9; i++) 
		number_sizes[i-1] = MeasureText( numbers[ i ] , FONTSIZE ) ; 
	
	InitBoard( MAX_HINTS ); 
	GameState state = MAIN;  
	
	while(!WindowShouldClose()){ 
		mouse_pos = GetMousePosition(); 
		cell_x    = (int)mouse_pos.x / CELL_WIDTH; 
		cell_y    = (int)mouse_pos.y / CELL_HEIGHT; 
		board.cell_x = cell_x; 
		board.cell_y = cell_y; 

		//hovered cell
		switch( state ) { 
			case MAIN:
				for(int i =0; i < 9; i++){ 
					if(IsKeyPressed( num_keys[ i ] )){ 
						board.cells[cell_y][cell_x] = i; 
						if( i != board.solution[cell_y][cell_x] ) { 
							//FIXXX
							board.errors[cell_y][cell_x] = true;
							board.states[cell_y][cell_x] ^= ERROR ; 
							board.cells_left--;
							
						}
						else
							board.errors[cell_y][cell_x] = false;
					}
				}	
				BeginDrawing(); 
					ClearBackground( BACKGROUND ); 
					RenderBoard(); 	
				EndDrawing(); 
			break; 

			case GAMEOVER:
				break; 
			case FINISH: 
				break;

		}
	}
	return 0 ; 
}

uint8_t RenderBoard () { 
	for(int i=1; i < ROWS; i++){ 
		DrawLineEx( (Vector2){PAD, i * CELL_HEIGHT}, (Vector2){WINDOW_WIDTH - PAD , i * CELL_HEIGHT}, THICKNESS, FOREGROUND ); 	
		DrawLineEx( (Vector2){i * CELL_WIDTH + PAD, PAD}, (Vector2){i * CELL_WIDTH + PAD, WINDOW_HEIGHT - PAD}, THICKNESS , FOREGROUND); 
		if( i % 3 == 0){
			DrawLineEx((Vector2){CELL_WIDTH * i + PAD, PAD}, (Vector2){CELL_WIDTH * i + PAD, WINDOW_HEIGHT-PAD}, 4.0, FOREGROUND); 
			DrawLineEx((Vector2){PAD, CELL_HEIGHT * i}, (Vector2){WINDOW_WIDTH-PAD, CELL_HEIGHT * i}, 4.0, FOREGROUND); 
		}

	}
	DrawLineEx( (Vector2){PAD, PAD}, (Vector2){WINDOW_WIDTH-PAD, PAD}, 4.0,  FOREGROUND); 
	DrawLineEx( (Vector2){PAD, WINDOW_HEIGHT - PAD}, (Vector2){WINDOW_WIDTH - PAD , WINDOW_HEIGHT - PAD}, 4.0,  FOREGROUND); 
	DrawLineEx( (Vector2){PAD, PAD}, (Vector2){PAD, WINDOW_HEIGHT-PAD}, 4.0, FOREGROUND);
	DrawLineEx( (Vector2){WINDOW_WIDTH - PAD , PAD}, (Vector2){WINDOW_WIDTH - PAD , WINDOW_HEIGHT-PAD}, 4.0, FOREGROUND);

	//renders the numbers 
	for(int y= 0; y < ROWS; y++){ 
		for(int x=0; x < COLS; x++){ 
			Color color = FOREGROUND; 
			int   size  = FONTSIZE; 
			if(board.cells[y][x]){ 
				//renders the hovered cell
				if(board.cell_x == x && board.cell_y == y){ 
					color = GREEN;
					size  = FONTSIZE + 4; 
				}
				if( board.states[y][x] & ERROR ){
					color = RED; 
					size = FONTSIZE + 4; //make it slightly big
				}

				int index = board.cells[y][x] - 1;
				DrawText(numbers[index],  
						x * CELL_WIDTH + PAD + ( CELL_WIDTH / 2.0f) - (number_sizes[index] / 2.0f), 
						y * CELL_HEIGHT + PAD + 16,
						size, color);
			}
		}
	}

	return 0; 
}

uint8_t InitBoard  ( uint8_t hints ){ 	
	for(int i = 0; i < ROWS ; i++){ 
		for(int j = 0; j < COLS; j++){ 
			board.cells [i][j] = 0 ; 
			board.states[i][j] = NONE ; 
		}
	}

	Solve( 0 , 0 ); 
	//copy the solution 
	for(int i = 0; i < ROWS; i++){ 
		for(int j = 0; j < COLS; j++){ 
			board.cells[i][j] = board.solution[i][j]; 
		}
	}

	for(int i = 0; i < hints; i++){ 
		uint8_t row 	  = GetRandomValue( 0, 8 ); 
		uint8_t col     = GetRandomValue( 0, 8 ); 
		while(board.hints[col][row]){ 
			row = GetRandomValue( 0, 8 ); 
			col = GetRandomValue( 0, 8 ); 
		}
		// FIXXX
		board.hints[col][row] = 1; 
		board.states[col][row] ^= HINT; 
	}

	for(int i = 0; i < ROWS; i++){ 
		for(int j = 0; j < COLS; j++){ 
		  // FIXXX
			board.cells[i][j] *= board.hints[i][j]; 
		}
	}
	board.cells_left = CELLS - hints ; 
	return 0;
}

bool valid( uint8_t B[ROWS][COLS], uint8_t row, uint8_t col, uint8_t n){ 
	if(n == 0) return true;
	for(uint8_t i =0; i < COLS; i++){ 
		int valid_col = (B[row][i] == n) && i != col;
		int valid_row = (B[i][col] == n) && i != row;
		if(valid_col || valid_row) 
			return false;
	}

	int start_row = (int)(row/ 3 )* 3; 
	int start_col = (int)(col/ 3 )* 3;
	for (int i = 0; i < 3; i++){ 
		for(int j = 0; j < 3; j++){ 
			if(B[start_row + i][start_col + j] == n && \
					(start_row + row != start_row + i) && (start_col + col != start_col + i))
				return false; 
		}
	}
	return true; 
}

bool Solve(uint8_t row, uint8_t col ){ 
	// O(scary) time complexity 
	// calculates which 3x3 grid you are at (0-8) 
	int grid_x = (int)(col / 3) * 3; 
	int grid_y = (int)(row / 3) * 3;

	if( row == 9 ) 
		//finishes
		return true; 
	else if( col == 9 )  
		return Solve( row + 1, 0 ); 
	else if( board.solution[row][col] != 0) 
		return Solve( row, col+1); 

	else { 
		int sample[9] = {1, 2, 3, 4, 5, 6, 7, 8, 9};  
		// shuffles the values 
		for(int i = 0; i < 9; i++){ 
			int x1 = GetRandomValue(0, 8);
			int x2 = GetRandomValue(0, 8);
			int temp = sample[x1] ; 
			sample[x1] = sample[x2];
			sample[x2] = temp; 
		}

		for(int i = 0; i < 9; i++){ 
			if( valid(board.solution, row, col, sample[i])){ 
				//valid number, fills
				board.solution[row][col] = sample[i]; 
				if( Solve( row, col + 1))
					return true;
				//reset all the previous entries
				board.solution[row][col] = 0; 
			}
		}
		return false; 
	}
}

