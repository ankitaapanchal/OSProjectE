//Sivani Kommineni and Ankita Panchal

void printString(char* str);
void printChar(char c);
void readFile(char* fileName, char* buffer, int* bytesRead);
void readString(char* str);
void readSector(char* buffer, int sector);
void writeSector(char* buffer, int sector);
void deleteFile(char* name);
void writeFile(char* buffer, char* filename, int numberOfSectors);
void handleInterrupt21(int ax, int bx, int cx, int dx);
void handleTimerInterrupt(int segment, int sp);

void printNewLine();
void executeProgram(char* name);
void terminate();
void kill(int Process);
void waitOnProcess(int ProcessID);

int processActive[8];
int processStackPointer[8];
int processWaitingOn[8];
int currentProcess;


void main() {
    char inputLine[80];
    char sectorBuffer[512];
    char buffer[13312];   /*Maximum size of a file*/
    int sectorsRead, i, pid;
    for(i =0; i<8; i++){

        processActive[i]=0;
        processStackPointer[i]=0xff00;
        currentProcess=-1;
    }
    makeInterrupt21();
  
    // interrupt(0x21,8,"this is a test message","testmg",3);
    
    interrupt(0x21, 4, "shell",&pid,0);

    //printChar('m');
    makeTimerInterrupt();
     
    while(1);   /*hang up*/ 
}

void terminate(){

    int dataseg,i;

    dataseg = setKernelDataSegment();

    processActive[currentProcess]=0; 

    for(i=0;i<8;i++){
        
        if(processWaitingOn[i] == currentProcess) {

            processActive[i]=1;
        }
    }

    restoreDataSegment(dataseg);

    while(1);
}

void kill(int Process){

    int dataseg,i;

    dataseg= setKernelDataSegment();

    processActive[Process] = 0;

    // for(i=0;i<8;i++){
        
    //     if(processWaitingOn[i] == Process){

    //     processActive[i]=1;
    // }

    }
    restoreDataSegment(dataseg);


void waitOnProcess(int processID)
{
    int dataseg;
    dataseg= setKernelDataSegment();
    processWaitingOn[currentProcess]= processID;
    processActive[currentProcess] = 2; 
    restoreDataSegment(dataseg);
}
void readFile(char* fileName, char* buffer, int* bytesRead) {
    char directory[512];
    
    int fileEntrySize=32;  // Each file entry in the directory is 32 bytes
    int directorySize = 512;  // The directory sector is 512 bytes
    int fileEntry =0;
    int found = 0;
    int i, sector;
    readSector(directory, 2);  // Read the directory sector 

    for (fileEntry = 0; fileEntry < directorySize; fileEntry += fileEntrySize) {
        for (i = 0; i < 6; i++) {
            if (fileName[i] != directory[fileEntry + i]) {
                break;
            }
        }
        if (i == 6) {
            found = 1;
            break;  // File name matched, break out of the loop
        }
    }

    if (!found) {
        *bytesRead = 0;  // File not found, set the number of sectors read to 0
        return;
    }

    // File found, start loading the file sector by sector
    i=6;
    sector = directory[fileEntry + i];
    *bytesRead = 0;
    while (sector != 0) {
        readSector(buffer, sector);  // Read the sector into the buffer
        buffer = buffer + 512;  // Increment the buffer address
        i=i+1;
        sector = directory[fileEntry + i];  // Move to the next sector
        (*bytesRead)++;
    }
}


void printString(char* str) {
    while (*str != '\0') {
        printChar(*str);
        str += 0x1;
    }
}

void printChar(char c) {
    int ah, al, ax;
    ah = 0xe;
    al = c;
    ax = (ah * 256) + al;
    interrupt(0x10, ax, 0, 0, 0);
}

void deleteFile(char* filename){
    char map[512];
    char dir[512];
    int fileEntry, i, found,s; 

    readSector(map,1);
    readSector(dir,2);
    
    for (fileEntry = 0; fileEntry < 512; fileEntry += 32) {
        found=1;
        for (i = 0; i < 6; i++) {
            if (filename[i] != dir[fileEntry + i]) { 
                found=0;
            break;
            }
         }
         if (found) {
             
             break;  // File name matched, break out of the loop
         }
     }

     if (!found) {
         return;
     }
     
     dir[fileEntry]=0;
     writeSector(dir,2);
     
     for(i=0;i<26;i++){
         s = dir[fileEntry+6+i];
         if(s == 0) break;
         map[s]=0;
         }
     writeSector(map,1);
}

void readString(char* str) {
    char* start;
    char c;

    while (1) {
        c = interrupt(0x16, 0, 0, 0, 0);
        if (c == 0xd) {
            break;
        }

        if (c == 0x8) {
            if (str != start) {
                printChar(0x8);
                printChar(0x20);
                printChar(0x8);
                str -= 0x1;
                *str = 0x20;
            }
        } else {
            printChar(c);
            *str = c;
            str += 0x1;
        }
    }
    printChar(c);
    printChar(0xa);

    *str = 0x0;
}

