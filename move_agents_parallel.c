#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <time.h>
#include <pthread.h>

//#include <clcg4.h>
//#include <mpi.h>


typedef struct _tuple_ {
    int x;
    int y;
} tuple;


typedef struct _grid_location_ {
    unsigned int num_sugar;
    unsigned int max_sugar;
    int agent_id;
} grid_location;


typedef struct _agent_ {
    int sugar;
    unsigned int vision;
    unsigned int metabolism;
    int my_id;
    int x;
    int y;
} agent;

//FUNCTION DECLARATIONS
void initialize_board(int size);
void initialize_agents();
void move_agents();
void drawCircle(int centerX, int centerY, int radius, int mS, int size);
tuple* get_optimal_move(int);
void* get_optimal_moves(void*);
void* update_agents(void*);
void* update_board(void*);
void print_results();
void fix_board();

//GLOBALS
grid_location** board;

agent* agents;    //agentID is index number in this array, and must remain so

int board_size;
int num_agents;
int num_threads;


int main(int argc, char* argv[]){
    //./main.c board_size num_agents num_steps
    if(argc != 5){
        printf("not right format\n./a.out board_size num_agents num_steps num_threads\n");
        return EXIT_FAILURE;
    }
    
    board_size = atoi(argv[1]);
    num_agents = atoi(argv[2]);
    num_threads = atoi(argv[4]);
    
    //pthread_t tid_update_agents[num_threads];
    //pthread_t tid_update_board[num_threads];
    
    initialize_board(board_size);
    
    initialize_agents();
    
    int num_steps = atoi(argv[3]);
    int n;
    //int q;
    //int z;
    for(n = 0; n < num_steps; n++){
        move_agents();
        
        
        //parallel version of update

        //update agents in parallel
        /*
        int numAgentsPerThread = num_agents / num_threads;
        for(z = 0; z < num_threads; z++){
            //do that threading thing
            
            //I don't know how to make a thread so do that thing please
            tuple* temp = calloc(1, sizeof(tuple));
            temp->x = numAgentsPerThread;
            temp->y = z * numAgentsPerThread;
            pthread_create( &tid_update_agents[z], NULL, update_agents, temp );
            //update_agents(numAgentsPerThread, z * numAgentsPerThread);
        }
        
        for(q=0; q<num_threads; q++){
            pthread_join( tid_update_agents[q], NULL );
        }
        
        
        //update board in parallel
        int numRowsPerThread = num_agents / num_threads;
        for(z = 0; z < num_threads; z++){
            //do that threading thing
            
            //I don't know how to make a thread so do that thing please
            tuple* temp = calloc(1, sizeof(tuple));
            temp->x = numRowsPerThread;
            temp->y = z * numRowsPerThread;
            pthread_create( &tid_update_board[z], NULL, update_board, temp );
            //update_board(numRowsPerThread, z * numRowsPerThread);
        }
        
        for(q=0; q<num_threads; q++){
            pthread_join( tid_update_board[q], NULL );
        }
        */
        
        //update_agents();
        //update_board();

        //printf("step %d\n", n);
    }
    
    
    print_results();
    
    
    return EXIT_SUCCESS;
}

void initialize_board(int size){
    int i, j;
    board = (grid_location**)calloc(size, sizeof(grid_location*));
    for(i=0; i<size; i++){
        board[i] = (grid_location*)calloc(size, sizeof(grid_location));
        for(j=0; j<size; j++){
            board[i][j].max_sugar = 0;
            board[i][j].agent_id = -1;
        }
    }
    
    //printf("test: %d", )
    
    /*drawCircle(size/2 - 10, size/2 +10, size/4, 1, size);
    drawCircle(size/2 - 10, size/2 + 10, size/8, 2, size);
    drawCircle(size/2 - 10, size/2 + 10, size/10, 3, size);
    drawCircle(size/2 + 15, size/2 - 15, size/4, 1, size);
    drawCircle(size/2 + 15, size/2 - 15, size/8, 2, size);
    drawCircle(size/2 + 15, size/2 - 15, size/10, 3, size);*/
    
    int rad;
    for(rad = 0; rad < size/2; rad++){
        drawCircle(size/2-5, size/2 + 5, rad, 1, size);
    }
    for(rad = 0; rad < size/2; rad++){
        drawCircle(size/2 + 5, size/2 - 5, rad, 1, size);
    }
    
    for(rad = 0; rad < size/4; rad++){
        drawCircle(size/2 - 5, size/2 + 5, rad, 2, size);
    }
    for(rad = 0; rad < size/4; rad++){
        drawCircle(size/2 + 5, size/2 - 5, rad, 2, size);
    }
    
    for(rad = 0; rad < size/8; rad++){
        drawCircle(size/2 - 5, size/2 + 5, rad, 3, size);
    }
    for(rad = 0; rad < size/8; rad++){
        drawCircle(size/2 + 5, size/2 - 5, rad, 3, size);
    }
    
    fix_board();
    
    /*
    printf("sugar state of board\n");
    for(i=0; i<size; i++){
        for(j=0; j<size; j++){
            printf("%d ", board[i][j].agent_id);
        }
        printf("\n");
    }*/
    
    for(i=0; i<size; i++){
        for(j=0; j<size; j++){
            board[i][j].num_sugar = board[i][j].max_sugar;
        }
    }
}

