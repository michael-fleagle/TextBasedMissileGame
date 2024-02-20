// Michael Fleagle
// CS470 Lab 3

#include <iostream>
#include <pthread.h>
#include <stdlib.h>
#include <fstream>
#include <random>
#include <unistd.h>


//namespace declarations
using namespace std;


// create soldier info struct to hold information needed for making soldiers
typedef struct soldIntel_
{
	int rows;
	int cols;
	char type;
	
} soldIntel;


// mutex solution to shared variable
pthread_mutex_t mutex;


// global game finished flag
bool GAME_COMPLETE = false;


// generate random number within provided max
int randomNum(int max)
{
	int num;
	
	// generate random number between 0-max
	num = rand() % max;
	
	return num;
}


// method to check if a coordinate is out of bounds (used for majority area checking)
bool isSafe(int x, int y, int rows, int columns) 
{
	// return true if the coordinate is within the bounds
	return (x >= 0 && x < rows && y >= 0 && y < columns);
}


// method to handle missile firings from the soldier function
void missileFire(int firedFrom, int rows, int cols, char team)
{
	if(GAME_COMPLETE == false)
	{
		// get size from input row and columns
		int size = rows * cols;
		
		// generate coordinate to fire missile at
		int x = randomNum(rows);
		int y = randomNum(cols);
		
		// convert x, y coordinate to 1d coordinate
		int coord = y + (x * cols);
		
		// print the fire from
		cout << "Team " << team << " fired a missile from (" << (firedFrom / cols) << ", " << (firedFrom % cols) << ") to (" << x << ", " << y << ")" << endl;
		
		// create the soldiers in the file at random location (must not be occupied by other soldier)
		// create array to hold data
		char fileCheck[size];
			
		// check if the generated coordinate is viable for missile placement
		fstream mapFile;
		fstream writeToFile;
		
		// mutex lock
		pthread_mutex_lock(&mutex);
		
		// open the file for reading
		mapFile.open("mapBin.bin", ios::binary | ios::in);
		
		// read the information from the file
		mapFile.read((char *)&fileCheck, size);
		
		// close the file for reading
		mapFile.close();
		
		// check the conditions for the generated coordinate
		if(fileCheck[coord] == '\0' || fileCheck[coord] == 'a' || fileCheck[coord] == 'b')
		{
			// Team A hits a non team a location
			if(team == 'A' && fileCheck[coord] != 'a' && fileCheck[coord] != 'B')
			{	
				cout << "Missile successfully hit. Converting to Team A" << endl;
				// write the representation to the map
				fileCheck[coord] = 'a';
			}
			// Team B hits a non team b location
			else if(team == 'B' && fileCheck[coord] != 'b' && fileCheck[coord] != 'A')
			{	
				cout << "Missile successfully hit. Converting to Team B" << endl;
				// write the representation on the map
				fileCheck[coord] = 'b';
			}
			// Team A hits a team a location
			else if(team == 'A' && fileCheck[coord] == 'a')
			{
				cout << "Missile hit a friendly unit. Territory lost by Team A" << endl;
				// update the map
				fileCheck[coord] = '\0';
			}
			// Team B hits a team b location 
			else if(team == 'B' && fileCheck[coord] == 'b')
			{
				cout << "Missile hit a friendly unit. Territory lost by Team B" << endl;
				// update the map
				fileCheck[coord] = '\0';
			}
			else
			{
				cout << "Missile was a dud" << endl;
			}

		}
		
		// check for majority in area around coord
		// (x,y)
        // Considering 8 directions up, down , right, left, diagnals
        int count = 0;
		int numAdjacent = 0;
		
		char teamRep;
		
		// check which team to check for after the update
		if(team == 'A')
		{
			teamRep = 'a';
		}
		else
		{
			teamRep = 'b';
		}
		
		// one cell up
        if(isSafe(x - 1, y, rows, cols)) 
		{ 
			numAdjacent++;
            if(fileCheck[y + ((x - 1) * cols)] == teamRep || fileCheck[y + ((x - 1) * cols)] == team) {
                count ++;
            }
        }           
		
		// one cell down
        if(isSafe (x + 1, y, rows, cols)) 
		{
			numAdjacent++;
            if(fileCheck[y + ((x + 1) * cols)] == teamRep || fileCheck[y + ((x + 1) * cols)] == team) {
                count ++;
            }
        }
		
		// one cell to the left
        if(isSafe(x, y - 1, rows, cols)) 
		{
			numAdjacent++;
            if(fileCheck[(y - 1) + (x * cols)] == teamRep || fileCheck[(y - 1) + (x * cols)] == team) {
                count ++;
            }
        }
		
		// one cell to the right
        if(isSafe(x, y + 1, rows, cols)) 
		{ 
			numAdjacent++;
            if(fileCheck[(y + 1) + (x * cols)] == teamRep || fileCheck[(y + 1) + (x * cols)] == team) {
                count ++;
            }
        }
		
		// diagnal up left
		if(isSafe((x - 1), (y - 1), rows, cols))
		{
			numAdjacent++;
			if(fileCheck[(y - 1) + ((x - 1) * cols)] == teamRep || fileCheck[(y - 1) + ((x - 1) * cols)] == team) {
                count ++;
            }
		}
		
		// diagnal down left
		if(isSafe((x + 1), (y - 1), rows, cols))
		{
			numAdjacent++;
			if(fileCheck[(y - 1) + ((x + 1) * cols)] == teamRep || fileCheck[(y - 1) + ((x + 1) * cols)] == team) {
                count ++;
            }
		}
		
		// diagnal up right
		if(isSafe((x - 1), (y + 1), rows, cols))
		{
			numAdjacent++;
			if(fileCheck[(y + 1) + ((x - 1) * cols)] == teamRep || fileCheck[(y + 1) + ((x - 1) * cols)] == team) {
                count ++;
            }
		}
		
		// diagnal down right
		if(isSafe((x + 1), (y + 1), rows, cols))
		{
			numAdjacent++;
			if(fileCheck[(y + 1) + ((x + 1) * cols)] == teamRep || fileCheck[(y + 1) + ((x + 1) * cols)] == team) {
                count ++;
            }
		}
		
		// if a single team control the majority change all spaces to be that teams
		if(count > (numAdjacent / 2))
		{
			// one cell up
			if(isSafe(x - 1, y, rows, cols) && fileCheck[y + ((x - 1) * cols)] != 'A' && fileCheck[y + ((x - 1) * cols)] != 'B') 
			{ 
				fileCheck[y + ((x - 1) * cols)] = teamRep;
			}           
			
			// one cell down
			if(isSafe (x + 1, y, rows, cols) && fileCheck[y + ((x + 1) * cols)] != 'A' && fileCheck[y + ((x + 1) * cols)] != 'B') 
			{
				fileCheck[y + ((x + 1) * cols)] = teamRep;
			}
			
			// one cell to the left
			if(isSafe(x, y - 1, rows, cols) && fileCheck[(y - 1) + (x * cols)] != 'A' && fileCheck[(y - 1) + (x * cols)] != 'B') 
			{
				fileCheck[(y - 1) + (x * cols)] = teamRep;
			}
			
			// one cell to the right
			if(isSafe(x, y + 1, rows, cols) && fileCheck[(y + 1) + (x * cols)] != 'A' && fileCheck[(y + 1) + (x * cols)] != 'B') 
			{ 
				fileCheck[(y + 1) + (x * cols)] = teamRep;
			}
			
			// diagnal up left
			if(isSafe((x - 1), (y - 1), rows, cols) && fileCheck[(y - 1) + ((x - 1) * cols)] != 'A' && fileCheck[(y - 1) + ((x - 1) * cols)] != 'B')
			{
				fileCheck[(y - 1) + ((x - 1) * cols)] = teamRep;
			}
			
			// diagnal down left
			if(isSafe((x + 1), (y - 1), rows, cols) && fileCheck[(y - 1) + ((x + 1) * cols)] != 'A' && fileCheck[(y - 1) + ((x + 1) * cols)] != 'B')
			{
				fileCheck[(y - 1) + ((x + 1) * cols)] = teamRep;
			}
			
			// diagnal up right
			if(isSafe((x - 1), (y + 1), rows, cols) && fileCheck[(y + 1) + ((x - 1) * cols)] != 'A' && fileCheck[(y + 1) + ((x - 1) * cols)] != 'B')
			{
				fileCheck[(y + 1) + ((x - 1) * cols)] = teamRep;
			}
			
			// diagnal down right
			if(isSafe((x + 1), (y + 1), rows, cols) && fileCheck[(y + 1) + ((x + 1) * cols)] != 'A' && fileCheck[(y + 1) + ((x + 1) * cols)] != 'B')
			{
				fileCheck[(y + 1) + ((x + 1) * cols)] = teamRep;
			}
		}
		
		// open the file for writing
		writeToFile.open("mapBin.bin", ios::binary | ios::out | ios::trunc);
		
		// write updated array to the file
		for(int i = 0; i < size; i++)
		{
			writeToFile.write(&fileCheck[i], sizeof(char));
		}
		
		
		// close writing buffer
		writeToFile.close();
		
		// unlock the mutex
		pthread_mutex_unlock(&mutex);
	}
}


