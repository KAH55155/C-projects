#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define _POSIX_C_SOURCE 200112L

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
const int shipSizes[]    = { 0, 5, 4, 3, 2, 1 };
const char *shipToken[]  = { "  ", "CV", "BB", "CA", "SS", "DD" };

enum ShipType **shipGrid = NULL;           
enum ShotResult **shotGrid = NULL;       

enum ShipType **computerShipGrid = NULL;
enum ShotResult **computerShotGrid = NULL;

void Initialize(void);
void PlaceShips(void);
void Teardown(void);
void remove_newline(char *s);

void SetupSinglePlayer(void);
void TeardownSinglePlayer(void);
bool MakeSinglePlayerShot(void);
void GetSinglePlayerShot(int *row, int *column);
void SinglePlayerResponse(int row, int column);
bool SinglePlayerDidWin(enum ShipType **targetGrid, enum ShotResult **shots);

void UpdateState(enum ShipType **targetGrid, enum ShipType hitShip, const char *owner);

void free_grid_ships(enum ShipType **grid);
void free_grid_shots(enum ShotResult **grid);
enum ShipType **alloc_ship_grid(void);
enum ShotResult **alloc_shot_grid(void);

bool PlaceOneShip(enum ShipType ship);
void DisplayWorld(void);
void DisplayBoardPlayerShips(void);
void DisplayBoardPlayerShots(void);

bool SetupServerAndAccept(int port, int *out_sock);
bool SetupClientAndConnect(const char *ip, int port, int *out_sock);
ssize_t send_all(int sock, const char *buf, size_t len);
ssize_t recv_line(int sock, char *buf, size_t maxlen);

void SetupTwoPlayer(int sock, bool amClient);
void TeardownTwoPlayer(int sock);
bool MakeTwoPlayerShot(int sock, bool amClient);
void TwoPlayerResponse(int sock);
bool TwoPlayerDidWin(enum ShipType **targetGrid, enum ShotResult **shots);

void UpdateState(enum ShipType **targetGrid, enum ShipType hitShip, const char *owner);

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
    printf("Example of placing horizontal ships: For Example J59 means row J, columns 5 through 9\n");
    printf("Example of placing vertical ships: For Example CF4 means rows C through F, column 4\n");
    printf("To shoot an enemy ship enter a single coordinate (letter A-J then digit 0-9) Ex: C7.\n\n");
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
    for (int column = 0; column < 10; ++column){
         printf("%2d  ", column);
    }
    printf("\n\n");
  //Checks all rows and columns to see if all ships have been detroyed 
    for (int row = 0; row < 10; ++row) {
        printf("%c  ", 'A' + row);
        for (int column = 0; column < 10; ++column) {
            if (shipGrid[row][column] == NOSHIP){
             printf(" .  ");
            }
            else {
            printf("%s  ", shipToken[shipGrid[row][column]]);
            }
        }
        printf("\n\n");
    }
}

