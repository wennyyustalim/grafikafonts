/*
To test that the Linux framebuffer is set up correctly, and that the device permissions
are correct, use the program below which opens the frame buffer and draws a gradient-
filled red square:

retrieved from:
Testing the Linux Framebuffer for Qtopia Core (qt4-x11-4.2.2)

http://cep.xor.aps.anl.gov/software/qt4-x11-4.2.2/qtopiacore-testingframebuffer.html
*/

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

int read_line(FILE *in, char *buffer, size_t max)
{
  return fgets(buffer, max, in) == buffer;
}

int main()
{
    // Alphabet container
    char alphabets[26][33][100];

    // Read alphabet A
	FILE *in;
    for(char a= 'A'; a<= 'Z'; a++) {
        char fname[] = "alphabets/A.txt";
        fname[10] = a;
    	if ((in = fopen(fname, "rt")) != NULL) {
            int i = 0;
    	    while(read_line(in, alphabets[a-'A'][i], sizeof alphabets[0][i])) {  
              i++;
            }
    	    fclose(in);
    	}
    }

    int fbfd = 0;
    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;
    long int screensize = 0;
    char *fbp = 0;
    int x = 0, y = 0;
    long int location = 0;

    char input[1000];
    scanf("%[^\n]",input);

    // Open the file for reading and writing
    fbfd = open("/dev/fb0", O_RDWR);
    if (fbfd == -1) {
        perror("Error: cannot open framebuffer device");
        exit(1);
    }
    printf("The framebuffer device was opened successfully.\n");

    // Get fixed screen information
    if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo) == -1) {
        perror("Error reading fixed information");
        exit(2);
    }

    // Get variable screen information
    if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo) == -1) {
        perror("Error reading variable information");
        exit(3);
    }
    printf("%dx%d, %dbpp\n", vinfo.xres, vinfo.yres, vinfo.bits_per_pixel);

    // Figure out the size of the screen in bytes
    screensize = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;

    // Map the device to memory
    fbp = (char *)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
    if ((int)fbp == -1) {
        perror("Error: failed to map framebuffer device to memory");
        exit(4);
    }
    printf("The framebuffer device was mapped to memory successfully.\n");

    x = 0; y = 0;       // Where we are going to put the pixel

    // Figure out where in memory to put the pixel
    int i;
    int cursor = 0;
    int xoffset = 0;
    int yoffset = 0;
    for(i = 0; i<strlen(input);i++) {
        char temp = input[i];
        
        if(cursor >= 24) {
            cursor = 0;
            yoffset++;
        }
        for (y = 0 + (yoffset * 50); y < 33 + (yoffset * 50); y++) {
            for (x = cursor*55; x < (cursor+1)*55; x++) {
                int axis = x % 55;
                location = (x+vinfo.xoffset) * (vinfo.bits_per_pixel/8) +
                           (y+vinfo.yoffset) * finfo.line_length;           
                if (vinfo.bits_per_pixel == 32) {
                    if(temp == ' ') {
                        *(fbp + location) = 0;        // Some blue
                        *(fbp + location + 1) = 0;     // A little green
                        *(fbp + location + 2) = 0;    // A lot of red
                        *(fbp + location + 3) = 0;
                    } else if(alphabets[temp - 'A'][y % 50][axis] != ' ') {
                        *(fbp + location) = 255;        // Some blue
                        *(fbp + location + 1) = 255;     // A little green
                        *(fbp + location + 2) = 255;    // A lot of red
                        *(fbp + location + 3) = 0;   // No transparency 
                    } else {
                        *(fbp + location) = 0;        // Some blue
                        *(fbp + location + 1) = 0;     // A little green
                        *(fbp + location + 2) = 0;    // A lot of red
                        *(fbp + location + 3) = 0;   // No transparency
                    }
            //location += 4;
                } else { //assume 16bpp
                    int b = 10;
                    int g = (x-100)/6;     // A little green
                    int r = 31-(y-100)/16;    // A lot of red
                    unsigned short int t = r<<11 | g << 5 | b;
                    *((unsigned short int*)(fbp + location)) = t;
                }
            }
        }
        cursor++;
    }
    
    munmap(fbp, screensize);
    close(fbfd);
    return 0;
}
