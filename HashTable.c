#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"hashTable.h"

// The inner workings of my program
struct  HashTableObjectTag
{
        int sentinel;
        unsigned int size;
        struct treeNode ** bucket;
        int num_entries;
        HashTableInfo * pointer_info;
};

struct treeNode
{
        char * key;
        struct treeNode *right, *left;
        void *  data;
};

// MY LITTLE HELPER FUNCTIONS!
float GetUseFactor(HashTablePTR);
int GetLargestBucketNum ( struct treeNode *, unsigned int * );
void Expand(HashTablePTR, HashTablePTR);
void Contract (HashTablePTR, HashTablePTR);
void TreeInsert(struct treeNode **, char *, int *);
struct treeNode *FindItem(struct treeNode*, char *);
int Print (struct treeNode*, char ***, int * );

// Hash the key to a value
int StringHash( char *value, int range )
{
        int total = 0;
        for (int i = 0; i < strlen( value ); i++ )
        {
                total = total + (int) value[i];
        }

        while (total >= range)
        {
                total -= range;
        }

        return total;
}

// Creates each node which will be used to insert
static struct treeNode *NewTreeNode (char * key, void * data)
{

	struct treeNode * node;
	char * my_key = NULL;

	my_key = malloc (sizeof( key) + 1);
	strcpy (my_key, key);

	node = malloc (sizeof(struct treeNode) );
	node -> key = my_key;
	node -> left = NULL;
	node -> right = NULL;
	node -> data = data;

	return node;
}

int CreateHashTable ( HashTablePTR * HashTableHandle, unsigned int initialSize)
{
	// Return -1 if not enough memory
	if ( initialSize == 0)
	{
		return -1;
	}
	((*HashTableHandle)) = malloc(sizeof(HashTableObject));
	if ((*HashTableHandle) == NULL)
        {
                return -1;
        }

        (*(*HashTableHandle)).bucket = malloc(sizeof(struct treeNode )*initialSize);
        if ((*(*HashTableHandle)).bucket == NULL)
                return -1;

	for ( int i = 0; i < initialSize; i++ )
	{
		(*(*HashTableHandle)).bucket[i] = NULL;
	}

	(*(*HashTableHandle)).pointer_info = malloc (sizeof (HashTableInfo));
	if ((*(*HashTableHandle)).pointer_info == NULL)
		return -1;

        
	(*(*(HashTableHandle))).size = initialSize;
	(*(*(HashTableHandle))).num_entries = 0;

//	Assigning values
	(*(*(HashTableHandle))).pointer_info -> bucketCount = initialSize;
	(*(*(HashTableHandle))).pointer_info -> loadFactor = 0;
	(*(*(HashTableHandle))).pointer_info -> largestBucketSize = 0;
	(*(*(HashTableHandle))).pointer_info -> dynamicBehaviour = 1; 
	(*(*(HashTableHandle))).pointer_info -> expandUseFactor = (float)0.7;
	(*(*(HashTableHandle))).pointer_info -> contractUseFactor = (float)0.2;
        (*(*(HashTableHandle))).sentinel = (int)0xDEADBEEF;

	return 0;
}


