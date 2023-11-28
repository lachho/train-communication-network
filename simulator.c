// Carriage Simulator
// cs_cs.c
//
// This program was written 19/4/23
//
// Simulates a live rail Operation centre.
// The program continuously takes in set commands from the user to replicate 
// changing information to the trains such as the movemnt of passengers, and 
// the creation and removal of carriages and trains. 
// Simultion commands include adding passengers to different carriages, 
// moving passengers through the train, adding and removing carriages
// and trains, and merging and splitting trains. 
// Data monitoring commands such as printing individual train capacity,  
// train network capacities, and changing the selected train to interact with
// was also implemented.
// The program ensures there are no memory leaks. 
// This program assumes there will always be at least one train in the program,
// although there can exist 0 carriages. 
// Further limitations are that happiness scores and happiness optimisation
// were not implemented into the program.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

////////////////////////////////////////////////////////////////////////////////
///////////////////////////      Contants       ////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// Constants
#define ID_SIZE 6
#define HELP '?'
#define ADD 'a'
#define PRINT 'p'
#define INSERT 'i'
#define SEAT 's'
#define DISEMBARK 'd'
#define TOTAL 'T'
#define COUNT 'c'
#define MOVE 'm'
#define BLANK '\0'
#define NEW 'N'
#define NEXT '>'
#define PREVIOUS '<'
#define PRINT_ALL 'P'
#define REMOVE 'r'
#define REMOVE_TRAIN 'R'
#define MERGE 'M'
#define SPLIT 'S'

// Enums
enum carriage_type {INVALID_TYPE, PASSENGER, BUFFET, RESTROOM, FIRST_CLASS};

enum condition {INVALID, VALID};

////////////////////////////////////////////////////////////////////////////////
/////////////////////////// USER DEFINED TYPES  ////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// A Train Carriage
struct carriage {
    // carriage id in the form #"N1002", unique, null terminated
    char carriage_id[ID_SIZE];
    //  Type of the carriage
    enum carriage_type type;
    // Maximum number of passengers 
    int capacity;
    // Current number of passengers
    int occupancy;

    struct carriage *next;
};

// A Train
struct train {
    // The head pointer to a linked list of carriages.
    struct carriage *carriages;
    // A pointer to the next train in the linked list of trains.
    struct train *next;
    // A pointer to the previous train in the linked list of trains.
    struct train *previous;
};

struct space {
    int capacity;
    int unoccupied;
    int occupied;
};

struct ends {
    char start[ID_SIZE];
    char end[ID_SIZE];
};

////////////////////////////////////////////////////////////////////////////////
////////////////////// PROVIDED FUNCTION PROTOTYPE  ////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void print_usage(void);
void print_carriage(struct carriage *carriage);
void scan_id(char id_buffer[ID_SIZE]);
enum carriage_type scan_type(void);
void print_train_summary(
    int is_selected, 
    int n, 
    int capacity, 
    int occupancy,
    int num_carriages
);
int compare_double(double n1, double n2);

// Additional provided function prototypes
// You won't need to use these functions!
// We use just them to implement some of the provided helper functions.
int scan_token(char *buffer, int buffer_size);
char *type_to_string(enum carriage_type type);
enum carriage_type string_to_type(char *type_str);

////////////////////////////////////////////////////////////////////////////////
////////////////////////  YOUR FUNCTION PROTOTYPE  /////////////////////////////
////////////////////////////////////////////////////////////////////////////////
struct carriage *create_carriage(char id[ID_SIZE], enum carriage_type type,
                                 int capacity);
struct carriage *add_carriage(struct carriage *head, int new_position, 
                              char attachment);
void print_train(struct carriage *head);
int is_train_real(struct carriage *current);
int is_type_valid(enum carriage_type type);
int is_capacity_valid(int capacity);
int validity(int test);
int is_new_valid(char id[ID_SIZE], enum carriage_type type, int capacity, 
                 struct carriage *head, int position);
int is_id_in_train(char id[ID_SIZE], struct carriage *head);
int is_non_neg(int position);
void is_loading_valid(struct carriage *head, char command);
int is_pos(int num);
void add_passengers(struct carriage *current, int total, char command, 
                    char source_id[ID_SIZE]);
void remove_passengers(struct carriage *current, int total, char command);
struct carriage *find_id(struct carriage *current, char id[ID_SIZE]);
struct space count_passengers(struct carriage *head, char start[ID_SIZE], 
                              char end[ID_SIZE], char command);
int find_id_index(struct carriage *current, char id[ID_SIZE]);
void is_move_valid(struct carriage *head, char command);
struct carriage *find_end(struct carriage *head);
int is_enough_passengers(struct carriage *curent, int to_move);
struct train *create_train(void);
int train_length(struct carriage *head);
struct ends find_edges(struct carriage *head);
void print_all(struct train *selected);
struct train *head_train(struct train *selected);
struct carriage *remove_carriage(struct carriage *head, char id[ID_SIZE]);
struct train *arrange_trains(struct train *selected);
void remove_train(struct train *selected);
void remove_all(struct train *selected);
struct train *command_page(struct train *selected, char command);
struct carriage *merge_dupes(struct carriage *current,
                             struct carriage *next_carriage);