// thread method to handle game master thread
void * gameMasterFunc(void * param)
{
	// cout << "Entering gameMaster" << endl;
	// get the parameter struct from the param
	soldIntel * master = (soldIntel *)param;
	
	// while loop to check if the game is complete
	while(GAME_COMPLETE == false)
	{
		
		// create array to hold data from file 
		char mapCheck[(master->rows) * (master->cols)];
		
		// get the size of the data in the file
		int size = (master->rows) * (master->cols);
		
		// create fstream buffer
		fstream mapFile;
		
		// lock mutex
		pthread_mutex_lock(&mutex);
		
		mapFile.open("mapBin.bin", ios::binary | ios::in);
		
		// read the data from the file
		mapFile.read((char *)&mapCheck, size);
		
		// close the file
		mapFile.close();
		
		// unlock mutex
		pthread_mutex_unlock(&mutex);
		
		
		// print the map as a 2d map as intended 
		for(int i = 0; i < master->rows; i++)
		{
			for(int j = 0; j < master->cols; j++)
			{
				if(mapCheck[j + (i * master->cols)] == '\0')
				{
					cout << '0';
				}
				else
				{
					cout << mapCheck[j + (i * master->cols)];
				}
			}
			cout << endl;
		}
		cout << endl << endl;
		
		// create counter to check for number of filled spots
		int counter = 0;
		
		int numTA = 0;
		int numTB = 0;
		
		// check if the game is complete by iterating over all of the data
		for(int i = 0; i < (size); i++)
		{
			// if any space has a '\0' then the game is not over
			if(mapCheck[i] == '\0')
			{
				break;
			}
			else
			{
				// increase the completion counter
				counter++;
				
				// count the number of each type to determine who wins
				if(mapCheck[i] == 'A' || mapCheck[i] == 'a')
				{
					numTA++;
				}
				else if(mapCheck[i] == 'B' || mapCheck[i] == 'b')
				{
					numTB++;
				}
				
				// check if the counter is equal to the size
				if(counter == size)
				{
					// check who won
					if(numTA > numTB)
					{
						cout << "Game complete. The winner is Team A with " << numTA << " controlled spaces" << endl;
					}
					else
					{
						cout << "Game complete. The winner is Team B with " << numTB << " controlled spaces" << endl;
					}
					GAME_COMPLETE = true;
				}
			}
		}
		
		sleep(1);
	}
		
	return (void *)0;
}