int InsertEntry(HashTablePTR my_HashTable, char * key, void * data ,void ** previousDataHandle)
{
//	Make sure table is real
	if ( my_HashTable == NULL || my_HashTable -> sentinel != 0xDEADBEEF)
	{
		return -1;
	} 

	int head_ornot = 0;

	struct treeNode ** pointer_treeNode = NULL;
	int Bucket_Num;
	Bucket_Num = StringHash(key, (int)my_HashTable -> size );
	pointer_treeNode =  &(my_HashTable -> bucket[Bucket_Num]) ;

//	fprintf(stderr,"The Hash Value is: %d \n", Bucket_Num);
	
	struct treeNode * new = NULL;
	struct treeNode * curNode = NULL;

	new = NewTreeNode(key,data);	
	if (new == NULL)
	{
//		fprintf(stderr,"NOT ENOUGH SPACE! \n");
		return -1;
	}

	if ( ( *pointer_treeNode) == NULL )
	{
//		fprintf(stderr,"Placing head node! \n");
		*pointer_treeNode = new;
		my_HashTable -> num_entries +=1;
		head_ornot = 1;
//		return 0;
	}
	else 
	{
		curNode = *pointer_treeNode;
		while ( curNode != NULL )
		{
			if (strcmp (curNode -> key, key ) == 0 )
			{
				
			// They are equal
			//	fprintf(stderr,"Value alreading in struct \n");

				// Switching the values associated with each key
				void * temp = NULL;
				temp = curNode -> data;
				curNode -> data = data; 
				free ( new );				
				*previousDataHandle = temp;
				new = NULL;
				return 2;
			}

			else if ( strcmp (curNode -> key, key) > 0) 
			{
//				fprintf(stderr,"Moving left!\n"); Used for Debugging
				if (curNode -> left == NULL)
				{
//					fprintf(stderr,"Assigning New Node on Left \n"); Debuggin
					curNode -> left = new;
					my_HashTable -> num_entries +=1;
					break;
				}
				curNode = curNode -> left;
			}
			else if ( strcmp (curNode -> key, key ) < 0 )
			{
			//	printf("Moving right! \n");
				if (curNode -> right  == NULL)
				{
			//		printf("Assigning New Node on right \n");
					curNode -> right = new;	
					my_HashTable -> num_entries +=1;
					break;
				}
				curNode = curNode -> right;
			}

		}
	}

        if ( my_HashTable -> pointer_info -> dynamicBehaviour != 0  )
        {

                float my_use = GetUseFactor(my_HashTable);
                
//		printf("My use limit factor is: %f and my current use factor is: %f \n",my_HashTable->pointer_info->expandUseFactor, my_use/((float)(my_HashTable -> size))); // - Debugging
                if ( (my_HashTable -> pointer_info -> expandUseFactor )  < my_use/ ( (float)(my_HashTable -> size) ) )
                {
			HashTablePTR new_HashTable = NULL;
			Expand(my_HashTable, new_HashTable);
			my_HashTable = new_HashTable;
			// Are we suppose to return something different if you resize? I left it as 
			// return 1 for collision
                }
        }
//	Determines if there was a collision. whether to return 0 or -1;
	if (  head_ornot == 1 )
	{
		return 0;
	}
	return 1;
}

int FindEntry (HashTablePTR my_HashTable, char * key, void ** previousDataHandle)
{
//	Checker for real HashTable
	if ( my_HashTable == NULL || my_HashTable -> sentinel != 0xDEADBEEF)
	{
		return -1;
	} 
	struct treeNode * node = NULL;
	int Bucket_Num;
//	Hash it to Bucket_Num. From here, this is like my lab9
	Bucket_Num = StringHash(key, (int)my_HashTable -> size );
	node =  (my_HashTable -> bucket[Bucket_Num]);

	struct treeNode * curNode = NULL;	

	if (node == NULL)
	{
		return -1;		
	}
	
	curNode = node;
//	Searching through binary tree
	while ( curNode != NULL )
	{
		if (  strcmp(curNode -> key, key ) > 0)
		{
			curNode = curNode -> left;	
			continue;
		}
		else if ( strcmp (curNode -> key , key ) < 0)
		{
			curNode = curNode -> right;
			continue;	
		}
		else if (strcmp ( curNode -> key, key ) == 0)
		{
//			This was for debugging
/*
			if (curNode -> left != NULL && curNode -> right != NULL)
			{
				printf("Two Children!\n");
			}
			else if (curNode -> left != NULL)
			{
				printf("Right Child!\n");
			}
			else if (curNode -> right != NULL)
			{
				printf("Left Child\n");
			}
			else
			{
				printf("No Children.... forever alone \n");
			}
*/
			*previousDataHandle =  curNode -> data;
			return 0;	
		}
	}
//	No node was found
	return -2;
}