//Finds spaces missed by the circle making method and fills them
void fix_board(){
    int i,j;
    for(i = 1; i < board_size-1; i++){
        for(j = 1; j < board_size-1; j++){
            if(board[i-1][j].max_sugar == board[i+1][j].max_sugar){
                if(board[i-1][j].max_sugar == board[i][j+1].max_sugar){
                    if(board[i-1][j].max_sugar == board[i][j-1].max_sugar){
                        board[i][j].max_sugar = board[i-1][j].max_sugar;
                    }
                }
            }
        }
    }
}

void initialize_agents(){
    int i,j;
    agents = (agent*)calloc(num_agents, sizeof(agent));
    srand(time(NULL));
    for(i=0; i<num_agents; i++){
        agents[i].sugar = rand()%21 + 5;
        agents[i].vision = rand()%6 + 1;
        agents[i].metabolism = rand()%4 + 1;
        agents[i].my_id = i;
        agents[i].x = rand()%board_size;
        agents[i].y = rand()%board_size;
        j = 0;
        while(j!=i){
            if(agents[i].x == agents[j].x && agents[i].y == agents[j].y){
                j = 0;
                agents[i].x = rand()%board_size;
                agents[i].y = rand()%board_size;
                
                board[agents[i].x][agents[i].y].agent_id = i;
            }
            j++;
        }
    }
}

void testInit(){
    int i, j;
    
    for(i = 0; i < board_size; i++){
        for(j = 0; j < board_size; j++){
            printf("%d ",board[i][j].num_sugar);
        }
        printf("\n");
    }
}

void drawCircle(int centerX, int centerY, int radius, int mS, int size){
    int d, x, y;
    d = 3 - (2 * radius);
	x = 0;
	y = radius;
	while (x <= y) {
		if(centerY+y<size && centerX+x<size)
			board[centerY + y][centerX + x].max_sugar = mS;
		if(centerY+y<size && centerX-x>=0)
			board[centerY + y][centerX - x].max_sugar = mS;
		if(centerY-y>=0 && centerX+x<size)
			board[centerY - y][centerX + x].max_sugar = mS;
		if(centerY-y>=0 && centerX-x>=0)
			board[centerY - y][centerX - x].max_sugar = mS;
		if(centerY+x<size && centerX+y<size)
			board[centerY + x][centerX + y].max_sugar = mS;
		if(centerY+x<size && centerX-y>=0)
			board[centerY + x][centerX - y].max_sugar = mS;
		if(centerY-x>=0 && centerX+y<size)
			board[centerY - x][centerX + y].max_sugar = mS;
		if(centerY-x>=0 && centerX-y>=0)
			board[centerY - x][centerX - y].max_sugar = mS;
		if (d < 0) {
			d += 4 * x + 6;
		} 
		else {
			d += 4 * (x - y) + 10;
			y--;
		}
		x++;
	}
}


