#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>

int miss;
int hit;
int writes;
int reads;
int cachesize;
int blocksize;
int sets;
int assoc;
size_t shift;
size_t tag;
size_t ptag;
size_t pindex;
int offset=0;
int setIndex=0;
int index=0;
int counter;
char prefetch='n';
int i,j,least;


typedef struct matrix{
size_t tag;
size_t position;
}matrix;
matrix** cache;

void associativity(char *input){
   if (strcmp(input,"direct")==0){
     assoc = 1;
     sets = cachesize /(assoc *blocksize);
   }
   else if (strcmp(input,"assoc")==0){
     sets =1;
     assoc = cachesize / (sets*blocksize); 
   }
   
   else{
        sscanf(input,"assoc:%d",&assoc);
        sets = cachesize/(assoc*blocksize);
   }
}
void tagindexfinder(size_t address){
  offset=log(blocksize)/log(2);
  setIndex=log(sets)/log(2);
  shift=((1<<setIndex)-1);
  index=(address>>offset)&shift;
  tag=address>>(offset+setIndex);
}
void prefetcher(size_t tag,size_t index,int assoc){
   for(i=0;i<assoc;i++){
     if(cache[index][i].tag==tag)
             break;
     else if (cache[index][i].tag==-18){
        reads++;
	counter++;
	cache[index][i].tag=tag;
	cache[index][i].position=counter;
	return;
     }
     else if(i==(assoc-1)){
	reads++;
	least=0;
	for(j=0;j<assoc;j++){
	   if(cache[index][j].position<=cache[index][least].position)
               least=j;	
	}
	cache[index][least].tag=tag;
	counter++;
	cache[index][least].position=counter;
	break;
    }
  }
	
}
char power(int numb){
 if (numb<=0)
    return 'n';
  
 while (numb != 1){
   if (numb %2 !=0)
    return 'n';
  numb =numb/2;
  }
  return 'y';
 }
 

void nonPrefetch(char command){

 for(i=0;i<assoc;i++){
    if(cache[index][i].tag==tag){
       hit++;
       counter++;
       if (command=='W')
          writes++;
        cache[index][i].position=counter;
      break;
     }
     else if(cache[index][i].tag==-18){
      miss++;
      reads++;
      counter++;
      if (command=='W')
         writes++;
      cache[index][i].tag=tag;
      cache[index][i].position=counter;
      if (prefetch=='y')
         prefetcher(ptag,pindex,assoc);
         
      break;
    }
    else if(i==(assoc-1)){
	    miss++;
	    reads++;
	    if (command=='W')
               writes++;	
	    least=0;
	    for(j=0;j<assoc;j++){
		if(cache[index][j].position<=cache[index][least].position)
		   least=j;	
	    }
	    cache[index][least].tag=tag;
	    counter++;
	    cache[index][least].position=counter;
	    if (prefetch=='y')
	       prefetcher(ptag,pindex,assoc);
	  break;
	}
    
  }
return;
}
	
int main(int argc, char** argv){
  if (argc!=6){
     printf("Error, incorrect number of argument\n");
     return 0; 
  }
  cachesize=atoi(argv[1]);
  blocksize=atoi(argv[4]);
  char isPowerOf2;
  isPowerOf2 = power(blocksize);
   if (isPowerOf2 == 'n'){
       printf("Error, block size not power of 2\n");
       return 0;
  } 
  
  size_t address;
  size_t paddress;
  char progcont[50];
  char command;

  if(strcmp(argv[3],"lru")!=0){
     printf("Error, incorrect type of policy");
     return 0;
  }
  associativity(argv[2]);
  int row,col;
  cache=(matrix**)malloc(sizeof(matrix*)*sets);
  for(row=0;row<sets;row++){
    cache[row]=(matrix*)malloc(sizeof(matrix)*(assoc));
  }

  for(row=0;row<sets;row++){
    for(col=0;col<assoc;col++){
         cache[row][col].tag=-18;
    }
  }
    
  FILE* file;
  file=fopen(argv[5],"r");
  if(file==NULL){
    printf("Error unable to open file\n");
    return 0;
  }
 while (file){
    fscanf(file, "%s ",progcont);
    if (progcont[0]=='#')
       break;
    fscanf(file,"%c %zx\n",&command,&address);
    tagindexfinder(address);
    nonPrefetch(command);	
 }

fclose(file);
 printf("no-prefetch\n");
 printf("Memory reads: %d\n",reads);
 printf("Memory writes: %d\n",writes);
 printf("Cache hits: %d\n",hit);
 printf("Cache misses: %d\n",miss);
 for(row=0;row<sets;row++){
    for(col=0;col<assoc;col++){
         cache[row][col].tag=-18;
	  cache[row][col].position=0;
     }
 }
    miss=0;
    hit=0;
    reads=0;
    writes=0;
    counter=0;
    prefetch='y';
 file=fopen(argv[5],"r");
 if(file==NULL){
    printf("Error, unable to reopenfile for prefetch\n");
    return 0;
  }
  while (file){
     fscanf(file, "%s ",progcont);
     if (progcont[0]=='#')
       break;
     fscanf(file,"%c %zx\n",&command,&address);
     tagindexfinder(address);
     paddress=address+blocksize;
     
     pindex=(paddress>>offset)&shift;
     ptag=paddress>>(offset+setIndex);
     nonPrefetch(command);
  }
 for(row=0;row<sets;row++)
    free(cache[row]);
 free(cache);
 fclose(file);
 printf("with-prefetch\n");
 printf("Memory reads: %d\n",reads);
 printf("Memory writes: %d\n",writes);
 printf("Cache hits: %d\n",hit);
 printf("Cache misses: %d\n",miss);
return 0;
}