void readSector(char* buffer, int sector) {
    int ah, al, bx, ch, cl, dh, dl, ax, cx, dx;
    ah = 2;
    al = 1;
    bx = buffer;
    ch = 0; // track number
    cl = sector + 1; // relative sector number
    dh = 0; // head number
    dl = 0x80;

    ax = (ah * 256) + al;
    cx = (ch * 256) + cl;
    dx = (dh * 256) + dl;
    buffer = interrupt(0x13, ax, bx, cx, dx);
}

void writeSector(char* buffer, int sector) {
    int ah, al, bx, ch, cl, dh, dl, ax, cx, dx;
    ah = 3;
    al = 1;
    bx = buffer;
    ch = 0; // track number
    cl = sector + 1; // relative sector number
    dh = 0; // head number
    dl = 0x80;

    ax = (ah * 256) + al;
    cx = (ch * 256) + cl;
    dx = (dh * 256) + dl;
    buffer = interrupt(0x13, ax, bx, cx, dx);
}

void writeFile(char* buffer, char* filename, int numberOfSectors){
    char map[512];
    char dir[512];
    char BufferS[512];
    int Empty=0, s, i, j ,fileEntry=0;

    readSector(map, 1);
    readSector(dir, 2);

    for(fileEntry=0; fileEntry<512; fileEntry+=32){
        if(dir[fileEntry]==0){
            Empty=1;
            break;
        }
    }

    if(!Empty){
        return;
    }

    for(i=0;i<6;i++){
        if(filename[i]==0){
            for(j=1;j<6;j++){
                dir[fileEntry+j] =0;
            }
            break;

        }
        dir[fileEntry+i] = filename[i];
    }
    s =0;
    for(i=3; i<512; i++) {
        if(map[i] == 0) {
            // add sector to map and dir
            map[i] = 0xFF;
            dir[fileEntry+6+s]=i;
            
            // write 512 bytes from buffer into the sector
            for(j=0; j<512; j++) {
                BufferS[j] = buffer[(s*512)+j];
            }
            writeSector(BufferS, i);

            s++;
            if(s >= numberOfSectors) break;
        }
    }

    // fill remaining bytes in directory entry to 0
    for(i=numberOfSectors; i<26; i++) {
        dir[fileEntry+6+i] = 0;
    }

    writeSector(map, 1);
    writeSector(dir, 2);

}
void handleInterrupt21(int ax, int bx, int cx, int dx) {
    if (ax == 0) {
        printString(bx);
    } else if (ax == 1) {
        readString(bx);
    } else if (ax == 2) {
        readSector(bx, cx);
    } else if (ax == 3) {
        readFile(bx,cx,dx );
    }else if(ax == 4){
        executeProgram(bx,cx);        
    }else if (ax == 5){
        terminate();
    }else if(ax == 6){
        writeSector(bx, cx);
    }else if(ax == 7){
        deleteFile(bx);
    }else if(ax == 8){
        writeFile(bx,cx,dx);
    }else if(ax == 9){
        kill(bx);
    }else if(ax == 10){
        waitOnProcess(bx);
    }
    else{
        printString("Invalid ax");
    }
}


void executeProgram(char* name,int* processID) {
    int dataseg, a,j, index;
    char buffer[512 * 10];  
    int segment = 0x2000;
    int offset = 0;

    //Call readFile to load the file into a buffer
    int sectorsRead,i;
    readFile(name, buffer, &sectorsRead);

    if (sectorsRead == 0) {
        printString("Program not found.\n");
        return;
    }



    for(i=0;i<8;i++){

        dataseg = setKernelDataSegment();

        if(processActive[i]==0){
           a = (i + 2) * 0x1000;
           index = i;
        }

        restoreDataSegment(dataseg);
    }
//    printChar(index+'0');

     for (j = 0; j < 512 * sectorsRead; ++j) {
        putInMemory(a, offset, buffer[j]);
        ++offset;
/*        if (offset == 0x1000) {
            
            offset = 0;
            ++segment;
        }*/
    }
  
    initializeProgram(a);
    dataseg = setKernelDataSegment();

    processActive[index] = 1;
    processStackPointer[index] = 0xff00;
  //  printChar(buffer[0]);
    
    restoreDataSegment(dataseg);
    
   *processID=index;
}

void handleTimerInterrupt(int segment, int sp){
    int  dataseg, i;


    // printChar('T');
    // printChar('i');
    // printChar('c');

    dataseg = setKernelDataSegment();

    for(i=0; i<8; i++)
        {
                putInMemory(0xb800,60*2+i*4,i+0x30);
                if(processActive[i]==1)
                        putInMemory(0xb800,60*2+i*4+1,0x20);
                else
                        putInMemory(0xb800,60*2+i*4+1,0);
        }
        //printChar('h');

        if(currentProcess>=0){
    processStackPointer[currentProcess]=sp;
    } while(1){

        currentProcess++;
        if(currentProcess==8){
            currentProcess=0;
        }
        //printChar(currentProcess+'0');
        if(processActive[currentProcess]==1){
            break;
        }

    }
    segment=(currentProcess+2)*0x1000;
    sp = processStackPointer[currentProcess];
// printChar('t');
// printChar(currentProcess+'0');
// while(1);
    restoreDataSegment(dataseg);



    returnFromTimer(segment, sp);
}
