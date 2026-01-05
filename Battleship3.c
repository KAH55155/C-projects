#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include <time.h>

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
enum ShipType **computerShipGrid = NULL;    
enum ShotResult **computerShotGrid = NULL;  

void Initialize(void);
void SetupSinglePlayer(void);
void PlaceShips(void);
bool PlaceOneShip(enum ShipType ship);
void DisplayWorld(void);
void DisplayBoardPlayerShips(void);
void DisplayBoardPlayerShots(void);
void Teardown(void);
void TeardownSinglePlayer(void);
void remove_newline(char *s);

bool MakeSinglePlayerShot(void);
void GetSinglePlayerShot(int *row, int *column);
void SinglePlayerResponse(int row, int column);
bool SinglePlayerDidWin(enum ShipType **targetGrid, enum ShotResult **shots);

void UpdateState(enum ShipType **targetGrid, enum ShipType hitShip, const char *owner);

void free_grid_ships(enum ShipType **grid);
void free_grid_shots(enum ShotResult **grid);
enum ShipType **alloc_ship_grid(void);
enum ShotResult **alloc_shot_grid(void);

void remove_newline(char *s) {
    if (!s){
        return;
    }
    char *p = strchr(s, '\n');
    if (p){
        *p = '\0';
    }
}

enum ShipType **alloc_ship_grid(void) {
    enum ShipType **grid = malloc(10 * sizeof(enum ShipType *));
    if (!grid) return NULL;
    for (int row = 0; row < 10; ++row) {
        grid[row] = malloc(10 * sizeof(enum ShipType));
        if (!grid[row]) {
            for (int i = 0; i < row; ++i) free(grid[i]);
            free(grid);
            return NULL;
        }
    }
    for (int row = 0; row < 10; ++row)
        for (int column = 0; column < 10; ++column)
            grid[row][column] = NOSHIP;
    return grid;
}

enum ShotResult **alloc_shot_grid(void) {
    enum ShotResult **grid = malloc(10 * sizeof(enum ShotResult *));
    if (!grid) return NULL;
    for (int row = 0; row < 10; ++row) {
        grid[row] = malloc(10 * sizeof(enum ShotResult));
        if (!grid[row]) {
            for (int i = 0; i < row; ++i){
                 free(grid[i]);
            }
            free(grid);
            return NULL;
        }
    }
    for (int row = 0; row < 10; ++row){
        for (int column = 0; column < 10; ++column){
            grid[row][column] = Open;
        }
    }
    return grid;
}

void free_grid_ships(enum ShipType **grid) {
    if (!grid){
         return;
    }
    for (int row = 0; row < 10; ++row){
     free(grid[row]);
    }
    free(grid);
}

void free_grid_shots(enum ShotResult **grid) {
    if (!grid){
        return;
    }
    for (int row = 0; row < 10; ++row){
         free(grid[row]);
    }
    free(grid);
}

void Initialize(void) {
    
    srand((unsigned int)time(NULL));
    
    shipGrid = alloc_ship_grid();
    shotGrid = alloc_shot_grid();
    
     if (!shipGrid || !shotGrid) {
        fprintf(stderr, "The memory allocation has failed.\n");
        exit(EXIT_FAILURE);
    }
    
    printf("\nWelcome to my Battleship Game!!!\n");
    printf("Example of placing horizontal ships: J59-means row J, columns 5 through 9\n");
    printf("Example of placing vertical ships: CF4-means rows C through F, column 4\n");
    printf("To shoot an opponent's ship enter a single coordinate (letter A-J then digit 0-9) Ex: C7.\n\n");
}


void SetupSinglePlayer(void) {
    computerShipGrid = alloc_ship_grid();
    computerShotGrid = alloc_shot_grid();
    if (!computerShipGrid || !computerShotGrid) {
        fprintf(stderr, "The memory allocation has failed for the computer grids.\n");
        exit(EXIT_FAILURE);
    }

    for (enum ShipType ship = CARRIER; ship <= DESTROYER; ++ship) {
        int size = shipSizes[ship];
        bool placed = false;
       
        while (!placed) {
            int orient = rand() % 2; 
            if (orient == 0) {
                int randrow = rand() % 10;
                int columnstart = rand() % (11 - size);//11 - size ensures user can't place ship off the grid
                bool ok = true;
                for (int column = columnstart; column < columnstart + size; ++column){
                    if (computerShipGrid[randrow][column] != NOSHIP) { 
                        ok = false; 
                        break; 
                    }
                }
                if (!ok){//If ship space is not valid, skip this loop and try a new space 
                    continue;
                }
                for (int column = columnstart; column < columnstart + size; ++column){
                     computerShipGrid[randrow][column] = ship;
                }
                placed = true;
            } else {//loops to find a starting row and random column to place ships
                int column = rand() % 10;
                int startrow = rand() % (11 - size);
                bool ok = true;
                
                for (int row = startrow; row < startrow + size; ++row){
                    if (computerShipGrid[row][column] != NOSHIP) { 
                    ok = false; 
                    break; 
                    }
                }    
                
                if (!ok){
                continue;
                }
                
                for (int row = startrow; row < startrow + size; ++row){
                    computerShipGrid[row][column] = ship;
                }
                placed = true;
            }
        }
    }
}

