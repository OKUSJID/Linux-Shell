#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <string.h>

using namespace std;

const int MAX_CMDS = 10;
const int MAX_AGRZ = 10;

void convertstring(string commands[][MAX_AGRZ], char *all_args[][MAX_AGRZ]);
int parseCommands(string parsedCommands[][MAX_AGRZ], string commandString);
void executeCommand(int commandNr, char *all_args[][MAX_AGRZ]);

int main()
{
    string line;

    while (true)
    {
        string commands[MAX_CMDS][MAX_AGRZ];

        //Enter commands
        cout << "Please Enter the command: ";
        getline(cin, line);

        // Quit shell program if these inputs given
        if (line == "exit" || cin.eof())
        {
            cout << endl
                 << "HAVE A NICE DAY !!" << endl;

            exit(1);
            // If enter pressed, don't pass for execution
        }
        else if (line == "\0")
        {
            continue;
        }

        // Parse all commands seperated by pipes
        int commandNr = parseCommands(commands, line);
        // cout << "Number of commands: " << commandNr << endl;
        //  Convert types and prepare for execuction
        char *all_args[MAX_CMDS][MAX_AGRZ];
        convertstring(commands, all_args);

        // Execute commands
        executeCommand(commandNr, all_args);
    }
    return 1;
}

