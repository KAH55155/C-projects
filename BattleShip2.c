#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>


enum ShipType { 
    NOSHIP = 0, 
    CARRIER = 1, 
    BATTLESHIP = 2, 
    CRUISER = 3, 
    SUBMARINE = 4, 
    DESTROYER = 5 
};

enum ShotResult { 
    Open = 0, 
    HIT = 1, 
    MISS = 2
};

const char *shipNames[] = { "None", "Carrier", "Battleship", "Cruiser", "Submarine", "Destroyer" };
const int shipSizes[] = { 0, 5, 4, 3, 2, 1 };

const char *shipToken[]  = { "  ", "CV", "BB", "CA", "SS", "DD" };

enum ShipType **shipGrid = NULL;
enum ShotResult **shotGrid = NULL;

void Initialize(void);
void PlaceAllShips(void);
bool PlaceOneShip(enum ShipType s);
void DisplayBoard(void);
void Teardown(void);
void remove_newline(char *s);

void Initialize(void) {
    shipGrid = malloc(10 * sizeof(enum ShipType *));
    shotGrid = malloc(10 * sizeof(enum ShotResult *));
    if (!shipGrid || !shotGrid) {
        fprintf(stderr, "The memory allocation has failed.\n");
        exit(EXIT_FAILURE);
    }

    for (int row = 0; row < 10; ++row) {
        shipGrid[row] = malloc(10 * sizeof(enum ShipType));
        shotGrid[row] = malloc(10 * sizeof(enum ShotResult));
        if (!shipGrid[row] || !shotGrid[row]) {
            fprintf(stderr, "The memory allocation has failed.\n");
            exit(EXIT_FAILURE);
        }
        
        //Empty's the grid before start
        for (int column = 0; column < 10; ++column) {
            shipGrid[row][column] = NOSHIP;
            shotGrid[row][column] = Open;
        }
    }

    printf("Welcome to Battleship!\n");
    printf("Horizontal format: for example J59 means row J, columns 5 through 9)\n");
    printf("Vertical format: for example CF4 means rows C through F, column 4)\n");
}

void remove_newline(char *s) {
    if (!s){
        return;
    } 
    char *p = strchr(s, '\n');
    if (p){
        *p = '\0';
    }
}

void DisplayBoard(void) {

    printf("\n   ");
    for (int column = 0; column < 10; ++column) {
        printf("%2d  ", column);
    }
    printf("\n\n");

    for (int row = 0; row < 10; ++row) {
        printf("%c  ", 'A' + row);
        for (int column = 0; column < 10; ++column) {
            if (shipGrid[row][column] == NOSHIP) {
                printf(" .  ");
            } else {
                printf("%s  ", shipToken[shipGrid[row][column]]);
            }
        }
        printf("\n\n");
    }
}

bool PlaceOneShip(enum ShipType s) {
   
    //gets the length of the ship that is being placed 
    char input[32];
    int size = shipSizes[s];

    printf("Please enter a location for a ship of %d squares (%s): ", size, shipNames[s]);
    if (!fgets(input, sizeof(input), stdin)) {
        printf("There was an error with your input. Please try again.\n");
        return false;
    }
    remove_newline(input);
    
    //accepts lowercase letetrs
    for (size_t i = 0; i < strlen(input); ++i) {
        input[i] = toupper((unsigned char)input[i]);
    }

    if (strlen(input) != 3) {
        printf("INCORRECT FORMAT. Examples: C37(for ships on a row) or CF4(for ships on a column).\n");
        return false;
    }

    //Scenario if 2nd and 3rd characters are numbers
    if (isdigit((unsigned char)input[1]) && isdigit((unsigned char)input[2])) {
        
        char rowCharacter = input[0];
        int column1 = input[1] - '0';
        int column2 = input[2] - '0';
        if (rowCharacter < 'A' || rowCharacter > 'J' || column1 < 0 || column1 > 9 || column2 < 0 || column2 > 9) {
            printf("That spot is not in bounds of the grid.\n");
            return false;
        }
        //calculates the length of the vertical ship
        int r = rowCharacter - 'A';
        int columnmin = column1;
        int columnmax = column2;
        if(column1>column2){
            columnmin = column2;
            columnmax = column1;
        }
        int length = columnmax - columnmin + 1;
        if (length != size) {
            printf("Your entry has a size of %d , but should be a size of %d.\n", length, size);
            return false;
        }

        for (int c = columnmin; c <= columnmax; ++c) {
            if (shipGrid[r][c] != NOSHIP) {
                printf("That spot is taken by a different ship. Try again.\n");
                return false;
            }
        }
     
        for (int c = columnmin; c <= columnmax; ++c){ 
            shipGrid[r][c] = s;
        }
        return true;
        //Scenario where 2nd character is a letter and 3rd character is a number
    } else if (isalpha((unsigned char)input[1]) && isdigit((unsigned char)input[2])) {
        
        char row1character = input[0];
        char row2character = input[1];
        int col = input[2] - '0';
        if (row1character < 'A' || row1character > 'J' || row2character < 'A' || row2character > 'J' || col < 0 || col > 9) {
            printf("That spot is not in bounds of the grid.\n");
            return false;
        }
        //Caluculates the length of the vertical ship 
        int row1 = row1character - 'A';
        int row2 = row2character - 'A';
        int row_min = row1;
        int row_max = row2;
        if(row1 > row2){
            row_min = row2;
            row_max = row1;
        }
        int length = row_max - row_min + 1;
        if (length != size) {
            printf("Ship is %d in size, but should be %d.\n", length, size);
            return false;
        }
        
        for (int r = row_min; r <= row_max; ++r) {
            if (shipGrid[r][col] != NOSHIP) {
                printf("That space overlaps another ship. Try again.\n");
                return false;
            }
        }

        for (int r = row_min; r <= row_max; ++r) shipGrid[r][col] = s;
        return true;
    } else {
        printf("INCORRECT FORMAT. Examples: C37(for ships on a row) or CF4(for ships on a column).\n");
        return false;
    }
}

void PlaceShips(void) {
    //For loop necessary to place all 5 ships 
    for (enum ShipType ship = CARRIER; ship <= DESTROYER; ship++) {
        bool placed = false;
        while (!placed) {
            DisplayBoard();
            placed = PlaceOneShip(ship);
            
        }
    }
    DisplayBoard();
}

void Teardown(void) {
    //Frees the allocated memory for the shipgrid and shotgrid
    if (shipGrid) {
        for (int row = 0; row < 10; ++row) {
            free(shipGrid[row]);
            shipGrid[row] = NULL;
        }
        free(shipGrid);
        shipGrid = NULL;
    }
    if (shotGrid) {
        for (int row = 0; row < 10; ++row) {
            free(shotGrid[row]);
            shotGrid[row] = NULL;
        }
        free(shotGrid);
        shotGrid = NULL;
    }
    printf("Thanks for playing!\n");
}

int main(void) {
    Initialize();
    PlaceShips();
    Teardown();
    return 0;
}
