#include <readline/readline.h>
#include <readline/history.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


char string[] = "A string/tof ,,tokens/nand some? more tokens";
char seps[] = " ,/t/n";
char *token;
int main( void )
{
	printf( "%s/n/nTokens:/n", string );
	/* Establish string and get the first token: */
	token = strtok( string, seps );
	while( token != NULL )
	{
		/* While there are tokens in "string" */
		printf( " %s/n", token );
		/* Get next token: */
		token = strtok( NULL, seps );
	}

	printf("\n");

	char *line;
	line = readline(NULL);	//reading user input
	/* Note that readline strips away the final \n */

	token = strtok(line, " ");
	while(token != NULL)
	{
		//printf("[%s]\n", token);
		printf("%s\n", token);
		token = strtok(NULL, " ");
		// if token is "exit" or "q", then you might break the while loop and stop the program
		// if (strcmp(token))
	}

	return 0;
}
