/*The crash(crashell) September 1 2023 - 6:06 IST
	Life Cycle of a Shell : Intitalize,Interpret,Terminate*/
/***Pre-processor Section***/
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/wait.h>
#include<limits.h>
#include<stdbool.h>	
#include "shell_cmds.h"

/***Macro Definitions***/
#define RL_BUFF_S 1024
#define TOK_BUFF_S 1024
#define PIPE_BUFF_S 1024
#define TOK_DELIM "\n\r\t\a "
/**************************************************************************************************************************************/
/***Operation Sections***/
/***Function Declarations for Operations***/
void main_loop();
char *read_line();
char **parse_line(char *line);
int launch_prog(char **args);
int exec_prog(char **args);
int launch_pipe(char **args,int pipe_loc,int argc);

/***Function Definitions for Operations***/
void main_loop(){
	char *line;
	char **args;
	int status;
	
	do{
		char cwd[PATH_MAX]; 
		printf("\n%s !:",getcwd(cwd,sizeof(cwd)));
		
		line = read_line();
		args = parse_line(line);
		status = exec_prog(args);
		
		free(line);
		free(args);
	}while(status);
}

char* read_line(){
	int buff_s = RL_BUFF_S;
	char* buffer = malloc(buff_s * sizeof(char));
	int pos = 0;
	
	if(!buffer){
		fprintf(stderr,"craSh : allocation error\n");
		exit(EXIT_FAILURE);
	}
	
	//loop to read characters
	while(1){
		char ch = getchar();
		
		//Adding null terminator
		if(ch == EOF || ch == '\n'){
			buffer[pos] = '\0';
			return buffer;
		}else{
			buffer[pos] = ch;
		}
		pos++;
		
		//reallocate on exceeding the limit
		if(pos >= buff_s){
			buff_s += RL_BUFF_S;
			buffer = realloc(buffer,buff_s);
			
			if(!buffer){
				fprintf(stderr,"craSh : allocation error\n");
			}
		}
	}
}
	
char **parse_line(char *line){
	int tok_buff_s = TOK_BUFF_S;
	char **tokens = malloc(tok_buff_s * sizeof(char*));
	char *tok;
	int pipe_loc;
	int argc = 0;
	int pos = 0;
	bool is_pipe = false;
	
	if(!tokens){
		fprintf(stderr,"craSh : allocation error\n");
		exit(EXIT_FAILURE);
	}
	//takes in a string and delimiters
	tok = strtok(line,TOK_DELIM);
	
	while(tok!=NULL){	//If the commands are piped
		if(strcmp(tok,"|") == 0){
			is_pipe = true;
			pipe_loc = argc;
		}
		tokens[pos] = tok;
		argc++;
		pos++;
		//reallocate on exceeding limit
		if(pos >= tok_buff_s){
			tok_buff_s += TOK_BUFF_S;
			tokens = realloc(tokens, tok_buff_s);
			
			if(!tokens){
				fprintf(stderr,"craSh : allocation error");
			}
		}
		tok = strtok(NULL,TOK_DELIM);
	}
	
	if(is_pipe == true){
		if(launch_pipe(tokens,pipe_loc,argc) == 1){
			main_loop();
		}
	}
	else{	
		return tokens;
	}
}

int launch_pipe(char **args,int pipe_loc,int argc){
	int pipe_buff_s = PIPE_BUFF_S;
	int status_1,status_2;
	char **arr_out = malloc(pipe_buff_s * sizeof(char *));
	char **arr_in = malloc(pipe_buff_s * sizeof(char *));
	pid_t child_out, child_in, wpid_1,wpid_2;
	
	int fd[2];
	pipe(fd);
	
	//closing pipes in the parent process
	
	child_out = fork();
	if(child_out == 0){	//pipe output end
		for(int i = 0; i < pipe_loc; i++){	//args for output child
			arr_out[i] = args[i];
		}
		
		dup2(fd[1],STDOUT_FILENO);	//duplicating to standard output - 0
		close(fd[0]);	
		close(fd[1]);
		
		if(execvp(arr_out[0],arr_out) == -1){
			perror("error pipe out");
			exit(EXIT_FAILURE);
		}
	}
	
	child_in = fork();
	if(child_in == 0){	//pipe input end
		int j = 0;
		for(int i = pipe_loc+1; i < argc ; i++){	//args for input child
			arr_in[j] = args[i];
			j++;
			
			if(j == argc - pipe_loc - 1){
				break;
			}
		}
		arr_in[j] = NULL;
		
		dup2(fd[0],STDIN_FILENO); 	//duplicating to standard input - 0
		close(fd[0]);
		close(fd[1]);
		
		execvp(arr_in[0],arr_in);
		/*if(execvp(arr_in[0],arr_in) == -1){
			perror("error pipe in");
		}else{
			fflush(stdout);
		}*/
	}
	
	close(fd[0]);
	close(fd[1]);

	do{
		wpid_1 = waitpid(child_out,&status_1,WUNTRACED); // return status if the child has exited
		wpid_2 = waitpid(child_in,&status_2,WUNTRACED);
	}while(!WIFEXITED(status_1) && WIFSIGNALED(status_1) && !WIFEXITED(status_2) && WIFSIGNALED(status_2));
	
	free(arr_in);
	free(arr_out);
	
	return 1;
}

int launch_prog(char **args){
	pid_t child, wpid;
	int status;
	
	child = fork();
	
	//Child Process
	if(child == 0){
		if(execvp(args[0],args) == -1){
			perror("error executing program :");
			exit(EXIT_FAILURE);
		}
	//Fork Error
	}else if(child < 0){
		perror("error forking :");
		exit(EXIT_FAILURE);
	//Parent Process waits for the child to exit or	get signalled
	}else{
		do{
			wpid = waitpid(child,&status,WUNTRACED); // return status if the child has exited
		}while(!WIFEXITED(status) && WIFSIGNALED(status));
	}
	return 1;
}

int exec_prog(char **args){
	if(args[0] == NULL){
		return 1;
	}
	
	for(int i = 0;i < shell_cmds_s();i++){
		if(strcmp(args[0],shell_cmds[i]) == 0){
			return (*shell_cmds_f[i])(args);
		}
	}
	return launch_prog(args);
} 
				
/**************************************************************************************************************************************/
/**************************************************************************************************************************************/
/***The Main Function***/

int main(){
	main_loop();
}
