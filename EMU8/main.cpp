#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <stdlib.h>
#include <windows.h>
#include <iostream>
#include <fstream>

#define SCREENH 32
#define SCREENW 64


/* Program entry point */

bool loadGame(const char * filename);
void display();

unsigned char memory [4096];
unsigned char reg [0xF];
unsigned char I [2];
unsigned int stimer = 0;
unsigned int timer = 0;
unsigned int pc = 0;
unsigned int sp = 0;

int main(int argc, char *argv[])
{
    glutInit(&argc, argv);
    glutInitWindowSize(640,480);
    glutInitWindowPosition(640,300);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
    glutDisplayFunc(display);
	glutIdleFunc(display);

    if(argc < 2){
        return 1;
    }
    if(!loadGame(argv[1])){
        return 1;
    }

    glutCreateWindow("EMU-8");

    for(int i = 0; i < 256; i++){
        printf("%c", memory[i + 512]);
    }

    //Sleep(3000);

    return EXIT_SUCCESS;
}


bool loadGame (const char * filename){
    FILE * pFile = fopen(filename, "rb");

    fseek(pFile , 0 , SEEK_END);
	long lSize = ftell(pFile);
	rewind(pFile);
	printf("Filesize: %d\n", (int)lSize);
    // Allocate memory to contain the whole file
	char * buffer = (char*)malloc(sizeof(char) * lSize);
	if (buffer == NULL)
	{
		fputs ("Memory error", stderr);
		return false;
	}

	// Copy the file into the buffer
	size_t result = fread (buffer, 1, lSize, pFile);
	if (result != lSize)
	{
		fputs("Reading error",stderr);
		return false;
	}

	// Copy buffer to Chip8 memory
	if((4096-512) > lSize)
	{
		for(int i = 0; i < lSize; ++i)
			memory[i + 512] = buffer[i];
	}
	else
		printf("Error: ROM too big for memory");

	// Close file, free buffer
	fclose(pFile);
	free(buffer);

	return true;
}

void display(){
    printf("dap");
}