int GetKeys ( HashTablePTR my_HashTable, char *** KeysArrayHandle, unsigned int * keyCount) 
{
	int space_checker = 0;

// Check for you know what
	if ( my_HashTable == NULL || my_HashTable -> sentinel != 0xDEADBEEF)
	{
		return -1;
	} 
//	Making room to store all keys
	*KeysArrayHandle = malloc( (unsigned int) my_HashTable -> num_entries * sizeof (char * ));
	if (*KeysArrayHandle == NULL)
		return -2;	

	*keyCount = (unsigned int )my_HashTable -> num_entries;
//	Counter used to determine at which count I am at so everything is ordered nicely.
	int (s_counter) = 0;
 //	Go through each bucket and count how many keys there are
	for (int i = 0; i < my_HashTable -> pointer_info -> bucketCount ; i++ )
	{
		if ( my_HashTable -> bucket[i] != NULL)
		{
			space_checker = Print ( my_HashTable -> bucket[i], KeysArrayHandle, &s_counter );
			if (space_checker == -1)
				return -1; // No space left

			s_counter +=1;
		}
	}
	return 0;	
}

int Print ( struct treeNode * head_node, char *** ArrayHandle, int * S_counter)
{
	printf("KEY: [%s]\n", head_node -> key);
	(*ArrayHandle)[(int)(*S_counter)] = malloc ( sizeof( head_node -> key ) + 1  );
//	If no space for memory
	if ( (*ArrayHandle)[(int)(*S_counter)] == NULL  )
		return -1;
	
	if (  (*ArrayHandle)[(int)(*S_counter)] != NULL ) 
	{ 
	    strcpy ( (*ArrayHandle)[(* S_counter)], head_node -> key);

//	    printf("The copied string is: %s \n", ( (*ArrayHandle)[*S_counter]) );

	if ( head_node -> left != NULL)
	{
		(*S_counter)++;

		Print ( head_node -> left, ArrayHandle, S_counter);
	}
	
	if (head_node -> right != NULL )
	{
		(*S_counter) ++;
		Print (head_node -> right, ArrayHandle, S_counter);	
	}
	}
	return 0;
}


void Expand (HashTablePTR HashTable, HashTablePTR new_HashTable)
{
	// increases the number of buckets to improve effeciency of data storage

	char ** Key_Copy = NULL;
	void * Data_Copy= NULL;	// NOT REALLY A COPY
	void * i_dont_care = NULL;

	unsigned int new_size = (unsigned int)((HashTable -> size) * 2); 
	unsigned int num_keys = 0;
	
	// Get the keys from all the binary tree
	GetKeys( HashTable, &Key_Copy, &num_keys );
	// Create a new hash table
	CreateHashTable(&new_HashTable, new_size);

	for (int i = 0; i < num_keys; i++)
	{
		// Find data with key and insert to new struct
		FindEntry(HashTable, Key_Copy[i], &Data_Copy);
		InsertEntry (new_HashTable, Key_Copy[i], Data_Copy, &i_dont_care);
		free(i_dont_care); // THERE SHOULDN'T BE ANY COLLISION

	}

	// free extraneous stuff and make the old bucket holder point to new bucket holder
	free ( HashTable -> bucket  );
	HashTable -> bucket = new_HashTable -> bucket; 

	HashTable -> size = new_size;
	HashTable -> pointer_info -> bucketCount = new_size;

	free( new_HashTable -> pointer_info);
	free( new_HashTable);
}

