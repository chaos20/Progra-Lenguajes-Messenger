#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_String_LEN 200


//the function to read lines of variable length

char* scan_line(char *line)
{
    int ch; //as getchar() returns `int`

    if( (line = malloc(sizeof(char))) == NULL) //allocating memory
    {
        //checking if allocation was successful or not
        printf("unsuccessful allocation");
        exit(1);
    }

    line[0]='\0';

    for(int index = 0; ( (ch = getchar())!='\n' ) && (ch != EOF) ; index++)
    {
        if( (line = realloc(line, (index + 2)*sizeof(char))) == NULL )
        {
            //checking if reallocation was successful or not
            printf("unsuccessful reallocation");
            exit(1);
        }

        line[index] = (char) ch; //type casting `int` to `char`
        line[index + 1] = '\0'; //inserting null character at the end
    }

    return line;
}


int main(){
  char* hl = NULL;
  char* check = "0n0";
  hl = scan_line(hl);


  while (strcmp(hl,check)){
    printf("%s\n",hl);
    hl = scan_line(hl);
  }

  free(hl);

}
