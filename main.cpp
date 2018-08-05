#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <stdlib.h>
#include <windows.h>
#include <iostream>
#include <fstream>
#include <time.h>

#define SCREENH 32
#define SCREENW 64
#define DRAWWITHTEXTURE


/* Program entry point */

bool loadGame(const char * filename);
void display();
void emulateCycle();
unsigned short popStack();
void pushStack (unsigned short data);
void setupTexture();
void updateTexture();
void updateQuads();
void drawPixel(int x, int y);
void keyboardDown(unsigned char key, int x, int y);
void keyboardUp(unsigned char key, int x, int y);

unsigned char memory [4096];
unsigned short stacc [16];
unsigned short Vreg [16];
unsigned short I;
unsigned int stimer = 0;
int dtimer = 0;
unsigned short pc = 0x200;
unsigned int sp = 0;
unsigned __int8 screenData[SCREENH][SCREENW][3];
unsigned char gfx [2048]; //total number of pixels
unsigned char keyA [16];
bool drawFlag = false;

unsigned char chip8_fontset[80] =
{
    0xF0, 0x90, 0x90, 0x90, 0xF0, //0
    0x20, 0x60, 0x20, 0x20, 0x70, //1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, //2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, //3
    0x90, 0x90, 0xF0, 0x10, 0x10, //4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, //5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, //6
    0xF0, 0x10, 0x20, 0x40, 0x40, //7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, //8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, //9
    0xF0, 0x90, 0xF0, 0x90, 0x90, //A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, //B
    0xF0, 0x80, 0x80, 0x80, 0xF0, //C
    0xE0, 0x90, 0x90, 0x90, 0xE0, //D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, //E
    0xF0, 0x80, 0xF0, 0x80, 0x80  //F
};


int main(int argc, char *argv[])
{
    srand(time(0));
    if(argc < 2){
        return 1;
    }
    if(!loadGame(argv[1])){
        return 1;
    }

    //for(int i = 0; i < 256; i++){
        //printf("%c", memory[i + 512]);
    //}

    for(int i = 0; i < 2048; i++){
        //if(i < 809){
            gfx[i] = 0;
        //}
       // else{gfx[i] = 1;}
    }

    drawFlag = true;

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(SCREENW * 10,SCREENH * 10);
    glutInitWindowPosition(640,300);
    glutCreateWindow("EMU-8");
    glutDisplayFunc(display);
    glutIdleFunc(display);
    glutKeyboardFunc(keyboardDown);
	glutKeyboardUpFunc(keyboardUp);
    //glutOverlayDisplayFunc(display);
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    gluOrtho2D( 0.0, 640.0, 320.0,0.0 );

    setupTexture();

    glutMainLoop();

    Sleep(300000);

    return EXIT_SUCCESS;
}


bool loadGame (const char * filename){
    FILE * pFile = fopen(filename, "rb");

    fseek(pFile , 0 , SEEK_END);
	unsigned long lSize = ftell(pFile);
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
		for(unsigned int i = 0; i < lSize; ++i)
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
    emulateCycle();
    if(drawFlag){
        glClear(GL_COLOR_BUFFER_BIT);
#ifdef DRAWWITHTEXTURE
        updateTexture();
#else
        updateQuads();
#endif
        glutSwapBuffers();
        drawFlag = false;
    }
}


void setupTexture()
{
	// Clear screen
	for(int y = 0; y < SCREENH; y++)
		for(int x = 0; x < SCREENW; x++)
			screenData[y][x][0] = screenData[y][x][1] = screenData[y][x][2] = 150;

	// Create a texture
	glTexImage2D(GL_TEXTURE_2D, 0, 3, SCREENW, SCREENH, 0, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*)screenData);

	// Set up the texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	// Enable textures
	glEnable(GL_TEXTURE_2D);
}

