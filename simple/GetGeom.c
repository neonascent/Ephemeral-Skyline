#ifdef _WIN32
#include <windows.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#ifndef __APPLE__
#include <GL/gl.h>
#include <GL/glut.h>
#else
#include <OpenGL/gl.h>
#include <GLUT/glut.h>
#endif
#include <AR/gsub.h>
#include <AR/video.h>
#include <AR/param.h>
#include <AR/ar.h>




/* Josh image processing */
/*DevIL*/
#include "IL/il.h"
#include "IL/ilu.h"
#include "IL/ilut.h"
/*DevIL*/

/* Directory Listing */
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>

#pragma comment(lib, "dirent.lib");
/* Directory Listing */
/* Josh image processing */




//
// Camera configuration.
//
#ifdef _WIN32
char			*vconf = "Data\\WDM_camera_flipV.xml";
#else
char			*vconf = "";
#endif

int             xsize, ysize;
int             thresh = 100;
int             count = 0;

char           *cparam_name    = "Data/camera_para.dat";
ARParam         cparam;

char           *patt_name      = "Data/patt.hiro";
int             patt_id;
double          patt_width     = 80.0;
double          patt_center[2] = {0.0, 0.0};
double          patt_trans[3][4];

static void   init(void);
static void   cleanup(void);
static void   keyEvent( unsigned char key, int x, int y);
static void   mainLoop(void);
static void   draw(void);

/* Josh image processing */

char * getNextFile(void);
ILubyte * GetNextImage(void);

// devil
char			filename[200];
// directory
DIR				*dp;
char            *directory      = "C:\\Documents and Settings\\Administrator\\Desktop\\carpark - AR\\";
char			*outputDir		= "C:\\Documents and Settings\\Administrator\\Desktop\\carpark - AR\\output\\";
/* Josh image processing */

/* Gets the next file in this directory, or at end, return NULL */
char * getNextFile(void)
{
	// directory
	struct dirent *ep;

	if (ep = readdir (dp)) {
		char *filename;
		filename = (char *)malloc(30*sizeof(char));
		strcpy( filename,	ep->d_name);
		return(filename);
	} 
	return NULL;
}



/* Get next image, and return, or return NULL */
ILubyte * GetNextImage(void)
{
	char			*nextFile;
	ILuint			image;
	// setup source directory
	char path[200];
	strcpy (path, directory);
	
	// read while the file not .jpg and there are still files
	while ((nextFile = getNextFile()) && !(strstr(nextFile, ".jpg") > 0)) {
		printf("Skipping invalid file: %s\n", nextFile);
	}

	if (nextFile) {
		//ilInit();
		ilGenImages(1,&image);
		ilBindImage(image);

		//puts( ep->d_name);
		strcpy(filename, nextFile);
		strcat(path, nextFile);
		//puts(path);
		printf("Opening %s\n", path);
		ilLoadImage(path);

		//ilLoadImage("C:\\Documents and Settings\\Administrator\\Desktop\\carpark - AR\\carpark - AR 001.JPG"); //bind .jpeg function to image
		ilConvertImage( IL_BGRA, IL_UNSIGNED_BYTE); //convert to ARUint8
		return(ilGetData());
	}

	return NULL;
}




int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	init();

	// pause 
	printf("Press a key to start  \n");
	getchar();


	//arVideoCapStart();
    argMainLoop( NULL, keyEvent, mainLoop );
	return (0);
}

static void   keyEvent( unsigned char key, int x, int y)
{
    /* quit if the ESC key is pressed */
    if( key == 0x1b ) {
        printf("*** %f (frame/sec)\n", (double)count/arUtilTimer());
        cleanup();
        exit(0);
    }
}


/* main loop */
static void mainLoop(void)
{
    ARUint8         *dataPtr;
    ARMarkerInfo    *marker_info;
    int             marker_num;
    int             j, k;


	if (dataPtr = GetNextImage()) {
	
		if( count == 0 ) arUtilTimerReset();
		count++;
		argDrawMode2D();
		argDispImage( dataPtr, 0,0 );

		/* detect the markers in the video frame */ 
		if( arDetectMarker(dataPtr, thresh, &marker_info, &marker_num) < 0 ) {
			printf("could'nt find any marker");
			cleanup();
			exit(0);
		}
		//else printf("i found the marker"); 




		/* check for object visibility */
		k = -1;
		for( j = 0; j < marker_num; j++ ) {
			if( patt_id == marker_info[j].id ) {
				if( k == -1 ) k = j;
				else if( marker_info[k].cf < marker_info[j].cf ) k = j;
			}
		}
		if( k == -1 ) {
			argSwapBuffers();
			return;
		}

		/* get the transformation between the marker and the real camera */
		arGetTransMat(&marker_info[k], patt_center, patt_width, patt_trans);

		draw();

		argSwapBuffers();
	} else  {
		
		printf("Done - press any key \n");
		getchar();
		exit(0);
	}

}


