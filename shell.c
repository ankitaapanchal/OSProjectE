//Sivani Kommineni and Ankita Panchal

int string_to_int(char numStr);
int isCommand(char *command, char *input);
#define Buffer_size 13312

void main() {

    char input[Buffer_size];
    char buffer[Buffer_size];
    char tInput[512];
    int  sectorsRead, Length, i,j,k, b;
    char filename[7];
    char dir[512];
    
    enableInterrupts();
    while(1) {
       // char input[Buffer_size];
       // char buffer[Buffer_size];
       // char tInput[512];
       // int  sectorsRead, Length, i,j,k;
       // char filename[7];
       // char dir[512];

        for(i=0; i<512; i++) {
                input[i] = '\0';
                buffer[i] = '\0';
        }

        syscall(0, "A:>");
        syscall(1, input);

        if(isCommand("type",input)) {
            syscall(3, input+5, buffer, &sectorsRead);
            if(sectorsRead <= 0) {
                 syscall(0, "Invalid file\r\n");
                 
            }else{
            syscall(0, buffer);
            syscall(0, "\r\n");
          }
        } else if(isCommand("exec", input)) {
            syscall(3, input+5, buffer, &sectorsRead);
            if(sectorsRead<=0) syscall(0, "Invalid file\r\n");
            else {
                    syscall(4, input+5);
            }   

        } else if(isCommand("runb", input)) {
            syscall(3, input+5, buffer, &sectorsRead);
            if(sectorsRead<=0) syscall(0, "Invalid file\r\n");
            else {
                    syscall(4, input+5, &b);
            }
            syscall(10, b);
        } 
      
        else if(isCommand(input, "dir\0")) {
            // list files in the directory
            syscall(2, dir, 2);
            for (i=0; i<512; i+=32) {
                if (dir[i] == '\0') continue;
                for (j=0; j<6; j++) {
                    filename[j] = dir[i+j];
                }
                filename[6] = '\0';
                syscall(0, filename);
                syscall(0, "\r\n");
            }
        } 
        else if(isCommand("del\0", input)){
            syscall(7, input+4);
        }
        else if(isCommand("copy\0", input)) {
            // copy a file
            for(i=5; i<512; i++) {
                if(input[i] == ' ') {
                    input[i] = 0;
                    i++;
                    break;
                }
            }
            syscall(3, input+5, buffer, &sectorsRead); // filename1
            if(sectorsRead<=0) syscall(0, "file not found\r\n");

            syscall(8, buffer, input+i, sectorsRead); // filename2
        } 
        else if(isCommand("create\0", input)) {
            // create a text file
            Length = 0;
            for(i=0; i<26; i++) { // max file length is 26 sectors
                // clear line input
                for(j=0; j<512; j++) {
                    tInput[j] = '\0';
                }
                syscall(1, tInput);
                if(tInput[0]=='\0') break;
                
                
                for(j=0; j<510; j++) { 
                    if(tInput[j]=='\0') {
                        break;
                    }
                }
                tInput[j++] = '\r';
                tInput[j++] = '\n';   

                
                for(k=0; k<j; k++) {
                    buffer[Length+k] = tInput[k];
                }
                Length += j;
            }
            syscall(8, buffer, input+7, i);
        }else if(isCommand("kill\0", input)){
            
            b = string_to_int(input[5]);
            syscall(9, b );
        }
        else {
            syscall(0, "Invalid Command\r\n");
        }
    }
}
    
int isCommand(char *command, char *input) {
    while (*command != '\0' && *command != ' ' && *input != '\0' && *input != ' ') {
        if (*command != *input) {
            return 0; // false
        }
        command++;
        input++;
    }

    if(*command=='\0') return 1; // true
    else return 0;
}

int string_to_int(char numStr) {

    int num;
    if(numStr=='0') {
        num=0;
    } else if(numStr=='1') {
        num=1;
    } else if(numStr=='2') {
        num=2;
    } else if(numStr=='3') {
        num=3;
    } else if(numStr=='4') {
        num=4;
    } else if(numStr=='5') {
        num=5;
    } else if(numStr=='6') {
        num=6;
    } else if(numStr=='7') {
        num=7;
    }
    return num;
}

