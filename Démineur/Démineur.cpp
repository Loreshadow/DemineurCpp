
#include <iostream>
#include <Windows.h>
#include <string>
#include <chrono>
#include <fstream>

// Include requis pour l'affichage des caractère spéciaux
#include <fcntl.h> 

const int CELL_EMPTY = 0;
const int CELL_MINE = -1;
// caractère UTF8 utilisé dans le jeu.
const char* ICN_HIDDEN = "■ ";
const char* ICN_BLANK = "□ ";
const char* ICN_MINE = "💣";
const char* ICN_FLAG = "🚩";
const char* ICN_EXPLODE = "💥";

// init le tableau a false partout ( toutes les cellules vide par défaut)
void initArray(int* array, int* displayArray, int size) {
	for (int i = 0; i < size; ++i) {
		array[i] = CELL_EMPTY;
		displayArray[i] = 0;
	}
}

void placeMines(int* grid, int size, int mines) {
	int placedMines = 0;
	while (placedMines < mines) {
		int randomIndex = std::rand() % size;
		if (grid[randomIndex] != CELL_MINE) {
			grid[randomIndex] = CELL_MINE;
			placedMines++;
		}
	}
}
int checkNeighbor(int* array, int x, int y, int sizeX, int sizeY) {
	int nb = 0;  
	for (int dirY = -1; dirY <= 1; dirY++) {
		for (int dirX = -1; dirX <= 1; dirX++) {
			if (dirY == 0 && dirX == 0) continue;

			// check pour les bord de la grid
			int bx = x + dirX;
			int by = y + dirY;
			if (bx >= 0 && bx < sizeX && by >= 0 && by < sizeY) {
				if (array[by * sizeX + bx] == CELL_MINE) {
					nb++;
				}
			}
		}
	}
	return nb;
}

void countCloseMines(int* array, int sizeX, int sizeY) {
	for (int y = 0; y < sizeY; y++) {
		for (int x = 0; x < sizeX; x++) {
			int index = y * sizeX + x;
			if (array[index] != CELL_MINE) {
				int mines = checkNeighbor(array, x, y, sizeX, sizeY);
				array[index] = mines;
			}
		}
	}
}

void revealCascade(int* realGrid, int* showingGrid, int startX, int startY, int sizeX, int sizeY) {
	int totalSize = sizeX * sizeY;
	int* queue = new int[totalSize];

	int readIndex = 0;  // début
	int writeIndex = 0; // fin

	// on fait le départ
	int startIndex = startY * sizeX + startX;
	showingGrid[startIndex] = 1;
	queue[writeIndex] = startIndex;
	writeIndex++;

	// ne pas stop tant que le début et la fin ne se rejoignent pas
	while (readIndex < writeIndex) {

		int currentIndex = queue[readIndex];
		readIndex++;
		if (realGrid[currentIndex] == CELL_MINE) continue;
		// si c'est vide on va chercher les voisins
		if (realGrid[currentIndex] == CELL_EMPTY) {

			int x = currentIndex % sizeX; // retrouve la colonne
			int y = currentIndex / sizeX; // retrouve la ligne

			for (int dirY = -1; dirY <= 1; dirY++) {
				for (int dirX = -1; dirX <= 1; dirX++) {
					int bx = x + dirX;
					int by = y + dirY;

					if (bx >= 0 && bx < sizeX && by >= 0 && by < sizeY) {
						int neighborIndex = by * sizeX + bx;

						// ajoute la case voisine a la queue
						if (showingGrid[neighborIndex] == 0) {

							showingGrid[neighborIndex] = 1; 
							queue[writeIndex] = neighborIndex;
							writeIndex++;
						}
					}
				}
			}
		}
	}


	
	delete[] queue;
	queue = nullptr;
}