void Contract (HashTablePTR HashTable, HashTablePTR new_HashTable)
{
        if ( HashTable -> pointer_info -> dynamicBehaviour != 0  )
        {
                float my_use = GetUseFactor(HashTable);
                //printf("My use limit factor is: %f and my current use factor is: %f \n",HashTable->pointer_info->contractUseFactor, my_use/((float)(HashTable -> size))); - Debugging
                if ( (HashTable -> pointer_info -> contractUseFactor )  > my_use/ ( (float)(HashTable -> size) ) )
                {
                        HashTablePTR new_HashTable = NULL;

			//float my_use  = GetUseFactor(HashTable) / ((float)(HashTable -> size)); - Useless Variable but helpful for debugging

		        char ** Key_Copy = NULL;
		        void * Data_Copy= NULL; // NOT REALLY A COPY
        		void * i_dont_care = NULL;

		        unsigned int new_size = (unsigned int)((HashTable -> size) / 2);
        		unsigned int num_keys = 0;

		  //      printf("My use factor is: %f \n", my_use);

			// Same method as the void Expand() above
		        GetKeys( HashTable, &Key_Copy, &num_keys );
		        CreateHashTable(&new_HashTable, new_size);

		        for (int i = 0; i < num_keys; i++)
        		{
                		FindEntry(HashTable, Key_Copy[i], &Data_Copy);

		                InsertEntry (new_HashTable, Key_Copy[i], Data_Copy, &i_dont_care);
                		free(i_dont_care); // THERE SHOULDN'T BE ANY COLLISION

        		}
        		free ( HashTable -> bucket  );
		        HashTable -> bucket = new_HashTable -> bucket;

		        HashTable -> size = new_size;
		        HashTable -> pointer_info -> bucketCount = new_size;

		        free( new_HashTable -> pointer_info);
		        free( new_HashTable);
		}
	}
}

int DestroyHashTable(HashTablePTR * hashTableHandle )
{
	char ** list_keys;
	unsigned int num_keys;
	void * i_dont_care = NULL;
//	Check Sentinel	
	if ( (*hashTableHandle) == NULL || ((*hashTableHandle) -> sentinel != 0xDEADBEEF))
	{
		return -1;
	} 
//	Get All the keys
	GetKeys( *hashTableHandle, &list_keys, &num_keys);
//	Delete for each bucket
	for (int j = 0; j < num_keys; j++)
	{
		DeleteEntry ( *hashTableHandle, list_keys[j], &i_dont_care );	
	}
//	Free everything else
	free ( (*hashTableHandle) -> bucket );	
	free (  (*hashTableHandle));

	*hashTableHandle = NULL;
	hashTableHandle = NULL;
	
	return 0;
}

float GetUseFactor (HashTablePTR my_HashTable)
{
//	Helper function used to aid in calculating load factor
	float count = 0;

	for (int i = 0; i <  my_HashTable -> size; i++)
	{
		if ( my_HashTable -> bucket[i] != NULL )
		{
			count++;
		}	
	}
	return count;
}

int SetResizeBehaviour ( HashTablePTR HashTable, int dynamicBehaviour, float expandUseFactor, float contractUseFactor  )
{

	if ( HashTable == NULL || HashTable -> sentinel != 0xDEADBEEF)
	{
		return -1;
	}

	if ( expandUseFactor <= contractUseFactor )
	{
		//printf("Expand smaller than contract!");
		return 1;
	}

        unsigned int highest_num = 0;
	unsigned int temp_count = 0;


        HashTable -> pointer_info -> loadFactor = (float)((float)(HashTable -> num_entries) / (float)(HashTable -> size));
//        fprintf(stderr,"The load factor is: %f \n", HashTable -> pointer_info -> loadFactor ); - Debuggin

        HashTable -> pointer_info -> useFactor = (GetUseFactor( HashTable ) / (float)HashTable -> size );
//        fprintf(stderr,"The useFactor is %f \n", HashTable -> pointer_info -> useFactor); - Debugging

        for (int i = 0; i < HashTable -> size; i++)
        {
		// Gets number of keys in each tree and saves it in temp count
                GetLargestBucketNum( HashTable->bucket[i], &temp_count );
                if (temp_count > highest_num)
                {
                        highest_num = temp_count;
                }
        }

	HashTable -> pointer_info -> largestBucketSize = highest_num;
	HashTable -> pointer_info -> dynamicBehaviour = dynamicBehaviour;
	HashTable -> pointer_info -> expandUseFactor = expandUseFactor;
	HashTable -> pointer_info -> contractUseFactor = contractUseFactor;
	
	return 0;
}