void merge_trains(struct train *selected);
void split_trains(struct train *start);
int split_train_once(struct train *selected, char id[ID_SIZE], 
                     int *check_trains);

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

int main(void) {
    printf("Welcome to Carriage Simulator\n");
    printf("All aboard!\n");

    // Pointer to our first train when our program starts. 
    // All carriages are stored here until we change trains.
    struct train *trains = create_train();

    // We also need another pointer to keep track of 
    // which train we have selected.
    struct train *selected = trains;

    // Loops through the commands provided by the user
    //TURN THIS INTO A FUNCTION
    printf("Enter command: ");
    char command;
    while (scanf(" %c", &command) != EOF) {
        selected = command_page(selected, command);
        printf("Enter command: ");
    }
    remove_all(selected);
    printf("\nGoodbye\n");

    return 0;
}

////////////////////////////////////////////////////////////////////////////////
/////////////////////////////  YOUR FUNCTIONS //////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// Mallocs a new node and then inserts the input data into the carriage node
// 
// Parameters:
//      id[ID_SIZE] - string, which contains the carriage ID
//      type        - enum, what type the carriage node is
//      capacity    - int capacity of the carriage node
// Returns:
//      The new node filled with the data
//
struct carriage *create_carriage(char id[ID_SIZE], enum carriage_type type,
                                 int capacity) {
    // malloc the new node
    struct carriage *new = malloc(sizeof(struct carriage));
    
    // copy the inputs into the new carriage node
    strcpy(new->carriage_id, id);
    new->type = type;
    new->capacity = capacity;
    new->occupancy = 0;
    new->next = NULL;
    // return the node filled with data.
    return new; 
}

// Scans in the values for the carriage. Then, 
// Inserts a new carriage at the inputted position in the linked list.
//
// Parameters: 
//      *head       - struct *, contains the head pointer of the linked list.
//      new_position- int, index at which the carriage should be inserted
//      command     - char, command given by the user
// Returns:
//      The new head node, if it has changed.
//
struct carriage *add_carriage(struct carriage *head, int new_position,
                              char attachment) {
    // Scans in the values for the carriage provided by the user
    char new_id[ID_SIZE];
    scan_id(new_id);
    enum carriage_type new_type = scan_type();
    int new_capacity;
    scanf(" %d", &new_capacity);

    struct carriage *current = head;
    // checks if the carriage data is valid
    if (is_new_valid(new_id, new_type, new_capacity, head, new_position)) {
        struct carriage *new = create_carriage(new_id, new_type, new_capacity);  
        // checks if carriages exist, 
        // then loops through to the inputted position along the linked list. 
        if (is_train_real(head)) {     
            int i = 0;       
            while (i < new_position - 1 && current->next != NULL) {
                current = current->next;
                i++;
            }
            // Appends the new carriage to the the list.
            if (new_position == 0) {
                new->next = head;
                head = new;
            } else {       
                new->next = current->next;
                current->next = new;
            }
        } else {
            // points head to the newly created node.
            head = new;
        }
        
        // Print confirmation of carriage attached
        if (attachment == ADD) {
            printf("Carriage: '%s' attached!\n", new_id);
        } else {
            printf("Carriage: '%s' inserted!\n", new_id);
        }
    }
    // returns the potentially updated starting node.
    return head;
}

// Checks if carriages exist in the linked list
//
// Parameters: 
//      current -  struct *, current node along the linked list
//
// Return:
//      VALID   - if valid
//      INVALID - if invalid
//
int is_train_real(struct carriage *current) {
    return validity(current != NULL);
}

// Checks if position, type and capacity is within range 
// and if the carriage ID is unique. 
// If invalid, prints error message.
//
// Parameters: 
//      id[ID_SIZE] - string, which contains the carriage ID
//      type        - enum, what type the carriage node is
//      capacity    - int capacity of the carriage node
//      *head       - struct *, contains the head pointer of the linked list.
//
// Return:
//      VALID   - if valid (all checks pass)
//      INVALID - if invalid (at least 1 check fails)
//
int is_new_valid(char id[ID_SIZE], enum carriage_type type, int capacity, 
                 struct carriage *head, int position) {
    // test if position is positive
    if (!is_non_neg(position)) {
        printf("ERROR: n must be at least 0\n");
        return INVALID;        
    }
    // test if carriage type
    else if (!is_type_valid(type)) {
        printf("ERROR: Invalid carriage type\n");
        return INVALID;
    }
    // test capacity
    else if (!is_capacity_valid(capacity)) {
        printf("ERROR: Capacity should be between 1 and 999\n");
        return INVALID;       
    } 
    // test if ID has been used already
    else if (is_id_in_train(id, head)) {
        printf("ERROR: a carriage with id: '%s' already exists in this train\n", 
                id);
        return INVALID;
    } else {
        return VALID;
    }
}

// Checks if num is a non negative number
//
// Parameters: 
//      num - int, number to test
//
// Return:
//      VALID   - if valid
//      INVALID - if invalid
//
int is_non_neg(int num) {
    return validity(num >= 0);
}

// Checks if num is a positive number
//
// Parameters: 
//      num - int, number to test
//
// Return:
//      VALID   - if valid
//      INVALID - if invalid
//
int is_pos(int num) {
    return validity(num > 0);
}

