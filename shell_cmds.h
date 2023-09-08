/***Shell Command Sections***/
/***Function Declarations for Shell Commands***/
int crash_cd(char **args);
int crash_help();
int crash_exit();
int shell_cmds_s();

/***Mapping Shell Commands to Functions***/
char *shell_cmds[] = {
	"cd",
	"help",
	"exit"
};

int (*shell_cmds_f[])(char **) = { //array of function pointers that return int
	&crash_cd,
	&crash_help,
	&crash_exit
};

int shell_cmds_s(){
	return sizeof(shell_cmds)/sizeof(shell_cmds[0]);
}
/**************************************************************************************************************************************/
/***Function Definitions for Shell Commands***/
int crash_cd(char **args){
	if(args[1] == NULL){
		fprintf(stderr,"Usage : cd <path>");
	}else if(chdir(args[1]) != 0){
		perror("craSh :");
	}
	return 1;
}

int crash_help(){
	printf("\tGowtham Manohar's craSh\n");
	printf("Enter Program Name,Arguments & hit Enter");
	printf("The following shell commands are built-in");
	
	for(int i =0;i<(int)shell_cmds_s();i++){
		printf("\t%d.%s\n",i+1,shell_cmds[i]);
	}
	return 1;	
}

int crash_exit(char **args){
	printf("\nAdios Amigo!\n");
	return 0;
}
