#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <time.h>

#define EARTH_RADIUS 6371000 //mean radius of the earth
#define PI 3.141592654 //number pi

const unsigned long startid = 240949599, goalid = 195977239 ; //start and finishing nodes obtained from professor's solution

	//					*****	STRUCTS AND FUNCTIONS DECLARATION		*****

typedef struct {
    unsigned long id;           // Node identification
    char *name;                 // Pointer to the name of the node
    double lat, lon;            // Node location (latitude and longitude)
    unsigned short nsucc;       // Number of node successors; i. e. length of successors
    unsigned long *successors;  // Pointer to the vector of successors
} node;

typedef struct element{
    double cost; // f function of the node. It is the cost function.
    unsigned long vec_pos; //position of the node in the vector
    struct element *next;  //pointer to the next element
}Element;

typedef char Queue;
enum whichQueue {NONE, OPEN, CLOSED};

typedef struct {
    double g, h; //values of distance g and heuristic of the node
    unsigned long parent; //parent of the node
    Queue whq; //in what list is the node
} AStarStatus; 

AStarStatus *Astar_function(node *nodes, unsigned long nnodes, unsigned long node_start, unsigned long node_goal, AStarStatus *Astatus); //Astar function
Element      *sortedll (Element *first, double val,unsigned long node_position);                 // This is the list that arranges the linked list
void          ExitError(const char *miss, int errcode);                                         // Function to track if a error is triggered
double        h (unsigned long inode, unsigned long node_goal, node *nodes);                    // Returns the heuristic distance
double        w (unsigned long node_current, unsigned long node_successor, node *nodes);        // Returns the distance between two nodes
unsigned long binsearch( node *nodes, unsigned long vector_size, unsigned long ID);         // Binary Search

	//					*****	MAIN FUNCTION		*****
	
int main (int argc, char *argv[]){

printf("\n Program starts. \n");

    char *name2read="spain.bin";    
	double elapsed_time = 0.0;
	clock_t begin = clock();

    FILE *fin;
    unsigned long nnodes, ntotnsucc, *allsuccessors=NULL;
    char *auxname=NULL;
    node *nodes=NULL;
    size_t len=0;
    unsigned long i, j;

//error detecting sequence for reading the file

    if ((fin = fopen (name2read, "rb")) == NULL){
        ExitError("the data file does not exist or cannot be opened", 11);
    }

    /* Global data --- header */
    if( fread(&nnodes, sizeof(unsigned long), 1, fin) + fread(&ntotnsucc, sizeof(unsigned long), 1, fin) != 2 ) {
            ExitError("when reading the header of the binary data file", 12);
    }

    /* getting memory for all data */
    if((nodes = (node *) malloc(nnodes*sizeof(node))) == NULL){
        ExitError("when allocating memory for the nodes vector", 13);
    }
    if( (allsuccessors = (unsigned long *) malloc(ntotnsucc*sizeof(unsigned long))) == NULL){
        ExitError("when allocating memory for the edges vector", 15);
    }

    /*Reading all data from file */
    if( fread(nodes, sizeof(node), nnodes, fin) != nnodes ){
        ExitError("when reading nodes from the binary data file", 17);
    }
    if(fread(allsuccessors, sizeof(unsigned long), ntotnsucc, fin) != ntotnsucc){
        ExitError("when reading successors from the binary data file", 18);
    }

    /*Setting pointers to successors */
    for(i=0; i < nnodes; i++) if(nodes[i].nsucc){
        nodes[i].successors = allsuccessors; allsuccessors += nodes[i].nsucc;
    }
    

    fclose(fin);

printf("\n BIN FILE HAS BEEN READ\n");

unsigned long node_start, node_goal;
node_start = binsearch(nodes,nnodes,startid);  //Posició del node amb id: node start
node_goal  = binsearch(nodes,nnodes,goalid);

	//					*****	A* Algorithm		*****

AStarStatus *Astatus = malloc(nnodes*sizeof(AStarStatus));  

Astatus = Astar_function( nodes, nnodes, node_start, node_goal, Astatus);


	//					*****	Saving Results		*****

unsigned long *optimalpath = malloc(nnodes*sizeof(unsigned long)); //size of the closed list
//we are wasting memory because the closed list is way bigger than the nodes we pass through, but it's a reasonable solution
unsigned long  path_node = node_goal;      //we set the last node as part of the optimal path

//initilisation of the while that will check the id of all the nodes that are in our path
//by looking at the parent of every node from the end to the beginning.

unsigned long npath_node = 0; optimalpath[npath_node] = path_node;

while(path_node!=node_start){
    path_node = Astatus[path_node].parent;
    npath_node ++;
    optimalpath[npath_node] = path_node;
}
clock_t end = clock();
elapsed_time = (double)(end - begin) / CLOCKS_PER_SEC;

printf("\n Total distance for shortest path: %f meters. Time elapsed to find the path is %f seconds. \n \n",Astatus[optimalpath[0]].g, elapsed_time);



	//					*****	CREATE TXT FILE AND PRINT RESULTS		*****
FILE * route_file;
route_file = fopen("Optimal_Path.txt","wt");
fprintf(route_file,"Optimal Path Found. Time elapsed is %f seconds. \n", elapsed_time);
for(int i=npath_node;i>-1;i--){
    fprintf (route_file, "Node ID: %2lu \t Lat: %f  | Long: %f  | Distance: %f \n \n", nodes[optimalpath[i]].id,
	nodes[optimalpath[i]].lat,nodes[optimalpath[i]].lon,  Astatus[optimalpath[i]].g );
}
}

		//					*****	FUNCTIONS CODE 		*****