// Checks if type is a real type
//
// Parameters: 
//      type - enum, type of the carriage
//
// Return:
//      VALID   - if valid
//      INVALID - if invalid
//
int is_type_valid(enum carriage_type type) {
    return validity(type != INVALID_TYPE);
}

// Checks if carriage is within limits
//
// Parameters: 
//      capacity - capacity of the carriage
//
// Return:
//      VALID   - if valid
//      INVALID - if invalid
//
int is_capacity_valid(int capacity) {
    return validity(capacity > 0 && capacity <= 999);
}

// Checks if carriage id is already in the train
//
// Parameters: 
//      id[ID_SIZE] - string, which contains the carriage ID
//      *head       - struct *, contains the head pointer of the linked list.
//
// Return:
//      VALID   - if id is in linked list
//      INVALID - if not
//
int is_id_in_train(char id[ID_SIZE], struct carriage *head) {
    if (is_train_real(head)) {
        struct carriage *current = head;
        while (current != NULL) {
            if (strcmp(current->carriage_id, id) == 0) {
                return VALID;
            }
            current = current->next;  
        }     
    }
    return INVALID;
}

// Checks if input is true or false
//
// Parameters: 
//      test - int, true (1) or false (0)
//
// Return:
//      VALID   - if valid
//      INVALID - if invalid
//
int validity(int test) {
    if (test == VALID) {
        return VALID;
    } else {
        return INVALID;
    }
}

// Checks if there are nodes in the linked list.
// If there is, loops through the list and prints each node's data. 
//
// Parameters: 
//      *head   - struct *, contains the head pointer of the linked list.
//
void print_train(struct carriage *head) {
    struct carriage *current = head;

    // checks if linked list is empty, if not prints the linked lists' data.
    if (is_train_real(head)) {
        while (current != NULL) {
            print_carriage(current);
            current = current->next;
        }
    } else {
        printf("This train is empty!\n");
    }
}

// scans in the inputs and checks to ensure input is valid
// then calls function to add/remove passengers from the train.
//
// Parameters: 
//      *head   - struct *, contains the head pointer of the linked list.
//      command     - char, command given by the user
//
void is_loading_valid(struct carriage *head, char command) {
    char id[ID_SIZE];
    scan_id(id);
    int total;
    scanf(" %d", &total);

    if (!is_pos(total)) {
        printf("ERROR: n must be a positive integer\n");
    } 
    else if (!is_train_real(head) || !is_id_in_train(id, head)) {
        printf("ERROR: No carriage exists with id: '%s'\n", id);
    } else {
        
        // find the node of the carriage id provided
        struct carriage *current = find_id(head, id);
        if (command == SEAT) {
            add_passengers(current, total, command, id);
        } else {
            remove_passengers(current, total, command);
        }
    }
}

// Loops through list to find node with a given carriage id
//
// Parameters: 
//      current - struct *, starting node to search from
//      id      - char, id of the carriage to find
//
// Return:
//      pointer to the node containing id.
//
struct carriage *find_id(struct carriage *current, char id[ID_SIZE]) {
    while (strcmp(current->carriage_id, id) != 0) {
        current = current->next;
    }
    return current;
}

// Loops through list to find index of a given carriage id
//
// Parameters: 
//      current - struct *, starting node to search from
//      id      - char, id of the carriage to find
//
// Return:
//      position in the linked list of the node containing id.
//
int find_id_index(struct carriage *current, char id[ID_SIZE]) {
    int count = 0;
    while (strcmp(current->carriage_id, id) != 0) {
        count++;
        current = current->next;
    }
    return count;
}

// adds passengers to the carriages, overflow passengers are seated in 
// proceeding carriages
//
// Parameters: 
//      *current    - struct *, contains a pointer to where to add passengers
//      total       - int, total number of passengers to add
//
void add_passengers(struct carriage *current, int total, char command, 
                    char source_id[ID_SIZE]) {
    // Loops through the linked list until all passengers are loaded 
    // or we reach the end of the linked list. 
    while (total > 0 && current != NULL) {
        int count = 0;
        // fills up carriage until carriage is full or no more passengers
        // are required to load the train. 
        while (current->capacity > current->occupancy && total > 0) {
            current->occupancy++;
            count++;
            total--;
        }
        if (count > 0) {
            if (command == SEAT) {
                printf("%d passengers added to %s\n", count, 
                        current->carriage_id);
            }
            else if (command == MOVE) {
                printf("%d passengers moved from %s to %s\n", count, source_id,
                        current->carriage_id);
            }
        }
        current = current->next;
    }
    if (total > 0) {
        printf("%d passengers could not be seated\n", total);
    }
}

// removes passengers to the carriages
//
// Parameters: 
//      *current    - struct *, contains a pointer to where to remove passengers
//      total       - int, total number of passengers to remove
//
void remove_passengers(struct carriage *current, int total, char command) {
    // checks if theres enough passengers and removes them.
    if (!is_enough_passengers(current, total)) {
        printf("ERROR: Cannot remove %d passengers from %s\n", total, 
            current->carriage_id);
    } else {
        current->occupancy -= total;
        if (command == DISEMBARK) {
            printf("%d passengers removed from %s\n", total, 
                    current->carriage_id);
        }
    }
}

