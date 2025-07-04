#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HASH_SIZE 15000
#define STRING_SIZE 30
#define LINE_SIZE 700
#define NUM_OF_ELEMENTS 10

int time = 0;

typedef struct Recipe {
	char name[STRING_SIZE];
	char ingredients[NUM_OF_ELEMENTS][STRING_SIZE];
	unsigned int hashOfIngredient[NUM_OF_ELEMENTS];
	int quantity[NUM_OF_ELEMENTS];
	int num_of_ingredients;
	int leastAmountNotMakable;
	int timeOfLeastAmount;
	struct Recipe* next;
} Recipe;

typedef struct Batch {
	char name[STRING_SIZE];
	unsigned int hashOfBatch;
	int quantity;
	int expiration;
	struct Batch* next;
} Batch;

typedef struct Storage {
	Batch* batches[HASH_SIZE];
} Storage;

typedef struct RecipesHashMap {
	Recipe* recipes[HASH_SIZE];
} RecipesHashMap;

typedef struct Order {
	char name[STRING_SIZE];
	int quantity;
	int totalWeight;
	int time;
	struct Order* next;
} Order;

typedef struct WaitingOrder {
	Recipe* recipe;
	int quantity;
	int time;
	struct WaitingOrder* next;
} WaitingOrder;

typedef struct WaitingList {
	WaitingOrder* head;
	WaitingOrder* tail;
} WaitingList;

Order* orderList;
WaitingList* waitingList;

// MurmurOAAT_32 hash function mapped on hash_size
// Collision free and high performance hash function
unsigned int hash_function(const char* str) {
	int h = 0;
	for (; *str; ++str) {
		h ^= *str;
		h *= 0x5bd1e995;
		h ^= h >> 15;
	}
	return h % HASH_SIZE;
}


Recipe* createRecipe(RecipesHashMap* recipesHashMap, char* recipeName) {
	unsigned int hash = hash_function(recipeName);
	Recipe* recipeComparator = recipesHashMap->recipes[hash];
	while(recipeComparator != NULL) {
		if(strcmp(recipeComparator->name, recipeName) == 0) {
			return NULL;
		}
		recipeComparator = recipeComparator->next;
	}
	Recipe* recipe = (Recipe*) malloc(sizeof(Recipe));
	strncpy(recipe->name, recipeName, STRING_SIZE - 1);
	recipe->name[STRING_SIZE - 1] = '\0';
	recipe->num_of_ingredients = 0;
	recipe->leastAmountNotMakable = 0;
	recipe->timeOfLeastAmount = 0;
	recipe->next = recipesHashMap->recipes[hash];
	recipesHashMap->recipes[hash] = recipe;
	return recipe;
}

void addIngredient(Recipe* recipe, char* ingredientName, int quantity) {
	strncpy(recipe->ingredients[recipe->num_of_ingredients], ingredientName, STRING_SIZE - 1);
	recipe->ingredients[recipe->num_of_ingredients][STRING_SIZE - 1] = '\0';
	recipe->hashOfIngredient[recipe->num_of_ingredients] = hash_function(ingredientName);
	recipe->quantity[recipe->num_of_ingredients] = quantity;
	recipe->num_of_ingredients += 1;
}

void deleteRecipe(RecipesHashMap* recipesHashMap, char* recipeName, Order** orderList, WaitingList** waitingList) {
	unsigned int hash = hash_function(recipeName);
	if(recipesHashMap->recipes[hash] == NULL) {
		printf("non presente\n");
		return;
	}
	
	Order* currentOrder = *orderList;
	while(currentOrder != NULL) {
		if(strcmp(currentOrder->name, recipeName) == 0) {
			printf("ordini in sospeso\n");
			return;
		}
		currentOrder = currentOrder->next;
	}
	
	WaitingOrder* waitingOrder = (*waitingList)->head;
	while(waitingOrder != NULL) {
		if(strcmp(waitingOrder->recipe->name, recipeName) == 0) {
			printf("ordini in sospeso\n");
			return;
		}
		waitingOrder = waitingOrder->next;
	}
	
	Recipe* previousRecipe = recipesHashMap->recipes[hash];
	if(strcmp(previousRecipe->name, recipeName) == 0) {
		recipesHashMap->recipes[hash] = previousRecipe->next;
		free(previousRecipe);
		printf("rimossa\n");
		return;
	} else {
		Recipe* toRemove = previousRecipe->next;
		while(toRemove) {
			if (strcmp(toRemove->name, recipeName) == 0) {
				previousRecipe->next = toRemove->next;
				free(toRemove);
				printf("rimossa\n");
				return;
			}
			previousRecipe = toRemove;
			toRemove = toRemove->next;
		}
		printf("non presente\n");
	}
}