void updateTexture()
{
	// Update pixels
	for(int y = 0; y < 32; y++)
		for(int x = 0; x < 64; x++)
			if(gfx[(y * 64) + x] == 0)
				screenData[y][x][0] = screenData[y][x][1] = screenData[y][x][2] = 0;	// Disabled
			else
				screenData[y][x][0] = screenData[y][x][1] = screenData[y][x][2] = 255;  // Enabled

	// Update Texture
	glTexSubImage2D(GL_TEXTURE_2D, 0 ,0, 0, SCREENW, SCREENH, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*)screenData);

	glBegin( GL_QUADS );
		glTexCoord2d(0.0, 0.0);		glVertex2d(0.0,			    0.0);
		glTexCoord2d(1.0, 0.0); 	glVertex2d((SCREENW * 10),  0.0);
		glTexCoord2d(1.0, 1.0); 	glVertex2d((SCREENW * 10),  (SCREENH * 10));
		glTexCoord2d(0.0, 1.0); 	glVertex2d(0.0,			    (SCREENH * 10));
	glEnd();
}

void drawPixel(int x, int y)
{
	glBegin(GL_QUADS);
		glVertex3f((x * 10) + 0.0f,     (y * 10) + 0.0f,	 0.0f);
		glVertex3f((x * 10) + 0.0f,     (y * 10) + 10,       0.0f);
		glVertex3f((x * 10) + 10,       (y * 10) + 10,       0.0f);
		glVertex3f((x * 10) + 10,       (y * 10) + 0.0f,	 0.0f);
	glEnd();
}

void updateQuads()
{
	// Draw
	for(int y = 0; y < 32; ++y)
		for(int x = 0; x < 64; ++x)
		{
			if(gfx[(y*64) + x] == 0)
				glColor3f(0.0f,0.0f,0.0f);
			else
				glColor3f(1.0f,1.0f,1.0f);

			drawPixel(x, y);
		}
}