int DeleteEntry (HashTablePTR my_HashTable, char * key, void ** previousDataHandle)
{
//	Checking Sentinel
	if ( my_HashTable == NULL || my_HashTable -> sentinel != 0xDEADBEEF)
	{
		return -1;
	} 
        int Bucket_Num;
        Bucket_Num = StringHash(key, (int)my_HashTable -> size );

	struct treeNode ** pointer_treeNode = NULL;
        pointer_treeNode =  &(my_HashTable -> bucket[Bucket_Num]);

	HashTablePTR new_HashTable = NULL; // Used for Contract()
        float my_use = GetUseFactor(my_HashTable);
//	fprintf(stderr,"Hash Value of deleted key: %d \n",Bucket_Num); Debugging

	if ( *pointer_treeNode == NULL )
	{
		return -1;
	}

	struct treeNode * node = NULL;
	struct treeNode * previous_node = NULL;
	struct treeNode * delete_node = NULL;
	delete_node = *pointer_treeNode;

	node = *pointer_treeNode;
	char l_r; // Determines if the last switch was left or right.	
	char * delete_key;

	// Deleting Head Node if this is trigger
	if ( strcmp ( node -> key, key) == 0)
	{	
		// Checks the three cases
		if (node -> left != NULL && node -> right == NULL)
		{
			*pointer_treeNode  =  node -> left; 
			free ( node -> key );
			node -> key = NULL;
			*previousDataHandle = node -> data; // MAKING DATAHANDLE POINT TO THE DATA BEFORE DELETION! 
			free (node);
			node = NULL;
			my_HashTable -> num_entries -=1;
		// While to ensure below contractFactor	
                while ( ((my_HashTable -> pointer_info -> contractUseFactor )  > (my_use/ ( (float)(my_HashTable -> size)))) && my_HashTable -> pointer_info -> dynamicBehaviour != 0 )
		{
			Contract(my_HashTable, new_HashTable);
		}
			return 0;
		}
		if (node -> left == NULL && node -> right != NULL)
		{
			*pointer_treeNode  =  node -> right;
			free ( node -> key );
			node -> key = NULL;
			*previousDataHandle = node -> data; // MAKING DATAHANDLE POINT TO THE DATA BEFORE DELETION! 
			free (node);
			node = NULL;
			my_HashTable -> num_entries -=1;
		// While to ensure below contractFactor	
                while ( ((my_HashTable -> pointer_info -> contractUseFactor )  > (my_use/ ( (float)(my_HashTable -> size)))) && my_HashTable -> pointer_info -> dynamicBehaviour != 0 )
			{
				Contract(my_HashTable, new_HashTable);
			}

			return 0;
		}
		
		if (node -> left == NULL && node -> right == NULL)
		{
			*pointer_treeNode  =  NULL; 	
			free (node -> key);
			node -> key = NULL;
			*previousDataHandle = node -> data; // MAKING DATAHANDLE POINT TO THE DATA BEFORE DELETION! 
			free (node);
			node = NULL;
			my_HashTable -> num_entries -=1;
		// While to ensure below contractFactor	
                while ( ((my_HashTable -> pointer_info -> contractUseFactor )  > (my_use/ ( (float)(my_HashTable -> size)))) && my_HashTable -> pointer_info -> dynamicBehaviour != 0 )
			{
				Contract(my_HashTable, new_HashTable);
			}
			return 0;
		}
	}

	while (node != NULL)
	{
		if ( strcmp ( node -> key, key ) > 0)
		{
			previous_node = node;
			node = node -> left;
			// Determine last switch. Used later
			l_r = 'l';	
		}
		else if ( strcmp ( node -> key , key ) < 0 )
		{
			previous_node = node;
			node = node -> right;	
			// Determine last switch. Used later
			l_r = 'r';
		}
		
		else if (strcmp ( node -> key, key ) == 0 )
		{
			// No children at delete key
			if ( (node -> right == NULL) && (node -> left == NULL) )
			{
				
				free (node -> key);
				node -> key = NULL;
				*previousDataHandle = node -> data; // MAKING DATAHANDLE POINT TO THE DATA BEFORE DELETION! 
				free (node);
				node = NULL;
				// Determine if left or right WAS deleted
				if ( l_r == 'l') 
					previous_node -> left = NULL;
				if (l_r == 'r' )
					previous_node -> right = NULL;

				my_HashTable -> num_entries -=1;

                		while ( ((my_HashTable -> pointer_info -> contractUseFactor )  > (my_use/ ( (float)(my_HashTable -> size)))) && my_HashTable -> pointer_info -> dynamicBehaviour != 0 )
				{
					Contract(my_HashTable, new_HashTable);
				}	
				return 0;
			}

			// Only one child who is left
			else if ( (node -> right == NULL) && (node -> left != NULL ) )
			{
				if ( l_r == 'l')
				{
					previous_node -> left = node -> left;	
				}
				if ( l_r == 'r')
				{
					previous_node -> right = node -> left;
				}
				free (node -> key); 
				node -> key = NULL;
				*previousDataHandle = node -> data; // MAKING DATAHANDLE POINT TO THE DATA BEFORE DELETION! 
				free (node);
				node = NULL;
				my_HashTable -> num_entries -=1;
		
                		while ( ((my_HashTable -> pointer_info -> contractUseFactor )  > (my_use/ ( (float)(my_HashTable -> size)))) && my_HashTable -> pointer_info -> dynamicBehaviour != 0 )
				{
					Contract(my_HashTable, new_HashTable);
				}
				return 0;
			}

			// Only one child who is larger (right)
			else if ( (node -> right != NULL) && (node -> left == NULL ) )
			{
				// Same reasons as earlier
				if ( l_r == 'l')
				{
					//printf("Going through left side");
					previous_node -> left = node -> right;	
				}
				if ( l_r == 'r')
				{
					//printf("Going through right side");
					previous_node -> right = node -> right;
				}
				free (node -> key); 
				node -> key = NULL;
				*previousDataHandle = node -> data; // MAKING DATAHANDLE POINT TO THE DATA BEFORE DELETION! 
				free (node);
				node = NULL;
				my_HashTable -> num_entries -=1;
                		while ( ((my_HashTable -> pointer_info -> contractUseFactor )  > (my_use/ ( (float)(my_HashTable -> size)))) && my_HashTable -> pointer_info -> dynamicBehaviour != 0 )
				{
					Contract(my_HashTable, new_HashTable);
				}	
				return 0;
			}
			// Has two children. You murderer
			
			char delete_l_r; // Used to determine if it is the right most left or the end is the left end.
			struct treeNode * previous_delete_node = NULL;

			if ( (node -> right != NULL) && (node -> left != NULL ) )
			{
				//fprintf(stderr,"Has two children! \n"); - Debugging

//					I am implementing a delete for two child parent where the replaced value is
//					go left once, and then keep going right.
//					A "left most right delete".
					delete_node = node;
					previous_delete_node = delete_node;
					delete_node = delete_node -> left;
					delete_l_r = 'l';

					while (delete_node -> right != NULL)
					{
						previous_delete_node = delete_node;
						delete_node = delete_node -> right;
						delete_l_r = 'r';
					}
						//fprintf(stderr,"I moved x amount right and here I am: %s \n ", delete_node -> key ); - Debugging

						delete_key = malloc (sizeof (delete_node -> key ) + 1);
						strcpy(delete_key, delete_node -> key);

						if ( delete_l_r == 'r' )
						{
							previous_delete_node -> right = delete_node -> left;
	
							free (delete_node -> key); 
							delete_node -> key = NULL;
							*previousDataHandle = node -> data; // MAKING DATAHANDLE POINT TO THE DATA BEFORE DELETION! 
							// free and assign all this extra stuff
							free (delete_node);
							my_HashTable -> num_entries -=1;
							free(node -> key);
							node->key = NULL;
							node -> key = delete_key;
							// Ensure that contract stays true()
                					while ( ((my_HashTable -> pointer_info -> contractUseFactor )  > (my_use/ ( (float)(my_HashTable -> size)))) && my_HashTable -> pointer_info -> dynamicBehaviour != 0 )
							{
								Contract(my_HashTable, new_HashTable);
							}
						}

						if (delete_l_r == 'l')
						{
							previous_delete_node -> left = delete_node -> left;
	
							free (delete_node -> key); 
							delete_node -> key = NULL;
	
							*previousDataHandle = node -> data; // MAKING DATAHANDLE POINT TO THE DATA BEFORE DELETION! 
							my_HashTable -> num_entries -=1;
							free (delete_node);
							free(node -> key);

							node->key = NULL;
							node -> key = delete_key;
					                while ( ((my_HashTable -> pointer_info -> contractUseFactor )  > (my_use/ ( (float)(my_HashTable -> size)))) && my_HashTable -> pointer_info -> dynamicBehaviour != 0 )
							{
								Contract(my_HashTable, new_HashTable);
							}				
						}
				return 0;

			}
		}
	}
//	Entry not found
	return -2;

}