void executeCommand(int commandNr, char *all_args[][MAX_AGRZ])
{
    int pid[commandNr];

    int totalPipes = (commandNr - 1) * 2;
    int pipes[totalPipes];

    // Initializing pipes
    for (int x = 0; x < totalPipes; x += 2)
        pipe(pipes + x);

    // Lets create as many processes as there are commands
    for (int i = 0; i < commandNr; i++)
    {
        // For each command lets create a new process
        if ((pid[i] = fork()) < 0)
        {
            // If failed to created new process inform user and quit
            cout << ("*** ERROR: forking child process failed\n");
            exit(-1);
        }
        else if (pid[i] == 0)
        {
            // If its child process

            // First process only produces result to pip write end,
            // as input it gets from stdin - console
            if (i == 0)
            {

                int x = 0;
                int out_index = 0;
                bool output_re = false;
                string o_args1[10][10];
                string o_args2[10][10];

                int in_index = 0;
                bool input_re = false;
                string in_args1[10][10];
                string in_args2[10][10];

                while (all_args[i][x] != NULL)
                {

                    if (strcmp(all_args[i][x], ">") == 0)
                    {
                        out_index = x;
                        // cout << "out_index = " << x << endl;

                        for (int y = 0; y < out_index; y++)
                        {
                            // cout << "COMMAND: " << endl;
                            o_args1[i][y] = all_args[i][y];
                            // cout << y << " :" << o_args1[i][y] << endl;
                        }

                        int z = 0;
                        for (int y = out_index + 1; all_args[i][y] != NULL; y++)
                        {
                            // cout << "PATH:" << endl;
                            o_args2[i][z] = all_args[i][y];
                            // cout << y << " :" << o_args2[i][z] << endl;
                            z++;
                        }
                        output_re = true;
                    }

                    if (strcmp(all_args[i][x], "<") == 0)
                    {
                        in_index = x;
                        // cout << "out_index = " << x << endl;

                        for (int y = 0; y < in_index; y++)
                        {
                            // cout << "COMMAND: " << endl;
                            in_args1[i][y] = all_args[i][y];
                            // cout << y << " :" << in_args1[i][y] << endl;
                        }

                        int z = 0;
                        for (int y = in_index + 1; all_args[i][y] != NULL; y++)
                        {
                            // cout << "PATH:" << endl;
                            in_args2[i][z] = all_args[i][y];
                            // cout << y << " :" << in_args2[i][z] << endl;
                            z++;
                        }
                        input_re = true;
                    }
                    x++;
                }

                if (output_re == true && input_re == false)
                {
                    string n = *o_args2[i];
                    char *name = new char[n.length()];
                    char *command[10][10];
                    for (int i = 0; i < n.length(); i++)
                    {
                        name[i] = n[i];
                    }

                    // cout << "Path: " << name << endl;
                    int backup_fd = dup(1);
                    int openfd = open(name, O_WRONLY | O_CREAT, 0777);
                    dup2(openfd, 1);

                    convertstring(o_args1, command);

                    execvp(*command[0], command[0]);
                    perror("Wrong Command Specified!");
                    exit(1);
                }

                if (output_re == false && input_re == true)
                {
                    string n = *in_args2[i];
                    char *name = new char[n.length()];
                    char *command[10][10];
                    for (int i = 0; i < n.length(); i++)
                    {
                        name[i] = n[i];
                    }

                    // cout << "Path: " << name << endl;
                    int backup_fd = dup(0);
                    int openfd = open(name, O_RDONLY, 0777);
                    dup2(openfd, 0);
                    dup2(pipes[1], 1);
                    convertstring(in_args1, command);

                    execvp(*command[0], command[0]);
                    perror("Wrong Command Specified!");
                    exit(1);
                }
                if (output_re == false && input_re == false)
                {
                    dup2(pipes[1], 1);

                    // Closing all the other pipes ends, we don't need them
                    for (int x = 0; x < totalPipes; x++)
                        close(pipes[x]);

                    // cout << *all_args[i] << endl;
                    //  Execute program
                    execvp(*all_args[i], all_args[i]);
                    perror("Wrong Command Specified!");
                    exit(1);
                }

                if (output_re == true && input_re == true)
                {

                    cout << "INVALID COMMAND....!!!" << endl;
                    exit(1);
                }
                // Last process only gets input from read pipe end end prints
                // to stdout - console
            }
            else if (i == commandNr - 1)
            {

                int x = 0;
                int out_index = 0;
                bool output_re = false;
                char *o_args1[10][10];
                string o_args2[10][10];

                int in_index = 0;
                bool input_re = false;
                char *in_args1[10][10];
                string in_args2[10][10];

                while (all_args[i][x] != NULL)
                {

                    // cout<<"Command: "<<i<<" "<<x<<" : "<<all_args[i][x]<<endl;
                    if (strcmp(all_args[i][x], ">") == 0)
                    {
                        // cout<<"MEIII YAHANNN HUNNN!!!!"<<endl;
                        out_index = x;
                        // cout << "out_index = " << x << endl;

                        for (int y = 0; y < out_index; y++)
                        {
                            // cout << "COMMAND: " << endl;
                            o_args1[0][y] = all_args[i][y];
                            // cout << y << " :" << o_args1[i][y] << endl;
                        }

                        int z = 0;
                        for (int y = out_index + 1; all_args[i][y] != NULL; y++)
                        {
                            // cout << "PATH:" << endl;
                            o_args2[i][z] = all_args[i][y];
                            // cout << z << " :" << o_args2[i][z] << endl;
                            z++;
                        }
                        output_re = true;
                    }

                    if (strcmp(all_args[i][x], "<") == 0)
                    {
                        in_index = x;
                        // cout << "out_index = " << x << endl;

                        for (int y = 0; y < in_index; y++)
                        {
                            // cout << "COMMAND: " << endl;
                            in_args1[0][y] = all_args[i][y];
                            // cout << y << " :" << in_args1[i][y] << endl;
                        }

                        int z = 0;
                        for (int y = in_index + 1; all_args[i][y] != NULL; y++)
                        {
                            // cout << "PATH:" << endl;
                            in_args2[i][z] = all_args[i][y];
                            // cout << y << " :" << in_args2[i][z] << endl;
                            z++;
                        }
                        input_re = true;
                    }
                    x++;
                }

                if (output_re == true && input_re == false)
                {
                    string n = *o_args2[i];
                    char *name = new char[n.length()];
                    char *command[10][10];
                    for (int i = 0; i < n.length(); i++)
                    {
                        name[i] = n[i];
                    }

                    // cout << "Path: " << name << endl;
                    int backup_fd = dup(1);
                    int openfd = open(name, O_WRONLY | O_CREAT, 0777);
                    dup2(openfd, 1);

                    dup2(pipes[totalPipes - 2], 0);

                    // Closing unsed pipe ends
                    for (int x = 0; x < totalPipes; x++)
                        close(pipes[x]);

                    execvp(*o_args1[0], o_args1[0]);
                    perror("Wrong Command Specified!");
                    exit(1);
                }

                if (output_re == false && input_re == true)
                {
                    string n = *in_args2[i];
                    char *name = new char[n.length()];
                    char *command[10][10];
                    for (int i = 0; i < n.length(); i++)
                    {
                        name[i] = n[i];
                    }

                    // cout << "Path: " << name << endl;
                    int backup_fd = dup(0);
                    int openfd = open(name, O_RDONLY, 0777);
                    dup2(openfd, 0);

                    execvp(*in_args1[0], in_args1[0]);
                    perror("Wrong Command Specified!");
                    exit(1);
                }

                // // setting (duplicating) process stdin to receive input
                // // from pipes read end
                if (input_re == false && output_re == false)
                {
                    dup2(pipes[totalPipes - 2], 0);

                    // Closing unsed pipe ends
                    for (int x = 0; x < totalPipes; x++)
                        close(pipes[x]);
                    // cout << *all_args[i] << endl;
                    //  Execute program
                    execvp(*all_args[i], all_args[i]);
                    perror("Wrong Command Specified!");
                    exit(1);
                }

                if (input_re == true && output_re == true)
                {
                    cout << "Invalid command!!!" << endl;
                }
            }
            // Middle process, receives input from pipe read end passes
            // to another process to write end of pipe
            else
            {
                int x = 0;
                int out_index = 0;
                bool output_re = false;
                char *o_args1[10][10];
                string o_args2[10][10];

                int in_index = 0;
                bool input_re = false;
                char *in_args1[10][10];
                string in_args2[10][10];

                while (all_args[i][x] != NULL)
                {

                    // cout<<"Command: "<<i<<" "<<x<<" : "<<all_args[i][x]<<endl;
                    if (strcmp(all_args[i][x], ">") == 0)
                    {
                        // cout<<"MEIII YAHANNN HUNNN!!!!"<<endl;
                        out_index = x;
                        // cout << "out_index = " << x << endl;

                        for (int y = 0; y < out_index; y++)
                        {
                            // cout << "COMMAND: " << endl;
                            o_args1[0][y] = all_args[i][y];
                            // cout << y << " :" << o_args1[i][y] << endl;
                        }

                        int z = 0;
                        for (int y = out_index + 1; all_args[i][y] != NULL; y++)
                        {
                            // cout << "PATH:" << endl;
                            o_args2[i][z] = all_args[i][y];
                            // cout << z << " :" << o_args2[i][z] << endl;
                            z++;
                        }
                        output_re = true;
                    }

                    if (strcmp(all_args[i][x], "<") == 0)
                    {
                        in_index = x;
                        // cout << "out_index = " << x << endl;

                        for (int y = 0; y < in_index; y++)
                        {
                            // cout << "COMMAND: " << endl;
                            in_args1[0][y] = all_args[i][y];
                            // cout << y << " :" << in_args1[i][y] << endl;
                        }

                        int z = 0;
                        for (int y = in_index + 1; all_args[i][y] != NULL; y++)
                        {
                            // cout << "PATH:" << endl;
                            in_args2[i][z] = all_args[i][y];
                            // cout << y << " :" << in_args2[i][z] << endl;
                            z++;
                        }
                        input_re = true;
                    }
                    x++;
                }

                if (output_re == true && input_re == false)
                {
                    string n = *o_args2[i];
                    char *name = new char[n.length()];
                    char *command[10][10];
                    for (int i = 0; i < n.length(); i++)
                    {
                        name[i] = n[i];
                    }

                    // cout << "Path: " << name << endl;
                    int backup_fd = dup(1);
                    int openfd = open(name, O_WRONLY | O_CREAT, 0777);
                    dup2(openfd, 1);

                    dup2(pipes[i + (i - 2)], 0);

                    // Closing unsed pipe ends
                    for (int x = 0; x < totalPipes; x++)
                        close(pipes[x]);

                    execvp(*o_args1[0], o_args1[0]);
                    perror("Wrong Command Specified!");
                    exit(1);
                }

                if (output_re == false && input_re == true)
                {
                    string n = *in_args2[i];
                    char *name = new char[n.length()];
                    char *command[10][10];
                    for (int i = 0; i < n.length(); i++)
                    {
                        name[i] = n[i];
                    }

                    // cout << "Path: " << name << endl;
                    int backup_fd = dup(0);
                    int openfd = open(name, O_RDONLY, 0777);
                    dup2(openfd, 0);
                    dup2(pipes[i + i + 1], 1);

                    execvp(*in_args1[0], in_args1[0]);
                    perror("Wrong Command Specified!");
                    exit(1);
                }

                if (input_re == false && output_re == false)
                {
                    // Let pipe "read end" go to process stdin
                    dup2(pipes[i + (i - 2)], 0);
                    // Let process stdout pass to pipe "write end"
                    dup2(pipes[i + i + 1], 1);

                    // Closing unsed pipe ends
                    for (int x = 0; x < totalPipes; x++)
                        close(pipes[x]);

                    // cout << *all_args[i] << endl;
                    //  Execute program
                    execvp(*all_args[i], all_args[i]);
                    perror("Wrong Command Specified!");
                    exit(1);
                }
            }
        }
    }

    // In parent wait untill all process finished
    // Closing unsed pipe ends
    for (int x = 0; x < totalPipes; x++)
        close(pipes[x]);

    // Lets waite untill all proccesses finshed!
    for (int i = 0; i < commandNr; i++)
        waitpid(pid[i], NULL, 0);
}