void emulateCycle(){
        unsigned int instruction = memory[pc] << 8 | memory[pc + 1];
        unsigned int opcode = instruction >> 12;
        dtimer--;
        if(dtimer < 0){
            dtimer = 0;
        }
        //std::cout << Vreg[0] << std::endl;
        //std::cout << instruction << " | " << opcode << std::endl;
        switch(opcode){
            case 0x00:  if((instruction & 0x00FF) == 0x00E0){ //std::cout << std::hex << opcode << " | " << instruction << " | CLS\n";
            				pc += 2;
            				break;
            			}
                        if((instruction & 0x00FF) == 0x00EE){ //std::cout << std::hex << opcode << " | " << instruction << " | RET\n";
                        	pc = popStack() + 2;
                        	sp--;
                        	break;
                        }
            case 0x01:  std::cout << std::hex << opcode << " | " << instruction << " | JP\n";
                        pc = (instruction & 0x0FFF);
                        //std::cout << pc << "\n";
                        break;
            case 0x02:  //std::cout << std::hex << opcode << " | " << instruction << " | CALL\n";
                        pushStack(pc);
                        pc = (instruction & 0x0FFF);
                        break;
            case 0x03:  std::cout << std::hex << opcode << " | " << instruction << " | SE [skip if Vx = byte] (" << Vreg[0xF] << ") " << (instruction&0x00FF) << "\n";
                        if(Vreg[((instruction & 0x0F00) >> 8)] == (instruction & 0x00FF)){
                        	pc += 2;
                        }
                        pc += 2;
                        break;
            case 0x04:  //std::cout << std::hex << opcode << " | " << instruction << " | SNE\n";
                        if(Vreg[((instruction & 0x0F00) >> 8)] != (instruction & 0x00FF)){
                        	pc+=2;
                        }
                        pc += 2;
                        break;
            case 0x05:  //std::cout << std::hex << opcode << " | " << instruction << " | SE [skip if Vx = Vy]\n";
                        if(Vreg[((instruction & 0x0F00) >> 8)] == Vreg[((instruction & 0x00F0) >> 8)]){
                        	pc+=2;
                        }
                        pc += 2;
                        break;
            case 0x06:  //std::cout << std::hex << opcode << " | " << instruction << " | LD [Vx, byte]\n";
                        Vreg[(instruction & 0x0F00) >> 8] = (instruction & 0x00FF);
                        /*std::cout << "V" << ((instruction & 0x0F00) >> 8) << ": " << Vreg[(instruction & 0x0F00) >> 8] << "\n";*/ pc += 2;
                        break;
            case 0x07:  //std::cout << std::hex << opcode << " | " << instruction << " | ADD [Vx, byte]\n";
                        Vreg[(instruction & 0x0F00) >> 8] += (instruction & 0x00FF); pc += 2;
                        break;
            case 0x08:  if((instruction & 0x000F) == 0x0000){
            				//std::cout << std::hex << opcode << " | " << instruction << " | LD [Vx = Vy]\n";
                            Vreg[((instruction & 0x0F00) >> 8)] = Vreg[((instruction & 0x00F0) >> 8)];
                            pc += 2;
                            break;
                        }
                        if((instruction & 0x000F) == 0x0001){
                        	//std::cout << std::hex << opcode << " | " << instruction << " | OR [Vx | Vy]\n";
                            Vreg[((instruction & 0x0F00) >> 8)] = Vreg[((instruction & 0x0F00) >> 8)] | Vreg[((instruction & 0x00F0) >> 4)];
                            pc += 2;
                            break;
                        }
                        if((instruction & 0x000F) == 0x0002){
                        	//std::cout << std::hex << opcode << " | " << instruction << " | AND [Vx & Vy]\n";
                            Vreg[((instruction & 0x0F00) >> 8)] = Vreg[((instruction & 0x0F00) >> 8)] & Vreg[((instruction & 0x00F0) >> 4)];
                            pc += 2;
                            break;
                        }
                        if((instruction & 0x000F) == 0x0003){
                        	//std::cout << std::hex << opcode << " | " << instruction << " | XOR [Vx, Vy]\n";
                            Vreg[((instruction & 0x0F00) >> 8)] = Vreg[((instruction & 0x0F00) >> 8)] ^ Vreg[((instruction & 0x00F0) >> 4)];
                            pc += 2;
                            break;
                        }
                        if((instruction & 0x000F) == 0x0004){
                        	//std::cout << std::hex << opcode << " | " << instruction << " | ADD [Vx + Vy]\n";
                            Vreg[((instruction & 0x0F00) >> 8)] = Vreg[((instruction & 0x0F00) >> 8)] + Vreg[((instruction & 0x00F0) >> 4)];
                            pc += 2;
                            break;
                        } //Vf CARRY
                        if((instruction & 0x000F) == 0x0005){
                        	//std::cout << std::hex << opcode << " | " << instruction << " | SUB [Vx - Vy]\n";
                            Vreg[((instruction & 0x0F00) >> 8)] = Vreg[((instruction & 0x0F00) >> 8)] - Vreg[((instruction & 0x00F0) >> 4)];
                            pc += 2;
                            break;
                        } //Vf NOT BORROW
                        if((instruction & 0x000F) == 0x0006){
                        	//std::cout << std::hex << opcode << " | " << instruction << " | SHR [Vx >> 1]\n";
                            Vreg[((instruction & 0x0F00) >> 8)] = Vreg[((instruction & 0x0F00) >> 8)] >> 1;
                            pc += 2;
                            break;
                        }
                        if((instruction & 0x000F) == 0x0007){
                        	//std::cout << std::hex << opcode << " | " << instruction << " | SUBN [Vx = Vx - Vy, Vf NOT BORROW]\n";
                            Vreg[((instruction & 0x0F00) >> 8)] = Vreg[((instruction & 0x0F00) >> 8)] - Vreg[((instruction & 0x00F0) >> 4)];
                            pc += 2;
                            break;
                        } //Vf NOT BORROW
                        if((instruction & 0x000F) == 0x000E){
                        	//std::cout << std::hex << opcode << " | " << instruction << " | SHL [Vx << 1]\n";
                        	Vreg[((instruction & 0x0F00) >> 8)] = Vreg[((instruction & 0x0F00) >> 8)] << 1;
                        	pc += 2;
                        	break;
                    	}
            case 0x09:  //std::cout << std::hex << opcode << " | " << instruction << " | SNE [skip if Vx != Vy]\n";
                        if(Vreg[((instruction & 0x0F00) >> 8)] != Vreg[((instruction & 0x00F0) >> 4)]){
                           pc += 2;
                        }
            			pc += 2;
                        break;
            case 0x0A:  //std::cout << std::hex << opcode << " | " << instruction << " | LD I\n";
                        I = (instruction & 0x0FFF);
            			pc += 2;
                        break;
            case 0x0B:  //std::cout << std::hex << opcode << " | " << instruction << " | JP [V0 + nnn]\n";
            			pc = (instruction & 0x0FFF) + Vreg[0];
                        break;
            case 0x0C:  //std::cout << std::hex << opcode << " | " << instruction << " | RND\n";
                        Vreg[((instruction % 0x0F00) >> 8)] = ((rand()%0xFF) & (instruction & 0x0FF));
            			pc += 2;
                        break;
            case 0x0D:  //std::cout << std::hex << opcode << " | " << instruction << " | DRW [Vx, Vy, n]\n"; //VF = collision
                        {
                        unsigned short x = Vreg[(instruction & 0x0F00) >> 8];
                        unsigned short y = Vreg[(instruction & 0x00F0) >> 4];
                        unsigned short height = instruction & 0x000F;
                        unsigned short pixel;
                        Vreg[0xF] = 0;
                        for (int yline = 0; yline < height; yline++)
                        {
                            pixel = memory[I + yline];
                            //printf(" %d , mem[I+yl]: %d\n", I, memory[I + yline]);
                            for(int xline = 0; xline < 8; xline++)
                            {
                                //printf("/ p:%d - x:%d - &:%d", pixel, (0x80 >> xline), (pixel & (0x80 >> xline)));
                                if((pixel & (0x80 >> xline)) != 0)
                                {
                                    if(gfx[(x + xline + ((y + yline) * 64))] == 1)
                                    {
                                        //printf("%d ", Vreg[0xF]);
                                        Vreg[0xF] = 1;
                                        //printf("%d\n", Vreg[0xF]);
                                    }
                                    gfx[x + xline + ((y + yline) * 64)] ^= 1;
                                }
                                //gfx[x + xline + ((y + yline) * 64)] ^= 1;
                            }
                        }
                        drawFlag = true;
                        pc += 2;
                        }
                        break;
            case 0x0E:  if((instruction & 0x00FF) == 0x009E){
            				//std::cout << std::hex << opcode << " | " << instruction << " | SKP if key with val Vx is pressed\n";
            				if(keyA[Vreg[(instruction & 0x0F00) >> 8]] != 0){
                                pc += 2;
            				}
            				pc += 2;
            				break;
            			}
                        if((instruction & 0x00FF) == 0x00A1){
                        	//std::cout << std::hex << opcode << " | " << instruction << " | SKNP if key with val !Vx is pressed\n";
                        	if(keyA[Vreg[(instruction & 0x0F00) >> 8]] == 0){
                                pc += 2;
                            }
                        	pc += 2;
                        	break;
                        }
            case 0x0F:  if((instruction & 0x00FF) == 0x0007){
            				//std::cout << std::hex << opcode << " | " << instruction << " | LD [Vx = Dt]\n";
            				Vreg[((instruction & 0x0F00) >> 8)] = dtimer;
            				pc += 2;
            				break;
            			}
                        if((instruction & 0x00FF) == 0x000A){
                        	//std::cout << std::hex << opcode << " | " << instruction << " | LD [Vx, Key]\n";
                        	bool keyPress = false;
                            for(int i = 0; i < 16; ++i)
                            {
                                if(keyA[i] != 0)
                                {
                                    Vreg[(instruction & 0x0F00) >> 8] = i;
                                    keyPress = true;
                                }
                            }
                            // If we didn't received a keypress, skip this cycle and try again.
                            if(!keyPress){
                                return;
                            }
                        	pc += 2;
                        	break;
                        }
                        if((instruction & 0x00FF) == 0x0015){
                        	//std::cout << std::hex << opcode << " | " << instruction << " | LD [Dt = Vx]\n";
                        	dtimer = Vreg[((instruction & 0x0F00) >> 8)];
                        	pc += 2;
                        	break;
                        }
                        if((instruction & 0x00FF) == 0x0018){
                        	//std::cout << std::hex << opcode << " | " << instruction << " | LD [St = Vx]\n";
                        	stimer = Vreg[((instruction & 0x0F00) >> 8)];
                        	pc += 2;
                        	break;
                        }
                        if((instruction & 0x00FF) == 0x001E){
                        	//std::cout << std::hex << opcode << " | " << instruction << " | ADD [I += Vx]\n";
                        	if(I + Vreg[(instruction & 0x0F00) >> 8] > 0xFFF){	// VF is set to 1 when range overflow (I+VX>0xFFF), and 0 when there isn't.
                                Vreg[0xF] = 1;
                        	}
                            else{
                                Vreg[0xF] = 0;
                            }
                            I += Vreg[(instruction & 0x0F00) >> 8];
                        	pc += 2;
                        	break;
                        }
                        if((instruction & 0x00FF) == 0x0029){
                        	//std::cout << std::hex << opcode << " | " << instruction << " | LD [F, Vx]\n";
                        	I = Vreg[(instruction & 0x0F00) >> 8] * 0x5;
                        	pc += 2;
                        	break;
                        }
                        if((instruction & 0x00FF) == 0x0033){
                        	//std::cout << std::hex << opcode << " | " << instruction << " | LD [B, Vx]\n";
                        	memory[I]     = Vreg[(instruction & 0x0F00) >> 8] / 100;
                            memory[I + 1] = (Vreg[(instruction & 0x0F00) >> 8] / 10) % 10;
                            memory[I + 2] = (Vreg[(instruction & 0x0F00) >> 8] % 100) % 10;
                        	pc += 2;
                        	break;
                        }
                        if((instruction & 0x00FF) == 0x0055){
                        	//std::cout << std::hex << opcode << " | " << instruction << " | LD [[I], Vx]\n";
                        	for (int i = 0; i <= ((instruction & 0x0F00) >> 8); ++i)
                            memory[I + i] = Vreg[i];

                            // On the original interpreter, when the operation is done, I = I + X + 1.
                            I += ((instruction & 0x0F00) >> 8) + 1;
                        	pc += 2;
                        	break;
                        }
                        if((instruction & 0x00FF) == 0x0065){
                        	//std::cout << std::hex << opcode << " | " << instruction << " | LD [Vx, [I]]\n";
                        	for (int i = 0; i <= ((instruction & 0x0F00) >> 8); ++i)
                            Vreg[i] = memory[I + i];
                            I += ((instruction & 0x0F00) >> 8) + 1;
                        	pc += 2;
                        	break;
                        }

            default:
                std::cout << "unrecognized instruction\n";pc += 2;
                break;

        }
        //std::cout << "SP: " << sp << " | stack[sp]: " << stacc[sp] << " | PC:" << pc << std::endl;



}