int GetLargestBucketNum (struct treeNode * head_node, unsigned int * counter )
{
// Helper function that get the number of keys in each bucket
	if (head_node == NULL)
	{
		return 1;
		
	}
        if ( head_node -> left != NULL)
        {
		(*counter)++;
		GetLargestBucketNum ( head_node -> left,  counter);
	}
	if (head_node -> right != NULL )
	{
		(*counter)++;
		GetLargestBucketNum (head_node -> right, counter);
	}

	return 0;

}

int GetLoadFactor(HashTablePTR HashTable, float * load_factor)
{
// Don't think we are needed to have this function. I asked other EngScis and they put it in so I included it
	if ( HashTable == NULL || HashTable -> sentinel != 0xDEADBEEF)
	{
		return -1;
	} 
	
	*load_factor = (float)((float)(HashTable -> num_entries) / (float)(HashTable -> size));
	return 0;

}

int GetHashTableInfo (HashTablePTR HashTable, HashTableInfo *pHashTableInfo)
{
	if ( HashTable == NULL || HashTable -> sentinel != 0xDEADBEEF)
	{
		return -1;
	} 
	unsigned int temp_count = 0;
	unsigned int highest_num = 0;
	
	HashTable -> pointer_info -> loadFactor = (float)((float)(HashTable -> num_entries) / (float)(HashTable -> size));
	HashTable -> pointer_info -> useFactor = (GetUseFactor( HashTable ) / (float)HashTable -> size );

	for (int i = 0; i < HashTable -> size; i++)
	{
		temp_count = 0;
		GetLargestBucketNum( HashTable->bucket[i], &temp_count );
		if (temp_count > highest_num)
		{
			highest_num = temp_count;
		}
	}
	HashTable -> pointer_info -> largestBucketSize = highest_num;
	pHashTableInfo -> bucketCount = HashTable -> pointer_info -> bucketCount;
	pHashTableInfo -> loadFactor = HashTable -> pointer_info -> loadFactor;
	pHashTableInfo -> useFactor = HashTable -> pointer_info -> useFactor;
	pHashTableInfo -> largestBucketSize = HashTable -> pointer_info -> largestBucketSize;
	pHashTableInfo -> dynamicBehaviour = HashTable -> pointer_info -> dynamicBehaviour;
	pHashTableInfo -> expandUseFactor = HashTable -> pointer_info -> expandUseFactor;
	pHashTableInfo -> contractUseFactor = HashTable -> pointer_info -> contractUseFactor;
	return 0;
}