void DisplayBoardPlayerShots(void) {
    printf("\nYour shots at the opponent:\n\n   ");
    for (int column = 0; column < 10; ++column){
         printf("%2d  ", column);//Prints numbers 0-9 on the board
    }
    printf("\n\n");
    for (int row = 0; row < 10; ++row) {
        printf("%c  ", 'A' + row);
        for (int column = 0; column < 10; ++column) {
            if (shotGrid[row][column] == Open) {
                 printf(" .  ");
            }
            else if (shotGrid[row][column] == MISS){
            printf(" M  ");
            }
            else if (shotGrid[row][column] == HIT) { 
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
    for (size_t i = 0; i < strlen(input); ++i){
         input[i] = toupper((unsigned char)input[i]);
    }

    if (strlen(input) != 3) {
        printf("INCORRECT FORMAT. Examples: C37(for ships on a row) or CF4(for ships on a column).\n");
        return false;
    }

    if (isdigit((unsigned char)input[1]) && isdigit((unsigned char)input[2])) {
        char rowCharacter = input[0];
        int column1 = input[1] - '0';
        int column2 = input[2] - '0';
        if (rowCharacter < 'A' || rowCharacter > 'J' || column1 < 0 || column1 > 9 || column2 < 0 || column2 > 9) {
            printf("That spot is not in bounds of the grid.\n");
            return false;
        }
        int row = rowCharacter - 'A';
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

    } else if (isalpha((unsigned char)input[1]) && isdigit((unsigned char)input[2])) {
        char row1character = input[0];
        char row2character = input[1];
        int column = input[2]-'0';
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
        for (int row = row_min; row <= row_max; ++row){
            if (shipGrid[row][column] != NOSHIP) { printf("That space overlaps another ship. Try again.\n"); 
                return false; 
            }
        }
        for (int row = row_min; row <= row_max; ++row){
             shipGrid[row][column] = s;}
        return true;
    } else {
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
    if (hitShip == NOSHIP) {
        return;
    }
    bool remains = false;
    for (int r = 0; r < 10 && !remains; ++r){
        for (int c = 0; c < 10; ++c){
            if (targetGrid[r][c] == hitShip) { 
                remains = true;
                 break;
             }
         }
    }
    if (!remains){
         printf("%s %s has been sunk!\n", owner, shipNames[hitShip]);
    }
}

bool MakeSinglePlayerShot(void) {
    char input[32];
    printf("Enter coordinate to fire at (e.g. A5): ");
    if (!fgets(input, sizeof(input), stdin)) {
         printf("Input error. Try again.\n");
          return false; 
    }
    remove_newline(input);

    for (size_t i = 0; i < strlen(input); ++i){// Loops through each character in the input and changes it to uppercase
         input[i] = toupper((unsigned char)input[i]);
    }
   
    if (strlen(input) != 2) { 
        printf("INCORRECT FORMAT. Use Letter(A-J) then Digit(0-9), e.g. A5.\n"); 
        return false;
     }
    char rowCharacter = input[0];
    char columnCharacter = input[1];
    if (rowCharacter  < 'A' || rowCharacter  > 'J' || columnCharacter < '0' || columnCharacter > '9') { 
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
        printf("HIT! (%s)\n", shipNames[hitShip]);
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

ssize_t send_all(int sock, const char *buffer, size_t length) {
    size_t total = 0;
    while (total < length) {//loops so that if the length is greater then the total it sends a socket
        ssize_t sent = send(sock, buffer + total, length - total, 0);
        if (sent < 0){
        return sent;
        }
        total += (size_t)sent;
    }
    return (ssize_t)total;
}

ssize_t recv_line(int sock, char *buffer, size_t maxlength) {
    size_t index = 0;
    while (index + 1 < maxlength) {
        char character;
        ssize_t row = recv(sock, &character, 1, 0);
        if (row == 0) {  
            return 0; 
        }
        if (row < 0) {
            if (errno == EINTR){
             continue;
            }
            return -1;
        }
        buffer[index++] = character;
        if (character == '\n'){
        break;
        }
    }
    buffer[index] = '\0';
    return (ssize_t)index;
}

bool SetupServerAndAccept(int port, int *out_socket) {
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) { 
        perror("socket"); 
        return false; 
    }

    int option = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons((uint16_t)port);

    if (bind(server_socket, (struct sockaddr *)&address, sizeof(address)) < 0){ 
        perror("bind"); 
        close(server_socket); 
        return false; 
    }
    if (listen(server_socket, 1) < 0){ 
        perror("listen"); 
        close(server_socket); 
        return false; 
    }

    printf("Server listening on port %d — waiting for connection...\n", port);
    struct sockaddr_in client_address;
    socklen_t socket_address = sizeof(client_address);
    int conn = accept(server_socket, (struct sockaddr *)&client_address, &socket_address);
    if (conn < 0) { 
        perror("accept"); 
        close(server_socket); 
        return false; 
    }

    char client_addressip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_address.sin_addr, client_addressip, sizeof(client_addressip));
    printf("Accepted connection from %s:%d\n", client_addressip, ntohs(client_address.sin_port));
    close(server_socket);
    *out_socket = conn;
    return true;
}

bool SetupClientAndConnect(const char *ip, int port, int *out_sock) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) { 
        perror("socket"); 
        return false; 
    }
    struct sockaddr_in server;
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons((uint16_t)port);
    if (inet_pton(AF_INET, ip, &server.sin_addr) != 1) {
        fprintf(stderr, "Invalid IP address: %s\n", ip);
        close(sock);
        return false;
    }
    printf("Connecting to %s:%d ...\n", ip, port);
    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
         perror("connect"); 
         close(sock); 
         return false; 
    }
    printf("Connected to server.\n");
    *out_sock = sock;
    return true;
}

void SetupTwoPlayer(int sock, bool amClient) {
    (void)sock;
    (void)amClient;
}

void TeardownTwoPlayer(int sock) {
    if (sock >= 0) {
        close(sock);
    }
}

bool parse_coord_twochar(const char *input, int *row, int *column) {
    if (!input || strlen(input) < 2){
         return false;
    }
    char rowCharacter = toupper((unsigned char)input[0]);
    char columnCharacter = input[1];
    if (rowCharacter < 'A' || rowCharacter > 'J'){
        return false;
    }
    if (columnCharacter < '0' || columnCharacter > '9'){
        return false;
    }
    *row = rowCharacter - 'A'; *column = columnCharacter - '0';
    return true;
}

bool MakeTwoPlayerShot(int sock, bool amClient) {
    char input[32];
    printf("Enter coordinate to fire at (e.g. A5): ");
    if (!fgets(input, sizeof(input), stdin)) { 
        printf("Input error. Try again.\n"); 
        return false; 
    }
    remove_newline(input);
    for (size_t i = 0; i < strlen(input); ++i){
         input[i] = toupper((unsigned char)input[i]);
    }
    if (strlen(input) != 2) { 
        printf("INCORRECT FORMAT. Use Letter(A-J) then Digit(0-9), e.g. A5.\n"); 
        return false;
    }
    int row;
    int column;
    if (!parse_coord_twochar(input, &row, &column)) { 
        printf("That spot is not in bounds of the grid.\n");
         return false; 
        }
    if (shotGrid[row][column] != Open) { 
        printf("You've already fired at that square. Try again.\n"); 
        return false; 
    }

    char outbuffer[64];
    snprintf(outbuffer, sizeof(outbuffer), "SHOT %c%d\n", 'A' + row, column);
    if (send_all(sock, outbuffer, strlen(outbuffer)) < 0) {
         perror("send"); 
         return false; 
        }


    char inbuffer[256];
    bool got_result = false;
    bool reported_win = false;
    while (1) {
        ssize_t row = recv_line(sock, inbuffer, sizeof(inbuffer));
        if (row <= 0) {
         fprintf(stderr, "Connection closed or error while waiting for result.\n"); 
         return false; 
        }
        remove_newline(inbuffer);
        if (strncmp(inbuffer, "RESULT ", 7) == 0) {
            const char *result = inbuffer + 7;
            if (strcmp(result, "HIT") == 0) {
                printf("Result: HIT\n");
                shotGrid[row][column] = HIT;
            } else if (strcmp(result, "MISS") == 0) {
                printf("Result: MISS\n");
                shotGrid[row][column] = MISS;
            } else {
                printf("Result: %s\n", result);
                if(strcmp(result, "HIT")==0){
                    shotGrid[row][column] = HIT;
                }else{
                    shotGrid[row][column] = MISS;
                }
            }
            got_result = true;
        } else if (strncmp(inbuffer, "SUNK ", 5) == 0) {
            printf("Opponent says: %s\n", inbuffer); 
        } else if (strcmp(inbuffer, "WIN") == 0) {
            printf("Opponent reports they have won. You lose.\n");
            reported_win = true;
            return true;
        } else {
           
            printf("Received: %s\n", inbuffer);
        }
        if (got_result){
             break;
         } 
    }
    return true;
}


void TwoPlayerResponse(int sock) {
    char inbuffer[256];
    ssize_t r = recv_line(sock, inbuffer, sizeof(inbuffer));
    if (r <= 0) {
        fprintf(stderr, "Connection closed or error while waiting for incoming shot.\n");
        return;
    }
    remove_newline(inbuffer);
    if (strncmp(inbuffer, "SHOT ", 5) != 0) {
         printf("Unexpected network message: %s\n", inbuffer);
        return;
    }
    const char *input = inbuffer + 5;
    int row = -1;
    int column = -1;
    if (!parse_coord_twochar(input, &row, &column)) {
        printf("Peer sent invalid coordinate: %s\n", input);
        return;
    }

    printf("Opponent fires at %c%d... ", 'A' + row, column);
    char outbuffer[256];
    if (shipGrid[row][column] != NOSHIP) {
        enum ShipType hitShip = shipGrid[row][column];
        printf("HIT! (%s)\n", shipNames[hitShip]);

        snprintf(outbuffer, sizeof(outbuffer), "RESULT HIT\n");
        send_all(sock, outbuffer, strlen(outbuffer));

        bool remains = false;
        for (int randomRow = 0; randomRow < 10 && !remains; ++randomRow){
            for (int randomColumn = 0; randomColumn < 10; ++randomColumn){
                if (shipGrid[randomRow][randomColumn] == hitShip) { 
                    remains = true; 
                    break;
                 }
             }
         }
        if (!remains) {
            snprintf(outbuffer, sizeof(outbuffer), "SUNK %s\n", shipNames[hitShip]);
            send_all(sock, outbuffer, strlen(outbuffer));
            printf("Your %s has been sunk by opponent!\n", shipNames[hitShip]);
        }
    } else {
        printf("Miss.\n");
        snprintf(outbuffer, sizeof(outbuffer), "RESULT MISS\n");
        send_all(sock, outbuffer, strlen(outbuffer));
    }

}


bool TwoPlayerDidWin(enum ShipType **targetGrid, enum ShotResult **shots) {
    
    (void)shots;
    (void)targetGrid;
    for (int row = 0; row < 10; ++row){
        for (int column = 0; column < 10; ++column){
            if (shipGrid[row][column] != NOSHIP) {
            return false;
        }
    }
}  
    return true;
}

void Teardown(void) {
    free_grid_ships(shipGrid);
    free_grid_shots(shotGrid);
    printf("Thanks for playing Battleship!\n");
}

int main(int argc, char *argv[]) {
    Initialize();
    PlaceShips();

    if (argc == 1) {
        SetupSinglePlayer();
        bool game_over = false;
        while (!game_over) {
            DisplayWorld();
            bool madeShot = false;
            while (!madeShot) { 
                madeShot = MakeSinglePlayerShot();
            }
            if (SinglePlayerDidWin(computerShipGrid, shotGrid)) {
                printf("\nYou destroyed all the computer's ships. YOU WIN! :) !\n");
                break;
            }

            int clientRow;
            int clientColumn;
            GetSinglePlayerShot(&clientRow, &clientColumn);
            SinglePlayerResponse(clientRow, clientColumn);

            if (SinglePlayerDidWin(shipGrid, computerShotGrid)) {
                printf("\nThe computer has destroyed all your ships. YOU LOSE :( !\n");
                break;
            }
        }
        TeardownSinglePlayer();
        Teardown();
        return 0;
    } else if (argc == 2 || argc == 3) {
        int sock = -1;
        bool amClient = false;
        if (argc == 2) {
            int port = atoi(argv[1]);
            if (port <= 0) { 
                fprintf(stderr, "Invalid port: %s\n", argv[1]); 
                exit(EXIT_FAILURE); 
            }
            if (!SetupServerAndAccept(port, &sock)) { 
                fprintf(stderr, "Failed to setup server.\n"); 
                exit(EXIT_FAILURE); 
            }
            amClient = false;
        } else {
            const char *ip = argv[1];
            int port = atoi(argv[2]);
            if (port <= 0) { 
                fprintf(stderr, "Invalid port: %s\n", argv[2]); 
                exit(EXIT_FAILURE); 
            }
            if (!SetupClientAndConnect(ip, port, &sock)) { 
                fprintf(stderr, "Failed to connect to server.\n");
                 exit(EXIT_FAILURE); 
            }
            amClient = true;
        }

        SetupTwoPlayer(sock, amClient);

        if (amClient) {
    printf("\nTwo-player mode: This is the client, you shoot first.\n");
    } else {
    printf("\nTwo-player mode: This is the server, you shoot second.\n");
    }
     printf("Game start!\n");

        bool game_over = false;

        while (!game_over) {
            if (amClient) {
                DisplayWorld();
                bool ok = false;
                while (!ok){
                     ok = MakeTwoPlayerShot(sock, amClient); 
                }

                if (TwoPlayerDidWin(NULL, NULL)) {
                    printf("All your ships are gone. YOU LOSE.\n");
                    send_all(sock, "WIN\n", 4);
                    break;
                }
                TwoPlayerResponse(sock);
                if (TwoPlayerDidWin(NULL, NULL)) {
                    printf("\nThe opponent has destroyed all your ships. YOU LOSE :( !\n");
                    send_all(sock, "WIN\n", 4);
                    break;
                }
            } else {
                DisplayWorld();
                TwoPlayerResponse(sock);
                if (TwoPlayerDidWin(NULL, NULL)) {
                    printf("\nThe opponent has destroyed all your ships. YOU LOSE :( !\n");
                    send_all(sock, "WIN\n", 4);
                    break;
                }

                bool ok = false;
                while (!ok){
                     ok = MakeTwoPlayerShot(sock, amClient);
                }
                if (TwoPlayerDidWin(NULL, NULL)) {
                    printf("All your ships are gone. YOU LOSE.\n");
                    send_all(sock, "WIN\n", 4);
                    break;
                }
            }
        }
     TeardownTwoPlayer(sock);
        Teardown();
        return 0;
    } else {
        fprintf(stderr, "Usage:\n  %s            # single-player\n  %s <port>     # server\n  %s <ip> <port> # client\n", argv[0], argv[0], argv[0]);
        return 1;
    }
    return 0;
}