// Convert types so that is could later be passed to execv function
void convertstring(string cmnds[][MAX_AGRZ], char *all_args[][MAX_AGRZ])
{
    for (int x = 0; x < MAX_CMDS; x++)
    {
        if (cmnds[x][0] == "")
            break;

        for (int y = 0; y < MAX_AGRZ; y++)
        {
            // If empty string found, means no point of iterating further. end of arguments
            if (cmnds[x][y] == "")
            {
                all_args[x][y] = NULL;
                break;
            }

            all_args[x][y] = (char *)cmnds[x][y].c_str();
        }
    }
}

// Parsing user given input as string
int parseCommands(string parsedCommands[][MAX_AGRZ], string commandString) // 2D string , //Line
{
    // Command and its arguments numbers
    int commandNr = 0;
    int argumentNr = -1;

    // repeat for each pipe found
    for (unsigned int x = 0; x < commandString.size(); x++)
    {

        if (commandString[x] != ' ')
            argumentNr++;

        string argument = "";

        // repeat for each space found
        while (commandString[x] != ' ' && x < commandString.size())
        {
            // if bar found, means next command, break and later save as whole command
            if (commandString[x] == '|')
                break;

            argument += commandString[x];
            x++;
        }

        if (argumentNr != -1 && argument != "")
        {
            parsedCommands[commandNr][argumentNr] = argument;
            // cout<<"commandNr: "<<commandNr<<" argumentNr: "<<argumentNr<<endl;
            // cout<<parsedCommands[commandNr][argumentNr]<<endl;
        }

        if (commandString[x] == '|')
        {
            argumentNr = -1;
            commandNr++;
        }
    }

    return commandNr + 1;
}