/*
function for the creation of the sorted linked list. This function is just ran once.
*/
Element *sortedll (Element *first, double val, unsigned long node_position){
/*
function for the creation of the sorted linked list. This function is just ran once.
Then, the function keeps being updated by changing the pointer to the next element
as it is seen in the while clause in the main code
*/   
Element *inode  = NULL;
Element *previous = NULL;
Element *new      = NULL;

new = (Element*)malloc(sizeof(Element));
new->cost = val;
new->vec_pos = node_position;
new->next = NULL;
    
if (first == NULL){
    first = new;
    inode = first;
}

else if (new->cost < first->cost){
    new->next = first;
    first = new;
    inode = first;
}

else {
    previous = first;
    inode = first;
    
    while (inode!=NULL && new->cost >= inode->cost){
        previous = inode;
        inode = inode->next;
    }
    
    new->next = previous->next;
    previous->next = new;
    inode = new;
    
    }
    
    return first;
}


void ExitError(const char *miss, int errcode) { // Error function
    fprintf (stderr, "\nERROR: %s.\nStopping...\n\n", miss); exit(errcode);
}


double h(unsigned long inode, unsigned long goal, node *nodes){  
//heuristic distance calculation, the function is very self-explanatory
    double lat1, lon1, lat2, lon2;
    double lat1_rad, lat2_rad, lon1_rad, lon2_rad;
    double a, c, d;
    double delta_lon, delta_lat;
    //in degrees
    lat1=nodes[inode].lat;
    lon1=nodes[inode].lon;
    lat2=nodes[goal].lat;
    lon2=nodes[goal].lon;
    //deg to rad
    lat1_rad = lat1 * (PI/180);
    lat2_rad = lat2 * (PI/180);
    lon1_rad = lon1 * (PI/180);
    lon2_rad = lon2 * (PI/180);
	//difference in lat and long
    delta_lon = (lon2_rad-lon1_rad);
    delta_lat = (lat2_rad-lat1_rad);
    //harvesine function
    a = sin(delta_lat/2)*sin(delta_lat/2) + cos(lat1_rad)*cos(lat2_rad)*sin(delta_lon/2)*sin(delta_lon/2);
    c = 2*atan2(sqrt(a),sqrt(1-a));
    d = EARTH_RADIUS*c;
    
    return d;
    
}

double w (unsigned long inode, unsigned long goal, node *nodes){  
//distance between to node calculation, the function is very self-explanatory
    double lat1, lon1, lat2, lon2;
    double lat1_rad, lat2_rad, lon1_rad, lon2_rad;
    double a, c, d, x, y;
    double delta_lon, delta_lat;
    //in degrees
    lat1=nodes[inode].lat;
    lon1=nodes[inode].lon;
    lat2=nodes[goal].lat;
    lon2=nodes[goal].lon;
    //deg to rad
    lat1_rad = lat1 * (PI/180);
    lat2_rad = lat2 * (PI/180);
    lon1_rad = lon1 * (PI/180);
    lon2_rad = lon2 * (PI/180);
	//difference in lat and long
    delta_lon = (lon2_rad-lon1_rad);
    delta_lat = (lat2_rad-lat1_rad);
	//trigonometric solution
    x=delta_lon*cos((lat1_rad+lat2_rad)/2);
    y=delta_lat;
    d=EARTH_RADIUS*sqrt(x*x+y*y);
      //harvesine function
  //  a = sin(delta_lat/2)*sin(delta_lat/2) + cos(lat1_rad)*cos(lat2_rad)*sin(delta_lon/2)*sin(delta_lon/2);
  //  c = 2*atan2(sqrt(a),sqrt(1-a));
  //  d = EARTH_RADIUS*c;
    
    return d;
}