void DisplayBoardPlayerShips(void) {
    printf("\nYour ships:\n\n   ");
   for (int column = 0; column < 10; ++column) {
        printf("%2d  ", column);
    }
    
    printf("\n\n");
    //Checks all rows and columns to see if all ships have been detroyed 
    for (int row = 0; row < 10; ++row) {
        printf("%c  ", 'A' + row);
        for (int column = 0; column < 10; ++column) {
            if (shipGrid[row][column] == NOSHIP)
                printf(" .  ");
            else{
                printf("%s  ", shipToken[shipGrid[row][column]]);
            }
        }
        printf("\n\n");
    }
}

void DisplayBoardPlayerShots(void) {
    printf("\nYour shots at the computer:\n\n   ");
    for (int column = 0; column < 10; ++column) {
        printf("%2d  ", column);//Prints numbers 0-9 on the board
    }
    printf("\n\n");
    for (int row = 0; row < 10; ++row) {
        printf("%c  ", 'A' + row);
        for (int column = 0; column < 10; ++column) {
            if (shotGrid[row][column] == Open) {
                printf(" .  ");
            } else if (shotGrid[row][column] == MISS) {
                printf(" M  ");
            } else if (shotGrid[row][column] == HIT) {
                printf(" H  ");
            } else {
                printf(" ?  ");
            }
        }
        printf("\n\n");
    }
}

void DisplayWorld(void) {
    DisplayBoardPlayerShips();
    DisplayBoardPlayerShots();
}

bool PlaceOneShip(enum ShipType s) {
    char input[32];
    int size = shipSizes[s];

    printf("Please enter a location for a ship of %d squares (%s): ", size, shipNames[s]);
    if (!fgets(input, sizeof(input), stdin)) {
        printf("There was an error with your input. Please try again.\n");
        return false;
    }
    remove_newline(input);

    for (size_t i = 0; i < strlen(input); ++i) {
        input[i] = toupper((unsigned char)input[i]);
    }

     if (strlen(input) != 3) {
        printf("INCORRECT FORMAT. Examples: C37(for ships on a row) or CF4(for ships on a column).\n");
        return false;
    }

     if (isdigit((unsigned char)input[1]) && isdigit((unsigned char)input[2])) {
        char rowCharacterharacter = input[0];
        int column1 = input[1] - '0';
        int column2 = input[2] - '0';
        if (rowCharacterharacter < 'A' || rowCharacterharacter > 'J' || column1 < 0 || column1 > 9 || column2 < 0 || column2 > 9) {
            printf("That spot is not in bounds of the grid.\n");
            return false;
        }
        int row = rowCharacterharacter - 'A';
        int columnmin = column1;
        int columnmax = column2;
        if(column1 > column2){
            columnmin = column2;
            columnmax = column1;
        }
        int length = columnmax - columnmin + 1;//Equates the length value to size of ship. Ex:J59 = 5 spaces
        if (length != size) {
            printf("Your entry has a size of %d , but should be a size of %d.\n", length, size);
            return false;
        }

        for (int column = columnmin; column <= columnmax; ++column) {
            if (shipGrid[row][column] != NOSHIP) {
                printf("That spot is taken by a different ship. Try again.\n");
                return false;
            }
        }
        for (int column = columnmin; column <= columnmax; ++column){ 
            shipGrid[row][column] = s;
        }
        return true;

    }
    else if (isalpha((unsigned char)input[1]) && isdigit((unsigned char)input[2])) {
        char row1character = input[0];
        char row2character = input[1];
        int column = input[2] - '0';
        if (row1character < 'A' || row1character > 'J' || row2character < 'A' || row2character > 'J' || column < 0 || column > 9) {
            printf("That spot is not in bounds of the grid.\n");
            return false;
        }
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
        for (int row = row_min; row <= row_max; ++row) {
            if (shipGrid[row][column] != NOSHIP) {
                printf("That space overlaps another ship. Try again.\n");
                return false;
            }
        }
        for (int row = row_min; row <= row_max; ++row){
             shipGrid[row][column] = s;
        }
        return true;
    }
    else {
        printf("INCORRECT FORMAT. Examples: C37(for ships on a row) or CF4(for ships on a column).\n");
        return false;
    }
}