// thread method to handle each soldier
void * soldierFunc(void * param)
{ 
	// check if the game complete flag has been set to true
	if(GAME_COMPLETE == true)
	{
		return (void *)0;
	}
	
	// get the data from the passed structure
	soldIntel * soldParam = (soldIntel *)param;
	
	int size = soldParam->rows * soldParam->cols;
	
	// generate coordinate to place soldier
	int x = randomNum(soldParam->rows);
	int y = randomNum(soldParam->cols);
	
	// convert x, y coordinate to 1d coordinate
	int coord = y + (x * soldParam->cols);
	
	// create the soldiers in the file at random location (must not be occupied by other soldier)
	// create array to hold data
	char fileCheck[size];
		
	// check if the generated coordinate is viable for soldier placement
	fstream mapFile;
	fstream writeToFile;
	
	pthread_mutex_lock(&mutex);
	
	mapFile.open("mapBin.bin", ios::binary | ios::in);
	
	mapFile.read((char *)&fileCheck, size);
	
	// create boolean flag for if a spot is found where it can go
	bool spotFound = false;
	
	// while loop to find a coordinate which works 
	while(spotFound == false)
	{
		if(fileCheck[coord] == '\0')
		{
			fileCheck[coord] = soldParam->type;
			spotFound = true;
		}
		else
		{
			// if the generated coordinate is not available, find another available space
			coord += 1;
		}
	}
	
	// close the file for reading
	mapFile.close();
	
	// open the file for writing
	writeToFile.open("mapBin.bin", ios::binary | ios::out | ios::trunc);
	
	// write updated array to the file
	for(int i = 0; i < size; i++)
	{
		writeToFile.write(&fileCheck[i], sizeof(char));
	}
	
	
	// close writing buffer
	writeToFile.close();
	
	// unlock the mutex
	pthread_mutex_unlock(&mutex);
	
	// fire the missiles of the soldier until the game is over
	while(GAME_COMPLETE == false)
	{
		if(soldParam->type == 'A')
		{
			missileFire(coord, soldParam->rows, soldParam->cols, 'A');
		}
		// then type is 'B'
		else
		{
			missileFire(coord, soldParam->rows, soldParam->cols, 'B');
		}
		
		// sleep function for reloading
		int sleepVal = randomNum(3);
		
		sleep(sleepVal);
	}
	
	return (void *)0;
}