unsigned long binsearch(node *nodes, unsigned long nnodes, unsigned long val){  
    /*  
    
    This function finds the position in the nodes vector given an ID of a node.
    If the node ID does not exist in the list, nnodes is returned because in a non-existing position.
    An if-clause is checked later to denote that the node ID does not exist in the list.
    
    Variables used by this function:
    *nodes: pointer to the vector nodes
    nnodes: total number of nodes
    val: ID that we are going to look for
    
    */
    
    unsigned long binmin=0, binmax=nnodes-1, binmid;      // declaration of minimum, maximum and middle position
    
    while(binmax>=binmin){                              // search while some value exists between min and max
        binmid=(binmin+binmax)/2;                         // computation of the middle value
        if(nodes[binmid].id==val) return binmid;        // if the node is found, return the position
       
        else if(nodes[binmid].id<val) binmin=binmid+1;    // if the node we look for is has a greater ID than the node in the middle, change the minimum to the middle one from before+1
       
        else{                                       // if the node we look for is smaller than the node in the middle:
            
            if(binmax==0) return nnodes;              // This clause is skipped always except when the value we are looking for has a smaller ID than the smallest ID in the node list. In this case, return nnodes.
            else binmax=binmid-1;                       // The max value for the binary search is switched to the middle one from before-1
        }
    }
   
    return nnodes;  // In the node list, there are jumps between ID. This return will be triggered if this is the case. When the while from before is completed and no match has been found.
}

AStarStatus *Astar_function(node *nodes, unsigned long nnodes, unsigned long node_start, unsigned long node_goal, AStarStatus *Astatus){

	unsigned long node_successor, node_current, i;
	double   f, g_successor_current_cost;
	Element *first_element   = NULL;
	unsigned short nsucc;
      
for (int i = 0; i<nnodes; i++){
	
	Astatus[i].parent = nnodes; // put all parents as nonexistant. Remember we use nnodes as nonexistant value because is out of the range.
	
	Astatus[i].whq = NONE; //All nodes are set to none list, not open, not closed.
	}

/* 
From here on, we can find the initialisation of the first node and then the while clause that
is the whole algorithm of Astar. The code is only slightly commented because it totally
follows the pseudo-code given by the teacher and the code is self-explanatory.
*/
Astatus[node_start].parent = nnodes; // start node has no parent, nonexisting value again
Astatus[node_start].g = 0; //g fistance of first node is 0
Astatus[node_start].h = h(node_start,node_goal,nodes);
f = 0+ h(node_start,node_goal,nodes); // f of the first node
Astatus[node_start].whq = OPEN; //set the status of the node as open
first_element = sortedll(first_element,f,node_start); //put the first node in the first position of linked list

while (first_element!= NULL){
  
    node_current = first_element->vec_pos; //takes the first node because linked list is arranged
    first_element = first_element->next; //pointer points to the node that was pointing the taken node
   
    if (node_current == node_goal){
        printf(" \n arrived to destination\n");
        break;
    }

    unsigned short temp_successors = nodes[node_current].nsucc;
    unsigned long  node_successor;
    double         g_successor_current_cost;

    for(int i=0;i<temp_successors;i++){

        node_successor           = *(nodes[node_current].successors+i);
        g_successor_current_cost =  Astatus[node_current].g + w(node_current,node_successor,nodes);
        
        if(Astatus[node_successor].whq == OPEN){
            
            if (Astatus[node_successor].g <= g_successor_current_cost){
                
                continue;
            }

        } 
        
        else if (Astatus[node_successor].whq == CLOSED){

            if (Astatus[node_successor].g <= g_successor_current_cost){
                
                continue;
            }

            f = g_successor_current_cost + Astatus[node_successor].h;
            first_element = sortedll(first_element,f,node_successor);
            Astatus[node_successor].whq = OPEN;

        } 
        
        else {
            Astatus[node_successor].whq = OPEN;
            
            Astatus[node_successor].h = h(node_successor,node_goal,nodes);
            
            f = g_successor_current_cost + Astatus[node_successor].h;
           
            first_element = sortedll(first_element,f,node_successor);
            
        }
        
        Astatus[node_successor].g = g_successor_current_cost;
        Astatus[node_successor].parent = node_current;
    }
    Astatus[node_current].whq = CLOSED;
}

if (node_current != node_goal){
    ExitError("Error. The algorithm didn't find a path to the goal.",20);
}

printf("\n Astar algorithm terminated with exit. \n");
return Astatus;

}