void PlaceShips(void) {
    for (enum ShipType ship = CARRIER; ship <= DESTROYER; ++ship) {
        bool placed = false;
        while (!placed) {
            DisplayBoardPlayerShips();
            placed = PlaceOneShip(ship);
        }
    }
    DisplayBoardPlayerShips();
}

void UpdateState(enum ShipType **targetGrid, enum ShipType hitShip, const char *owner) {
    if (hitShip == NOSHIP) return;

    bool remains = false;
    for (int row = 0; row < 10 && !remains; ++row)
        for (int column = 0; column < 10; ++column)
            if (targetGrid[row][column] == hitShip) { remains = true; break; }

    if (!remains)
        printf("%s %s has been sunk!\n", owner, shipNames[hitShip]);
}

bool MakeSinglePlayerShot(void) {
    char input[32];
    printf("Enter coordinate to fire at (e.g. A5): ");
    if (!fgets(input, sizeof(input), stdin)) {
        printf("Input error. Try again.\n");
        return false;
    }
    remove_newline(input);
  
    for (size_t i = 0; i < strlen(input); ++i) {// Loops through each character in the input and changes it to uppercase
    input[i] = toupper((unsigned char)input[i]);
    }

    if (strlen(input) != 2) {
        printf("INCORRECT FORMAT. Use Letter(A-J) then Digit(0-9), e.g. A5.\n");
        return false;
    }
    char rowCharacter = input[0];
    char columnCharacter = input[1];
    if (rowCharacter < 'A' || rowCharacter > 'J' || columnCharacter < '0' || columnCharacter > '9') {
        printf("That spot is not in bounds of the grid.\n");
        return false;
    }
    int row = rowCharacter - 'A';
    int column = columnCharacter - '0';
    if (shotGrid[row][column] != Open) {
        printf("You've already fired at that square. Try again.\n");
        return false;
    }

    if (computerShipGrid[row][column] != NOSHIP) {//checks to see if user's shot hit a ship on compters grid
        enum ShipType hitShip = computerShipGrid[row][column];
        printf("HIT! (%s)\n", shipNames[computerShipGrid[row][column]]);
        shotGrid[row][column] = HIT;
        UpdateState(computerShipGrid, hitShip, "Computer's");
    } else {
        printf("Miss.\n");
        shotGrid[row][column] = MISS;
    }
    return true;
}

void GetSinglePlayerShot(int *row, int *column) {
    while (1) {//Loops so computer keeps taking random shots at user's grid
        int randomrow = rand() % 10;
        int randomcolumn = rand() % 10;
        if (computerShotGrid[randomrow][randomcolumn] == Open) {
            *row = randomrow;
            *column = randomcolumn;
            return;
        }
    }
}

void SinglePlayerResponse(int row, int column) {
    printf("Computer fires at %c%d... ", 'A' + row, column);
    if (shipGrid[row][column] != NOSHIP) {
        enum ShipType hitShip = shipGrid[row][column];
        printf("HIT! (%s)\n", shipNames[hitShip]);
        computerShotGrid[row][column] = HIT;
        UpdateState(shipGrid, hitShip, "Your");
    } else {
        printf("Miss.\n");
        computerShotGrid[row][column] = MISS;
    }
}

bool SinglePlayerDidWin(enum ShipType **targetGrid, enum ShotResult **shots) {
    for (int row = 0; row < 10; ++row){
        for (int column = 0; column < 10; ++column){
            if (targetGrid[row][column] != NOSHIP && shots[row][column] != HIT){ 
                return false;
            }
        }
    }
    return true;
}

void TeardownSinglePlayer(void) {
    free_grid_ships(computerShipGrid);
    free_grid_shots(computerShotGrid);
}

void Teardown(void) {
    free_grid_ships(shipGrid);
    free_grid_shots(shotGrid);
    printf("Thanks for playing Battleship 3!\n");
}

int main(void) {
    Initialize();
    PlaceShips();
    SetupSinglePlayer();

    bool game_over = false;
    
    while (!game_over) {
        
        DisplayWorld();

        bool madeShot = false;
        
        while (!madeShot){
            madeShot = MakeSinglePlayerShot();
        }
       
         if (SinglePlayerDidWin(computerShipGrid, shotGrid)) {
            printf("\nYou destroyed all the computers ships. YOU WIN! :) !\n");
            break;
        }

       int computerRow;
       int computerColumn;
       GetSinglePlayerShot(&computerRow, &computerColumn);
       SinglePlayerResponse(computerRow, computerColumn);

         if (SinglePlayerDidWin(shipGrid, computerShotGrid)) {
            printf("\nThe computer has destroyed all your ships. YOU LOSE :( !\n");
            break;
        }
    }

    TeardownSinglePlayer();
    Teardown();
    return 0;
}