int checkIfCompletable(RecipesHashMap* recipesHashMap, Storage* storage, Recipe* recipe, int quantity) {
	if(recipe->timeOfLeastAmount == time && quantity > recipe->leastAmountNotMakable) {
		return 0;
	} else {
		for(int j = 0; j < recipe->num_of_ingredients; j++) {
			char* ingredientName = recipe->ingredients[j];
			unsigned int hash = recipe->hashOfIngredient[j];
			Batch* batch = storage->batches[hash];
			Batch* previousBatch = NULL;
			int total = 0;
			
			while(batch != NULL) {
				if(batch->expiration < time) {
					if(previousBatch == NULL) {
						storage->batches[hash] = batch->next;
						free(batch);
						batch = storage->batches[hash];
					} else {
						previousBatch->next = batch->next;
						free(batch);
						batch = previousBatch->next;
					}
				} else {
					if(strcmp(batch->name, ingredientName) == 0) {
						total += batch->quantity;
					}
					batch = batch->next;
					if(total >= recipe->quantity[j]*quantity)
						break;
				}
			}
			
			if(total < recipe->quantity[j]*quantity) {
				recipe->leastAmountNotMakable = quantity;
				recipe->timeOfLeastAmount = time;
				return 0;
			}  
		}
		return 1;
	}
}

void removeBatches(Storage* storage, Batch* toRemove, char* batchName, int quantity) {
	int amountLeft = quantity;
	
	Batch* previousBatch = NULL;
	while(toRemove->expiration < time && strcmp(toRemove->name, batchName) != 0) {
		previousBatch = toRemove;
		toRemove = toRemove->next;
	}
	
	if (amountLeft < toRemove->quantity) {
		toRemove->quantity = toRemove->quantity - amountLeft;
		return;
	} else if(amountLeft >= toRemove->quantity) {
		if(previousBatch != NULL) {
			previousBatch->next = toRemove->next;
		} else {
			storage->batches[toRemove->hashOfBatch] = toRemove->next;
		}
		amountLeft = amountLeft - toRemove->quantity;
		Batch* nextBatch = toRemove->next;
		free(toRemove);
		if(amountLeft == 0) {
			return;
		} else {
			return removeBatches(storage, nextBatch, batchName, amountLeft);
		}
	}
}

int calculateTotalWeight(RecipesHashMap* recipesHashMap, Recipe* recipe, int numOfElements) {
	int totalWeight = 0;
	for(int i = 0; i < recipe->num_of_ingredients; i++) {
		totalWeight = totalWeight + recipe->quantity[i];
	}
	return totalWeight * numOfElements;
}

void order(RecipesHashMap* recipesHashMap, Storage* storage, Recipe* recipe, int numOfElements, Order** orderList, int t) {
	Order* order = (Order*) malloc(sizeof(Order));
	
	for(int j = 0; j < recipe->num_of_ingredients; j++) {
		Batch* batch = storage->batches[recipe->hashOfIngredient[j]];
		removeBatches(storage, batch, recipe->ingredients[j], recipe->quantity[j]*numOfElements);
	}
	
	strncpy(order->name, recipe->name, STRING_SIZE - 1);
	order->name[STRING_SIZE - 1] = '\0';
	order->quantity = numOfElements;
	order->totalWeight = calculateTotalWeight(recipesHashMap, recipe, numOfElements);
	order->time = t;
	order->next = NULL;
	
	if(*orderList == NULL) {
		*orderList = order;
	} else {
		Order* currentOrder = *orderList;
		Order* previousOrder = NULL;
		while(currentOrder != NULL && currentOrder->time < order->time) {
			previousOrder = currentOrder;
			currentOrder = currentOrder->next;
		}
		if(previousOrder == NULL) {
			*orderList = order;
			order->next = currentOrder;
		} else {
			order->next = currentOrder;
			previousOrder->next = order;
		}
	}
}

void checkForWaitingOrders(RecipesHashMap* recipesHashMap, Storage* storage, Order** orderList, WaitingList** waitingList) {
	if(*waitingList != NULL) {
		WaitingOrder* waitingOrder = (*waitingList)->head;
		WaitingOrder* previousOrder = NULL;
		while(waitingOrder != NULL) {
			if(checkIfCompletable(recipesHashMap, storage, waitingOrder->recipe, waitingOrder->quantity)) {
				order(recipesHashMap, storage, waitingOrder->recipe, waitingOrder->quantity, orderList, waitingOrder->time);
				if(previousOrder == NULL) {
					(*waitingList)->head = waitingOrder->next;
					free(waitingOrder);
					waitingOrder = (*waitingList)->head;
				} else {
					previousOrder->next = waitingOrder->next;
					if(waitingOrder->next == NULL) {
						(*waitingList)->tail = previousOrder;
					}
					free(waitingOrder);
					waitingOrder = previousOrder->next;
				}
			} else {
				previousOrder = waitingOrder;
				waitingOrder = waitingOrder->next;
			}
		}
	}
}