// crée et initialise la grid dans la console.
void displayGrid(int* realGrid, int* showingGrid, int sizeX, int sizeY, int cursorX, int cursorY, bool isDead,int score,int totalGames) {
	std::cout << "===============================================================\n Démineur Console C++\n Score :" << score << " Nombres total de manche: " << totalGames << "\n===============================================================";
	std::cout << "\n    ";
	for (int j = 0; j < sizeX; ++j) {
		if (j < 10) std::cout << j << "   ";
		else        std::cout << j << "  ";
	}
	std::cout << "\n   ";
	for (int j = 0; j < sizeX; ++j) std::cout << "----";
	std::cout << "\n";

	for (int i = 0; i < sizeY; ++i) {
		if (i < 10) std::cout << i << "  | ";
		else        std::cout << i << " | ";

		for (int j = 0; j < sizeX; ++j) {
			int index = i * sizeX + j;
			bool isCursor = (j == cursorX && i == cursorY);

			std::string icon = "";
			if (isCursor && isDead) {
				icon = ICN_EXPLODE;
			}
			else if (showingGrid[index] == 0) {
				icon = ICN_HIDDEN;
			}
			else if (showingGrid[index] == 2) {
				icon = ICN_FLAG;
			}
			else {
				if (realGrid[index] == CELL_MINE) {
					icon = ICN_MINE;
				}
				else if (realGrid[index] == CELL_EMPTY) {
					icon = ICN_BLANK;
				}
				else {
					// ajoute un espace au chiffre a coté des mines
					icon = std::to_string(realGrid[index]) + " ";
				}
			}
			if (isCursor && !isDead) {
				std::cout << "[" << icon << "]";
			}
			else {
				std::cout << " " << icon << " ";
			}
		}
		std::cout << "\n";
	}
	std::cout << "\n";
}

bool checkWinCondition(int* realGrid, int* showingGrid, int size) {
	for (int i = 0; i < size; ++i) {
		if (realGrid[i] != CELL_MINE && showingGrid[i] != 1) {
			return false;
		}
	}
	return true;
}

void saveGame(int score, int totalGames) {
	std::ofstream saveGame("Save.txt");
	if (saveGame.is_open()) {
		saveGame << score << "\n";
		saveGame << totalGames << "\n";

		saveGame.close();
		std::cout << "Sauvegarde effectué.";

	}
	else {
		std::cout << "Erreur dans la sauvegarde, impossible d'ouvrir le fichier de sauvegarde.";
	}
}
void LoadSavedGame() {
	std::ifstream savedGame("Save.txt");
	if (savedGame.is_open()) {
		std::string lines;
		while (std::getline(savedGame, lines)) {
			std::cout << lines << "\n";
		}


		savedGame.close();
	}
	else {
		std::cout << "Impossible de lire le fichier de sauvegarde.";
	}
}