// Checks if theres enough occupants when moving passengers
//
// Parameters: 
//      to_move     - int, how many passengers we wish to move
//      *current    - struct *, contains the head pointer of the linked list.
//
// Return:
//      VALID   - if id is in linked list
//      INVALID - if not
//
int is_enough_passengers(struct carriage *current, int to_move) {
    return validity(current->occupancy >= to_move);
}

// Checks to ensure the start and end ids are in the train, 
// then prints out the occupied and unoccupied seats
//
// Parameters: 
//      *head   - struct *, contains the head pointer of the linked list.
//      start   - char, carriage id of the starting carriage
//      end     - char, carriage id of the ending carriage  
//      command - char, command given by the user
//
// Return:
//      number of available seats in the range of carriages.
// 
struct space count_passengers(struct carriage *head, char start[ID_SIZE], 
                              char end[ID_SIZE], char command) {
    struct space total;   
    total.occupied = INVALID;
    total.unoccupied = INVALID;
    total.capacity = INVALID;               
    if (!is_id_in_train(start, head)) {
        printf("ERROR: No carriage exists with id: '%s'\n", start);
    }
    else if (!is_id_in_train(end, head)) {
        printf("ERROR: No carriage exists with id: '%s'\n", end);
    }
    else if (find_id_index(head, start) > find_id_index(head, end)) {
        printf("ERROR: Carriages are in the wrong order\n");
    } else {
        struct carriage *current = find_id(head, start);
        // stop at the node after the end node so we can count it too. 
        struct carriage *stop = find_id(head, end)->next;
        int passengers = 0;
        int seats = 0;
        // count seats and capacity until the end node
        while (current != stop) {
            passengers += current->occupancy;
            seats += current->capacity;
            current = current->next;
        }

        total.occupied = passengers;
        total.unoccupied = seats - passengers;
        total.capacity = seats;

        // Print message depending on 'T' or 'c' command
        if (command == COUNT) {
            printf("Occupancy: %d\n", total.occupied);
            printf("Unoccupied: %d\n", total.unoccupied);
        } 
        else if (command == TOTAL) {
            printf("Total occupancy: %d\n", total.occupied);
            printf("Unoccupied capacity: %d\n", total.unoccupied);            
        }
        return total;
    }
    return total;
}

// Checks to ensure the capacity, source and destinations ids are valid, 
// then moves the passengers around.  
//
// Parameters: 
//      *head   - struct *, contains the head pointer of the linked list.
//      command - char, command given by the user
//
void is_move_valid(struct carriage *head, char command) {
    char source_id[ID_SIZE];
    scan_id(source_id);
    char destination_id[ID_SIZE];
    scan_id(destination_id);    
    int to_move;
    scanf(" %d", &to_move);

    if (!is_pos(to_move)) {
        printf("ERROR: n must be a positive integer\n");
    }
    else if (!is_id_in_train(source_id, head)) {
        printf("ERROR: No carriage exists with id: '%s'\n", source_id);
    }
    else if (!is_enough_passengers(find_id(head, source_id), to_move)) {
        printf("ERROR: Cannot remove %d passengers from %s\n", 
               to_move, source_id);
    }
    else if (!is_id_in_train(destination_id, head)) {
        printf("ERROR: No carriage exists with id: '%s'\n", destination_id);
    } else {
        // unboards the passengers wanting to move
        struct carriage *source = find_id(head, source_id);
        remove_passengers(source, to_move, BLANK);
        // Counts to see how many seats are available at the carriage 
        // + following carriages. 
        // BLANK command used since we dont want to print anything.
        struct space total = count_passengers(head, destination_id, 
                             find_end(head)->carriage_id, BLANK);
        struct carriage *destination = find_id(head, destination_id);                                
        // if no room, passengers are returned to original carriage.
        if (to_move > total.unoccupied) {
            add_passengers(source, to_move, BLANK, source_id);
            printf("ERROR: not enough space to move passengers\n");
        } else {
            add_passengers(destination, to_move, MOVE, source_id);
        }
    }
}

// Loops to end of linked link and returns the last carriage
//
// Parameters: 
//      *head   - struct *, contains the head pointer of the linked list.
//      end     - char, carriage id of the ending carriage  
//
// Return:
//      struct * to the last carriage in the linked list
// 
struct carriage *find_end(struct carriage *head) {
    struct carriage *current = head;
    while (current->next != NULL) {
        current = current->next;  
    } 
    return current;
}

// Mallocs a new node and then inserts the input data into the train node
// 
// Returns:
//      The new node filled with NULL in all fields
//
struct train *create_train(void) {
    // malloc the new node
    struct train *new = malloc(sizeof(struct train));

    // Creates blank data for the new train.
    new->carriages = NULL;
    new->next = NULL;
    new->previous = NULL;
    // return the node filled with data.
    return new; 
}

// Checks if the current train to print is the selected train. 
//
// Parameters: 
//      selected    - struct *, current train selected in the main function
//      position    - struct *, train to check if selected train.
//
// Return:
//      VALID   - if position is the selected
//      INVALID - if not
//
int is_selected(struct train *selected, struct train *position) {
    return validity(selected == position);
}