void addBatch(RecipesHashMap* recipesHashMap, Storage* storage, char* batchName, int quantity, int expiration) {
	if(expiration > time) {
		Batch* batch = (Batch*) malloc(sizeof(Batch));
		strncpy(batch->name, batchName, STRING_SIZE - 1);
		unsigned int hash = hash_function(batchName);
		batch->name[STRING_SIZE - 1] = '\0';
		batch->hashOfBatch = hash;
		batch->quantity = quantity;
		batch->expiration = expiration;
		if(storage->batches[hash] == NULL || storage->batches[hash]->expiration > batch->expiration) {
			batch->next = storage->batches[hash];
			storage->batches[hash] = batch;
			return;
		} else {
			Batch* previousBatch = storage->batches[hash];
			Batch* nextBatch = previousBatch->next;
			while(nextBatch) {
				if(previousBatch->expiration <= expiration && nextBatch->expiration >= expiration) {
					if(previousBatch->expiration == expiration && strcmp(previousBatch->name, batchName) == 0) {
						previousBatch->quantity += quantity;
						free(batch);
						return;
					} else if(nextBatch->expiration == expiration && strcmp(nextBatch->name, batchName) == 0) {
						nextBatch->quantity += quantity;
						free(batch);
						return;
					} else {
						previousBatch->next = batch;
						batch->next = nextBatch;
						return;
					} 
				}
				previousBatch = nextBatch;
				nextBatch = nextBatch->next;
			}
			previousBatch->next = batch;
			batch->next = NULL;
			return;
		}
	}    
}

void loadCourier(Order** orderList, int maxCapacity) {
	int empty = 1;
	int currentLoaded = 0;
	
	Order* currentOrder = *orderList;
	Order* previousOrder = NULL;
	Order* ordersToPrint = NULL;
	
	while(currentOrder != NULL && currentLoaded < maxCapacity) {
		if(currentLoaded + currentOrder->totalWeight < maxCapacity) {
			empty = 0;
			currentLoaded += currentOrder->totalWeight;
			Order* newOrder = (Order*)malloc(sizeof(Order));
			strncpy(newOrder->name, currentOrder->name, STRING_SIZE - 1);
			newOrder->name[STRING_SIZE - 1] = '\0';
			newOrder->time = currentOrder->time;
			newOrder->quantity = currentOrder->quantity;
			newOrder->totalWeight = currentOrder->totalWeight;
			newOrder->next = NULL;
			if(ordersToPrint == NULL) {
				ordersToPrint = newOrder;
			} else {
				Order* printableOrder = ordersToPrint;
				Order* previousPrintableOrder = NULL;
				while(printableOrder != NULL && newOrder->totalWeight < printableOrder->totalWeight) {
					previousPrintableOrder = printableOrder;
					printableOrder = printableOrder->next;
				}
				
				while(printableOrder != NULL && newOrder->totalWeight == printableOrder->totalWeight && newOrder->time > printableOrder->time) {
					previousPrintableOrder = printableOrder;
					printableOrder = printableOrder->next;
				}
				
				if(previousPrintableOrder == NULL) {
					ordersToPrint = newOrder;
					newOrder->next = printableOrder;
				} else {
					newOrder->next = printableOrder;
					previousPrintableOrder->next = newOrder;
				}
			}
			
			if(previousOrder == NULL) {
				*orderList = currentOrder->next;
				free(currentOrder);
				currentOrder = *orderList;
			} else {
				previousOrder->next = currentOrder->next;
				free(currentOrder);
				currentOrder = previousOrder;
			}
		} else {
			break;
		}
	}
	
	if(empty == 1) {
		printf("camioncino vuoto\n");
	} else {
		Order* toPrint = ordersToPrint;
		Order* toRemove = NULL;
		while(toPrint != NULL) {
			toRemove = toPrint;
			printf("%d %s %d\n", toPrint->time, toPrint->name, toPrint->quantity);
			toPrint = toPrint->next;
			free(toRemove);
		}
	}
}