int main()
{
	// console UTF-8
	SetConsoleOutputCP(65001);
	
	std::srand(std::time(nullptr));
	bool isGame = true;

	// joueur 
	int playerX = 0, playerY = 0;
	bool isDead = false;
	int score = 0;
	int totalGames = 0;

	int sizeY = 0, sizeX = 0, mines = 0;
	// choix taille par l'utilisateur
	std::cout << "Choisissez la taille du tableau pour le démineur\n";
	std::cout << "Taille X :\n";
	std::cin >> sizeX;
	std::cout << "Taille Y :\n";
	std::cin >> sizeY;
	std::cout << "Nombre de mines :\n";
	std::cin >> mines;
	std::cout << "\n";
	int totalSize = sizeY * sizeX;
	if (mines >= totalSize) {
		std::cout << "Le nombre de mines doit être inférieur à la taille du tableau.\n";
		return 1;
	}


	// définition des grid	
	int* realGrid = new int[totalSize];
	int* showingGrid = new int[totalSize];
	// initialisation des grids.
	initArray(realGrid, showingGrid, totalSize);
	placeMines(realGrid, totalSize, mines);;
	countCloseMines(realGrid, sizeX, sizeY);
	

	bool updateDisplay = true;
	int revealCount = 0;
	int correctFlag = 0;
	long long timeSinceLastInput = 0;
	int delay = 300;
	std::string isSaving = false;

	while (isGame) {
		if (updateDisplay) {
			SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), { 0, 0 });
			displayGrid(realGrid, showingGrid, sizeX, sizeY, playerX, playerY, isDead, score, totalGames);
			LoadSavedGame();
			updateDisplay = false;
		}

		if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
			isDead = true;
			isGame = false;
		}
		auto currentTime = std::chrono::steady_clock::now();
		long long timespec = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime.time_since_epoch()).count();
		if (timespec - timeSinceLastInput > delay) {

			if (GetAsyncKeyState(VK_LEFT) & 0x8000) {
				if (playerX > 0) playerX--;
				updateDisplay = true;
				timeSinceLastInput = timespec;
			}
			if (GetAsyncKeyState(VK_RIGHT) & 0x8000) {
				if (playerX < sizeX - 1) playerX++;
				updateDisplay = true;
				timeSinceLastInput = timespec;
			}
			if (GetAsyncKeyState(VK_UP) & 0x8000) {
				if (playerY > 0) playerY--;
				updateDisplay = true;
				timeSinceLastInput = timespec;
			}
			if (GetAsyncKeyState(VK_DOWN) & 0x8000) {
				if (playerY < sizeY - 1) playerY++;
				updateDisplay = true;
				timeSinceLastInput = timespec;
			}

			if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
				int index = playerY * sizeX + playerX;
				if (showingGrid[index] == 0) {
					revealCount++;
					updateDisplay = true;
					// triche pour pas que le joueur perde à la première case reveal
					if (realGrid[index] == CELL_MINE && revealCount == 1) {
						int newMineIndex;
						do {
							newMineIndex = std::rand() % totalSize;
						} while (realGrid[newMineIndex] == CELL_MINE || showingGrid[newMineIndex] == 1);
						realGrid[newMineIndex] = CELL_MINE;
						realGrid[index] = CELL_EMPTY;
						countCloseMines(realGrid, sizeX, sizeY);

					}
					else if (realGrid[index] == CELL_MINE) {
						for (int i = 0; i < totalSize; i++) {
							SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), { 0, 0 });
							displayGrid(realGrid, showingGrid, sizeX, sizeY, playerX, playerY, isDead, score, totalGames);
							showingGrid[i] = 1;
						}
						std::cout << "BOOM! Vous avez touché une mine!\n";
						totalGames++;
						isDead = true;
					}
					// reveal toutes les cases vides qui se touchent
					revealCascade(realGrid, showingGrid, playerX, playerY, sizeX, sizeY);
					timeSinceLastInput = timespec;
				}
			}

			if (GetAsyncKeyState('F')) {
				int index = playerY * sizeX + playerX;
				if (showingGrid[index] == 0) {
					showingGrid[index] = 2; 
					if (showingGrid[index] == realGrid[index] == CELL_EMPTY) {
						correctFlag += 1;
					}
				}
				else if (showingGrid[index] == 2) {
					showingGrid[index] = 0; 
				}
				updateDisplay = true;
				timeSinceLastInput = timespec;
			}
		}

		if (checkWinCondition(realGrid, showingGrid, totalSize) && !isDead) {
			std::cout << "Félicitations! Vous avez gagné!\n";
			score++;
			totalGames++;
			isDead = true;
		}
		if (GetAsyncKeyState('R')) {
			isGame = false;
			initArray(realGrid, showingGrid, totalSize);
			placeMines(realGrid, totalSize, mines);;
			countCloseMines(realGrid, sizeX, sizeY);
			saveGame(score, totalGames);
			isDead = false;
			revealCount, correctFlag = 0;
			updateDisplay, isGame = true;
		}
		if (isDead == true) {
			std::cout << "Voulez vous sauvegarder votre partie ? y/n";
			std::cin >> isSaving;
			//match
		}
		

	}
	delete[] realGrid;
	realGrid = nullptr;
	delete[] showingGrid;
	showingGrid = nullptr;
}

