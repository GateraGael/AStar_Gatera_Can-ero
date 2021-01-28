#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>

	//					*º**	STRUCTS AND FUNCTIONS DECLARATION		*****

typedef struct {
    unsigned long id;           // Node identification
    char *name;                 // Pointer to the name of the node
    double lat, lon;            // Node location (latitude and longitude)
    unsigned short nsucc;       // Number of node successors; i. e. length of successors
    unsigned long *successors;  // Pointer to the vector of successors
} node;

void ExitError(const char *miss, int errcode);
unsigned long binsearch(node *nodes, unsigned long nnodes, unsigned long val);
void setsucc(node *nodes, char *succsize, unsigned long Apos, unsigned long Bpos);

	//					*****	MAIN FUNCTION		*****
	
int main (){

        char *readfile="spain.csv"; //file to read extracted from professor's website

    //					*****	VARIABLES DECLARATION		*****
    
    node *nodes;    							// Vector of all nodes of variables type: node
    
    unsigned long i=0, j=0;                     // Auxiliary variables for ways and nodes loops.
    unsigned long nnodes=0, nways=0;            // Variables that will count the number of nodes and ways.
    unsigned long nodeA, nodeB, Apos, Bpos;         // Pair of nodes that are connected. To be checked in one way or two ways.
    int l;                                      // Auxiliary variable for smaller loops.
    
    size_t len=79858;                           // memory size that the buffer requires to reserve in order to read each line
    char name[257];                             // .bin name vector (spain.bin, cataluna.bin, or whatever)
    char *field=NULL, *line=NULL, *auxline=NULL; // For the CSV file. *file will point to the beginning of each field in every line. 
   											 	// *line will point to the beginning of the current line and *auxline is useful for strtoul function
    char *succsize, *csvbuffer=NULL;           	// *succsize points to the vector indicating the dimension of each successors vector size in the memory
    bool isoneway;                              // indicates if a way is oneway or twoways

    // CHECK IF FILE CAN BE OPENED
    FILE *fp = fopen(readfile, "r");   // fp points to the file
    if(fp == NULL) {                    // check fp has been created
        perror("Unable to open file!");
        exit(1);
    }

    //					*****	ALLOCATE BUFFER MEMORY FOR TREATING LINES		*****
    
    if((csvbuffer = (char *) malloc(79858*sizeof(char))) == NULL) ExitError("Could not allocate memory for the CSV line buffer", 5);
    
    line=csvbuffer; // set the vector line to point to the beginning of the buffer line memory


    //					*****	COUNTING NODES AND WAYS		*****
    
  	while ((getline(&line, &len, fp)) > 0){ // while a line can be read
        switch(line[0]){   // select the first character of every line and differentiate cases:
            case 'n':           // if the first letter is an n, sum 1 node
                nnodes++;       
                break;
            
            case 'w':           // if the first letter is a w, sum 1 way
                nways++;        
                break;
            
            case 'r':
            goto endwhile1; 	// if the first letter is a r, we finished so break.
            
            break;
        }
        
        line=csvbuffer;    // pointer line assigned again to the buffer memory to read each line.
    }
   
    endwhile1:  // the codes jumps to this line after reading the first "r"

    //					*****	ALLOCATE MEMORY FOR THE NODES VECTOR AND FOR THE SUCCESSOR VECTOR OF EVERY NODE		*****
    
    if((nodes = (node *) malloc(nnodes*sizeof(node))) == NULL) ExitError("when allocating memory for the nodes vector", 5);
    if((succsize = (char *) malloc(nnodes*sizeof(char))) == NULL) ExitError("when allocating memory for the nsuccessors vector", 5);

    //					*****	REWIND FILE AND MAKE GRAPH	*****
    
    rewind(fp);     
    
    while ((getline(&line, &len, fp)) > 0) {    // while lines to be read exist:
        switch(line[0]){    	// select the first character of every line and differentiate cases:
            case 'n':           // if the line starts with n, we are dealing with a node:
                
     //					*****	SAVE LAT AND LONG FOR EVERY NODE IN VECTOR NODES	*****
     
                auxline=NULL;                            
                nodes[i].name=NULL;                     
                nodes[i].successors=NULL;                 
                nodes[i].nsucc=0;                       
                succsize[i]=0;                          // for now, no memory for the number of successors allocated
                
                field = strsep(&line,"|");              // pointing to the beginning of the field of the word "node"
                field = strsep(&line,"|");              // pointing to the beginning of the field of the node ID
                nodes[i].id=strtoul(field, &auxline, 0); // transform the ID from string to unsigned long			
                
                for(l=0; l<8; l++) field = strsep(&line,"|");  // skip 8 fields we don't want to read.
                	nodes[i].lat=atof(field);   // Transform latitude from string to double and save it
                	field = strsep(&line,"|");  // skip to next field
                	nodes[i].lon=atof(field);   // Transform longitude from string to double and save it
                	i++;    // next position in the vector (ready to the next line)
                	
                	break;
                
            case 'w':           // if the line starts with w, we are dealing with a way:
                
                
            //					*****	CLASSIFY THE SUCCESSORS OF EVERY NODE		*****
                     
                for(l=0; l<7; l++) field = strsep(&line,"|");  // skip 7 fields that we don't want to read
                
                isoneway = false;   //if something is read in the oneway field, it means the way is one way.
                if( (field = strsep(&line,"|")) != line-1 ){
                    
                    if(field[0]=='o') isoneway = true; 
                } 
                
                field = strsep(&line,"|"); // skip field
                
                Apos=nnodes; //done only to enter the while, that breaks after this relation is changed
                
                while( Apos == nnodes ){ // this while finds the first node of a line that exists in our list of nodes. 
                    
                    auxline=NULL; 
                    
                    if( (field = strsep(&line,"|")) != NULL ){  // if the first node of the list of successors of a way does not exist in our list, read the next.
                        nodeA=strtoul(field, &auxline, 0);       
                        Apos=binsearch(nodes, nnodes, nodeA); // once the first existing node is found, get its position in the vector
                    }
                }
                
                while( (field = strsep(&line,"|")) != NULL ){   // while the line is not finished, this means that we will continue reading successors
                    
                    auxline=NULL;
                    nodeB=strtoul(field, &auxline, 0);           
                    Bpos=binsearch(nodes, nnodes, nodeB);     // search its position in the vector
                    
                    if(Bpos != nnodes){   						// all the process is skipped if the sucessor is not in our list of nodes. This if clause checks it.
                        setsucc(nodes,succsize,Apos,Bpos);  	// set node B (Bpos position) as successor of node A (Apos position)
                        
                        if(isoneway==false) setsucc(nodes,succsize,Bpos,Apos);  //if the way has two directions, put A as succesor of B too.
                        	Apos=Bpos;  // put Bpos in Apos and set its successor again.
                    }
                }
               		
               		break;
            
            case 'r':
                goto endwhile2; // break while after first relation is read.
                
                break;
        } 
          
        line=csvbuffer; //line points again to the beginning of its allocated buffer memory
        
    }
    endwhile2:
    printf("Process finished \n");

    FILE *fin; // Pointing to the about to be created bin file.

	//					*****	WRITING		*****
	
    /* Computing the total number of successors */
    unsigned long ntotnsucc=0UL;
    for(i=0; i < nnodes; i++) ntotnsucc += nodes[i].nsucc;

    /* Setting the name of the binary file */
    strcpy(name, readfile); strcpy(strrchr(name, '.'), ".bin");
    if ((fin = fopen (name, "wb")) == NULL){
        ExitError("the output binary data file cannot be opened", 31);
    }

    /*Global data --- header  */
    if( fwrite(&nnodes, sizeof(unsigned long), 1, fin) + fwrite(&ntotnsucc, sizeof(unsigned long), 1, fin) != 2 ){
            ExitError("when initializing the output binary data file", 32);
    }

    /* Writing all nodes */
    if( fwrite(nodes, sizeof(node), nnodes, fin) != nnodes ){
        ExitError("when writing nodes to the output binary data file", 32);
    }

    /* Writing sucessors in blocks */
    for(i=0; i < nnodes; i++) if(nodes[i].nsucc) {
        if( fwrite(nodes[i].successors, sizeof(unsigned long), nodes[i].nsucc, fin) != nodes[i].nsucc ) {
            ExitError("when writing edges to the output binary data file", 32);
        }
    }
    
    fclose(fin);

    printf("\n Bin File Generated.");
    
}

		//					*****	FUNCTIONS CODE 		*****