static void init( void )
{
	/* Josh */
	ARParam wparam;
	xsize = 240; //my size of picture
	ysize = 320;




	//dp = opendir ("./");
	dp = opendir (directory);

	// devil
	ilInit();
	iluInit();

	if (dp == NULL)
	 {
		printf("Image directory could not be opened !!\n");
        exit(0);
	 }

/* Josh */





	arParamChangeSize( &wparam, xsize, ysize, &cparam );
	//arVideoInqSize(&xsize, &ysize);
	printf("Image size (x,y) = (%d,%d)\n", xsize, ysize);

    /* set the initial camera parameters */
    if( arParamLoad(cparam_name, 1, &wparam) < 0 ) {
        printf("Camera parameter load error !!\n");
        exit(0);
    }
    arParamChangeSize( &wparam, xsize, ysize, &cparam );
    arInitCparam( &cparam );
    printf("*** Camera Parameter ***\n");
    arParamDisp( &cparam );

    if( (patt_id=arLoadPatt(patt_name)) < 0 ) {
        printf("pattern load error !!\n");
        exit(0);
    }

    /* open the graphics window */
    argInit( &cparam, 1.0, 0, 0, 0, 0 );
}



/* cleanup function called when program exits */
static void cleanup(void)
{
    arVideoCapStop();
    arVideoClose();
    argCleanup();
	

	closedir (dp);
}


static void captureImage(void) {

	// capture
	//ILuint ratz;
	//vector<GLubyte> store;
	char *savePath[200];

	//GLubyte *store= GLubyte[xsize * ysize * 4];
	GLubyte store[307200];
	/* capture */



	//ilGenImages(1, &ratz);
	//ilBindImage(ratz);

	//store.resize(xsize * ysize * 4);
	glReadPixels(0, 0, xsize, ysize, GL_RGBA, GL_UNSIGNED_BYTE, &store[0]);
	//glReadPixels(0, 0, 240, 320, GL_RGBA, GL_UNSIGNED_BYTE, &store[0]);

	ilTexImage(xsize, ysize, 1, 4, IL_RGBA, IL_UNSIGNED_BYTE, &store[0]);
	
	ilEnable(IL_FILE_OVERWRITE);
	//ilSaveImage(sortie.c_str());
	
	strcpy(savePath, outputDir);
	strcat(savePath, filename);
	//ilSaveImage("C:\\Documents and Settings\\Administrator\\Desktop\\carpark - AR\\out.jpg");
	printf("Saving %s\n", savePath);
	ilSaveImage(savePath);
	//ilSaveImage("C:\\Documents and Settings\\Administrator\\Desktop\\carpark - AR\\output\\out.jpg");
	
	/* capture */
}

static void draw( void )
{
    double    gl_para[16];
    GLfloat   mat_ambient[]     = {0.0, 0.0, 1.0, 1.0};
    GLfloat   mat_flash[]       = {0.0, 0.0, 1.0, 1.0};
    GLfloat   mat_flash_shiny[] = {50.0};
    GLfloat   light_position[]  = {100.0,-200.0,200.0,0.0};
    GLfloat   ambi[]            = {0.1, 0.1, 0.1, 0.1};
    GLfloat   lightZeroColor[]  = {0.9, 0.9, 0.9, 0.1};

   
    argDrawMode3D();
    argDraw3dCamera( 0, 0 );
    glClearDepth( 1.0 );
    glClear(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    
	/* load the camera transformation matrix */
    argConvGlpara(patt_trans, gl_para);
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixd( gl_para );

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambi);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightZeroColor);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_flash);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_flash_shiny);	
    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
    glMatrixMode(GL_MODELVIEW);
    glTranslatef( 0.0, 0.0, 25.0 );
    glutSolidCube(50.0);

    glDisable( GL_LIGHTING );

    glDisable( GL_DEPTH_TEST );

	captureImage();

}
static void geometryout( void )
{
    double    gl_para[16];
    GLfloat   mat_ambient[]     = {0.0, 0.0, 1.0, 1.0};
    GLfloat   mat_flash[]       = {0.0, 0.0, 1.0, 1.0};
    GLfloat   mat_flash_shiny[] = {50.0};
    GLfloat   light_position[]  = {100.0,-200.0,200.0,0.0};
    GLfloat   ambi[]            = {0.1, 0.1, 0.1, 0.1};
    GLfloat   lightZeroColor[]  = {0.9, 0.9, 0.9, 0.1};

   
    argDrawMode3D();
    argDraw3dCamera( 0, 0 );
    glClearDepth( 1.0 );
    glClear(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    
	/* load the camera transformation matrix */
    argConvGlpara(patt_trans, gl_para);

    printf  this is it
	printf("Matrix:\n\t%d\t%d\t%d\t%d\n\t%d\t%d\t%d\t%d\n\t%d\t%d\t%d\t%d\n\t%d\t%d\t%d\t%d\n\n",
											gl_para[0], gl_para[1], gl_para[2], gl_para[3],
											gl_para[4], gl_para[5], gl_para[6], gl_para[7],
											gl_para[8], gl_para[9], gl_para[10], gl_para[11],
											gl_para[12], gl_para[13], gl_para[14], gl_para[15]);

}