void move_agents(){
    int unmoved_agents[num_agents];  //set to 1 if unmoved, 0 if moved
    tuple* requests = calloc(num_agents, sizeof(tuple));   //index in array is id of agent making request
            //make sure to free this

    int time_out = 100;  //Adjustable. The bigger the number the more accurate, but longer the simulation takes


    int i;
    
    for(i = 0; i < num_agents; i++){
        unmoved_agents[i] = 1;
    }

    //keep_going gets set to 0 once all agents have moved
    int keep_going = 1;

    pthread_t tid_get_requests[num_threads];

    int run = 0;
    
    while(keep_going){
        keep_going = 0;

        run++;

        int numAgentsPerThread = num_agents / num_threads;

        
        //Make a number of threads, each determining the optimal move for a section of agents
        for(i = 0; i < num_threads; i++){
            //info gets freed in the pthread method
            tuple* info = malloc(sizeof(tuple));

            info->x = numAgentsPerThread;
            info->y = numAgentsPerThread * i;

            pthread_create( &tid_get_requests[i], NULL, get_optimal_moves, info );
        }

        int q;

        void* status;
        tuple** tempPointer;

        //Get the requests from all the threads
        for(q=0; q<num_threads; q++){
            pthread_join( tid_get_requests[q], &status );

            tempPointer = (tuple**)status;

            int w;
            for(w = 0; w < numAgentsPerThread; w++){
                int agent_index = q*numAgentsPerThread + w;

                if(agents[agent_index].sugar >= 0){
                    if(unmoved_agents[agent_index]==1){
                        keep_going = 1;

                        requests[agent_index].x = tempPointer[w]->x;
                        requests[agent_index].y = tempPointer[w]->y;
                    }
                }
                else{
                    requests[agent_index].x = -1;
                    requests[agent_index].y = -1;
                }
            }

            for(w=0; w<numAgentsPerThread; w++){
                free(tempPointer[w]);
            }
            free(tempPointer);
        }

        


        //Make the actual moves given the requests
        //This part must remain serial, must wait until all requests are submitted
        grid_location to_check;
        for(i = 0; i < num_agents; i++){
            if(requests[i].x != -1){
                
                to_check = board[requests[i].x][requests[i].y];

                //check location is still empty
                if((to_check.agent_id==-1 || to_check.agent_id == i) && unmoved_agents[i]==1){

                    //printf("%d: (%d, %d) to (%d, %d) with %d\n", i, agents[i].x, agents[i].y, requests[i].x, requests[i].y, to_check.num_sugar);

                    //Make the move
                    board[agents[i].x][agents[i].y].agent_id = -1;

                    agents[i].x = requests[i].x;
                    agents[i].y = requests[i].y;

                    board[requests[i].x][requests[i].y].agent_id = i;

                    unmoved_agents[i] = 0;
                }
                else if(run == time_out && unmoved_agents[i]==1){
                    //In the rare case of time out, kill the agent that has no possible move
                    agents[i].sugar = -1;
                    agents[i].x = -1;
                    agents[i].y = -1;

                    unmoved_agents[i] = 0;
                }
            }
        }
    }

    free(requests);
}


void* get_optimal_moves(void* info){
    tuple* my_info = (tuple*)info;
    int numAgentsPerThread = my_info->x;
    int startingAgent = my_info->y;

    free(info);

    //Array of pointers to the optimal moves for this block of agents
    tuple** optimal_moves = calloc(numAgentsPerThread, sizeof(tuple));

    int n;
    for(n=0; n<numAgentsPerThread; n++){
        //Allocated tuple here will get later freed in make_move method
        if(agents[startingAgent + n].sugar >= 0){
            optimal_moves[n] = get_optimal_move(startingAgent + n);
        }
        else{
            optimal_moves[n] = malloc(sizeof(tuple));
            optimal_moves[n]->x = -1;
            optimal_moves[n]->y = -1;
        }
    }

    return (void*)optimal_moves;
}