int main() {
	Storage storage = {0};
	RecipesHashMap recipesHashMap = {0};
	orderList = NULL;
	waitingList = (WaitingList*)malloc(sizeof(WaitingList));
	waitingList->head = NULL;
	waitingList->tail = NULL;
	
	int courierPeriod = 0;
	int maxCapacity = 0;
	char line[LINE_SIZE];
	char command[30];
	
	if(fgets(line, sizeof(line), stdin) != NULL) {
		sscanf(line, "%d %d", &courierPeriod, &maxCapacity);
	} else {
		fprintf(stderr, "Error while reading the file\n");
		return 1;
	}
	
	while(fgets(line, sizeof(line), stdin) != NULL) {
		if(time != 0 && time % courierPeriod == 0) {
			loadCourier(&orderList, maxCapacity);
		}
		sscanf(line, "%29s", command);
		if(command[0] == 'a') {
			char recipeName[STRING_SIZE];
			char ingredientName[STRING_SIZE];
			int quantity;
			char* token;
			
			token = strtok(line + strlen(command) + 1, " ");
			if(token != NULL) {
				strncpy(recipeName, token, STRING_SIZE - 1);
				recipeName[STRING_SIZE - 1] = '\0';
				Recipe* newRecipe = createRecipe(&recipesHashMap, recipeName);
				if(newRecipe == NULL) {
					printf("ignorato\n");
				} else {
					printf("aggiunta\n");
					while ((token = strtok(NULL, " ")) != NULL) {
						memset(ingredientName, 0, sizeof(ingredientName));
						strncpy(ingredientName, token, STRING_SIZE - 1);
						ingredientName[STRING_SIZE - 1] = '\0';
						token = strtok(NULL, " ");
						if(token != NULL) {
							quantity = atoi(token);
							addIngredient(newRecipe, ingredientName, quantity);
						}
					}
				}
			}
		} else if(command[0] == 'r' && command[2] == 'm') {
			char recipeName[STRING_SIZE];
			char* token;
			token = strtok(line + strlen(command) + 1, " ");
			if (token != NULL) {
				strncpy(recipeName, token, STRING_SIZE - 1);
				recipeName[STRING_SIZE - 1] = '\0';
				char* newlinePos = strchr(recipeName, '\n');
				if (newlinePos != NULL) {
					*newlinePos = '\0';
				}
				deleteRecipe(&recipesHashMap, recipeName, &orderList, &waitingList);
			}
		} else if(command[0] == 'r' && command[2] == 'f') {
			char ingredientName[STRING_SIZE];
			int quantity;
			int expiration;
			char* token;
			token = strtok(line + strlen(command) + 1, " ");
			while(token != NULL) {
				memset(ingredientName, 0, sizeof(ingredientName));
				strncpy(ingredientName, token, STRING_SIZE - 1);
				ingredientName[STRING_SIZE - 1] = '\0';
				token = strtok(NULL, " ");
				quantity = atoi(token);
				token = strtok(NULL, " ");
				expiration = atoi(token);
				addBatch(&recipesHashMap, &storage, ingredientName, quantity, expiration);
				token = strtok(NULL, " ");
			}
			checkForWaitingOrders(&recipesHashMap, &storage, &orderList, &waitingList);
			printf("rifornito\n");
		} else if(command[0] == 'o') {
			char orderName[STRING_SIZE];
			int quantity;
			char* token = strtok(line + strlen(command) + 1, " ");
			strncpy(orderName, token, STRING_SIZE - 1);
			orderName[STRING_SIZE - 1] = '\0';
			quantity = atoi(strtok(NULL, " "));
			Recipe* recipe = recipesHashMap.recipes[hash_function(orderName)];
			
			while(recipe != NULL) {
				if(strcmp(recipe->name, orderName) == 0) {
					break;
				} else {
					recipe = recipe->next;
				}
			}
			
			if(recipe == NULL) {
				printf("rifiutato\n");
			} else {
				int completable = checkIfCompletable(&recipesHashMap, &storage, recipe, quantity);
				if(completable == 1) {
					order(&recipesHashMap, &storage, recipe, quantity, &orderList, time);
					printf("accettato\n");
				} else if(completable == 0) {
					WaitingOrder* waitingOrder = (WaitingOrder*) malloc(sizeof(WaitingOrder));
					waitingOrder->recipe = recipe;
					waitingOrder->quantity = quantity;
					waitingOrder->time = time;
					waitingOrder->next = NULL;
					if(waitingList->head == NULL) {
						waitingList->head = waitingOrder;
						waitingList->tail = waitingOrder;
					} else {
						waitingList->tail->next = waitingOrder;
						waitingList->tail = waitingOrder;
					}
					printf("accettato\n");
				}
			}
		} else {
			printf("Comando non esistente: %s\n", command);
		}
		time++;
	}
	
	if(time % courierPeriod == 0) {
		loadCourier(&orderList, maxCapacity);
	}
	
	return 0;
}