// Counts the number of carriages in the train
//
// Parameters: 
//      head    - struct *, start carriage of linked list.
//
// Return:
//      Number of carriages in the train.
//
int train_length(struct carriage *head) {
    return find_id_index(head, find_end(head)->carriage_id) + 1;
}

// Finds the ID's of the start and end carriages
//
// Parameters: 
//      head    - struct *, start carriage of linked list.
//
// Return:
//      ID's of the start and end carriages.
//
struct ends find_edges(struct carriage *head) {
    struct ends train_ends;
    // finds the first carriage in train's ID
    strcpy(train_ends.start, head->carriage_id);
    // finds the last carriage in train's ID
    strcpy(train_ends.end, find_end(head)->carriage_id);
    return train_ends;
}

// Prints all the trains in the station
//
// Parameters: 
//      selected    - struct *, current train selected in the main function
//
void print_all(struct train *selected) {
    struct train *position = selected;
    // Cycle to first train in linked list.
    position = head_train(position);

    int selection;
    int count = 0;
    struct ends train_ends;
    struct space total;
    int length;
    while (position != NULL) {
        // checks if train is currently selected train.
        selection = is_selected(selected, position);
        total.capacity = INVALID;
        total.occupied = INVALID;
        length = INVALID;
        // If carriages exist, find the correct data. 
        if (is_train_real(position->carriages)) {
            // finds start and end ID's for the count_passengers function
            train_ends = find_edges(position->carriages);
            // finds the capacity and occupancy
            total = count_passengers(position->carriages, train_ends.start, 
                                    train_ends.end, BLANK);

            // finds number of carriages in the train.
            length = train_length(position->carriages);
        }
        // Pints the train summary
        print_train_summary(selection, count, total.capacity, total.occupied,
                            length);
        // tracks position of the given train
        count++;
        position = position->next;
    }
}

// Cycles to the first train in the linked list
//
// Parameters: 
//      *selected   - struct *, some node along the train linked list.
//
// Returns:
//      The head node of the train linked list.
//
struct train *head_train(struct train *selected) {
    struct train *position = selected;
    // Cycle to first train in linked list.
    while (position->previous != NULL) {
        position = position->previous;
    }   
    return position;
}

// Removes the carriage from the train.
//
// Parameters: 
//      *head   - struct *, contains the head pointer of the linked list.
//      id      - string of the carriage ID.
//
// Returns:
//      The new head node, if it has changed.
//
struct carriage *remove_carriage(struct carriage *head, char id[ID_SIZE]) {
    // Error Testing if ID is in train.
    if (!is_id_in_train(id, head)) {
        printf("ERROR: No carriage exists with id: '%s'\n", id);
        return head;
    }

    // Checks if we need to remove the first carriage and removes it.
    struct carriage *current = head;
    if (strcmp(current->carriage_id, id) == 0) {
        head = current->next;
        free(current);
        return head;
    } 

    // Goes to 2nd carriage and finds the carriage to remove and removes it.
    struct carriage *previous = current;
    current = current->next;
    while (current != NULL) {
        if (strcmp(current->carriage_id, id) == 0) {
            struct carriage *temp = current;
            previous->next = current->next;
            current = current->next;
            free(temp);
            return head;
        } else {
            previous = current;
            current = current->next;
        }
    }       
    return head;
}

// Updates the selected train to the next available.
// then reattaches the linked list, removing the selected train from the list.
//
// Parameters: 
//      *selected   - struct *, node along the train linked list to remove.
//
// Returns:
//      The new selected node of the train linked list.
//
struct train *arrange_trains(struct train *selected) {
    struct train *temp = selected;
    // updates the selected train to the next available
    if (selected->previous != NULL) {
        selected = selected->previous;
        selected->next = temp->next;
        if (selected->next != NULL) {
            selected->next->previous = selected;
        }
    } 
    else if (selected->next != NULL) {
        selected = selected->next;
        selected->previous = NULL;
    } else {
        selected = create_train();
    }
    return selected;
}

// Frees all the carriage nodes in the train, as well as the train node itself
//
// Parameters: 
//      *selected   - struct *, node along the train linked list to remove.
//
void remove_train(struct train *selected) {
    struct carriage *current = selected->carriages;
    struct carriage *temp;

    if (is_train_real(current)) {
        while (current != NULL) {
            temp = current;
            current = current->next;
            free(temp);
        }
    }
    free(selected);
}

// Loops to the head node, then removes all the train and carriage nodes.
//
// Parameters: 
//      *selected   - struct *, node along the train linked list to remove.
//
void remove_all(struct train *selected) {
    struct train *position = selected;
    // Cycle to first train in linked list.
    position = head_train(position);
    
    // remove each train in the linked list.
    struct train *temp;
    while (position != NULL) {
        temp = position;
        position = position->next;
        remove_train(temp);
    }
    
}

