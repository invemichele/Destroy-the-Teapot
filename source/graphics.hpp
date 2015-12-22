// File: graphics.h

#ifndef GRAPHICS_HPP
#define GRAPHICS_HPP

//////////////////////////////////////////////////////////////////////////
/// all the stuff related only to the OpenGL drawing should be in here ///
//////////////////////////////////////////////////////////////////////////

#include <string>
#include <GL/glut.h>
#include <zpr.hpp>

//Graphics functions
void light();
void output(GLfloat, GLfloat, const char*, ...);

//Textures stuff
GLuint texture[1]; //Contains the texture
struct Image //Image type - contains height, width, and data
{
    unsigned long sizeX;
    unsigned long sizeY;
    char *data;
};
void LoadGLTextures(const char*);

void draw_cube(float dim, GLfloat* color) //for the explosions
{
  glNewList(0, GL_COMPILE);
   glMaterialfv(GL_FRONT, GL_DIFFUSE, color);
   glMaterialfv(GL_FRONT, GL_SPECULAR, color);
   glMaterialf(GL_FRONT, GL_SHININESS, 0.5);
  glEndList();
  glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);
  glutSolidCube(dim);
}

/////////////////////////////////////////////////
////////////////    MOUSE    ////////////////////
/////////////////////////////////////////////////
//See zpr.hpp (Zoom-pan-rotate mouse manipulation module for GLUT)


/////////////////////////////////////////////////
//////////////////     LIGHT    /////////////////
/////////////////////////////////////////////////
//Light on: enable the lighting of the scene
void light (void)
{  
  GLfloat white_light[]={1,1,1};
  GLfloat light_position[] = {0.0, 6.0, -7.0, 1}; //omogeneous coordinates (all divided by the 4th one)
  
  glLightfv(GL_LIGHT0, GL_POSITION, light_position);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, white_light);
  glLightfv(GL_LIGHT0, GL_SPECULAR, white_light);
  glEnable(GL_LIGHT0);
  
  GLfloat light_position1[] = {0.0, 5.0, 0.0, 1};
  glLightfv(GL_LIGHT1, GL_POSITION, light_position1);
  glLightfv(GL_LIGHT1, GL_DIFFUSE, white_light);
  glLightfv(GL_LIGHT1, GL_SPECULAR, white_light);
  glEnable(GL_LIGHT1);
}

/////////////////////////////////////////////////
//////////////////    OUTPUT    /////////////////
/////////////////////////////////////////////////
//Manage writing on the screen (After calling remember to put glutSwapBuffers() and glFlush())
void output(GLfloat x, GLfloat y, const char* format,...)
{
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  gluOrtho2D(0,3000,0,3000);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  glDisable(GL_LIGHTING);
  glColor3f(0.5,0.5,0.5); //change here if everything has a weird color
  glRasterPos2f(x,y);
  std::string s=(std::string) format;
  void * font = GLUT_BITMAP_HELVETICA_18;
  for (std::string::iterator i = s.begin(); i != s.end(); ++i)
  {
    char c = *i;
    glutBitmapCharacter(font, c);
  }
  glEnable(GL_LIGHTING);
  glPopMatrix();
  glMatrixMode(GL_PROJECTION);  
  glPopMatrix();
}

/////////////////////////////////////////////////
////////    IMAGE AND TEXTURE LOADING   /////////
/////////////////////////////////////////////////

//Functions courtesy of nehe.gamedev.net tutorials
int ImageLoad(const char *filename, Image *image) 
{
    FILE *file;
    unsigned long size;                 // size of the image in bytes.
    unsigned long i;                    // standard counter.
    unsigned short int planes;          // number of planes in image (must be 1) 
    unsigned short int bpp;             // number of bits per pixel (must be 24)
    char temp;                          // temporary color storage for bgr-rgb conversion.

    // make sure the file is there.
    if ((file = fopen(filename, "rb"))==NULL) {
	printf("File Not Found : %s\n",filename);
	return 0;
    }
    
    // seek through the bmp header, up to the width/height:
    fseek(file, 18, SEEK_CUR);

    // read the width
    if ((i = fread(&image->sizeX, 4, 1, file)) != 1) {
	printf("Error reading width from %s.\n", filename);
	return 0;
    }
    //printf("Width of %s: %lu\n", filename, image->sizeX);
    
    // read the height 
    if ((i = fread(&image->sizeY, 4, 1, file)) != 1) {
	printf("Error reading height from %s.\n", filename);
	return 0;
    }
    //printf("Height of %s: %lu\n", filename, image->sizeY);
    
    // calculate the size (assuming 24 bits or 3 bytes per pixel).
    size = image->sizeX * image->sizeY * 3;

    // read the planes
    if ((fread(&planes, 2, 1, file)) != 1) {
	printf("Error reading planes from %s.\n", filename);
	return 0;
    }
    if (planes != 1) {
	printf("Planes from %s is not 1: %u\n", filename, planes);
	return 0;
    }

    // read the bpp
    if ((i = fread(&bpp, 2, 1, file)) != 1) {
	printf("Error reading bpp from %s.\n", filename);
	return 0;
    }
    if (bpp != 24) {
	printf("Bpp from %s is not 24: %u\n", filename, bpp);
	return 0;
    }
	
    // seek past the rest of the bitmap header.
    fseek(file, 24, SEEK_CUR);

    // read the data. 
    image->data = (char *) malloc(size);
    if (image->data == NULL) {
	printf("Error allocating memory for color-corrected image data");
	return 0;	
    }

    if ((i = fread(image->data, size, 1, file)) != 1) {
	printf("Error reading image data from %s.\n", filename);
	return 0;
    }

    for (i=0;i<size;i+=3) { // reverse all of the colors. (bgr -> rgb)
	temp = image->data[i];
	image->data[i] = image->data[i+2];
	image->data[i+2] = temp;
    }
    
    // we're done.
    return 1;
}
    
// Load Bitmaps And Convert To Textures
void LoadGLTextures(const char* filename) 
{	
    // Load Texture
    Image *image1;
    
    // allocate space for texture
    image1 = (Image *) malloc(sizeof(Image));
    if (image1 == NULL) {
	printf("Error allocating space for image");
	exit(0);
    }

    if (!ImageLoad(filename, image1)) {
	exit(1);
    }        

    // Create Texture	
    glGenTextures(1, &texture[0]);
    glBindTexture(GL_TEXTURE_2D, texture[0]);   // 2d texture (x and y size)

    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR); // scale linearly when image bigger than texture
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); // scale linearly when image smaller than texture

    // 2d texture, level of detail 0 (normal), 3 components (red, green, blue), x size from image, y size from image, 
    // border 0 (normal), rgb color data, unsigned byte data, and finally the data itself.
    glTexImage2D(GL_TEXTURE_2D, 0, 3, image1->sizeX, image1->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, image1->data);
}

#endif