// method to create the game board
// game board is represented as a file
// translated from 2d (x, y) to 1d [y + (x * c)]
bool createBoard(int r, int c)
{	
	// create 1d array of '\0' (null) character
	char map[r * c];
	
	for(int i = 0; i < r; i++)
	{
		for(int j = 0; j < c; j++)
		{
			map[j + (i * c)] = '\0';
		}
	}
	
	// create a binary file to represent and store the board
	fstream binFile;
	
	// open the file
	binFile.open("mapBin.bin", ios::binary | ios::out | ios::trunc);
	
	// if the binary file did not open, return false for trying to create the map
	if(!binFile.is_open())
	{
		return false;
	}
	
	// initate the file with the map char array of size r * c
	for(int i =0; i < (r * c); i++)
	{
		binFile.write(&map[i], sizeof(char));
		
		// flush the stream
		binFile.flush();
	}
	
	// close the file
	binFile.close();
	
	// return true for successful map creation
	return true;
}


// method to dermine if the given input argments are correct and prints the corresponding message
// returns false if input is invalid
// returns true if input is valid
bool validInput(int numArgs, char *args[])
{
	// check if the number of inputs is greater than 4
	if(numArgs > 5)
	{
		cout << "Error: Too many arguments. The correct format is: " 
				<< "\nexecFile TA TB R C"
				<< "\n\tTA = number of Team A soldiers \n\tTB = number of Team B soldiers \n\tR = number of rows in the map \n\tC = number of columns in the map" 
				<< "\nAll arguments should be positive integer values. Any argument which is input as a decimal poin will be treated as it was its floored value (Ex: 1.5 -> 1)"
				<< endl;
				
		return false;
	}
	// check if the number of inputs is less than 4
	else if(numArgs < 5)
	{
		cout << "Error: Too few arguments. The correct format is: "
				<< "\nexecFile TA TB R C"
				<< "\n\tTA = number of Team A soldiers \n\tTB = number of Team B soldiers \n\tR = number of rows in the map \n\tC = number of columns in the map" 
				<< "\nAll arguments should be positive integer values. Any argument which is input as a decimal poin will be treated as it was its floored value (Ex: 1.5 -> 1)"
				<< endl;
				
		return false;
	}
	else
	{
		// check that all of the arguments are integer values
		for(int i = 1; i < numArgs; i++)
		{
			int val = atoi(args[i]);
			
			// 0 means letter or 0 as input
			if(val == 0)
			{
				cout << "Error: One or more arguments are not integer value or is 0. All arguments should be positive integers greater than 0" << endl;
				cout << "Any argument which is input as a decimal poin will be treated as it was its floored value (Ex: 1.5 -> 1)" << endl;
				return false;
			}
		}
		
		// create variables to hold data needed to check if there are more soldiers than spaces
		int soldierTotal = atoi(args[1]) + atoi(args[2]);
		
		int totalSpaces = atoi(args[3]) * atoi(args[4]);
		
		// check if there are more soldiers than spaces
		if(soldierTotal > totalSpaces)
		{
			cout << "Error: There are too many soldiers for the map size. Please try again." << endl;
			return false;
		}
		
		return true;
	}
}