tuple* get_optimal_move(int agent_index){
    //int* ai = (int*)info;

    agent a = agents[agent_index];

    int num_visible = a.vision*4 + 1;

    tuple adjacent_locations[num_visible];  //should always be this size due to wrap around
    
    int al = 0;
    
    tuple temp;
    
    temp.x = a.x;
    temp.y = a.y;
    adjacent_locations[al] = temp;
    al++;
    
    //TODO: implement wraparound
    
    int i;
    
    //store all visible locations
    for(i = 1; i <= a.vision; i++){
        //TODO: put these in the array in random order

        //there must be a shorter way to do this
        if(a.x+i >= board_size){
            temp.x = a.x+i - board_size;
        }
        else{
            temp.x = a.x+i;
        }
        temp.y = a.y;
        adjacent_locations[al] = temp;

        if(a.x-i < 0){
            temp.x = a.x-i + board_size;
        }
        else{
            temp.x = a.x-i;
        }
        temp.y = a.y;
        adjacent_locations[al+1] = temp;

        temp.x = a.x;
        if(a.y+i >= board_size){
            temp.y = a.y+i - board_size;
        }
        else{
            temp.y = a.y+i;
        }
        adjacent_locations[al+2] = temp;

        temp.x = a.x;
        if(a.y-i < 0){
            temp.y = a.y-i + board_size;
        }
        else{
            temp.y = a.y-i;
        }
        adjacent_locations[al+3] = temp;
        
        al+=4;
    }
    
    int max_found_sugar = 0;
    
    tuple current;
    current.x = a.x;
    current.y = a.y;
    
    tuple* max_loc = malloc(sizeof(tuple));

    max_loc->x = current.x;
    max_loc->y = current.y;

    //find max sugar space to move that is open
    for(i = 0; i < num_visible; i++){
        current = adjacent_locations[i];
        
        //printf("%d %d\n", current.x, current.y);
        
        grid_location to_check = board[current.x][current.y];
        
        //printf("%d\n",to_check.num_sugar);
        
        if(to_check.num_sugar > max_found_sugar && (to_check.agent_id == -1 || to_check.agent_id == i)) {
        //if(to_check.num_sugar > max_found_sugar && to_check.agent_id == -1){
            max_found_sugar = to_check.num_sugar;
            //max_loc = current;
            max_loc->x = current.x;
            max_loc->y = current.y;
        }
    }

    return max_loc;
}

void* update_agents(void* info){
    /*//serial version
    int i;
    agent currAgent;
    grid_location agentLocation;
    
    //update agents
    for(i = 0; i < num_agents; i++){
        currAgent = agents[i];
        if(currAgent.sugar >= 0){
            //currAgent = agents[i];
            agentLocation = board[currAgent.x][currAgent.y];
        
            //agents eat the sugar on their space
            agents[i].sugar += agentLocation.num_sugar;
            board[currAgent.x][currAgent.y].num_sugar = 0;
        
            //agents consume their metabolism
            agents[i].sugar -= currAgent.metabolism;
            
            
            if(currAgent.sugar < 0){
                //the agent dies
                printf("dead\n");
                
                //remove the agent from the board
                agentLocation.agent_id = -1;
            }
            
            
        }
    }*/
    
    
    //parallel version:
    //pass in: int numAgentsPerThread (x), int startAgent (y)
    tuple* temp = (tuple*)info;
    
    int numAgentsPerThread = temp->x;
    int startAgent = temp->y;
    int i;
    agent currAgent;
    grid_location agentLocation;
    
    for(i = startAgent; i < numAgentsPerThread + startAgent; i++){
        if(currAgent.sugar > 0){
            currAgent = agents[i];
            agentLocation = board[currAgent.x][currAgent.y];
        
            //agents eat the sugar on their space
            currAgent.sugar += agentLocation.num_sugar;
            agentLocation.num_sugar = 0;
        
            //agents consume their metabolism
            currAgent.sugar -= currAgent.metabolism;
        
            
            if(currAgent.sugar < 0){
                //the agent dies
                
                //remove the agent from the board
                agentLocation.agent_id = -1;
            }
        }
    }
    free(temp);
    return NULL;
}


void* update_board(void* info){
    //serial version
    /*int i, j;
    grid_location currLoc;
    
    //update sugar on board
    for(i = 0; i < board_size; i++){
        for(j = 0; j < board_size; j++){
            currLoc = board[i][j];
            if(currLoc.num_sugar < currLoc.max_sugar)
                board[i][j].num_sugar++;
        }
    }*/
    
    
    //parallel version
    //passed in: int numRowsPerThread, int startRow
    /*tuple* temp = (tuple*)info;
    
    int numRowsPerThread = temp->x;
    int startRow = temp->y;
    int i, j;
    grid_location currLoc;
    
    
    //update sugar on board
    for(i = startRow; i < numRowsPerThread + startRow; i++){
        for(j = 0; j < board_size; j++){
            currLoc = board[i][j];
            if(currLoc.num_sugar < currLoc.max_sugar)
                currLoc.num_sugar++;
        }
    }
    
    free(temp);*/
    return NULL;
}


void print_results(){
    printf("so done with this\n");
    //print out the agent population
    int i;
    int pop = 0;
    
    for(i = 0; i < num_agents; i++){
        //printf("sugar for agent %d: %d\n", i, agents[i].sugar);
        if(agents[i].sugar > 0){
            pop++;
        }
    }
    
    printf("%d\n", pop);

    int test = 3;

    printf("%f\n", pop/test);
    
}