void setsucc(node *nodes, char *succsize, unsigned long Apos, unsigned long Bpos){
    /*  
    This function adds successors to the nodes. It adds the node B as succesor of node A. 
    
    Variables used by this function:
    *nodes: pointer to the nodes vector.
    *succsize: pointer to a vector that contains the size of the successors vector for every node.
	Apos: position in the nodes vector of node A
    Bpos: position in the nodes vector of node B
    
    */
    
    nodes[Apos].nsucc++;  // Sums one successor more to the node in the nodes vector.
    
    if(nodes[Apos].nsucc>succsize[Apos]){   // checks if there is more space in the successors vector of this node.
        
        succsize[Apos]=succsize[Apos]+2;    // if there is no more space, update the successors dimension vector
        nodes[Apos].successors = (unsigned long *)realloc(nodes[Apos].successors, sizeof(unsigned long)*succsize[Apos]); // reallocate more memory to the vector of successors of this node.
    }
    
    *(nodes[Apos].successors+nodes[Apos].nsucc-1)=Bpos; // After doing the memory reallocation OR NOT, in any case,  the Node B is put as a successor of node A.
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

void ExitError(const char *miss, int errcode) {     
    fprintf (stderr, "\nERROR: %s.\nStopping...\n\n", miss); exit(errcode);
}