// Takes in commands from the user to change the properties of the train. 
//
// Parameters: 
//      *selected   - struct *, selected node along the train linked list 
//
// Returns:
//      The node to the current train in the train linked list. 
//
struct train *command_page(struct train *selected, char command) {
    // prints help message
    if (command == HELP) {
        print_usage();
    }
    // adds carriage to the start
    else if (command == ADD) {
        int end_position = 0;
        if (is_train_real(selected->carriages)) {
            // sets the insertion point at the end of the linked list
            end_position = train_length(selected->carriages);
        }
        selected->carriages = add_carriage(selected->carriages, 
                                            end_position, command);
    }
    // prints current train
    else if (command == PRINT) {
        print_train(selected->carriages);
    }
    // adds carriage anywhere in the linked list
    else if (command == INSERT) {
        // scans in position to insert carriage
        int new_position;
        scanf(" %d", &new_position);
        selected->carriages = add_carriage(selected->carriages, 
                                            new_position, command);
    }
    // add passengers to the carriage
    else if (command == SEAT) {
        is_loading_valid(selected->carriages, command);
    }
    // remove passengers from the carriage
    else if (command == DISEMBARK) {
        is_loading_valid(selected->carriages, command);
    }
    // counts the total occupants and spare seats in the train.
    else if (command == TOTAL) {
        // checks to see if there are carriages
        if (is_train_real(selected->carriages)) {
            // finds start and end IDs of the train
            struct ends train_ends = find_edges(selected->carriages);

            count_passengers(selected->carriages, train_ends.start, 
                                train_ends.end, command);
        
        } else {
            // edge case where there are no carriages
            printf("Total occupancy: 0\n");
            printf("Unoccupied capacity: 0\n");             
        }
    }
    // counts the total occupants and spare seats in a section of the train
    else if (command == COUNT) {
        // scans in start and end carriage ids.
        char start[ID_SIZE];
        scan_id(start);
        char end[ID_SIZE];
        scan_id(end);

        count_passengers(selected->carriages, start, end, command);
    }
    // moves passengers from one train to the next
    else if (command == MOVE) {
        is_move_valid(selected->carriages, command);
    }
    // creates a new train
    else if (command == NEW) {
        struct train *new = create_train();

        // connects new node to old previous node
        if (selected->previous != NULL) {
            selected->previous->next = new;
            new->previous = selected->previous;
        } else {
            new->previous = NULL;
        }
        // connects new node with currently selected node
        selected->previous = new;
        new->next = selected;
    }
    // cycles to the proceeding train
    else if (command == NEXT) {
        if (selected->next != NULL) {
            selected = selected->next;
        }
    }
    // cycles to the preceeding train
    else if (command == PREVIOUS) {
        if (selected->previous != NULL) {
            selected = selected->previous;
        }
    }
    // prints all the trains
    else if (command == PRINT_ALL) {
        print_all(selected);
    }
    // removes a carriage from the selected train
    else if (command == REMOVE) {
        char id[ID_SIZE];
        scan_id(id);
        selected->carriages = remove_carriage(selected->carriages, id);
    }
    // removes the entire train
    else if (command == REMOVE_TRAIN) {
        struct train *temp = selected;
        // arranges the trains next/previous links and updates selected
        selected = arrange_trains(selected);
        // removes the previouslly selected train.
        remove_train(temp);
    }
    // Merges current and next train together
    else if (command == MERGE) {
        if (selected->next != NULL) {
            merge_trains(selected);
        }
    }
    // Splits trains into parts at the given carriage ID's.
    else if (command == SPLIT) {
        split_trains(selected);
    }
    return selected;
}

// Merges the double carriage ID's into the first train and deletes it 
// from the 2nd train
//
// Parameters: 
//      *head           - struct *, start of the carriages of the train to keep
//      *next_carriage  - struct *, start of the carriages of the train to 
//                                  delete.
//
// Returns:
//      The head of the train to delete. 
//
struct carriage *merge_dupes(struct carriage *head, 
                             struct carriage *next_carriage) {
    struct carriage *current = next_carriage;
    while (current != NULL) {
        if (is_id_in_train(current->carriage_id, head)) {
            // find duplicate and add passengers to the current train
            struct carriage *to_fix = find_id(head, current->carriage_id);
            to_fix->capacity += current->capacity;
            to_fix->occupancy += current->occupancy;

            // remove empty carriage from next train.
            struct carriage *temp = current;
            current = current->next;
            next_carriage = remove_carriage(next_carriage, temp->carriage_id);
        } else {
            current = current->next;
        }
    }
    // returns head of next train's 
    return next_carriage;
}

// Merges 2 trains (carriages linked lists) into one.
// Then deletes the second train node. 
//
// Parameters: 
//      *selected   - struct *, selected node along the train linked list 
//
void merge_trains(struct train *selected) {
    struct carriage *current = selected->carriages;
    struct train *next_train = selected->next;
    struct carriage *next_carriage = next_train->carriages;

    // loop to end of current train if exists
    if (is_train_real(current)) {
        while (current->next != NULL) {
            current = current->next;
        }

        // connect 2nd train to first if both exist
        if (is_train_real(next_carriage)) {
            // removes duplicates from 2nd train first. 
            current->next = merge_dupes(selected->carriages, next_carriage);
        }

    } else if (is_train_real(next_carriage)) {
        // if no carriages in first, but there is in the 2nd, 
        // moves the carriages to the first train instead.
        selected->carriages = next_carriage;
    }

    // remove 2nd train from linked list
    selected->next = next_train->next;
    if (next_train->next != NULL) {
        next_train->next->previous = selected;
    }
    free(next_train);
}