unsigned short popStack(){
    return stacc[sp];
}

void pushStack (unsigned short data){
    sp++;
    //std::cout << data << std::endl;
    stacc[sp] = data;
}

void keyboardDown(unsigned char key, int x, int y)
{
	if(key == 27)    // esc
		exit(0);

	if(key == '1')		keyA[0x1] = 1;
	else if(key == '2')	keyA[0x2] = 1;
	else if(key == '3')	keyA[0x3] = 1;
	else if(key == '4')	keyA[0xC] = 1;

	else if(key == 'q')	keyA[0x4] = 1;
	else if(key == 'w')	keyA[0x5] = 1;
	else if(key == 'e')	keyA[0x6] = 1;
	else if(key == 'r')	keyA[0xD] = 1;

	else if(key == 'a')	keyA[0x7] = 1;
	else if(key == 's')	keyA[0x8] = 1;
	else if(key == 'd')	keyA[0x9] = 1;
	else if(key == 'f')	keyA[0xE] = 1;

	else if(key == 'z')	keyA[0xA] = 1;
	else if(key == 'x')	keyA[0x0] = 1;
	else if(key == 'c')	keyA[0xB] = 1;
	else if(key == 'v')	keyA[0xF] = 1;

	//printf("Press key %c\n", key);
}

void keyboardUp(unsigned char key, int x, int y)
{
	if(key == '1')		keyA[0x1] = 0;
	else if(key == '2')	keyA[0x2] = 0;
	else if(key == '3')	keyA[0x3] = 0;
	else if(key == '4')	keyA[0xC] = 0;

	else if(key == 'q')	keyA[0x4] = 0;
	else if(key == 'w')	keyA[0x5] = 0;
	else if(key == 'e')	keyA[0x6] = 0;
	else if(key == 'r')	keyA[0xD] = 0;

	else if(key == 'a') keyA[0x7] = 0;
	else if(key == 's')	keyA[0x8] = 0;
	else if(key == 'd')	keyA[0x9] = 0;
	else if(key == 'f')	keyA[0xE] = 0;

	else if(key == 'z')	keyA[0xA] = 0;
	else if(key == 'x')	keyA[0x0] = 0;
	else if(key == 'c')	keyA[0xB] = 0;
	else if(key == 'v')	keyA[0xF] = 0;
}
