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
void emulateCycle();
unsigned short popStack();
void pushStack (unsigned short data);

unsigned char memory [4096];
unsigned short stacc [16];
unsigned short Vreg [0xF];
unsigned char I [2];
unsigned int stimer = 0;
unsigned int timer = 0;
unsigned short pc = 0x200;
unsigned int sp = 0;

int tempCC = 0; //temporary Cycle Counter to imitate a constant refresh

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

    emulateCycle();
    Sleep(300000);

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



void emulateCycle(){
    do{
        unsigned int instruction = memory[pc] << 8 | memory[pc + 1];
        unsigned int opcode = instruction >> 12;
        //std::cout << instruction << " | " << opcode << std::endl;
        switch(opcode){
            case 0x00:  if((instruction & 0x00FF) == 0x00E0){ std::cout << std::hex << opcode << " | " << instruction << " | CLS\n";pc += 2;break;}
                        if((instruction & 0x00FF) == 0x00EE){ std::cout << std::hex << opcode << " | " << instruction << " | RET\n";pc = popStack() + 2;sp--;break;}
            case 0x01:  std::cout << std::hex << opcode << " | " << instruction << " | JP\n";
                        pc = (instruction & 0x0FFF);
                        std::cout << pc << "\n";
                        break;
            case 0x02:  std::cout << std::hex << opcode << " | " << instruction << " | CALL\n";
                        pushStack(pc);
                        pc = (instruction & 0x0FFF);
                        break;
            case 0x03:  std::cout << std::hex << opcode << " | " << instruction << " | SE [skip if Vx = byte]\n";
                        if(Vreg[(instruction & 0x0F00) >> 8] == (instruction & 0x00FF)){pc += 4;}
                        if(Vreg[(instruction & 0x0F00) >> 8] != (instruction & 0x00FF)){pc += 2;}
                        break;
            case 0x04:  std::cout << std::hex << opcode << " | " << instruction << " | SNE\n";
                        if(Vreg[((instruction & 0x0F00) >> 8)] == Vreg[((instruction & 0x00F0) >> 8)]){pc+=2;}
                        pc += 2;
                        break;
            case 0x05:  std::cout << std::hex << opcode << " | " << instruction << " | SE [skip if Vx = Vy]\n";
                        if(Vreg[((instruction & 0x0F00) >> 8)] == Vreg[((instruction & 0x00F0) >> 8)]){pc+=2;}
                        pc += 2;
                        break;
            case 0x06:  std::cout << std::hex << opcode << " | " << instruction << " | LD [Vx, byte]\n";
                        Vreg[(instruction & 0x0F00) >> 8] = (instruction & 0x00FF);
                        /*std::cout << "V" << ((instruction & 0x0F00) >> 8) << ": " << Vreg[(instruction & 0x0F00) >> 8] << "\n";*/ pc += 2;
                        break;
            case 0x07:  std::cout << std::hex << opcode << " | " << instruction << " | ADD [Vx, byte]\n";
                        Vreg[(instruction & 0x0F00) >> 8] += (instruction & 0x00FF); pc += 2;
                        break;
            case 0x08:  if((instruction & 0x000F) == 0x0000){ std::cout << std::hex << opcode << " | " << instruction << " | LD [Vx = Vy]\n";
                            Vreg[((instruction & 0x0F00) >> 8)] = Vreg[((instruction & 0x00F0) >> 8)];pc += 2;break;}

                        if((instruction & 0x000F) == 0x0001){ std::cout << std::hex << opcode << " | " << instruction << " | OR [Vx | Vy]\n";
                            Vreg[((instruction & 0x0F00) >> 8)] = Vreg[((instruction & 0x0F00) >> 8)] | Vreg[((instruction & 0x00F0) >> 4)];pc += 2; break;}

                        if((instruction & 0x000F) == 0x0002){ std::cout << std::hex << opcode << " | " << instruction << " | AND [Vx & Vy]\n";
                            Vreg[((instruction & 0x0F00) >> 8)] = Vreg[((instruction & 0x0F00) >> 8)] & Vreg[((instruction & 0x00F0) >> 4)];pc += 2; break;}

                        if((instruction & 0x000F) == 0x0003){ std::cout << std::hex << opcode << " | " << instruction << " | XOR [Vx, Vy]\n";
                            Vreg[((instruction & 0x0F00) >> 8)] = Vreg[((instruction & 0x0F00) >> 8)] ^ Vreg[((instruction & 0x00F0) >> 4)];pc += 2; break;}

                        if((instruction & 0x000F) == 0x0004){ std::cout << std::hex << opcode << " | " << instruction << " | ADD [Vx + Vy]\n";
                            Vreg[((instruction & 0x0F00) >> 8)] = Vreg[((instruction & 0x0F00) >> 8)] + Vreg[((instruction & 0x00F0) >> 4)];pc += 2; break;} //Vf CARRY

                        if((instruction & 0x000F) == 0x0005){ std::cout << std::hex << opcode << " | " << instruction << " | SUB [Vx - Vy]\n";
                            Vreg[((instruction & 0x0F00) >> 8)] = Vreg[((instruction & 0x0F00) >> 8)] - Vreg[((instruction & 0x00F0) >> 4)];pc += 2; break;} //Vf NOT BORROW

                        if((instruction & 0x000F) == 0x0006){ std::cout << std::hex << opcode << " | " << instruction << " | SHR [Vx >> 1]\n";
                            Vreg[((instruction & 0x0F00) >> 8)] = Vreg[((instruction & 0x0F00) >> 8)] >> 1;pc += 2; break;}

                        if((instruction & 0x000F) == 0x0007){ std::cout << std::hex << opcode << " | " << instruction << " | SUBN [Vx = Vx - Vy, Vf NOT BORROW]\n";
                            Vreg[((instruction & 0x0F00) >> 8)] = Vreg[((instruction & 0x0F00) >> 8)] - Vreg[((instruction & 0x00F0) >> 4)];pc += 2; break;} //Vf NOT BORROW

                        if((instruction & 0x000F) == 0x000E){ std::cout << std::hex << opcode << " | " << instruction << " | SHL [Vx << 1]\n";pc += 2;break;}
            case 0x09:  std::cout << std::hex << opcode << " | " << instruction << " | SNE [skip if Vx != Vy]\n";pc += 2;
                        break;
            case 0x0A:  std::cout << std::hex << opcode << " | " << instruction << " | LD I\n";pc += 2;
                        break;
            case 0x0B:  std::cout << std::hex << opcode << " | " << instruction << " | JP [V0 + nnn]\n";pc += 2;
                        break;
            case 0x0C:  std::cout << std::hex << opcode << " | " << instruction << " | RND\n";pc += 2;
                        break;
            case 0x0D:  std::cout << std::hex << opcode << " | " << instruction << " | DRW [Vx, Vy, n]\n";pc += 2;
                        break;
            case 0x0E:  if((instruction & 0x00FF) == 0x009E){ std::cout << std::hex << opcode << " | " << instruction << " | SKP if key with val Vx is pressed\n";pc += 2;break;}
                        if((instruction & 0x00FF) == 0x00A1){ std::cout << std::hex << opcode << " | " << instruction << " | SKNP if key with val !Vx is pressed\n";pc += 2;break;}
            case 0x0F:  if((instruction & 0x00FF) == 0x0007){ std::cout << std::hex << opcode << " | " << instruction << " | LD [Vx = Dt]\n";pc += 2;break;}
                        if((instruction & 0x00FF) == 0x000A){ std::cout << std::hex << opcode << " | " << instruction << " | LD [Vx, Key]\n";pc += 2;break;}
                        if((instruction & 0x00FF) == 0x0015){ std::cout << std::hex << opcode << " | " << instruction << " | LD [Dt = Vx]\n";pc += 2;break;}
                        if((instruction & 0x00FF) == 0x0018){ std::cout << std::hex << opcode << " | " << instruction << " | LD [St = Vx]\n";pc += 2;break;}
                        if((instruction & 0x00FF) == 0x001E){ std::cout << std::hex << opcode << " | " << instruction << " | ADD [I += Vx]\n";pc += 2;break;}
                        if((instruction & 0x00FF) == 0x0029){ std::cout << std::hex << opcode << " | " << instruction << " | LD [F, Vx]\n";pc += 2;break;}
                        if((instruction & 0x00FF) == 0x0033){ std::cout << std::hex << opcode << " | " << instruction << " | LD [B, Vx]\n";pc += 2;break;}
                        if((instruction & 0x00FF) == 0x0055){ std::cout << std::hex << opcode << " | " << instruction << " | LD [[I], Vx]\n";pc += 2;break;}
                        if((instruction & 0x00FF) == 0x0065){ std::cout << std::hex << opcode << " | " << instruction << " | LD [Vx, [I]]\n";pc += 2;break;}

            default:
                std::cout << "unrecognized instruction\n";pc += 2;
                break;

        }
        tempCC++;
        //std::cout << "SP: " << sp << " | stack[sp]: " << stacc[sp] << " | PC:" << pc << std::endl;
    } while (tempCC < 128);



}


unsigned short popStack(){
    return stacc[sp];
}

void pushStack (unsigned short data){
    sp++;
    std::cout << data << std::endl;
    stacc[sp] = data;
}