// If the inputs are valid, splits the train into multiple parts.
// Splits the selected train by the inputed carriage IDs by creating new trains
// and moving carriages into the new trains from the old train. 
//
// Parameters: 
//      *start  - struct *, selected node along the train linked list 
//
void split_trains(struct train *start) {
    int num_splits;
    scanf(" %d", &num_splits);

    if (!is_pos(num_splits)) {
        printf("ERROR: n must be a positive integer\n");
    } else {
        printf("Enter ids: \n");

        // Number of trains to check.
        // Note: after train is split at least once, must check multiple trains.
        int check_trains = 1;

        int split = 0;
        // current train to check into
        struct train *selected = start;
        // repeats the desired number of splits 
        while (split < num_splits) {
            // scann in ID to split at.
            char id[ID_SIZE];
            scan_id(id);
        
            int i = 0;
            enum condition is_id_found = INVALID;
            while (is_id_found == INVALID && i < check_trains) {
                // splits the train between a single ID
                // If splits, ends the loop, otherwise keeps searching. 
                is_id_found = split_train_once(selected, id, &check_trains);

                // checks into next train (which is part of original train)
                selected = selected->next;
                i++;
            }

            // Prints error message if ID is not found in the select train(s). 
            if (!is_id_found) {
                printf("No carriage exists with id: '%s'. Skipping\n", id);
            } 

            // reset to start of initial train and repeat for next ID. 
            selected = start;
            split++;
        }

    }
}

// Checks if the ID is in the train. If yes, performs the split.
// Creates a new train and moves the carriages into the new train.
//
// Parameters: 
//      *selected   - struct *, selected node along the train linked list 
//
// Returns:
//      VALID   - If the carriages split did occur 
//      INVALID - If the carriages split did not occur
// 
int split_train_once(struct train *selected, char id[ID_SIZE], 
                     int *check_trains) {
    struct carriage *current = selected->carriages;
    // find where next carriage is where new train should begin.
    if (is_train_real(current) && is_id_in_train(id, current)) {            
        // create new train
        struct train *new = create_train();
        // connects new train to next train
        if (selected->next != NULL) {
            selected->next->previous = new;
            new->next = selected->next;
        } else {
            new->next = NULL;
        }
        // connects new node with currently selected node
        selected->next = new;
        new->previous = selected;

        // find where split should occur
        int end_train = find_id_index(current, id);

        // Edge case if splitting at start of carriage linked list
        if (end_train == INVALID) {
            new->carriages = current;
            selected->carriages = NULL;
        } else {
            int j = 0;
            // loops to the end of the front half of the split train
            while (j < end_train - 1) {
                current = current->next;
                j++;
            }

            // Removes split from train and attaches to new train.
            new->carriages = current->next;
            current->next = NULL;
        }

        // since new train created, need to check more trains next time. 
        (*check_trains)++; 

        // we found the ID, so we exit the loop and move to the next ID.
        return VALID;
    } else {
        return INVALID;
    }
}

////////////////////////////////////////////////////////////////////////////////
///////////////////////////  PROVIDED FUNCTIONS  ///////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// The below functions are Written and provided by University of New South Wales (2023). 
// Prints the Carriage simulator usage instructions,
// displaying the different commands and their arguments.
//
void print_usage(void) {
    printf(
        "=====================[ Carriage Simulator ]=====================\n"
        "      ===============[     Usage Info     ]===============      \n"
        "  a [carriage_id] [type] [capacity]                             \n"
        "    Add a carriage to the train                                 \n"
        "  p                                                             \n"
        "    Print out all of the carriages in the train                 \n"
        "  i [n] [carriage_id] [type] [capacity]                         \n"
        "    Insert a carriage into the train at position `n`            \n"
        "                                                                \n"
        "  s [carriage_id] [n]                                           \n"
        "    Seat `n` passengers onto the train starting from carriage   \n"
        "    `carriage_id`                                               \n"
        "  d [carriage_id] [n]                                           \n"
        "    Remove `n` passengers from carriage `carriage_id`           \n"
        "  T                                                             \n"
        "    Display the total number of passengers and empty seats on   \n"
        "    the train                                                   \n"
        "  c [start_id] [end_id]                                         \n"
        "    Display the number of passengers and empty seats on between \n"
        "    carriage `start_id` and carriage `end_id`                   \n"
        "  m [source_id] [destination_id] [n]                            \n"
        "    Move `n` passengers from one carrige to another, without    \n"
        "    kicking anyone off the train.                               \n"
        "  h [carriage_id]                                               \n"
        "    Display the happiness of passengers in carriage             \n"
        "    `carriage_id`                                               \n"
        "  H                                                             \n"
        "    Display the average happiness of all passengers on the train\n"
        "                                                                \n"
        "  N                                                             \n"
        "    Create a new empty train                                    \n"
        "  >                                                             \n"
        "    Select the next train in the train list.                    \n"
        "  <                                                             \n"
        "    Select the previous train in the train list.                \n"
        "  P                                                             \n"
        "    Display the train list.                                     \n"
        "  r [carriage_id]                                               \n"
        "    Remove carriage `carriage_id` from the selected train.      \n"
        "  R                                                             \n"
        "    Remove the selected train.                                  \n"
        "                                                                \n"
        "  M                                                             \n"
        "    Merge the selected train with the train after it.           \n"
        "  S [n]                                                         \n"
        "    Split the current train into smaller trains.                \n"
        "  O                                                             \n"
        "    Rearrange passengers on the selected train to optimise      \n"
        "    happiness.                                                  \n"
        "  ?                                                             \n"
        "    Show help                                                   \n"
        "================================================================\n"
    );
}