// main method
int main(int argc, char * argv[])
{
	// if the input is invalid, exit
	if(!validInput(argc, argv))
	{
		cout << "Exiting" << endl;
		return 0;
	}
	
	// collect the inputs from the arguments provided by the user
	int numTA = atoi(argv[1]);
	int numTB = atoi(argv[2]);
	int mapRows = atoi(argv[3]);
	int mapCols = atoi(argv[4]);
	int mapSize = mapRows * mapCols;
	
	// create the game board
	if(!createBoard(mapRows, mapCols))
	{
		cout << "Error creating map" << endl;
		cout << "Exiting" << endl;
		return 0;
	}
	
	// seed random number generator
	srand(time(0));
	
	// create game master thread to continuously check if the game is done
	pthread_t gameMaster;
	
	// create struct for game master
	soldIntel * gameMast = (soldIntel *)malloc(sizeof(soldIntel));
	
	// initialize struct
	gameMast->rows = mapRows;
	gameMast->cols = mapCols;
	gameMast->type = 'Q';
	
	pthread_create(&gameMaster, NULL, gameMasterFunc, (void *)gameMast);
	
	// create threads for each soldier
	pthread_t teamA[numTA];
	pthread_t teamB[numTB];
	
	// team a soldiers
	for(int i = 0; i < numTA; i++)
	{
		// create team a objects
		soldIntel * soldIntelA = (soldIntel *)malloc(sizeof(soldIntel));
		
		// initialize the struct
		soldIntelA->rows = mapRows;
		soldIntelA->cols = mapCols;
		soldIntelA->type = 'A';
		
		// create threads
		pthread_create(&teamA[i], NULL, soldierFunc, (void *)soldIntelA);
	}
	
	// team b soldiers
	for(int i = 0; i < numTB; i++)
	{
		// create team a objects
		soldIntel * soldIntelB = (soldIntel *)malloc(sizeof(soldIntel));
		
		// initialize the struct
		soldIntelB->rows = mapRows;
		soldIntelB->cols = mapCols;
		soldIntelB->type = 'B';
		
		// create threads
		pthread_create(&teamB[i], NULL, soldierFunc, (void *)soldIntelB);
	}
	
	// Join game master thread
	pthread_join(gameMaster, NULL);
	
	// Join threads for team A
	for(int i = 0; i < numTA; i++)
	{
		// join threads
		pthread_join(teamA[i], NULL);
	}
	
	// Join threads for team B
	for(int i = 0; i < numTB; i++)
	{
		// join threads
		pthread_join(teamB[i], NULL);
	}
	
	free(gameMast);
	
	return 1;
	
}