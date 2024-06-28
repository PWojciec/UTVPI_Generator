#include "utvpiInterpreter.h"

#include <stdlib.h>
#include <string.h>

bool parseFile(struct UCS** U, FILE * fin){

	//Temporary Storage
	INT_T n, i;
	char *var;
	INT_T v[3], s[3];
	INT_T vcnt, scnt, error;
	struct econs* EC;

    //Open the file
	char *buf, *vars, *signs;

	//Initialize the storage struct if necessary
	if(*U == NULL){
		*U = (struct UCS *) malloc(sizeof(struct UCS));
		(*U) -> constraints = sllist_create();
	}

    //Read the file 
	var = (char *) malloc(10*sizeof(char));
	buf = (char *) malloc(500*sizeof(char));
	vars = (char *) malloc(500*sizeof(char));
	signs = (char *) malloc(500*sizeof(char));

	
	//Scan away any comments
	fgets(buf, 500, fin);
	while(strstr(buf, "//") != NULL){
		//Be sure to read the rest of the current line
		while(strstr(buf, "\n") == NULL)
			fgets(buf, 500, fin);
		//Get the next line
		fgets(buf, 500, fin);
	}
	strcpy(vars, buf);
	strcpy(signs, buf);
	
	//Get N count
	sscanf(buf, FORMAT" %s\n", &n, var);
	(*U) -> n = n;
	(*U) -> absolutes = (INT_T *) malloc (2 * (n + 1) * sizeof(INT_T));
	for(i = 0; i < 2*(n+1); i++)
		(*U) -> absolutes[i] = 0;
	(*U) -> m = 0;
	(*U) -> c_max = 0;

	free(var);
	var = NULL;
	
	while(fgets(buf, 500, fin) != NULL){
		strcpy(signs, buf);
		strcpy(vars, buf);
		vcnt = 0;
		scnt = 0;
		//printf("\n\nParsing: %s\n", buf);
		
		//Extract the Variables
		var = strtok(vars, " _-+xX<=\n");
    	while(var && (vcnt < 3)) {
     		//puts(var);
			v[vcnt] = (INT_T)strtol(var, NULL, 10);
			vcnt++;
      	  	var = strtok(NULL, " _-+xX<=\n");
			//fprintf(stdout, "Var #%d: %d", vcnt, v[vcnt-1]);
    	}
		if(var && (vcnt == 3)){
			error = 1;
			goto FAIL;
		}
			

		//Extract the corresponding signs
		var = strtok(signs, " _xX<=0123456789\n");
		//printf("Signs: %s\n", buf);
    	while(var && (scnt < 3)) {
     		//printf("Sign: %s\n", var);
			if(var[0] == '-')
				s[scnt] = -1;
			else
				s[scnt] = 1;
			scnt++;
      	  	var = strtok(NULL, " _xX<=0123456789\n");
    	}
		if(var && (scnt < 3)){
			error = 2;
			goto FAIL;
		}

		//fprintf(stdout, "Found VCNT: %d SCNT: %d\n", vcnt, scnt);

		//Store the extracted edge constraint in U
		//Update edge count
		(*U) -> m = ((*U) -> m) + 1;

		//Create new constraint
		EC = (struct econs *) malloc (sizeof(struct econs));
		if (vcnt == 3){
			if(scnt == 1){
				//Set V2's sign				
				EC -> x1 = v[0];
				EC -> x2 = s[0] * v[1];
				EC -> c = v[2];
			}
			else if (scnt == 3){
				//Set ALL signs			
				EC -> x1 = s[0] * v[0];
				EC -> x2 = s[1] * v[1];
				EC -> c =  s[2] * v[2];
			}
			else{ //2
				//printf("For x%d +- x%d <= %d\n", v[0], v[1], v[2]);
				if((buf[0] == '-') || (buf[0] == '+')){
					EC -> x1 = s[0] * v[0];
					EC -> x2 = s[1] * v[1];
					EC -> c =  v[2];
				}
				else{
					EC -> x1 = v[0];
					EC -> x2 = s[0] * v[1];
					EC -> c =  s[1] * v[2];
				}
			}
		}
		else{ //There are 2
			if(scnt == 1){
				//Check initial character to determine whether 1 & 2 or 2 & 3 are signed.
				if((buf[0] == '-') || (buf[0] == '+')){
					//Add Negative Grey or White Constraint
					EC -> x1 = IMAX;
					EC -> x2 = s[0] * v[0];
					EC -> c =  v[1];
				}				
				else{
					//Add White Constraint
					EC -> x1 = IMAX;
					EC -> x2 = v[0];
					EC -> c =  s[0] * v[1];
				}
			}
			else{  //2
				//Add Negative Grey or White Constraint
				EC -> x1 = IMAX;
				EC -> x2 = s[0] * v[0];
				EC -> c =  s[1] * v[1];
			}
			
			if((EC -> x2) > 0)
			{
				(*U) -> absolutes[(EC -> x2)] = 1;
			}
			else
			{
				(*U) -> absolutes[abs(EC -> x2) + ((*U) -> n)] = 1;
			}
		}
		if(abs(EC -> c) > abs((*U)-> c_max)){
			(*U) -> c_max  = (EC -> c);
		}
		error = sllist_push_back((*U) -> constraints, EC);
		//printf("Pushed Back %d, new size %d\n", error, ((*U) -> constraints) -> size);
	}
    
	//Deallocate Everything
	free(var);
	var = NULL;
	free(buf);
	buf = NULL;
	free(vars);
	vars = NULL;
	free(signs);
	signs = NULL;

    return true;

	FAIL: 
		//Deallocate Everything
		if(var){
			free(var);
			var = NULL;
		}
		if(buf){
			free(buf);
			buf = NULL;
		}
		if(vars){	
			free(vars);
			vars = NULL;
		}
		if(signs){
			free(signs);
			signs = NULL;
		}
		sllist_destroy((*U) -> constraints, 1); 
		free((*U) -> absolutes);
		free((*U));
		*U = NULL;
	
		//Output error code
		if( error == 1)
			fprintf(stderr, "Formating Issue.\n");
		else if( error == 2)
			fprintf(stderr, "Formating Issue.\n");
		else
			fprintf(stderr, "Formating Issue.\n");

		return false;
}