// Scan in the a carriage id string into the provided buffer, placing 
// '\0' at the end.
//
// Parameters:
//      id_buffer - a char array of length ID_SIZE, which will be used
//                  to store the id.
// 
// Usage: 
// ```
//      char id[ID_SIZE];
//      scan_id(id);
// ```
void scan_id(char id_buffer[ID_SIZE]) {
    scan_token(id_buffer, ID_SIZE);
}


// Scans a string and converts it to a carriage_type.
//
// Returns:
//      The corresponding carriage_type, if the string was valid,
//      Otherwise, returns INVALID_TYPE.
// 
// Usage: 
// ```
//      enum carriage_type type = scan_type();
// ```
//
enum carriage_type scan_type(void) {
    // This 20 should be #defined, but we've kept it like this to
    // avoid adding additional constants to your code.
    char type[20];
    scan_token(type, 20);
    return string_to_type(type);
}


// Formats and prints out a train carriage struct,
//
// Parameters:
//      carriage - The struct carriage to print.
// 
void print_carriage(struct carriage *carriage) {
    int line_length = 20;

    char *id = carriage->carriage_id;
    char *type = type_to_string(carriage->type);

    printf(" ---------\\/--------- \n");

    int padding = line_length - strlen(id);
    printf("|%*s%s%*s|\n", padding / 2, "", id, (padding + 1) / 2, "");

    padding = line_length - 2 - strlen(type);
    printf("|%*s(%s)%*s|\n", padding / 2, "", type, (padding + 1) / 2, "");

    printf("| Occupancy: %3d/%-3d |\n", 
            carriage->occupancy, 
            carriage->capacity);
    printf(" ---------||--------- \n");
}


// Formats and prints out various information about a given train.
//
// Parameters:
//      is_selected - 1, if this train is the currently selected train, 
//                    0, otherwise.
//      n           - The position of the given train in the list of trains, 
//                    starting from 0.
//      capacity    - The total capacity of the given train.
//      capacity    - The total occupancy of the given train
//      num_carriages   - The number of carriages in the given train.
//
void print_train_summary(
    int is_selected, 
    int n, 
    int capacity, 
    int occupancy,
    int num_carriages
) {
    if (is_selected) {
        printf("--->Train #%d\n", n);
    } else  {
        printf("    Train #%d\n", n);
    }

    printf("        Carriages: %3d\n", num_carriages);
    printf("        Capacity : %3d/%-3d\n", occupancy, capacity);
    printf("    ----------------------\n");

}



// Compares two double (floating point) values. Value are considered
// equal if there is a less than 0.01 difference between them.
// Note: You should use this function if you need to compare doubles
//       to eachother, as it reduces inconsistencies caused by double
//       imprecision.
//
// Parameters:
//      n1 - a floating point value
//      n2 - a floating point value
// 
// Returns:
//      0, if the two values are considered equal.
//      a negative number, if n1 is less than n2,
//      a positive number, if n2 is less than n1,
//
// Usage: 
// ```
//      if (compare_double(n1, n2) > 0) {
//          printf("n1 greater than n2\n");
//      } else if (compare_double(n1, n2) == 0) {
//          printf("n1 is equal to n2\n");
//      } else {
//          printf("n1 is less than n2\n");
//      }
// ```
int compare_double(double n1, double n2) {
    double delta = 0.01;

    double difference = n1 - n2;
    // abs(n1 - n2) < delta 
    if (difference < delta && difference > -delta) {
        return 0;
    }
    if (n1 < n2) {
        return -1;
    }

    return 1;
}

enum carriage_type string_to_type(char *type_str) {
    int len = strlen(type_str);

    if (strncasecmp(type_str, "passenger", len) == 0) {
        return PASSENGER;
    } 
    if (strncasecmp(type_str, "buffet", len) == 0) {
        return BUFFET;
    } 
    if (strncasecmp(type_str, "restroom", len) == 0) {
        return RESTROOM;
    }
    if (strncasecmp(type_str, "first_class", len) == 0) {
        return FIRST_CLASS;
    } 

    return INVALID_TYPE;
}


char *type_to_string(enum carriage_type type) {
    if (type == PASSENGER) {
        return "PASSENGER";
    } else if (type == BUFFET) {
        return "BUFFET";
    } else if (type == FIRST_CLASS) {
        return "FIRST CLASS";
    } else if (type == RESTROOM) {
        return "RESTROOM";
    }

    return "INVALID";
}

int scan_token(char *buffer, int buffer_size) {
    if (buffer_size == 0) {
        return 0;
    }

    char c;
    int i = 0;
    int num_scanned = 0;

    // consume all leading whitespace
    scanf(" ");

    // Scan in characters until whitespace
    while (i < buffer_size - 1
        && (num_scanned = scanf("%c", &c)) == 1 
        && !isspace(c)) {

        buffer[i++] = c;
    }

    if (i > 0) {
        buffer[i] = '\0';
    }

    return num_scanned;
}
